// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Diagnostics.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Perform diagnostics on VDB volumes to detect potential issues.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/math/Math.h> // Tolerance, isApproxEqual and isFinite
#include <openvdb/math/Operators.h> // ISGradientNormSqrd
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/LevelSetTracker.h> // LevelSetTracker::normalize

#include <UT/UT_Version.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_UniquePtr.h>
#include <PRM/PRM_Parm.h>

#ifdef SESI_OPENVDB
  #include <hboost/mpl/at.hpp>
  namespace boostmpl = hboost::mpl;
#else
  #include <boost/mpl/at.hpp>
  namespace boostmpl = boost::mpl;
#endif

#include <memory>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////

// Local Utility Methods

namespace {


////////////////////////////////////////

// Tests

struct AlwaysFalse {
    template<typename Iterator>
    bool operator()(const Iterator&) const { return false; }
};

struct FiniteValue {
    template<typename Iterator>
    bool operator()(const Iterator& it) const { return openvdb::math::isFinite(*it); }
};

template<typename ValueType>
struct ApproxEqual {
    ApproxEqual(const ValueType& val,
        const ValueType& tol = openvdb::math::Tolerance<ValueType>::value())
        : mValue(val), mTol(tol) {}
    template<typename Iterator>
    bool operator()(const Iterator& it) const {
        return openvdb::math::isApproxEqual(mValue, *it, mTol);
    }
    const ValueType mValue, mTol;
};

template<typename ValueType>
struct AbsApproxEqual {
    AbsApproxEqual(const ValueType& val,
        const ValueType& tol = openvdb::math::Tolerance<ValueType>::value())
        : mValue(openvdb::math::Abs(val)), mTol(tol) {}
    template<typename Iterator>
    bool operator()(const Iterator& it) const {
        return openvdb::math::isApproxEqual(mValue, openvdb::math::Abs(*it), mTol);
    }
    const ValueType mValue, mTol;
};


template<typename ValueType>
struct AbsLessThan {
    AbsLessThan(ValueType val) : mValue(openvdb::math::Abs(val)) {}
    template<typename Iterator>
    bool operator()(const Iterator& it) const {
        return !(ValueType(openvdb::math::Abs(*it)) < mValue);
    }
    const ValueType mValue;
};

template<typename T>
inline float toFloat(const T s) { return float(s); }

template<typename T>
inline float toFloat(const openvdb::math::Vec3<T> v) { return float(v[0]); }

struct InRange {
    InRange(float minValue, float maxValue) : mMin(minValue), mMax(maxValue) {}
    template<typename Iterator>
    bool operator()(const Iterator& it) const { return test(*it); }

    template<typename T>
    bool test(const T& s) const { return !(s < T(mMin) || T(mMax) < s); }

    template<typename T>
    bool test(const openvdb::math::Vec3<T>& v) const { return test(v.length()); }

    const float mMin, mMax;
};

template<typename TreeType>
struct GradientNorm {
    using ValueType = typename TreeType::ValueType;

    GradientNorm(const TreeType& tree, double voxelSize, ValueType tol)
        : mAcc(tree), mScale(ValueType(1.0 / voxelSize)), mTol(tol) {}

    GradientNorm(const GradientNorm& rhs)
        : mAcc(rhs.mAcc.tree()), mScale(rhs.mScale), mTol(rhs.mTol) {}

    template<typename Iterator>
    bool operator()(const Iterator& it) {

        const openvdb::Coord ijk = it.getCoord();

        // ignore voxels adjacent to the active narrow band boundary
        if (!mAcc.isValueOn(ijk.offsetBy(-1, 0, 0))) return true;
        if (!mAcc.isValueOn(ijk.offsetBy( 1, 0, 0))) return true;
        if (!mAcc.isValueOn(ijk.offsetBy( 0,-1, 0))) return true;
        if (!mAcc.isValueOn(ijk.offsetBy( 0, 1, 0))) return true;
        if (!mAcc.isValueOn(ijk.offsetBy( 0, 0,-1))) return true;
        if (!mAcc.isValueOn(ijk.offsetBy( 0, 0, 1))) return true;

        return openvdb::math::isApproxEqual(ValueType(1.0), gradientNorm(ijk, mScale), mTol);
    }

    template<typename T>
    inline T
    gradientNorm(const openvdb::Coord& ijk, const T scale) {
        return scale * T(std::sqrt(double(
            openvdb::math::ISGradientNormSqrd<openvdb::math::FIRST_BIAS>::result(mAcc, ijk))));
    }

    /// @{
    // The gradient magnitude test is applied only to scalar, floating-point grids,
    // but this class needs to compile for all grid types.

    template<typename T>
    inline openvdb::math::Vec3<T>
    gradientNorm(const openvdb::Coord&, const openvdb::math::Vec3<T>) {
        return openvdb::math::Vec3<T>(0);
    }

    inline bool gradientNorm(const openvdb::Coord&, bool) { return false; }
    /// @}

private:
    GradientNorm& operator=(const GradientNorm&); // disable assignment
    openvdb::tree::ValueAccessor<const TreeType> mAcc;
    const ValueType mScale, mTol;
};

template<typename TreeType>
struct SameSign {
    using ValueType = typename TreeType::ValueType;

    SameSign(const TreeType& tree) : mAcc(tree) {}
    SameSign(const SameSign& rhs) : mAcc(rhs.mAcc.tree()) {}

    template<typename Iterator>
    bool operator()(const Iterator& it) {
        ValueType val;
        const bool state = mAcc.probeValue(it.getCoord(), val);
        return state ? true : (val < ValueType(0)) == (*it < ValueType(0));
    }

private:
    SameSign& operator=(const SameSign&); // disable assignment
    openvdb::tree::ValueAccessor<const TreeType> mAcc;
};


////////////////////////////////////////


/// @brief Visits values and performs tests
template<typename GridType>
struct Visitor
{
    enum ValueKind { TILES_AND_VOXELS, TILES, VOXELS };
    enum ValueState { ALL_VALUES, ACTIVE_VALUES, INACTIVE_VALUES };

    using TreeType = typename GridType::TreeType;
    using LeafNodeType = typename TreeType::LeafNodeType;
    using RootNodeType = typename TreeType::RootNodeType;
    using NodeChainType = typename RootNodeType::NodeChainType;
    using InternalNodeType = typename boostmpl::at<NodeChainType, boostmpl::int_<1>>::type;

    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolTreePtr = typename BoolTreeType::Ptr;

    //////////

    Visitor(const TreeType& tree) : mTree(tree), mValueMask(new BoolTreeType(false))
    {
        tree.getNodes(mLeafNodes);
        tree.getNodes(mInternalNodes);
    }

    BoolTreePtr& valueMask() { return mValueMask; }

    std::string invalidValuesInfo() const
    {
        std::stringstream info;

         if (!mValueMask->empty()) {

            info << "invalid: ";

            const size_t voxelCount = size_t(mValueMask->activeLeafVoxelCount());
            if (voxelCount > 0) info << voxelCount << " voxels ";

            const size_t tileCount = size_t(mValueMask->activeTileCount());
            if (tileCount > 0) {
                if (voxelCount > 0) info << "& ";
                info << tileCount << " tiles";
            }
        }

        return info.str();
    }


