// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Prune.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief SOP to prune tree branches from OpenVDB grids

#include <houdini_utils/ParmFactory.h>
#include <openvdb/tools/Prune.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <stdexcept>
#include <string>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Prune: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Prune(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Prune() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDBs to be pruned.")
        .setDocumentation(
            "A subset of the input VDBs to be pruned"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "mode", "Mode")
        .setDefault("value")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "value",    "Value",
            "inactive", "Inactive",
            "levelset", "Level Set"
        })
        .setTooltip(
            "Value:\n"
            "    Collapse regions in which all voxels have the same\n"
            "    value and active state into tiles with those values\n"
            "    and active states.\n"
            "Inactive:\n"
            "    Collapse regions in which all voxels are inactive\n"
            "    into inactive background tiles.\n"
            "Level Set:\n"
            "    Collapse regions in which all voxels are inactive\n"
            "    into inactive tiles with either the inside or\n"
            "    the outside background value, depending on\n"
            "    the signs of the voxel values.\n"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "tolerance", "Tolerance")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1)
        .setTooltip(
            "Voxel values are considered equal if they differ\n"
            "by less than the specified threshold."));

    hvdb::OpenVDBOpFactory("VDB Prune", SOP_OpenVDB_Prune::factory, parms, *table)
        .setNativeName("")
        .addInput("Grids to process")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Prune::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Reduce the memory footprint of VDB volumes.\"\"\"\n\
\n\
@overview\n\
\n\
This node prunes branches of VDB\n\
[trees|http://www.openvdb.org/documentation/doxygen/overview.html#secTree]\n\
where all voxels have the same or similar values.\n\
This can help to reduce the memory footprint of a VDB, without changing its topology.\n\
With a suitably high tolerance, pruning can function as a simple\n\
form of lossy compression.\n\
\n\
@related\n\
- [OpenVDB Densify|Node:sop/DW_OpenVDBDensify]\n\
- [Node:sop/vdbactivate]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Prune::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Prune(net, name, op);
}


SOP_OpenVDB_Prune::SOP_OpenVDB_Prune(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


// Enable/disable or show/hide parameters in the UI.
bool
SOP_OpenVDB_Prune::updateParmsFlags()
{
    bool changed = false;

    changed |= enableParm("tolerance", evalStdString("mode", 0) == "value");

    return changed;
}


////////////////////////////////////////


namespace {
struct PruneOp {
    PruneOp(const std::string m, fpreal tol = 0.0): mode(m), pruneTolerance(tol) {}

    template<typename GridT>
    void operator()(GridT& grid) const
    {
        using ValueT = typename GridT::ValueType;

        if (mode == "value") {
            OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
            const ValueT tolerance(openvdb::zeroVal<ValueT>() + pruneTolerance);
            OPENVDB_NO_TYPE_CONVERSION_WARNING_END
            openvdb::tools::prune(grid.tree(), tolerance);
        } else if (mode == "inactive") {
            openvdb::tools::pruneInactive(grid.tree());
        } else if (mode == "levelset") {
            openvdb::tools::pruneLevelSet(grid.tree());
        }
    }

    std::string mode;
    fpreal pruneTolerance;
};
}


OP_ERROR
SOP_OpenVDB_Prune::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = this->matchGroup(*gdp, evalStdString("group", time));

        // Get other UI parameters.
        const fpreal tolerance = evalFloat("tolerance", 0, time);

        // Construct a functor to process grids of arbitrary type.
        const PruneOp pruneOp(evalStdString("mode", time), tolerance);

        UT_AutoInterrupt progress("Pruning OpenVDB grids");

        // Process each VDB primitive that belongs to the selected group.
        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {
            if (progress.wasInterrupted()) {
                throw std::runtime_error("processing was interrupted");
            }

            GU_PrimVDB* vdbPrim = *it;
            GEOvdbProcessTypedGridTopology(*vdbPrim, pruneOp);
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
