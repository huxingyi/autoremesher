// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Remap.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/math/Math.h> // Tolerance and isApproxEqual
#include <openvdb/tools/ValueTransformer.h>

#include <UT/UT_Version.h>
#include <UT/UT_Ramp.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>

#ifdef SESI_OPENVDB
  #include <hboost/mpl/at.hpp>
  namespace boostmpl = hboost::mpl;
#else
  #include <boost/mpl/at.hpp>
  namespace boostmpl = boost::mpl;
#endif

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <sstream>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////

// Local Utility Methods

namespace {


template<typename T>
inline T minValue(const T a, const T b) { return std::min(a, b); }

template<typename T>
inline T maxValue(const T a, const T b) { return std::max(a, b); }

template<typename T>
inline openvdb::math::Vec3<T>
minValue(const openvdb::math::Vec3<T>& a, const openvdb::math::Vec3<T>& b) {
    return openvdb::math::minComponent(a, b);
}

template<typename T>
inline openvdb::math::Vec3<T>
maxValue(const openvdb::math::Vec3<T>& a, const openvdb::math::Vec3<T>& b) {
    return openvdb::math::maxComponent(a, b);
}

template<typename T>
inline T minComponent(const T s) { return s; }

template<typename T>
inline T maxComponent(const T s) { return s; }

template<typename T>
inline T
minComponent(const openvdb::math::Vec3<T>& v) {
    return minValue(v[0], minValue(v[1], v[2]));
}

template<typename T>
inline T
maxComponent(const openvdb::math::Vec3<T>& v) {
    return maxValue(v[0], maxValue(v[1], v[2]));
}


////////////////////////////////////////


template<typename NodeType>
struct NodeMinMax
{
    using ValueType = typename NodeType::ValueType;

    NodeMinMax(const std::vector<const NodeType*>& nodes, ValueType background)
        : mNodes(&nodes[0]), mBackground(background), mMin(background), mMax(background)
    {}

    NodeMinMax(NodeMinMax& other, tbb::split)
        : mNodes(other.mNodes), mBackground(other.mBackground), mMin(mBackground), mMax(mBackground)
    {}

    void join(NodeMinMax& other) {
        mMin = minValue(other.mMin, mMin);
        mMax = maxValue(other.mMax, mMax);
    }

    void operator()(const tbb::blocked_range<size_t>& range) {
        ValueType minTmp(mMin), maxTmp(mMax);
        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
            const NodeType& node = *mNodes[n];
            for (typename NodeType::ValueAllCIter it = node.cbeginValueAll(); it; ++it) {

                if (node.isChildMaskOff(it.pos())) {
                    const ValueType val = *it;
                    minTmp = minValue(minTmp, val);
                    maxTmp = maxValue(maxTmp, val);
                }
            }
        }
        mMin = minValue(minTmp, mMin);
        mMax = maxValue(maxTmp, mMax);
    }

    NodeType const * const * const mNodes;
    ValueType mBackground, mMin, mMax;
};

template<typename NodeType>
struct Deactivate
{
    using ValueType = typename NodeType::ValueType;

    Deactivate(std::vector<NodeType*>& nodes, ValueType background)
        : mNodes(&nodes[0]), mBackground(background)
    {}

    void operator()(const tbb::blocked_range<size_t>& range) const {
        const ValueType
            background(mBackground),
            delta = openvdb::math::Tolerance<ValueType>::value();
        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
            for (typename NodeType::ValueOnIter it = mNodes[n]->beginValueOn(); it; ++it) {
                if (openvdb::math::isApproxEqual(background, *it, delta)) {
                    it.setValueOff();
                }
            }
        }
    }

    NodeType * const * const mNodes;
    ValueType mBackground;
};