    template<typename TestType>
    bool run(ValueKind kind, const ValueState& state, const TestType& test)
    {
        mValueMask.reset(new BoolTreeType(false));

        if (kind == TILES_AND_VOXELS || kind == VOXELS) {
            LeafNodeReduction<TestType> op(state, &mLeafNodes[0], test, *mValueMask);
            tbb::parallel_reduce(tbb::blocked_range<size_t>(0, mLeafNodes.size()), op);
        }

        if (kind == TILES_AND_VOXELS || kind == TILES) {

            InternalNodeReduction<TestType> op(state, &mInternalNodes[0], test, *mValueMask);
            tbb::parallel_reduce(tbb::blocked_range<size_t>(0, mInternalNodes.size()), op);

            TestType myTest(test);

            if (state == ACTIVE_VALUES) {
                typename TreeType::ValueOnCIter it(mTree);
                it.setMaxDepth(TreeType::ValueOnCIter::LEAF_DEPTH - 2);
                for ( ; it; ++it) {
                    if (!myTest(it)) {
                        mValueMask->fill(it.getBoundingBox(), true);
                    }
                }
            } else if (state == INACTIVE_VALUES) {
                typename TreeType::ValueOffCIter it(mTree);
                it.setMaxDepth(TreeType::ValueOffCIter::LEAF_DEPTH - 2);
                for ( ; it; ++it) {
                    if (!myTest(it)) {
                        mValueMask->fill(it.getBoundingBox(), true);
                    }
                }
            } else {
                typename TreeType::ValueAllCIter it(mTree);
                it.setMaxDepth(TreeType::ValueAllCIter::LEAF_DEPTH - 2);
                for ( ; it; ++it) {
                    if (!myTest(it)) {
                        mValueMask->fill(it.getBoundingBox(), true);
                    }
                }
            }
        }

        return mValueMask->empty(); // passed if mask is empty
    }

private:

    template<typename TestType>
    struct LeafNodeReduction
    {
        LeafNodeReduction(const ValueState& state, const LeafNodeType ** nodes,
            const TestType& test, BoolTreeType& mask)
            : mState(state), mNodes(nodes), mPrimMask(&mask), mTempMask(false),
              mMask(mPrimMask ? mPrimMask : &mTempMask), mTest(test)
        {}

        LeafNodeReduction(LeafNodeReduction& other, tbb::split)
            : mState(other.mState), mNodes(other.mNodes), mPrimMask(other.mPrimMask),
              mTempMask(false), mMask(&mTempMask), mTest(other.mTest)
        {}

        void join(LeafNodeReduction& other) { mMask->merge(*other.mMask); }

        void operator()(const tbb::blocked_range<size_t>& range) {

            openvdb::tree::ValueAccessor<BoolTreeType> mask(*mMask);

            for (size_t n = range.begin(), N = range.end(); n < N; ++n) {

                const LeafNodeType& node = *mNodes[n];

                if (mState == ACTIVE_VALUES) {
                    for (typename LeafNodeType::ValueOnCIter it = node.cbeginValueOn(); it; ++it) {
                        if (!mTest(it)) {
                            mask.setValueOn(it.getCoord());
                        }
                    }
                } else if (mState == INACTIVE_VALUES) {
                    for (typename LeafNodeType::ValueOffCIter it = node.cbeginValueOff();
                        it; ++it)
                    {
                        if (!mTest(it)) {
                            mask.setValueOn(it.getCoord());
                        }
                    }
                } else {
                    for (typename LeafNodeType::ValueAllCIter it=node.cbeginValueAll(); it; ++it) {
                        if (!mTest(it)) {
                            mask.setValueOn(it.getCoord());
                        }
                    }
                }
            }
        }

    private:
        ValueState                         mState;
        LeafNodeType const * const * const mNodes;
        BoolTreeType               * const mPrimMask;
        BoolTreeType                       mTempMask;
        BoolTreeType               * const mMask;
        TestType                           mTest;
    }; // struct LeafNodeReduction

    template<typename TestType>
    struct InternalNodeReduction
    {
        InternalNodeReduction(const ValueState& state, const InternalNodeType** nodes,
            const TestType& test, BoolTreeType& mask)
            : mState(state), mNodes(nodes), mPrimMask(&mask), mTempMask(false),
              mMask(mPrimMask ? mPrimMask : &mTempMask), mTest(test)
        {}

        InternalNodeReduction(InternalNodeReduction& other, tbb::split)
            : mState(other.mState), mNodes(other.mNodes), mPrimMask(other.mPrimMask),
              mTempMask(false), mMask(&mTempMask), mTest(other.mTest)
        {}

        void join(InternalNodeReduction& other) { mMask->merge(*other.mMask); }

        void operator()(const tbb::blocked_range<size_t>& range) {

            openvdb::Coord ijk;
            const int dim = int(InternalNodeType::ChildNodeType::DIM) - 1;

            for (size_t n = range.begin(), N = range.end(); n < N; ++n) {

                const InternalNodeType& node = *mNodes[n];

                if (mState == ACTIVE_VALUES) {
                    for (typename InternalNodeType::ValueOnCIter it = node.cbeginValueOn();
                        it; ++it)
                    {
                        if (!node.isChildMaskOn(it.pos()) && !mTest(it)) {
                            ijk = it.getCoord();
                            mMask->fill(openvdb::CoordBBox(ijk, ijk.offsetBy(dim)), true);
                        }
                    }
                } else if (mState == INACTIVE_VALUES) {
                    for (typename InternalNodeType::ValueOffCIter it = node.cbeginValueOff();
                        it; ++it)
                    {
                        if (!node.isChildMaskOn(it.pos()) && !mTest(it)) {
                            ijk = it.getCoord();
                            mMask->fill(openvdb::CoordBBox(ijk, ijk.offsetBy(dim)), true);
                        }
                    }
                } else {
                    for (typename InternalNodeType::ValueAllCIter it = node.cbeginValueAll();
                        it; ++it)
                    {
                        if (!node.isChildMaskOn(it.pos()) && !mTest(it)) {
                            ijk = it.getCoord();
                            mMask->fill(openvdb::CoordBBox(ijk, ijk.offsetBy(dim)), true);
                        }
                    }
                }
            }
        }

    private:
        ValueState                             mState;
        InternalNodeType const * const * const mNodes;
        BoolTreeType                   * const mPrimMask;
        BoolTreeType                           mTempMask;
        BoolTreeType                   * const mMask;
        TestType                               mTest;
     }; // struct InternalNodeReduction

    const TreeType&                         mTree;
    BoolTreePtr                             mValueMask;
    std::vector<const LeafNodeType*>        mLeafNodes;
    std::vector<const InternalNodeType*>    mInternalNodes;

}; // struct Visitor


////////////////////////////////////////

// HDK Points With Values Create and Transfer


template<typename BoolLeafNodeType>
struct GetPoints
{
    GetPoints(const BoolLeafNodeType ** maskNodes, UT_Vector3* points,
        const size_t* offsetTable, const openvdb::math::Transform& xform)
        : mMaskNodes(maskNodes)
        , mPoints(points)
        , mOffsetTable(offsetTable)
        , mXform(xform)
    {
    }

    void operator()(const tbb::blocked_range<size_t>& range) const {

        openvdb::Vec3d xyz;

        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {

            const BoolLeafNodeType& maskNode = *mMaskNodes[n];
            UT_Vector3* points = &mPoints[mOffsetTable[n]];

            size_t idx = 0;
            for (typename BoolLeafNodeType::ValueOnCIter it = maskNode.cbeginValueOn(); it; ++it) {
                xyz = mXform.indexToWorld(it.getCoord());
                UT_Vector3& pos = points[idx++];
                pos[0] = UT_Vector3::value_type(xyz[0]);
                pos[1] = UT_Vector3::value_type(xyz[1]);
                pos[2] = UT_Vector3::value_type(xyz[2]);
            }
         }
    }

