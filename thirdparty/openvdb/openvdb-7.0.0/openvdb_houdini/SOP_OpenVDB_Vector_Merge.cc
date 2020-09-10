// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Vector_Merge.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Merge groups of up to three scalar grids into vector grids.

#ifdef _WIN32
#define BOOST_REGEX_NO_LIB
#define HBOOST_REGEX_NO_LIB
#endif

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/ValueTransformer.h> // for tools::foreach()
#include <openvdb/tools/Prune.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_SharedPtr.h>
#include <UT/UT_String.h>
#include <UT/UT_Version.h>
#include <hboost/regex.hpp>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

// HAVE_MERGE_GROUP is disabled in Houdini
#ifdef SESI_OPENVDB
#define HAVE_MERGE_GROUP 0
#else
#define HAVE_MERGE_GROUP 1
#endif


class SOP_OpenVDB_Vector_Merge: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Vector_Merge(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Vector_Merge() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;

    static void addWarningMessage(SOP_OpenVDB_Vector_Merge* self, const char* msg)
        { if (self && msg) self->addWarning(SOP_MESSAGE, msg); }
};


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Group of X grids
    parms.add(hutil::ParmFactory(PRM_STRING, "xgroup", "X Group")
        .setDefault("@name=*.x")
        .setTooltip(
            "Specify a group of scalar input VDB grids to be used\n"
            "as the x components of the merged vector grids.\n"
            "Each x grid will be paired with a y and a z grid\n"
            "(if provided) to produce an output vector grid.")
        .setChoiceList(&hutil::PrimGroupMenuInput1));

    // Group of Y grids
    parms.add(hutil::ParmFactory(PRM_STRING, "ygroup", "Y Group")
        .setDefault("@name=*.y")
        .setTooltip(
            "Specify a group of scalar input VDB grids to be used\n"
            "as the y components of the merged vector grids.\n"
            "Each y grid will be paired with an x and a z grid\n"
            "(if provided) to produce an output vector grid.")
        .setChoiceList(&hutil::PrimGroupMenuInput1));

    // Group of Z grids
    parms.add(hutil::ParmFactory(PRM_STRING, "zgroup", "Z Group")
        .setDefault("@name=*.z")
        .setTooltip(
            "Specify a group of scalar input VDB grids to be used\n"
            "as the z components of the merged vector grids.\n"
            "Each z grid will be paired with an x and a y grid\n"
            "(if provided) to produce an output vector grid.")
        .setChoiceList(&hutil::PrimGroupMenuInput1));

    // Use X name
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usexname", "Use Basename of X VDB")
#ifdef SESI_OPENVDB
        .setDefault(PRMoneDefaults)
#else
        .setDefault(PRMzeroDefaults)
#endif
        .setDocumentation(
            "Use the base name of the __X Group__ as the name for the output VDB."
            " For example, if __X Group__ is `Cd.x`, the generated vector VDB"
            " will be named `Cd`.\n\n"
            "If this option is disabled or if the __X__ primitive has no `name` attribute,"
            " the output VDB will be given the __Merged VDB Name__."));

    // Output vector grid name
    parms.add(hutil::ParmFactory(PRM_STRING, "merge_name", "Merged VDB Name")
        .setDefault("merged#")
        .setTooltip(
            "Specify a name for the merged vector grids.\n"
            "Include a '#' character in the name to number the output grids\n"
            "(starting from 1) in the order that they are processed."));

    {
        // Output grid's vector type (invariant, covariant, etc.)
        std::vector<std::string> items;
        for (int i = 0; i < openvdb::NUM_VEC_TYPES ; ++i) {
            items.push_back(openvdb::GridBase::vecTypeToString(openvdb::VecType(i)));
            items.push_back(openvdb::GridBase::vecTypeExamples(openvdb::VecType(i)));
        }
        parms.add(hutil::ParmFactory(PRM_ORD, "vectype", "Vector Type")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDocumentation("\
Specify how the output VDB's vector values should be affected by transforms:\n\
\n\
Tuple / Color / UVW:\n\
    No transformation\n\
\n\
Gradient / Normal:\n\
    Inverse-transpose transformation, ignoring translation\n\
\n\
Unit Normal:\n\
    Inverse-transpose transformation, ignoring translation,\n\
    followed by renormalization\n\
\n\
Displacement / Velocity / Acceleration:\n\
    \"Regular\" transformation, ignoring translation\n\
\n\
Position:\n\
    \"Regular\" transformation with translation\n"));
    }