template<typename TreeType>
void
evalMinMax(const TreeType& tree,
    typename TreeType::ValueType& minVal, typename TreeType::ValueType& maxVal)
{
    minVal = tree.background();
    maxVal = tree.background();

    { // eval voxels
        using LeafNodeType = typename TreeType::LeafNodeType;
        std::vector<const LeafNodeType*> nodes;
        tree.getNodes(nodes);

        NodeMinMax<LeafNodeType> op(nodes, tree.background());
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, nodes.size()), op);

        minVal = minValue(minVal, op.mMin);
        maxVal = maxValue(maxVal, op.mMax);
    }

    { // eval first tiles
        using RootNodeType = typename TreeType::RootNodeType;
        using NodeChainType = typename RootNodeType::NodeChainType;
        using InternalNodeType = typename boostmpl::at<NodeChainType, boostmpl::int_<1>>::type;

        std::vector<const InternalNodeType*> nodes;
        tree.getNodes(nodes);

        NodeMinMax<InternalNodeType> op(nodes, tree.background());
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, nodes.size()), op);

        minVal = minValue(minVal, op.mMin);
        maxVal = maxValue(maxVal, op.mMax);
    }

    { // eval remaining tiles
        typename TreeType::ValueType minTmp(minVal), maxTmp(maxVal);
        typename TreeType::ValueAllCIter it(tree);
        it.setMaxDepth(TreeType::ValueAllCIter::LEAF_DEPTH - 2);
        for ( ; it; ++it) {
            const typename TreeType::ValueType val = *it;
            minTmp = minValue(minTmp, val);
            maxTmp = maxValue(maxTmp, val);
        }

        minVal = minValue(minVal, minTmp);
        maxVal = maxValue(maxVal, maxTmp);
    }
}

template<typename TreeType>
void
deactivateBackgroundValues(TreeType& tree)
{
    { // eval voxels
        using LeafNodeType = typename TreeType::LeafNodeType;
        std::vector<LeafNodeType*> nodes;
        tree.getNodes(nodes);

        Deactivate<LeafNodeType> op(nodes, tree.background());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, nodes.size()), op);
    }

    { // eval first tiles
        using RootNodeType = typename TreeType::RootNodeType;
        using NodeChainType = typename RootNodeType::NodeChainType;
        using InternalNodeType = typename boostmpl::at<NodeChainType, boostmpl::int_<1>>::type;

        std::vector<InternalNodeType*> nodes;
        tree.getNodes(nodes);

        Deactivate<InternalNodeType> op(nodes, tree.background());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, nodes.size()), op);
    }

    { // eval remaining tiles
        using ValueType = typename TreeType::ValueType;
        const ValueType
            background(tree.background()),
            delta = openvdb::math::Tolerance<ValueType>::value();
        typename TreeType::ValueOnIter it(tree);
        it.setMaxDepth(TreeType::ValueAllCIter::LEAF_DEPTH - 2);
        for ( ; it; ++it) {
            if (openvdb::math::isApproxEqual(background, *it, delta)) {
               it.setValueOff();
            }
        }
    }
}




////////////////////////////////////////


struct RemapGridValues {

    enum Extrapolation { CLAMP, PRESERVE, EXTRAPOLATE };

    RemapGridValues(Extrapolation belowExt, Extrapolation aboveExt, UT_Ramp& ramp,
        const fpreal inMin, const fpreal inMax, const fpreal outMin, const fpreal outMax,
        bool deactivate, UT_ErrorManager* errorManager = nullptr)
        : mBelowExtrapolation(belowExt)
        , mAboveExtrapolation(aboveExt)
        , mRamp(&ramp)
        , mErrorManager(errorManager)
        , mPrimitiveIndex(0)
        , mPrimitiveName()
        , mInfo("Remapped grids: (first range shows actual min/max values)\n")
        , mInMin(inMin)
        , mInMax(inMax)
        , mOutMin(outMin)
        , mOutMax(outMax)
        , mDeactivate(deactivate)
    {
        mRamp->ensureRampIsBuilt();
    }

    ~RemapGridValues() {
        if (mErrorManager) {
            mErrorManager->addMessage(SOP_OPTYPE_NAME, SOP_MESSAGE, mInfo.c_str());
        }
    }

    void setPrimitiveIndex(int i) { mPrimitiveIndex = i; }
    void setPrimitiveName(const std::string& name) { mPrimitiveName = name; }