    BoolLeafNodeType const * const * const mMaskNodes;
    UT_Vector3                     * const mPoints;
    size_t                   const * const mOffsetTable;
    openvdb::math::Transform               mXform;
}; // struct GetPoints


template<typename BoolTreeType>
inline size_t
getPoints(const openvdb::math::Transform& xform, const BoolTreeType& mask,
    UT_UniquePtr<UT_Vector3[]>& points)
{
    using BoolLeafNodeType = typename BoolTreeType::LeafNodeType;

    std::vector<const BoolLeafNodeType*> nodes;
    mask.getNodes(nodes);

    const size_t tileCount = mask.activeTileCount();
    size_t voxelCount = 0, totalCount = tileCount;

    if (!nodes.empty()) {
        UT_UniquePtr<size_t[]> offsetTable(new size_t[nodes.size()]);

        for (size_t n = 0, N = nodes.size(); n < N; ++n) {
            offsetTable[n] = voxelCount;
            voxelCount += nodes[n]->onVoxelCount();
        }

        totalCount += voxelCount;
        points.reset(new UT_Vector3[totalCount]);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, nodes.size()),
            GetPoints<BoolLeafNodeType>(&nodes[0], points.get(), offsetTable.get(), xform));
    }

    if (tileCount > 0) {

        if (!points) {
            points.reset(new UT_Vector3[tileCount]);
        }

        openvdb::Vec3d xyz;

        typename BoolTreeType::ValueOnCIter it(mask);
        it.setMaxDepth(BoolTreeType::ValueOnCIter::LEAF_DEPTH - 1);

        for (size_t idx = voxelCount; it; ++it, ++idx) {
            xyz = xform.indexToWorld(it.getCoord());
            UT_Vector3& pos = points[idx];
            pos[0] = UT_Vector3::value_type(xyz[0]);
            pos[1] = UT_Vector3::value_type(xyz[1]);
            pos[2] = UT_Vector3::value_type(xyz[2]);
        }
    }

    return totalCount;
}


inline GA_Offset
transferPoints(GU_Detail& detail, const UT_UniquePtr<UT_Vector3[]>& points, size_t pointCount)
{
    const GA_Offset startOffset = detail.getNumPointOffsets();

    detail.appendPointBlock(pointCount);

    GA_Offset offset = startOffset;
    for (size_t n = 0, N = pointCount; n < N; ++n) {
        detail.setPos3(offset++, points[n]);
    }

    return startOffset;
}


template<typename TreeType>
struct GetValues
{
    using ValueType = typename TreeType::ValueType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolLeafNodeType = typename BoolTreeType::LeafNodeType;

    GetValues(const TreeType& tree, const BoolLeafNodeType ** maskNodes,
        ValueType* values, const size_t* offsetTable)
        : mTree(&tree)
        , mMaskNodes(maskNodes)
        , mValues(values)
        , mOffsetTable(offsetTable)
    {
    }

    void operator()(const tbb::blocked_range<size_t>& range) const {

        openvdb::tree::ValueAccessor<const TreeType> acc(*mTree);

        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {

            const BoolLeafNodeType& maskNode = *mMaskNodes[n];
            ValueType* values = &mValues[mOffsetTable[n]];

            size_t idx = 0;
            for (typename BoolLeafNodeType::ValueOnCIter it = maskNode.cbeginValueOn(); it; ++it) {
                values[idx++] = acc.getValue(it.getCoord());
            }
         }
    }

    TreeType                 const * const mTree;
    BoolLeafNodeType const * const * const mMaskNodes;
    ValueType                      * const mValues;
    size_t                   const * const mOffsetTable;
}; // struct GetValues


template<typename TreeType>
inline size_t
getValues(const TreeType& tree,
    const typename TreeType::template ValueConverter<bool>::Type& mask,
    UT_UniquePtr<typename TreeType::ValueType[]>& values)
{
    using ValueType = typename TreeType::ValueType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolLeafNodeType = typename BoolTreeType::LeafNodeType;

    std::vector<const BoolLeafNodeType*> nodes;
    mask.getNodes(nodes);

    const size_t tileCount = mask.activeTileCount();
    size_t voxelCount = 0, totalCount = tileCount;

    if (!nodes.empty()) {

        UT_UniquePtr<size_t[]> offsetTable(new size_t[nodes.size()]);

        for (size_t n = 0, N = nodes.size(); n < N; ++n) {
            offsetTable[n] = voxelCount;
            voxelCount += nodes[n]->onVoxelCount();
        }

        totalCount += voxelCount;
        values.reset(new ValueType[totalCount]);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, nodes.size()),
            GetValues<TreeType>(tree, &nodes[0], values.get(), offsetTable.get()));
    }

    if (tileCount > 0) {

        if (!values) {
            values.reset(new ValueType[tileCount]);
        }

        typename BoolTreeType::ValueOnCIter it(mask);
        it.setMaxDepth(BoolTreeType::ValueOnCIter::LEAF_DEPTH - 1);

        openvdb::tree::ValueAccessor<const TreeType> acc(tree);

        for (size_t idx = voxelCount; it; ++it, ++idx) {
            values[idx] = acc.getValue(it.getCoord());
        }
    }

    return totalCount;
}

template<typename ValueType>
inline void
transferValues(GU_Detail& detail, const std::string& name, GA_Offset startOffset,
    const UT_UniquePtr<ValueType[]>& values, size_t pointCount)
{
    GA_RWAttributeRef attr = detail.addFloatTuple(
        GA_ATTRIB_POINT, (name + "_scalar").c_str(), 1, GA_Defaults(0));
    GA_RWHandleF handle = attr.getAttribute();

    for (size_t n = 0, N = pointCount; n < N; ++n) {
        handle.set(startOffset++, float(values[n]));
    }
}

template<typename ValueType>
inline void
transferValues(GU_Detail& detail, const std::string& name, GA_Offset startOffset,
    const UT_UniquePtr<openvdb::math::Vec3<ValueType>[]>& values, size_t pointCount)
{
    GA_RWAttributeRef attr = detail.addFloatTuple(
        GA_ATTRIB_POINT, (name + "_vector").c_str(), 3, GA_Defaults(0));
    GA_RWHandleV3 handle = attr.getAttribute();

    UT_Vector3 vec(0.0f, 0.0f, 0.0f);
    using VectorType = openvdb::math::Vec3<ValueType>;

    for (size_t n = 0, N = pointCount; n < N; ++n) {
        const VectorType& val = values[n];
        vec[0] = float(val[0]);
        vec[1] = float(val[1]);
        vec[2] = float(val[2]);
        handle.set(startOffset++, vec);
    }
}

////////////////////////////////////////

// Utility Objects


struct TestData
{
    // settings
    bool useMask, usePoints, respectGridClass;
    // general tests
    bool testFinite, idFinite, fixFinite;
    bool testUniformBackground, idUniformBackground, fixUniformBackground;
    bool testInRange, idInRange, fixInRange;
    bool testUniformVoxelSize;
    float rangeMin, rangeMax;
    // level set tests
    bool testSymmetricNarrowBand;
    bool testMinimumBandWidth;
    bool testClosedSurface;
    bool testGradientMagnitude, idGradientMagnitude, fixGradientMagnitude;
    bool testNoActiveTiles, idNoActiveTiles, fixNoActiveTiles;
    float gradientTolerance, minBandWidth;
    // fog volume tests
    bool testBackgroundZero, idBackgroundZero, fixBackgroundZero;
    bool testActiveValuesFromZeroToOne, idActiveValuesFromZeroToOne, fixActiveValuesFromZeroToOne;
}; // struct TestData

struct GridTestLog
{
    GridTestLog(int primitiveIndex, const std::string& gridName)
        : mGridName(), mFailedMsg(), mFailed(0), mPassed(0), mSkipped(0)
    {
        std::stringstream name;
        name << "  (" << primitiveIndex << ") '" << gridName << "'";
        mGridName = name.str();
    }

    size_t failedCount() const { return mFailed; }
    size_t passedCount() const { return mPassed; }
    size_t skippedCount() const { return mSkipped; }

    void appendFailed(const std::string& testName, const std::string& msg = "") {
        mFailed++;
        mFailedMsg += "   - '" + testName + "' " + msg + "\n";
    }

    void appendPassed() { mPassed++; }
    void appendSkipped() { mSkipped++; }