#if HAVE_MERGE_GROUP
    // Toggle to enable/disable grouping
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enable_grouping", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, create a group for all merged vector grids."));

    // Output vector grid group name
    parms.add(hutil::ParmFactory(PRM_STRING, "group",  "Merge Group")
        .setTooltip("Specify a name for the output group of merged vector grids."));
#endif

    // Toggle to keep/remove source grids
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "remove_sources", "Remove Source VDBs")
        .setDefault(PRMoneDefaults)
        .setTooltip("Remove scalar grids that have been merged."));

    // Toggle to copy inactive values in addition to active values
    parms.add(
        hutil::ParmFactory(PRM_TOGGLE, "copyinactive", "Copy Inactive Values")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "If enabled, merge the values of both active and inactive voxels.\n"
            "If disabled, merge the values of active voxels only, treating\n"
            "inactive voxels as active background voxels wherever\n"
            "corresponding input voxels have different active states."));

#ifndef SESI_OPENVDB
    // Verbosity toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "verbose", "Verbose")
        .setDocumentation("If enabled, print debugging information to the terminal."));
#endif

    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "scalar_x_group", "X Group")
        .setDefault("@name=*.x"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "scalar_y_group", "Y Group")
        .setDefault("@name=*.y"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "scalar_z_group", "Z Group")
        .setDefault("@name=*.z"));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Vector Merge",
        SOP_OpenVDB_Vector_Merge::factory, parms, *table)
        .addInput("Scalar VDBs to merge into vector")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Vector_Merge::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Merge three scalar VDB primitives into one vector VDB primitive.\"\"\"\n\
\n\
@overview\n\
\n\
This node will create a vector-valued VDB volume using the values of\n\
corresponding voxels from up to three scalar VDBs as the vector components.\n\
The scalar VDBs must have the same voxel size and transform; if they do not,\n\
use the [OpenVDB Resample node|Node:sop/DW_OpenVDBResample] to resample\n\
two of the VDBs to match the third.\n\
\n\
TIP:\n\
    To reverse the merge (i.e., to split a vector VDB into three scalar VDBs),\n\
    use the [OpenVDB Vector Split node|Node:sop/DW_OpenVDBVectorSplit].\n\
\n\
@related\n\
- [OpenVDB Vector Split|Node:sop/DW_OpenVDBVectorSplit]\n\
- [Node:sop/vdbvectormerge]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Vector_Merge::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    for (auto axis: { "x", "y", "z" }) {
        const std::string
            oldName = "scalar_" + (axis + std::string{"_group"}),
            newName = axis + std::string{"group"};
        resolveRenamedParm(*obsoleteParms, oldName.c_str(), newName.c_str());
    }

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


bool
SOP_OpenVDB_Vector_Merge::updateParmsFlags()
{
    bool changed = false;

#if HAVE_MERGE_GROUP
    changed |= enableParm("group", evalInt("enable_grouping", 0, 0) != 0);
#endif

    return changed;
}


OP_Node*
SOP_OpenVDB_Vector_Merge::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Vector_Merge(net, name, op);
}


SOP_OpenVDB_Vector_Merge::SOP_OpenVDB_Vector_Merge(OP_Network* net,
    const char* name, OP_Operator* op):
    SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