    template<typename GridType>
    void operator()(GridType& grid)
    {
        using ValueType = typename GridType::ValueType;
        using LeafNodeType = typename GridType::TreeType::LeafNodeType;

        std::vector<LeafNodeType*> leafnodes;
        grid.tree().getNodes(leafnodes);

        ValueType inputMin, inputMax;
        evalMinMax(grid.tree(), inputMin, inputMax);

        ValueTransform<GridType> op(*mRamp, leafnodes, mBelowExtrapolation, mAboveExtrapolation,
            mInMin, mInMax, mOutMin, mOutMax);

        // update voxels
        tbb::parallel_for(tbb::blocked_range<size_t>(0, leafnodes.size()), op);

        // update tiles
        typename GridType::ValueAllIter it = grid.beginValueAll();
        it.setMaxDepth(GridType::ValueAllIter::LEAF_DEPTH - 1);
        openvdb::tools::foreach(it, op, true);

        // update background value
        grid.tree().root().setBackground(op.map(grid.background()), /*updateChildNodes=*/false);
        grid.setGridClass(openvdb::GRID_UNKNOWN);

        ValueType outputMin, outputMax;
        evalMinMax(grid.tree(), outputMin, outputMax);

        size_t activeVoxelDelta = size_t(grid.tree().activeVoxelCount());
        if (mDeactivate) {
            deactivateBackgroundValues(grid.tree());
            activeVoxelDelta -= size_t(grid.tree().activeVoxelCount());
        }

        { // log
            std::stringstream msg;
            msg << "  (" << mPrimitiveIndex << ") '" << mPrimitiveName << "'"
                << " [" << minComponent(inputMin) << ", " << maxComponent(inputMax) << "]"
                << " -> [" << minComponent(outputMin) << ", " << maxComponent(outputMax) << "]";

            if (mDeactivate && activeVoxelDelta > 0) {
                msg << ", deactivated " << activeVoxelDelta << " voxels.";
            }

            msg << "\n";
            mInfo += msg.str();
        }
    }

private:
    template<typename GridType>
    struct ValueTransform
    {
        using LeafNodeType = typename GridType::TreeType::LeafNodeType;

        ValueTransform(const UT_Ramp& utramp, std::vector<LeafNodeType*>& leafnodes,
            Extrapolation belowExt, Extrapolation aboveExt, const fpreal inMin,
            const fpreal inMax, const fpreal outMin, const fpreal outMax)
            : ramp(&utramp)
            , nodes(&leafnodes[0])
            , belowExtrapolation(belowExt)
            , aboveExtrapolation(aboveExt)
            , xMin(inMin)
            , xScale((inMax - inMin))
            , yMin(outMin)
            , yScale((outMax - outMin))
        {
            xScale = std::abs(xScale) > fpreal(0.0) ? fpreal(1.0) / xScale : fpreal(0.0);
        }

        inline void operator()(const tbb::blocked_range<size_t>& range) const {
            for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
                typename GridType::ValueType * data = nodes[n]->buffer().data();
                for (size_t i = 0, I = LeafNodeType::SIZE; i < I; ++i) {
                    data[i] = map(data[i]);
                }
            }
        }

        inline void operator()(const typename GridType::ValueAllIter& it) const {
            it.setValue(map(*it));
        }

        template<typename T>
        inline T map(const T s) const {

            fpreal pos = (fpreal(s) - xMin) * xScale;

            if (pos < fpreal(0.0)) { // below (normalized) minimum
                if (belowExtrapolation == PRESERVE) return s;
                if (belowExtrapolation == EXTRAPOLATE) return T((pos * xScale) * yScale);
                pos = std::max(pos, fpreal(0.0)); // clamp
            }

            if (pos > fpreal(1.0)) { // above (normalized) maximum
                if (aboveExtrapolation == PRESERVE) return s;
                if (aboveExtrapolation == EXTRAPOLATE) return T((pos * xScale) * yScale);
                pos = std::min(pos, fpreal(1.0)); //clamp
            }

            float values[4] = { 0.0f };
            ramp->rampLookup(pos, values);
            return T(yMin + (values[0] * yScale));
        }

        template<typename T>
        inline openvdb::math::Vec3<T> map(const openvdb::math::Vec3<T>& v) const {
            openvdb::math::Vec3<T> out;
            out[0] = map(v[0]);
            out[1] = map(v[1]);
            out[2] = map(v[2]);
            return out;
        }