    std::string str() const {

        std::stringstream log;
        log << mGridName;

        if (mPassed > 0) {
            log << " passed " << mPassed;
        }

        if (mFailed > 0) {
            log << " failed " << mFailed;
        }

        if ((mPassed + mFailed) == 0) {
            log << " not tested";
        }

        log << "\n";

        if (mSkipped > 0) {
            log << "   - skipped " << mSkipped << " scalar floating-point specific test"
                << (mSkipped > 1 ? "s.\n" : ".\n");
        }

        if (!mFailedMsg.empty()) {
            log << mFailedMsg << "\n";
        }

        return log.str();
    }

private:
    std::string mGridName, mFailedMsg;
    size_t mFailed, mPassed, mSkipped;
}; // struct GridTestLog


template<typename GridType>
struct MaskData {
    using TreeType = typename GridType::TreeType;
    using ValueType = typename GridType::ValueType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;

    MaskData() : mask(), minValue(ValueType(0)), maxValue(ValueType(0)), isRange(false) {}

    MaskData(typename BoolTreeType::Ptr& tree, ValueType val)
        : mask(tree), minValue(val), maxValue(val), isRange(false) {}

    MaskData(typename BoolTreeType::Ptr& tree, ValueType minval, ValueType maxval)
        : mask(tree), minValue(minval), maxValue(maxval), isRange(true) {}

    typename BoolTreeType::Ptr mask;
    ValueType minValue, maxValue;
    bool isRange;
}; // struct MaskData


////////////////////////////////////////


template<typename GridType>
inline
typename std::enable_if<std::is_floating_point<typename GridType::ValueType>::value, void>::type
normalizeLevelSet(GridType& grid)
{
    openvdb::tools::LevelSetTracker<GridType> op(grid);
    op.setNormCount(3);
    op.setSpatialScheme(openvdb::math::FIRST_BIAS);
    op.setTemporalScheme(openvdb::math::TVD_RK3);
    op.normalize();
}

template<typename GridType>
inline
typename std::enable_if<!std::is_floating_point<typename GridType::ValueType>::value, void>::type
normalizeLevelSet(GridType&)
{
}


template<typename T>
inline T
clampValueAndVectorMagnitude(T s, const T& minVal, const T& maxVal)
{
    if (s < minVal) s = minVal;
    if (s > maxVal) s = maxVal;
    return s;
}

template<typename T>
inline openvdb::math::Vec3<T>
clampValueAndVectorMagnitude(openvdb::math::Vec3<T> v,
    const openvdb::math::Vec3<T>& minVal,
    const openvdb::math::Vec3<T>& maxVal)
{
    const T scale = clampValueAndVectorMagnitude(v.length(), minVal[0], maxVal[0]);
    v.normalize();
    v *= scale;
    return v;
}


template<typename GridType>
struct FixVoxelValues
{
    using TreeType = typename GridType::TreeType;
    using ValueType = typename TreeType::ValueType;
    using LeafNodeType = typename TreeType::LeafNodeType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolLeafNodeType = typename BoolTreeType::LeafNodeType;
    using MaskDataType = MaskData<GridType>;

    FixVoxelValues(TreeType& tree, const BoolLeafNodeType ** maskNodes,
        const MaskDataType& maskdata)
        : mTree(&tree)
        , mMaskNodes(maskNodes)
        , mMaskData(&maskdata)
    {
    }

    void operator()(const tbb::blocked_range<size_t>& range) const {

        using ValueOnCIter = typename BoolLeafNodeType::ValueOnCIter;
        openvdb::tree::ValueAccessor<TreeType> acc(*mTree);

        const ValueType minVal = mMaskData->minValue;
        const ValueType maxVal = mMaskData->maxValue;
        const bool isRange = mMaskData->isRange;

        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {

            const BoolLeafNodeType& maskNode = *mMaskNodes[n];
            LeafNodeType* node = acc.probeLeaf(maskNode.origin());
            if (!node) continue;

            if (isRange) { // clamp
                for (ValueOnCIter it = maskNode.cbeginValueOn(); it; ++it) {
                    node->setValueOnly(it.pos(),
                        clampValueAndVectorMagnitude(node->getValue(it.pos()), minVal, maxVal));
                }
            } else { // replace
                for (ValueOnCIter it = maskNode.cbeginValueOn(); it; ++it) {
                    node->setValueOnly(it.pos(), minVal);
                }
            }
        }
    }

    TreeType                       * const mTree;
    BoolLeafNodeType const * const * const mMaskNodes;
    MaskDataType             const * const mMaskData;
}; // struct FixVoxelValues


template<typename GridType>
inline typename GridType::Ptr
fixValues(const GridType& grid, std::vector<MaskData<GridType> > fixMasks,
    bool inactivateTiles = false,
    bool renormalizeLevelSet = false)
{
    using TreeType = typename GridType::TreeType;
    using ValueType = typename GridType::ValueType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolLeafNodeType = typename BoolTreeType::LeafNodeType;
    using MaskDataType = MaskData<GridType>;

    typename GridType::Ptr replacementGrid = grid.deepCopy();

    BoolTreeType alreadyFixedValues(false);

    for (size_t n = 0, N = fixMasks.size(); n < N; ++n) {

        MaskDataType& fix = fixMasks[n];

        BoolTreeType mask(false);
        mask.topologyUnion(*fix.mask);
        mask.topologyDifference(alreadyFixedValues);

        // fix voxels
        {
            std::vector<const BoolLeafNodeType*> nodes;
            mask.getNodes(nodes);

            tbb::parallel_for(tbb::blocked_range<size_t>(0, nodes.size()),
                FixVoxelValues<GridType>(replacementGrid->tree(), &nodes[0], fix));
        }

        // fix tiles
        typename BoolTreeType::ValueOnCIter it(mask);
        it.setMaxDepth(BoolTreeType::ValueOnCIter::LEAF_DEPTH - 1);

        openvdb::tree::ValueAccessor<TreeType> acc(replacementGrid->tree());
        openvdb::Coord ijk;

        if (fix.isRange) { // clamp

            for (; it; ++it) {

                ijk = it.getCoord();

                const ValueType val = clampValueAndVectorMagnitude(
                    acc.getValue(ijk), fix.minValue, fix.maxValue);
                acc.addTile(it.getLevel(), ijk, val, acc.isValueOn(ijk));
            }

        } else { // replace

            const ValueType val = fix.minValue;

            for (; it; ++it) {
                ijk = it.getCoord();
                acc.addTile(it.getLevel(), ijk, val, acc.isValueOn(ijk));
            }
        }

        alreadyFixedValues.topologyUnion(mask);
    }

    if (inactivateTiles) {
        typename TreeType::ValueOnIter it(replacementGrid->tree());
        it.setMaxDepth(TreeType::ValueOnIter::LEAF_DEPTH - 1);
        for (; it; ++it) {
            it.setActiveState(false);
        }
    }

    if (renormalizeLevelSet) {
        normalizeLevelSet(*replacementGrid);
    }

    return replacementGrid;
}


template<typename GridType>
inline void
outputMaskAndPoints(const GridType& grid, const std::string& gridName,
    std::vector<typename GridType::TreeType::template ValueConverter<bool>::Type::Ptr> masks,
    bool outputMask,
    bool outputPoints,
    GU_Detail& detail,
    hvdb::Interrupter& interupter,
    const GridType* replacementGrid = nullptr)
{
    using TreeType = typename GridType::TreeType;
    using ValueType = typename GridType::ValueType;
    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    using BoolGridType = typename openvdb::Grid<BoolTreeType>;

    if (outputMask || outputPoints) {

        const TreeType& tree = grid.tree();

        typename BoolGridType::Ptr maskGrid = openvdb::createGrid<BoolGridType>(false);
        BoolTreeType& mask = maskGrid->tree();

        for (size_t n = 0, N = masks.size(); n < N; ++n) {
            BoolTreeType* maskPt = masks[n].get();
            if (maskPt && !maskPt->empty()) {
                mask.merge(*masks[n]);
            }
        }

        if (outputPoints && !mask.empty()) {

            if (interupter.wasInterrupted()) return;

            UT_UniquePtr<UT_Vector3[]> points;
            const size_t totalPointCount = getPoints(grid.transform(), mask, points);

            if (interupter.wasInterrupted()) return;

            if (totalPointCount > 0) {
                const GA_Offset startOffset = transferPoints(detail, points, totalPointCount);
                points.reset(); // clear

                UT_UniquePtr<ValueType[]> values;
                getValues(tree, mask, values);

                if (interupter.wasInterrupted()) return;

                transferValues(detail, "input", startOffset, values, totalPointCount);

                if (replacementGrid) {
                    if (interupter.wasInterrupted()) return;
                    getValues(replacementGrid->tree(), mask, values);
                    if (interupter.wasInterrupted()) return;
                    transferValues(detail, "output", startOffset, values, totalPointCount);
                }
            }
        }

        if (interupter.wasInterrupted()) return;

        if (outputMask && !mask.empty()) {
            maskGrid->setName(gridName + "_mask");
            maskGrid->setTransform(grid.transform().copy());
            hvdb::createVdbPrimitive(detail, maskGrid, maskGrid->getName().c_str());
        }
    }
}