// Mapping from scalar ValueTypes to Vec3::value_types of registered vector-valued Grid types
template<typename T> struct VecValueTypeMap { using Type = T; static const bool Changed = false; };
//template<> struct VecValueTypeMap<bool> {
//    using Type = int32_t; static const bool Changed = true;
//};
//template<> struct VecValueTypeMap<uint32_t> {
//    using Type = int32_t; static const bool Changed = true;
//};
//template<> struct VecValueTypeMap<int64_t> {
//    using Type = int32_t; static const bool Changed = true;
//};
//template<> struct VecValueTypeMap<uint64_t> {
//    using Type = int32_t; static const bool Changed = true;
//};


struct InterruptException {};


// Functor to merge inactive tiles and voxels from up to three
// scalar-valued trees into one vector-valued tree
template<typename VectorTreeT, typename ScalarTreeT>
class MergeActiveOp
{
public:
    using VectorValueT = typename VectorTreeT::ValueType;
    using ScalarValueT = typename ScalarTreeT::ValueType;
    using ScalarAccessor = typename openvdb::tree::ValueAccessor<const ScalarTreeT>;

    MergeActiveOp(const ScalarTreeT* xTree, const ScalarTreeT* yTree, const ScalarTreeT* zTree,
        UT_Interrupt* interrupt)
        : mDummyTree(new ScalarTreeT)
        , mXAcc(xTree ? *xTree : *mDummyTree)
        , mYAcc(yTree ? *yTree : *mDummyTree)
        , mZAcc(zTree ? *zTree : *mDummyTree)
        , mInterrupt(interrupt)
    {
        /// @todo Consider initializing x, y and z dummy trees with background values
        /// taken from the vector tree.  Currently, though, missing components are always
        /// initialized to zero.

        // Note that copies of this op share the original dummy tree.
        // That's OK, since this op doesn't modify the dummy tree.
    }

    MergeActiveOp(const MergeActiveOp& other)
        : mDummyTree(other.mDummyTree)
        , mXAcc(other.mXAcc)
        , mYAcc(other.mYAcc)
        , mZAcc(other.mZAcc)
        , mInterrupt(other.mInterrupt)
    {
        if (mInterrupt && mInterrupt->opInterrupt()) { throw InterruptException(); }
    }

    // Given an active tile or voxel in the vector tree, set its value based on
    // the values of corresponding tiles or voxels in the scalar trees.
    void operator()(const typename VectorTreeT::ValueOnIter& it) const
    {
        const openvdb::Coord xyz = it.getCoord();
        ScalarValueT
            xval = mXAcc.getValue(xyz),
            yval = mYAcc.getValue(xyz),
            zval = mZAcc.getValue(xyz);
        it.setValue(VectorValueT(xval, yval, zval));
    }

private:
    UT_SharedPtr<const ScalarTreeT> mDummyTree;
    ScalarAccessor mXAcc, mYAcc, mZAcc;
    UT_Interrupt* mInterrupt;
}; // MergeActiveOp


// Functor to merge inactive tiles and voxels from up to three
// scalar-valued trees into one vector-valued tree
template<typename VectorTreeT, typename ScalarTreeT>
struct MergeInactiveOp
{
    using VectorValueT = typename VectorTreeT::ValueType;
    using VectorElemT = typename VectorValueT::value_type;
    using ScalarValueT = typename ScalarTreeT::ValueType;
    using VectorAccessor = typename openvdb::tree::ValueAccessor<VectorTreeT>;
    using ScalarAccessor = typename openvdb::tree::ValueAccessor<const ScalarTreeT>;

    MergeInactiveOp(const ScalarTreeT* xTree, const ScalarTreeT* yTree, const ScalarTreeT* zTree,
        VectorTreeT& vecTree, UT_Interrupt* interrupt)
        : mDummyTree(new ScalarTreeT)
        , mXAcc(xTree ? *xTree : *mDummyTree)
        , mYAcc(yTree ? *yTree : *mDummyTree)
        , mZAcc(zTree ? *zTree : *mDummyTree)
        , mVecAcc(vecTree)
        , mInterrupt(interrupt)
    {
        /// @todo Consider initializing x, y and z dummy trees with background values
        /// taken from the vector tree.  Currently, though, missing components are always
        /// initialized to zero.

        // Note that copies of this op share the original dummy tree.
        // That's OK, since this op doesn't modify the dummy tree.
    }