        UT_Ramp         const * const ramp;
        LeafNodeType  * const * const nodes;
        const Extrapolation belowExtrapolation, aboveExtrapolation;
        fpreal xMin, xScale, yMin, yScale;
    }; // struct ValueTransform

    //////////

    Extrapolation mBelowExtrapolation, mAboveExtrapolation;
    UT_Ramp * const mRamp;
    UT_ErrorManager * const mErrorManager;
    int mPrimitiveIndex;
    std::string mPrimitiveName, mInfo;
    const fpreal mInMin, mInMax, mOutMin, mOutMax;
    const bool mDeactivate;
}; // struct RemapGridValues

} // unnamed namespace


////////////////////////////////////////

// SOP Implementation

struct SOP_OpenVDB_Remap: public hvdb::SOP_NodeVDB
{
    SOP_OpenVDB_Remap(OP_Network*, const char* name, OP_Operator*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int sortInputRange();
    int sortOutputRange();

    class Cache: public SOP_VDBCacheOptions
    {
    public:
        void evalRamp(UT_Ramp&, fpreal time);
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
    }; // class Cache
};


int inputRangeCB(void*, int, float, const PRM_Template*);
int outputRangeCB(void*, int, float, const PRM_Template*);


int
inputRangeCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
{
   SOP_OpenVDB_Remap* sop = static_cast<SOP_OpenVDB_Remap*>(data);
   if (sop == nullptr) return 0;
   return sop->sortInputRange();
}

int
outputRangeCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
{
   SOP_OpenVDB_Remap* sop = static_cast<SOP_OpenVDB_Remap*>(data);
   if (sop == nullptr) return 0;
   return sop->sortOutputRange();
}


int
SOP_OpenVDB_Remap::sortInputRange()
{
    const fpreal inMin = evalFloat("inrange", 0, 0);
    const fpreal inMax = evalFloat("inrange", 1, 0);

    if (inMin > inMax) {
        setFloat("inrange", 0, 0, inMax);
        setFloat("inrange", 1, 0, inMin);
    }

    return 1;
}

int
SOP_OpenVDB_Remap::sortOutputRange()
{
    const fpreal outMin = evalFloat("outrange", 0, 0);
    const fpreal outMax = evalFloat("outrange", 1, 0);

    if (outMin > outMax) {
        setFloat("outrange", 0, 0, outMax);
        setFloat("outrange", 1, 0, outMin);
    }

    return 1;
}


void
SOP_OpenVDB_Remap::Cache::evalRamp(UT_Ramp& ramp, fpreal time)
{
    const auto rampStr = evalStdString("function", time);
    UT_IStream strm(rampStr.c_str(), rampStr.size(), UT_ISTREAM_ASCII);
    ramp.load(strm);
}


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenu)
        .setTooltip("Specify a subset of the input grids.")
        .setDocumentation(
            "A subset of the input VDBs to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    { // Extrapolation
        char const * const items[] = {
            "clamp",        "Clamp",
            "preserve",     "Preserve",
            "extrapolate",  "Extrapolate",
            nullptr
        };

        parms.add(hutil::ParmFactory(PRM_ORD, "below", "Below Minimum")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip(
                "Specify how to handle input values below the input range minimum:"
                " either by clamping to the output minimum (Clamp),"
                " leaving out-of-range values intact (Preserve),"
                " or extrapolating linearly from the output minimum (Extrapolate).")
            .setDocumentation(
                "How to handle input values below the input range minimum\n\n"
                "Clamp:\n"
                "    Clamp values to the output minimum.\n"
                "Preserve:\n"
                "    Leave out-of-range values intact.\n"
                "Extrapolate:\n"
                "    Extrapolate values linearly from the output minimum.\n"));

        parms.add(hutil::ParmFactory(PRM_ORD, "above", "Above Maximum")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip(
                "Specify how to handle input values above the input range maximum:"
                " either by clamping to the input maximum (Clamp),"
                " leaving out-of-range values intact (Preserve),"
                " or extrapolating linearly from the input maximum (Extrapolate).")
            .setDocumentation(
                "How to handle output values above the input range maximum\n\n"
                "Clamp:\n"
                "    Clamp values to the input maximum.\n"
                "Preserve:\n"
                "    Leave out-of-range values intact.\n"
                "Extrapolate:\n"
                "    Extrapolate values linearly from the input maximum.\n"));
    }

    std::vector<fpreal> defaultRange;
    defaultRange.push_back(fpreal(0.0));
    defaultRange.push_back(fpreal(1.0));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "inrange", "Input Range")
        .setDefault(defaultRange)
        .setVectorSize(2)
        .setTooltip("Input min/max value range")
        .setCallbackFunc(&inputRangeCB));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "outrange", "Output Range")
        .setDefault(defaultRange)
        .setVectorSize(2)
        .setTooltip("Output min/max value range")
        .setCallbackFunc(&outputRangeCB));

    {
        std::map<std::string, std::string> rampSpare;
        rampSpare[PRM_SpareData::getFloatRampDefaultToken()] =
            "1pos ( 0.0 ) 1value ( 0.0 ) 1interp ( linear ) "
            "2pos ( 1.0 ) 2value ( 1.0 ) 2interp ( linear )";

        rampSpare[PRM_SpareData::getRampShowControlsDefaultToken()] = "0";

        parms.add(hutil::ParmFactory(PRM_MULTITYPE_RAMP_FLT, "function", "Transfer Function")
            .setDefault(PRMtwoDefaults)
            .setSpareData(rampSpare)
            .setTooltip("X Axis: 0 = input range minimum, 1 = input range maximum.\n"
                "Y Axis: 0 = output range minimum, 1 = output range maximum.\n")
            .setDocumentation(
                "Map values through a transfer function where _x_ = 0 corresponds to"
                " the input range minimum, _x_ = 1 to the input range maximum,"
                " _y_ = 0 to the output range minimum, and _y_ = 1 to the"
                " output range maximum."));
    }

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "deactivate", "Deactivate Background Voxels")
        .setTooltip("Deactivate voxels with values equal to the remapped background value."));


    hvdb::OpenVDBOpFactory("VDB Remap",
        SOP_OpenVDB_Remap::factory, parms, *table)
        .setNativeName("")
        .addInput("VDB Grids")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Remap::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Perform a remapping of the voxel values in a VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node remaps voxel values to a new range, optionally through\n\