////////////////////////////////////////

struct TestCollection
{
    TestCollection(const TestData& test, GU_Detail& detail,
        hvdb::Interrupter& interupter, UT_ErrorManager* errorManager = nullptr)
        : mTest(test)
        , mDetail(&detail)
        , mInterupter(&interupter)
        , mErrorManager(errorManager)
        , mMessageStr()
        , mPrimitiveName()
        , mPrimitiveIndex(0)
        , mGridsFailed(0)
        , mReplacementGrid()
    {
    }

    ~TestCollection()
    {
        if (mErrorManager) {
            if (mGridsFailed > 0) {
                std::stringstream msg;
                msg << mGridsFailed << " grid" << (mGridsFailed > 1 ? "s" : "")
                    << " failed one or more tests.";
                mErrorManager->addWarning(SOP_OPTYPE_NAME, SOP_MESSAGE, msg.str().c_str());
            }

            if (!mMessageStr.empty()) {
                std::stringstream msg;
                msg << "Diagnostics results\n";
                msg << mMessageStr;
                mErrorManager->addMessage(SOP_OPTYPE_NAME, SOP_MESSAGE, msg.str().c_str());
            }
        }
     }


    void setPrimitiveIndex(int i) { mPrimitiveIndex = i; }

    void setPrimitiveName(const std::string& name) { mPrimitiveName = name; }

    bool hasReplacementGrid() const { return mReplacementGrid != nullptr; }

    openvdb::GridBase::Ptr replacementGrid() { return mReplacementGrid; }


    template<typename GridType>
    void operator()(const GridType& grid)
    {
        mReplacementGrid.reset(); // clear

        using TreeType = typename GridType::TreeType;
        using ValueType = typename GridType::ValueType;
        using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
        using MaskDataType = MaskData<GridType>;
        using VisitorType = Visitor<GridType>;

        //////////

        const double voxelSize = grid.transform().voxelSize()[0];
        const std::string gridName = mPrimitiveName.empty() ? grid.getName() : mPrimitiveName;
        const TreeType& tree = grid.tree();

        GridTestLog log(mPrimitiveIndex, gridName);

        VisitorType visitor(tree);

        std::vector<typename BoolTreeType::Ptr> idMasks;
        std::vector<MaskDataType> fixMasks;

        // General tests

        bool inactivateTiles = false, renormalizeLevelSet = false;

        if (mTest.testFinite) {

            if (!visitor.run(VisitorType::TILES_AND_VOXELS,
                VisitorType::ALL_VALUES, FiniteValue()))
            {
                log.appendFailed("Finite Values", visitor.invalidValuesInfo());

                if (mTest.fixFinite) {
                    fixMasks.push_back(MaskDataType(visitor.valueMask(), tree.background()));
                }

                if (mTest.idFinite) idMasks.push_back(visitor.valueMask());

            } else {
                log.appendPassed();
            }
        }

        if (mInterupter->wasInterrupted()) return;

        if (mTest.testUniformBackground
            && (!mTest.respectGridClass || grid.getGridClass() != openvdb::GRID_LEVEL_SET))
        {
            ApproxEqual<ValueType> test(tree.background());
            if (!visitor.run(VisitorType::TILES_AND_VOXELS, VisitorType::INACTIVE_VALUES, test)) {

                log.appendFailed("Uniform Background", visitor.invalidValuesInfo());

                if (mTest.fixUniformBackground) {
                    fixMasks.push_back(MaskDataType(visitor.valueMask(), tree.background()));
                }

                if (mTest.idUniformBackground) idMasks.push_back(visitor.valueMask());

            } else {
                log.appendPassed();
            }
        }

        if (mInterupter->wasInterrupted()) return;


        if (mTest.testInRange) {

            InRange test(mTest.rangeMin, mTest.rangeMax);
            if (!visitor.run(VisitorType::TILES_AND_VOXELS, VisitorType::ALL_VALUES, test)) {

                log.appendFailed("Values in Range", visitor.invalidValuesInfo());

                if (mTest.fixInRange) {

                    fixMasks.push_back(MaskDataType(visitor.valueMask(),
                        ValueType(mTest.rangeMin), ValueType(mTest.rangeMax)));
                }

                if (mTest.idInRange) idMasks.push_back(visitor.valueMask());

            } else {
                log.appendPassed();
            }
        }


        if (mInterupter->wasInterrupted()) return;


        // Level Set tests
        if (!mTest.respectGridClass || grid.getGridClass() == openvdb::GRID_LEVEL_SET) {

            if (mTest.testUniformVoxelSize) {
                if (!grid.hasUniformVoxels()) log.appendFailed("'Uniform Voxel Size'");
                else log.appendPassed();
            }

            if (mTest.testNoActiveTiles) {

                if (!visitor.run(VisitorType::TILES, VisitorType::ACTIVE_VALUES, AlwaysFalse())) {

                    log.appendFailed("Inactive Tiles", visitor.invalidValuesInfo());

                    if (mTest.fixNoActiveTiles) inactivateTiles = true;

                    if (mTest.idNoActiveTiles) idMasks.push_back(visitor.valueMask());

                } else {
                    log.appendPassed();
                }
            }

            if (mTest.testSymmetricNarrowBand) {

                if (std::is_floating_point<ValueType>::value) {
                    const ValueType background = openvdb::math::Abs(tree.background());
                    AbsApproxEqual<ValueType> bgTest(background);
                    InRange valueTest(-toFloat(background), toFloat(background));
                    if (!visitor.run(VisitorType::TILES_AND_VOXELS,
                        VisitorType::INACTIVE_VALUES, bgTest)
                        || !visitor.run(VisitorType::VOXELS, VisitorType::ACTIVE_VALUES, valueTest))
                    {
                        log.appendFailed("Symmetric Narrow Band");
                    } else {
                        log.appendPassed();
                    }
                } else {
                    log.appendSkipped();
                }

            }

            if (mInterupter->wasInterrupted()) return;

            if (mTest.testMinimumBandWidth) {

                if (std::is_floating_point<ValueType>::value) {
                    const ValueType width = ValueType(mTest.minBandWidth) * ValueType(voxelSize);

                    AbsLessThan<ValueType> test(width);

                    if (tree.background() < width || !visitor.run(
                        VisitorType::TILES_AND_VOXELS, VisitorType::INACTIVE_VALUES, test))
                    {
                        log.appendFailed("Minimum Band Width");
                    } else {
                        log.appendPassed();
                    }
                } else {
                    log.appendSkipped();
                }
            }

            if (mInterupter->wasInterrupted()) return;

            if (mTest.testClosedSurface) {

                if (std::is_floating_point<ValueType>::value) {
                    typename GridType::Ptr levelSet = openvdb::tools::levelSetRebuild(
                        grid, 0.0f, 2.0f, 2.0f, nullptr, mInterupter);

                    SameSign<TreeType> test(levelSet->tree());
                    if (!visitor.run(VisitorType::TILES_AND_VOXELS,
                        VisitorType::ALL_VALUES, test))
                    {
                        log.appendFailed("Closed Surface");
                    } else {
                        log.appendPassed();
                    }
                } else {
                    log.appendSkipped();
                }
            }

            if (mInterupter->wasInterrupted()) return;


            if (mTest.testGradientMagnitude) {

                if (std::is_floating_point<ValueType>::value) {

                    GradientNorm<TreeType> test(tree, voxelSize,
                        ValueType(mTest.gradientTolerance));
                    if (!visitor.run(VisitorType::VOXELS, VisitorType::ACTIVE_VALUES, test)) {

                        log.appendFailed("Gradient Magnitude", visitor.invalidValuesInfo());

                        if (mTest.fixGradientMagnitude) renormalizeLevelSet = true;

                        if (mTest.idGradientMagnitude) idMasks.push_back(visitor.valueMask());

                    } else {
                        log.appendPassed();
                    }

                } else {
                    log.appendSkipped();
                }
            }
        } // end Level Set tests


        // Fog Volume tests
        if (!mTest.respectGridClass || grid.getGridClass() == openvdb::GRID_FOG_VOLUME) {

            if (mTest.testBackgroundZero) {

                ApproxEqual<ValueType> test(ValueType(0.0));
                if (!visitor.run(VisitorType::TILES_AND_VOXELS,
                    VisitorType::INACTIVE_VALUES, test))
                {
                    log.appendFailed("Background Zero", visitor.invalidValuesInfo());

                    if (mTest.fixBackgroundZero) {
                        fixMasks.push_back(MaskDataType(visitor.valueMask(), ValueType(0.0)));
                    }

                    if (mTest.idBackgroundZero) idMasks.push_back(visitor.valueMask());

                } else {
                    log.appendPassed();
                }
            }

            if (mTest.testActiveValuesFromZeroToOne) {

                InRange test(0.0f, 1.0f);
                if (!visitor.run(VisitorType::TILES_AND_VOXELS, VisitorType::ACTIVE_VALUES, test)) {

                    log.appendFailed("Active Values in [0, 1]", visitor.invalidValuesInfo());

                    if (mTest.fixActiveValuesFromZeroToOne) {
                        fixMasks.push_back(
                            MaskDataType(visitor.valueMask(), ValueType(0.0), ValueType(1.0)));
                    }

                    if (mTest.idActiveValuesFromZeroToOne) idMasks.push_back(visitor.valueMask());

                } else {
                    log.appendPassed();
                }
            }
        } // end Fog Volume tests


        typename GridType::Ptr replacement;

        if (!fixMasks.empty() || inactivateTiles || renormalizeLevelSet) {
            replacement = fixValues(grid, fixMasks, inactivateTiles, renormalizeLevelSet);
            mReplacementGrid = replacement;
        }

        if (mInterupter->wasInterrupted()) return;

        outputMaskAndPoints<GridType>(grid, gridName, idMasks, mTest.useMask, mTest.usePoints,
            *mDetail, *mInterupter, replacement.get());

        // log diagnostics info
        mMessageStr += log.str();
        if (log.failedCount() > 0) ++mGridsFailed;
    }

private:
    TestData                    mTest;
    GU_Detail           * const mDetail;
    hvdb::Interrupter   * const mInterupter;
    UT_ErrorManager     * const mErrorManager;
    std::string                 mMessageStr, mPrimitiveName;
    int                         mPrimitiveIndex, mGridsFailed;
    openvdb::GridBase::Ptr      mReplacementGrid;
}; // struct TestCollection


} // unnamed namespace