    MergeInactiveOp(const MergeInactiveOp& other)
        : mDummyTree(other.mDummyTree)
        , mXAcc(other.mXAcc)
        , mYAcc(other.mYAcc)
        , mZAcc(other.mZAcc)
        , mVecAcc(other.mVecAcc)
        , mInterrupt(other.mInterrupt)
    {
        if (mInterrupt && mInterrupt->opInterrupt()) { throw InterruptException(); }
    }

    // Given an inactive tile or voxel in a scalar tree, activate the corresponding
    // tile or voxel in the vector tree.
    void operator()(const typename ScalarTreeT::ValueOffCIter& it) const
    {
        // Skip background tiles and voxels, since the output vector tree
        // is assumed to be initialized with the correct background.
        if (openvdb::math::isExactlyEqual(it.getValue(), it.getTree()->background())) return;

        const openvdb::Coord& xyz = it.getCoord();
        if (it.isVoxelValue()) {
            mVecAcc.setActiveState(xyz, true);
        } else {
            // Because the vector tree was constructed with the same node configuration
            // as the scalar trees, tiles can be transferred directly between the two.
            mVecAcc.addTile(it.getLevel(), xyz,
                openvdb::zeroVal<VectorValueT>(), /*active=*/true);
        }
    }

    // Given an active tile or voxel in the vector tree, set its value based on
    // the values of corresponding tiles or voxels in the scalar trees.
    void operator()(const typename VectorTreeT::ValueOnIter& it) const
    {
        const openvdb::Coord xyz = it.getCoord();
        ScalarValueT
            xval = mXAcc.getValue(xyz),
            yval = mYAcc.getValue(xyz),
            zval = mZAcc.getValue(xyz);
        it.setValue(VectorValueT(xval, yval, zval));
    }

    // Deactivate all voxels in a leaf node of the vector tree.
    void operator()(const typename VectorTreeT::LeafIter& it) const
    {
        it->setValuesOff();
    }

private:
    UT_SharedPtr<const ScalarTreeT> mDummyTree;
    ScalarAccessor mXAcc, mYAcc, mZAcc;
    mutable VectorAccessor mVecAcc;
    UT_Interrupt* mInterrupt;
}; // MergeInactiveOp


////////////////////////////////////////


class ScalarGridMerger
{
public:
    using WarnFunc = std::function<void (const char*)>;

    ScalarGridMerger(
        const hvdb::Grid* x, const hvdb::Grid* y, const hvdb::Grid* z,
        const std::string& outGridName, bool copyInactiveValues,
        WarnFunc warn, UT_Interrupt* interrupt = nullptr):
        mOutGridName(outGridName),
        mCopyInactiveValues(copyInactiveValues),
        mWarn(warn),
        mInterrupt(interrupt)
    {
        mInGrid[0] = x; mInGrid[1] = y; mInGrid[2] = z;
    }

    const hvdb::GridPtr& getGrid() { return mOutGrid; }