a user-specified transfer function.\n\
\n\
@related\n\
- [Node:sop/volumevop]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}

OP_Node*
SOP_OpenVDB_Remap::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Remap(net, name, op);
}

SOP_OpenVDB_Remap::SOP_OpenVDB_Remap(OP_Network* net, const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}

OP_ERROR
SOP_OpenVDB_Remap::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Remapping values");

        const fpreal inMin = evalFloat("inrange", 0, time);
        const fpreal inMax = evalFloat("inrange", 1, time);
        const fpreal outMin = evalFloat("outrange", 0, time);
        const fpreal outMax = evalFloat("outrange", 1, time);
        const bool deactivate = bool(evalInt("deactivate", 0, time));

        RemapGridValues::Extrapolation belowExtrapolation = RemapGridValues::CLAMP;
        RemapGridValues::Extrapolation aboveExtrapolation = RemapGridValues::CLAMP;

        auto extrapolation = evalInt("below", 0, time);
        if (extrapolation == 1) belowExtrapolation = RemapGridValues::PRESERVE;
        else if (extrapolation == 2) belowExtrapolation = RemapGridValues::EXTRAPOLATE;

        extrapolation = evalInt("above", 0, time);
        if (extrapolation == 1) aboveExtrapolation = RemapGridValues::PRESERVE;
        else if (extrapolation == 2) aboveExtrapolation = RemapGridValues::EXTRAPOLATE;

        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        size_t vdbPrimCount = 0;

        UT_Ramp ramp;
        evalRamp(ramp, time);
        RemapGridValues remap(belowExtrapolation, aboveExtrapolation, ramp,
            inMin, inMax, outMin, outMax, deactivate, UTgetErrorManager());

        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

            if (boss.wasInterrupted()) break;

            remap.setPrimitiveName(it.getPrimitiveName().toStdString());
            remap.setPrimitiveIndex(int(it.getIndex()));

            GEOvdbProcessTypedGrid(**it, remap, /*makeUnique=*/true);

            GU_PrimVDB* vdbPrim = *it;
            const GEO_VolumeOptions& visOps = vdbPrim->getVisOptions();
            vdbPrim->setVisualization(GEO_VOLUMEVIS_SMOKE , visOps.myIso, visOps.myDensity);

            ++vdbPrimCount;
        }

        if (vdbPrimCount == 0) {
            addWarning(SOP_MESSAGE, "Did not find any VDBs.");
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