////////////////////////////////////////

// SOP Implementation

class SOP_OpenVDB_Diagnostics: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Diagnostics(OP_Network*, const char* name, OP_Operator*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);
    int isRefInput(unsigned i) const override { return (i == 1); }

    int selectOperationTests();
    int validateOperationTests();

    class Cache: public SOP_VDBCacheOptions
    {
        OP_ERROR cookVDBSop(OP_Context&) override;
        TestData getTestData(const fpreal time) const;
    };

protected:
    bool updateParmsFlags() override;
};


int
SOP_OpenVDB_Diagnostics::selectOperationTests()
{
    setInt("test_valrange", 0, 0, 0);

    setInt("test_backgroundzero", 0, 0, 0);
    setInt("test_fogvalues", 0, 0, 0);
    setInt("test_voxelsize", 0, 0, 0);
    setInt("test_activetiles", 0, 0, 0);
    setInt("test_symmetric", 0, 0, 0);
    setInt("test_surface", 0, 0, 0);
    setInt("test_bandwidth", 0, 0, 0);

    if (bool(evalInt("verify_fogvolume", 0, 0))) {
        setInt("test_finite", 0, 0, 1);
        setInt("test_backgroundzero", 0, 0, 1);
        setInt("test_fogvalues", 0, 0, 1);
    }

    if (bool(evalInt("verify_csg", 0, 0))) {
        setInt("test_finite", 0, 0, 1);
        setInt("test_voxelsize", 0, 0, 1);
        setInt("test_activetiles", 0, 0, 1);

        setInt("test_symmetric", 0, 0, 1);
        setInt("test_surface", 0, 0, 1);

        setInt("test_background", 0, 0, 0);
    }

    if (bool(evalInt("verify_filtering", 0, 0))) {
        setInt("test_finite", 0, 0, 1);
        setInt("test_voxelsize", 0, 0, 1);
        setInt("test_activetiles", 0, 0, 1);

        setInt("test_symmetric", 0, 0, 1);
        setInt("test_bandwidth", 0, 0, 1);
        setInt("bandwidth", 0, 0, 3);

        setInt("test_background", 0, 0, 0);
    }

    if (bool(evalInt("verify_advection", 0, 0))) {
        setInt("test_finite", 0, 0, 1);
        setInt("test_voxelsize", 0, 0, 1);
        setInt("test_activetiles", 0, 0, 1);

        setInt("test_surface", 0, 0, 1);
        setInt("test_symmetric", 0, 0, 1);
        setInt("test_bandwidth", 0, 0, 1);
        setInt("bandwidth", 0, 0, 3);

        setInt("test_background", 0, 0, 0);
    }

    return 1;
}

int
SOP_OpenVDB_Diagnostics::validateOperationTests()
{
    // general tests
    const bool testFinite = bool(evalInt("test_finite", 0, 0));
    const bool testUniformBackground = bool(evalInt("test_background", 0, 0));
    const bool testInRange = bool(evalInt("test_valrange", 0, 0));
    const bool testUniformVoxelSize = bool(evalInt("test_voxelsize", 0, 0));

    // level set
    const bool testSymmetricNarrowBand = bool(evalInt("test_symmetric", 0, 0));
    const bool minBandWidth =
        bool(evalInt("test_bandwidth", 0, 0)) && evalInt("bandwidth", 0, 0) > 2;
    const bool testClosedSurface = bool(evalInt("test_surface", 0, 0));
    const bool testNoActiveTiles = bool(evalInt("test_activetiles", 0, 0));
    const bool basicLevelSetChecks = testFinite && !testUniformBackground && !testInRange
        && testUniformVoxelSize && testNoActiveTiles;

    // fog volume tests
    const bool basicFogVolumeChecks = testFinite && !testInRange &&
        bool(evalInt("test_backgroundzero", 0, 0)) && bool(evalInt("test_fogvalues", 0, 0));

    { // Validate fog volume operations
        setInt("verify_fogvolume", 0, 0, int(basicFogVolumeChecks));
    }

    { // Validate level set CSG tests
        bool isValid = basicLevelSetChecks && testClosedSurface && testSymmetricNarrowBand;
        setInt("verify_csg", 0, 0, int(isValid));
    }

    { // Validate level set filtering tests
        bool isValid = basicLevelSetChecks && testSymmetricNarrowBand && minBandWidth;
        setInt("verify_filtering", 0, 0, int(isValid));
    }

    { // Validate level set advection tests
        bool isValid = basicLevelSetChecks && testClosedSurface
            && testSymmetricNarrowBand && minBandWidth;
        setInt("verify_advection", 0, 0, int(isValid));
    }

    return 1;
}