    template<typename ScalarGridT>
    void operator()(const ScalarGridT& /*ignored*/)
    {
        if (!mInGrid[0] && !mInGrid[1] && !mInGrid[2]) return;

        using ScalarTreeT = typename ScalarGridT::TreeType;

        // Retrieve a scalar tree from each input grid.
        const ScalarTreeT* inTree[3] = { nullptr, nullptr, nullptr };
        if (mInGrid[0]) inTree[0] = &UTvdbGridCast<ScalarGridT>(mInGrid[0])->tree();
        if (mInGrid[1]) inTree[1] = &UTvdbGridCast<ScalarGridT>(mInGrid[1])->tree();
        if (mInGrid[2]) inTree[2] = &UTvdbGridCast<ScalarGridT>(mInGrid[2])->tree();
        if (!inTree[0] && !inTree[1] && !inTree[2]) return;

        // Get the type of the output vector tree.
        // 1. ScalarT is the input scalar tree's value type.
        using ScalarT = typename ScalarTreeT::ValueType;
        // 2. VecT is Vec3<ScalarT>, provided that there is a registered Tree with that
        //    value type.  If not, use the closest match (e.g., vec3i when ScalarT = bool).
        using MappedVecT = VecValueTypeMap<ScalarT>;
        using VecT = openvdb::math::Vec3<typename MappedVecT::Type>;
        // 3. VecTreeT is the type of a tree with the same height and node dimensions
        //    as the input scalar tree, but with value type VecT instead of ScalarT.
        using VecTreeT = typename ScalarTreeT::template ValueConverter<VecT>::Type;
        using VecGridT = typename openvdb::Grid<VecTreeT>;

        if (MappedVecT::Changed && mWarn) {
            std::ostringstream ostr;
            ostr << "grids of type vec3<" << openvdb::typeNameAsString<ScalarT>()
                << "> are not supported; using " << openvdb::typeNameAsString<VecT>()
                << " instead";
            if (!mOutGridName.empty()) ostr << " for " << mOutGridName;
            mWarn(ostr.str().c_str());
        }

        // Determine the background value and the transform.
        VecT bkgd(0, 0, 0);
        const openvdb::math::Transform* xform = nullptr;
        for (int i = 0; i < 3; ++i) {
            if (inTree[i]) bkgd[i] = inTree[i]->background();
            if (mInGrid[i] && !xform) xform = &(mInGrid[i]->transform());
        }
        openvdb::math::Transform::Ptr outXform;
        if (xform) outXform = xform->copy();

        // Construct the output vector grid, with a background value whose
        // components are the background values of the input scalar grids.
        typename VecGridT::Ptr vecGrid = VecGridT::create(bkgd);
        mOutGrid = vecGrid;

        if (outXform) {
            mOutGrid->setTransform(outXform);

            // Check that all three input grids have the same transform.
            bool xformMismatch = false;
            for (int i = 0; i < 3 && !xformMismatch; ++i) {
                if (mInGrid[i]) {
                    const openvdb::math::Transform* inXform = &(mInGrid[i]->transform());
                    if (*outXform != *inXform) xformMismatch = true;
                }
            }
            if (xformMismatch && mWarn) {
                mWarn("component grids have different transforms");
            }
        }

        if (mCopyInactiveValues) {
            try {
                MergeInactiveOp<VecTreeT, ScalarTreeT>
                    op(inTree[0], inTree[1], inTree[2], vecGrid->tree(), mInterrupt);

                // 1. For each non-background inactive value in each scalar tree,
                //    activate the corresponding region in the vector tree.
                for (int i = 0; i < 3; ++i) {
                    if (mInterrupt && mInterrupt->opInterrupt()) { mOutGrid.reset(); return; }
                    if (!inTree[i]) continue;
                    // Because this is a topology-modifying operation, it must be done serially.
                    openvdb::tools::foreach(inTree[i]->cbeginValueOff(),
                        op, /*threaded=*/false, /*shareOp=*/false);
                }

                // 2. For each active value in the vector tree, set v = (x, y, z).
                openvdb::tools::foreach(vecGrid->beginValueOn(),
                    op, /*threaded=*/true, /*shareOp=*/false);

                // 3. Deactivate all values in the vector tree, by processing
                //    leaf nodes in parallel (which is safe) and tiles serially.
                openvdb::tools::foreach(vecGrid->tree().beginLeaf(), op, /*threaded=*/true);
                typename VecTreeT::ValueOnIter tileIt = vecGrid->beginValueOn();
                tileIt.setMaxDepth(tileIt.getLeafDepth() - 1); // skip leaf nodes
                for (int count = 0; tileIt; ++tileIt, ++count) {
                    if (count % 100 == 0 && mInterrupt && mInterrupt->opInterrupt()) {
                        mOutGrid.reset();
                        return;
                    }
                    tileIt.setValueOff();
                }
            } catch (InterruptException&) {
                mOutGrid.reset();
                return;
            }
        }

        // Activate voxels (without setting their values) in the output vector grid so that
        // its tree topology is the union of the topologies of the input scalar grids.
        // Transferring voxel values from the scalar grids to the vector grid can then
        // be done safely from multiple threads.
        for (int i = 0; i < 3; ++i) {
            if (mInterrupt && mInterrupt->opInterrupt()) { mOutGrid.reset(); return; }
            if (!inTree[i]) continue;
            vecGrid->tree().topologyUnion(*inTree[i]);
        }

        // Set a new value for each active tile or voxel in the output vector grid.
        try {
            MergeActiveOp<VecTreeT, ScalarTreeT>
                op(inTree[0], inTree[1], inTree[2], mInterrupt);
            openvdb::tools::foreach(vecGrid->beginValueOn(),
                op, /*threaded=*/true, /*shareOp=*/false);
        } catch (InterruptException&) {
            mOutGrid.reset();
            return;
        }
        openvdb::tools::prune(vecGrid->tree());
    }

private:
    const hvdb::Grid* mInGrid[3];
    hvdb::GridPtr mOutGrid;
    std::string mOutGridName;
    bool mCopyInactiveValues;
    WarnFunc mWarn;
    UT_Interrupt* mInterrupt;
}; // class ScalarGridMerger

} // unnamed namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Vector_Merge::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        const bool copyInactiveValues = evalInt("copyinactive", 0, time);
        const bool removeSourceGrids = evalInt("remove_sources", 0, time);