int selectOperationTestsCB(void*, int, float, const PRM_Template*);
int validateOperationTestsCB(void*, int, float, const PRM_Template*);


int
selectOperationTestsCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
{
   SOP_OpenVDB_Diagnostics* sop = static_cast<SOP_OpenVDB_Diagnostics*>(data);
   if (sop == nullptr) return 0;
   return sop->selectOperationTests();
}

int
validateOperationTestsCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
{
   SOP_OpenVDB_Diagnostics* sop = static_cast<SOP_OpenVDB_Diagnostics*>(data);
   if (sop == nullptr) return 0;
   return sop->validateOperationTests();
}


// Hack to work around lack of grid layout in parameter pane
// (one space character is four pixels wide, but the middle column is centered)
inline std::string
spacing(int widthInPixels)
{
    return std::string(widthInPixels >> 1, ' '); // 2 * width / 4
}


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenu)
        .setTooltip("Specify a subset of the input VDBs to examine.")
        .setDocumentation(
            "A subset of the input VDBs to be examined"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usemask", "Mark in Mask VDB")
        .setTooltip(
            "For tests set to Mark, output a mask VDB that highlights\n"
            "problematic regions in input VDBs."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usepoints", "Mark as Points With Values")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "For tests set to Mark, output a point cloud that highlights\n"
            "problematic regions in input VDBs."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "respectclass", "Respect VDB Class")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "If disabled, apply fog volume and level set tests to all VDBs,\n"
            "not just VDBs classified as fog volumes or level sets."));


    //////////
    // Operation

    parms.add(hutil::ParmFactory(PRM_SEPARATOR,"operation", ""));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "verify_fogvolume", "Validate Fog Volumes")
        .setCallbackFunc(&selectOperationTestsCB)
        .setTooltip("Verify that VDBs classified as fog volumes are valid fog volumes."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE,
        "verify_csg", "Validate for Level Set CSG and Fracture")
        .setCallbackFunc(&selectOperationTestsCB)
        .setTooltip(
            "Verify that level set VDBs meet the requirements\n"
            "for CSG and fracture operations."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE,
        "verify_filtering", "Validate for Level Set Filtering and Renormalization")
        .setCallbackFunc(&selectOperationTestsCB)
        .setTooltip(
            "Verify that level set VDBs meet the requirements\n"
            "for filtering and renormalization."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE,
        "verify_advection", "Validate for Level Set Advection and Morphing")
        .setCallbackFunc(&selectOperationTestsCB)
        .setTooltip(
            "Verify that level set VDBs meet the requirements\n"
            "for advection and morphing."));

    //////////
    // General

    parms.add(hutil::ParmFactory(PRM_HEADING, "general", "General Tests")
        .setDocumentation(
            "In the following, enable __Mark__ to add incorrect values"
            " to the output mask and/or point cloud, and enable __Fix__"
            " to replace incorrect values."));

    // { Finite values
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_finite", "Finite Values"
        + spacing(35)
    )
        .setCallbackFunc(&validateOperationTestsCB)
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip("Verify that all values are finite and non-NaN.")
        .setDocumentation(
            "Verify that all values are finite and non-NaN.\n\n"
            "If __Fix__ is enabled, replace incorrect values with the background value."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_finite", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_finite", "Fix")
        .setTooltip("Replace incorrect values with the background value.")
        .setDocumentation(nullptr));
    // }

    // { Uniform background values
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_background", "Uniform Background"
        )
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip("Verify that all inactive voxels are set to the background value.")
        .setDocumentation(
            "Verify that all inactive voxels are set to the background value.\n\n"
            "If __Fix__ is enabled, replace incorrect values with the background value."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_background", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_background", "Fix")
        .setTooltip("Replace incorrect values with the background value.")
        .setDocumentation(nullptr));
    // }

    // { Values in range
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_valrange", "Values in Range"
        + spacing(23)
        )
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip(
            "Verify that all scalar voxel values and vector magnitudes\n"
            "are in the given range.")
        .setDocumentation(
            "Verify that all scalar voxel values and vector magnitudes are in the given range.\n\n"
            "If __Fix__ is enabled, clamp values and vector magnitudes to the given range."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_valrange", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_valrange", "Fix")
        .setTooltip("Clamp values and vector magnitudes to the given range.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_LABEL | PRM_TYPE_JOIN_NEXT, "label_valrange", ""));

    std::vector<fpreal> defaultRange;
    defaultRange.push_back(fpreal(0.0));
    defaultRange.push_back(fpreal(1.0));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "valrange", "Range")
        .setDefault(defaultRange)
        .setVectorSize(2)
        .setTooltip("Minimum and maximum allowed values (inclusive)"));
    // }


    //////////
    // Level Set

    parms.add(hutil::ParmFactory(PRM_HEADING, "ls_heading", "Level Set Tests"));

    // { Symmetric Narrow Band
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_symmetric", "Symmetric Narrow Band")
        .setCallbackFunc(&validateOperationTestsCB)
        .setTooltip("Verify that level set inside and outside values are of equal magnitude."));
    // }

    // { Min Band Width
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_bandwidth", "Minimum Band Width")
        .setCallbackFunc(&validateOperationTestsCB)
        .setTooltip(
            "Verify that interior and exterior narrow band widths"
            " are sufficiently large."));

    parms.add(hutil::ParmFactory(PRM_LABEL | PRM_TYPE_JOIN_NEXT, "label_bandwidth", ""));

    parms.add(hutil::ParmFactory(PRM_INT_J, "bandwidth", "Minimum Width in Voxels")
        .setCallbackFunc(&validateOperationTestsCB)
        .setDefault(3));
    // }

    // { Closed Surface
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_surface", "Closed Surface")
        .setCallbackFunc(&validateOperationTestsCB)
        .setTooltip("Verify that level sets represent watertight surfaces."));
    // }

    // { Gradient magnitude
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_gradient", "Gradient Magnitude"
        + spacing(7)
        )
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip(
            "Verify that the level set gradient has magnitude one everywhere\n"
            "(within a given tolerance).")
        .setDocumentation(
            "Verify that the level set gradient has magnitude one everywhere"
            " (within a given tolerance).\n\n"
            "If __Fix__ is enabled, renormalize level sets."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_gradient", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_gradient", "Fix")
        .setTooltip("Renormalize level sets.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_LABEL | PRM_TYPE_JOIN_NEXT, "label_gradient", ""));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "gradienttolerance", "Tolerance")
        .setDefault(0.2f)
        .setDocumentation(nullptr));
    // }

    // { Inactive Tiles
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_activetiles", "Inactive Tiles"
        + spacing(36)
        )
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip("Verify that level sets have no active tiles.")
        .setDocumentation(
            "Verify that level sets have no active tiles.\n\n"
            "If __Fix__ is enabled, deactivate all tiles."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_activetiles", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_activetiles", "Fix")
        .setTooltip("Deactivate all tiles.")
        .setDocumentation(nullptr));
    // }

    // { Uniform Voxel Size
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_voxelsize", "Uniform Voxel Size")
        .setTooltip("Verify that level sets have uniform voxel sizes."));
    // }

    //////////
    // Fog Volume

    parms.add(hutil::ParmFactory(PRM_HEADING, "fog_heading", "Fog Volume Tests")
        .setTooltip("Fog Volume specific tests."));

    // { Background values
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "test_backgroundzero", "Background Zero"
        + spacing(17)
        )
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip("Verify that all inactive voxels in fog volumes have value zero.")
        .setDocumentation(
            "Verify that all inactive voxels in fog volumes have value zero.\n\n"
            "If __Fix__ is enabled, set inactive voxels to zero."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_backgroundzero", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_backgroundzero", "Fix")
        .setTooltip("Set inactive voxels to zero.")
        .setDocumentation(nullptr));
    // }

    // { Active values
    parms.add(hutil::ParmFactory(PRM_TOGGLE,
        // Note: this label currently determines the spacing of the second column of toggles.
        "test_fogvalues", "Active Values in [0, 1]")
        .setCallbackFunc(&validateOperationTestsCB)
        .setTypeExtended(PRM_TYPE_MENU_JOIN)
        .setTooltip(
            "Verify that all active voxels in fog volumes\n"
            "have values in the range [0, 1].")
        .setDocumentation(
            "Verify that all active voxels in fog volumes have values in the range"
            " &#91;0, 1&#93;.\n\n" // "[0, 1]"
            "If __Fix__ is enabled, clamp active voxels to the range &#91;0, 1&#93;."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE | PRM_TYPE_JOIN_NEXT, "id_fogvalues", "Mark")
        .setTooltip("Add incorrect values to the output mask and/or point cloud.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fix_fogvalues", "Fix")
        .setTooltip("Clamp active values to the range [0, 1].")
        .setDocumentation(nullptr));
    // }

    hvdb::OpenVDBOpFactory("VDB Diagnostics", SOP_OpenVDB_Diagnostics::factory, parms, *table)
        .addInput("VDB Volumes")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Diagnostics::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Examine VDB volumes for bad values.\"\"\"\n\
\n\
@overview\n\
\n\
This node runs a suite of tests to validate and correct common errors in VDB volumes.\n\
It provides the option to output either a mask VDB or a point cloud that identifies\n\
the troublesome voxels, and it is optionally able to correct most types of errors.\n\
\n\
@related\n\
- [Node:sop/vdbdiagnostics]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Diagnostics::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Diagnostics(net, name, op);
}


SOP_OpenVDB_Diagnostics::SOP_OpenVDB_Diagnostics(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


bool
SOP_OpenVDB_Diagnostics::updateParmsFlags()
{
    bool changed = false;

    const bool identify = bool(evalInt("usemask", 0, 0)) || bool(evalInt("usepoints", 0, 0));

    // general

    const bool testFinite = bool(evalInt("test_finite", 0, 0));
    changed |= enableParm("id_finite", identify && testFinite);
    changed |= enableParm("fix_finite", testFinite);

    const bool testUniformBackground = bool(evalInt("test_background", 0, 0));
    changed |= enableParm("id_background", identify && testUniformBackground);
    changed |= enableParm("fix_background", testUniformBackground);

    const bool testInRange = bool(evalInt("test_valrange", 0, 0));
    changed |= enableParm("id_valrange", identify && testInRange);
    setVisibleState("label_valrange", testInRange);
    setVisibleState("valrange", testInRange);
    changed |= enableParm("fix_valrange", testInRange);

    // level set

    setVisibleState("label_bandwidth", evalInt("test_bandwidth", 0, 0));
    setVisibleState("bandwidth", evalInt("test_bandwidth", 0, 0));

    const bool testGradientMagnitude = bool(evalInt("test_gradient", 0, 0));
    changed |= enableParm("id_gradient", identify && testGradientMagnitude);
    setVisibleState("label_gradient", testGradientMagnitude);
    setVisibleState("gradienttolerance", testGradientMagnitude);
    changed |= enableParm("fix_gradient", testGradientMagnitude);

    const bool testNoActiveTiles = bool(evalInt("test_activetiles", 0, 0));
    changed |= enableParm("id_activetiles", identify && testNoActiveTiles);
    changed |= enableParm("fix_activetiles", testNoActiveTiles);

    // fog volume

    const bool testBackgroundZero = bool(evalInt("test_backgroundzero", 0, 0));
    changed |= enableParm("id_backgroundzero", identify && testBackgroundZero);
    changed |= enableParm("fix_backgroundzero", testBackgroundZero);

    const bool testActiveValuesFromZeroToOne = bool(evalInt("test_fogvalues", 0, 0));
    changed |= enableParm("id_fogvalues", identify && testActiveValuesFromZeroToOne);
    changed |= enableParm("fix_fogvalues", testActiveValuesFromZeroToOne);

    return changed;
}


TestData
SOP_OpenVDB_Diagnostics::Cache::getTestData(const fpreal time) const
{
    TestData test;

    test.useMask = bool(evalInt("usemask", 0, time));
    test.usePoints = bool(evalInt("usepoints", 0, time));
    test.respectGridClass = bool(evalInt("respectclass", 0, time));

    const bool identify = test.useMask || test.usePoints;

    // general

    test.testFinite = bool(evalInt("test_finite", 0, time));
    test.idFinite = identify && bool(evalInt("id_finite", 0, time));
    test.fixFinite = bool(evalInt("fix_finite", 0, time));

    test.testUniformBackground = bool(evalInt("test_background", 0, time));
    test.idUniformBackground = identify && bool(evalInt("id_background", 0, time));
    test.fixUniformBackground = bool(evalInt("fix_background", 0, time));

    test.testInRange = bool(evalInt("test_valrange", 0, time));
    test.idInRange = identify && bool(evalInt("id_valrange", 0, time));
    test.fixInRange = bool(evalInt("fix_valrange", 0, time));
    test.rangeMin = float(evalFloat("valrange", 0, time));
    test.rangeMax = float(evalFloat("valrange", 1, time));

    // level set

    test.testSymmetricNarrowBand = bool(evalInt("test_symmetric", 0, time));

    test.testMinimumBandWidth = bool(evalInt("test_bandwidth", 0, time));
    test.minBandWidth = float(evalInt("bandwidth", 0, time));

    test.testClosedSurface = bool(evalInt("test_surface", 0, time));

    test.testGradientMagnitude = bool(evalInt("test_gradient", 0, time));
    test.idGradientMagnitude = identify && bool(evalInt("id_gradient", 0, time));
    test.fixGradientMagnitude = bool(evalInt("fix_gradient", 0, time));
    test.gradientTolerance = float(evalFloat("gradienttolerance", 0, time));

    test.testNoActiveTiles = bool(evalInt("test_activetiles", 0, time));
    test.idNoActiveTiles = identify && bool(evalInt("id_activetiles", 0, time));
    test.fixNoActiveTiles = bool(evalInt("fix_activetiles", 0, time));
    test.testUniformVoxelSize = bool(evalInt("test_voxelsize", 0, time));

    // fog volume

    test.testBackgroundZero = bool(evalInt("test_backgroundzero", 0, time));
    test.idBackgroundZero = identify && bool(evalInt("id_backgroundzero", 0, time));
    test.fixBackgroundZero = bool(evalInt("fix_backgroundzero", 0, time));

    test.testActiveValuesFromZeroToOne = bool(evalInt("test_fogvalues", 0, time));
    test.idActiveValuesFromZeroToOne = identify && bool(evalInt("id_fogvalues", 0, time));
    test.fixActiveValuesFromZeroToOne = bool(evalInt("fix_fogvalues", 0, time));

    return test;
}


OP_ERROR
SOP_OpenVDB_Diagnostics::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Performing diagnostics");

        TestCollection tests(getTestData(time), *gdp, boss, UTgetErrorManager());

        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        size_t vdbPrimCount = 0;

        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

            if (boss.wasInterrupted()) break;

            tests.setPrimitiveName(it.getPrimitiveName().toStdString());
            tests.setPrimitiveIndex(int(it.getIndex()));

            GEOvdbProcessTypedGridTopology(**it, tests, /*makeUnique=*/false);

            if (tests.replacementGrid()) {
                hvdb::replaceVdbPrimitive(*gdp, tests.replacementGrid(), **it, true,
                    tests.replacementGrid()->getName().c_str());
            }

            ++vdbPrimCount;
        }

        if (vdbPrimCount == 0) {
            addWarning(SOP_MESSAGE, "Did not find any VDBs to diagnose.");
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