#ifndef SESI_OPENVDB
        const bool verbose = evalInt("verbose", 0, time);
#else
        const bool verbose = false;
#endif

        openvdb::VecType vecType = openvdb::VEC_INVARIANT;
        {
            const int vtype = static_cast<int>(evalInt("vectype", 0, time));
            if (vtype >= 0 && vtype < openvdb::NUM_VEC_TYPES) {
                vecType = static_cast<openvdb::VecType>(vtype);
            }
        }

        // Get the name (or naming pattern) for merged grids.
        UT_String mergeName;
        evalString(mergeName, "merge_name", 0, time);
        const bool useXName = evalInt("usexname", 0, time);

#if HAVE_MERGE_GROUP
        // Get the group name for merged grids.
        UT_String mergeGroupStr;
        if (evalInt("enable_grouping", 0, time)) {
            evalString(mergeGroupStr, "group", 0, time);
        }
#endif

        UT_AutoInterrupt progress("Merging VDB grids");

        using PrimVDBSet = std::set<GEO_PrimVDB*>;
        PrimVDBSet primsToRemove;

        // Get the groups of x, y and z scalar grids to merge.
        const GA_PrimitiveGroup *xGroup = nullptr, *yGroup = nullptr, *zGroup = nullptr;
        {
            UT_String groupStr;
            evalString(groupStr, "xgroup", 0, time);
            xGroup = matchGroup(*gdp, groupStr.toStdString());
            evalString(groupStr, "ygroup", 0, time);
            yGroup = matchGroup(*gdp, groupStr.toStdString());
            evalString(groupStr, "zgroup", 0, time);
            zGroup = matchGroup(*gdp, groupStr.toStdString());
        }

        using PrimVDBVec = std::vector<GEO_PrimVDB*>;
        PrimVDBVec primsToGroup;

        // Iterate over VDB primitives in the selected groups.
        hvdb::VdbPrimIterator
            xIt(xGroup ? gdp : nullptr, xGroup),
            yIt(yGroup ? gdp : nullptr, yGroup),
            zIt(zGroup ? gdp : nullptr, zGroup);
        for (int i = 1; xIt || yIt || zIt; ++xIt, ++yIt, ++zIt, ++i) {
            if (progress.wasInterrupted()) return error();

            GU_PrimVDB *xVdb = *xIt, *yVdb = *yIt, *zVdb = *zIt, *nonNullVdb = nullptr;

            // Extract grids from the VDB primitives and find one that is non-null.
            // Process the primitives in ZYX order to ensure the X grid is preferred.
            /// @todo nonNullGrid's ValueType determines the ValueType of the
            /// output grid's vectors, so ideally nonNullGrid should be the
            /// grid with the highest-precision ValueType.
            const hvdb::Grid *xGrid = nullptr, *yGrid = nullptr, *zGrid = nullptr,
                *nonNullGrid = nullptr;
            if (zVdb) { zGrid = nonNullGrid = &zVdb->getGrid(); nonNullVdb = zVdb; }
            if (yVdb) { yGrid = nonNullGrid = &yVdb->getGrid(); nonNullVdb = yVdb; }
            if (xVdb) { xGrid = nonNullGrid = &xVdb->getGrid(); nonNullVdb = xVdb; }

            if (!nonNullGrid) continue;

            std::string outGridName;
            if (mergeName.isstring()) {
                UT_String s; s.itoa(i);
                outGridName = hboost::regex_replace(
                    mergeName.toStdString(), hboost::regex("#+"), s.toStdString());
            }

            if (useXName && nonNullVdb) {
                UT_String gridName(nonNullVdb->getGridName());
                UT_String basename = gridName.pathUpToExtension();
                if (basename.isstring()) {
                    outGridName = basename.toStdString();
                }
            }

            // Merge the input grids into an output grid.
            // This does not support a partial set so we quit early in that case.
            ScalarGridMerger op(xGrid, yGrid, zGrid, outGridName, copyInactiveValues,
                [this](const char* msg) { addWarning(SOP_MESSAGE, msg); });
            UTvdbProcessTypedGridScalar(UTvdbGetGridType(*nonNullGrid), *nonNullGrid, op);

            if (hvdb::GridPtr outGrid = op.getGrid()) {
                outGrid->setName(outGridName);
                outGrid->setVectorType(vecType);

                if (verbose) {
                    std::ostringstream ostr;
                    ostr << "Merged ("
                        << (xVdb ? xVdb->getGridName() : "0") << ", "
                        << (yVdb ? yVdb->getGridName() : "0") << ", "
                        << (zVdb ? zVdb->getGridName() : "0") << ")";
                    if (!outGridName.empty()) ostr << " into " << outGridName;
                    addMessage(SOP_MESSAGE, ostr.str().c_str());
                }

                if (GEO_PrimVDB* outVdb = GU_PrimVDB::buildFromGrid(*gdp, outGrid, nonNullVdb, outGridName.c_str())) {
                    primsToGroup.push_back(outVdb);
                }

                // Flag the input grids for removal.
                primsToRemove.insert(xVdb);
                primsToRemove.insert(yVdb);
                primsToRemove.insert(zVdb);
            }
        }

#if HAVE_MERGE_GROUP
        // Optionally, add the newly-created vector grids to a group.
        if (!primsToGroup.empty() && mergeGroupStr.isstring()) {
            GA_PrimitiveGroup* mergeGroup =
                gdp->findPrimitiveGroup(mergeGroupStr.buffer());
            if (mergeGroup == nullptr) {
                mergeGroup = gdp->newPrimitiveGroup(mergeGroupStr.buffer());
            }
            if (mergeGroup != nullptr) {
                for (PrimVDBVec::iterator i = primsToGroup.begin(), e = primsToGroup.end();
                    i != e; ++i)
                {
                    mergeGroup->add(*i);
                }
            }
        }
#endif

        if (removeSourceGrids) {
            // Remove scalar grids that were merged.
            primsToRemove.erase(nullptr);
            for (PrimVDBSet::iterator i = primsToRemove.begin(), e = primsToRemove.end();
                i != e; ++i)
            {
                gdp->destroyPrimitive(*(*i), /*andPoints=*/true);
            }
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
