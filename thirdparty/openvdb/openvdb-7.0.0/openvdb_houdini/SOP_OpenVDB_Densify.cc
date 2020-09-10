// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Densify.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief SOP to replace active tiles with active voxels in OpenVDB grids

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <stdexcept>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Densify: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Densify(OP_Network*, const char* name, OP_Operator*);

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };
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
        .setTooltip("Specify a subset of the input VDBs to be densified.")
        .setDocumentation(
            "A subset of the input VDBs to be densified"
            " (see [specifying volumes|/model/volumes#group])"));

    hvdb::OpenVDBOpFactory("VDB Densify", SOP_OpenVDB_Densify::factory, parms, *table)
        .setNativeName("")
        .addInput("VDBs to densify")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Densify::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Densify sparse VDB volumes.\"\"\"\n\
\n\
@overview\n\
\n\
This node replaces active\n\
[tiles|http://www.openvdb.org/documentation/doxygen/overview.html#secSparsity]\n\
in VDB [trees|http://www.openvdb.org/documentation/doxygen/overview.html#secTree]\n\
with dense, leaf-level voxels.\n\
This is useful for subsequent processing with nodes like [Node:sop/volumevop]\n\
that operate only on leaf voxels.\n\
\n\
WARNING:\n\
    Densifying a sparse VDB can significantly increase its memory footprint.\n\
\n\
@related\n\
- [OpenVDB Fill|Node:sop/DW_OpenVDBFill]\n\
- [OpenVDB Prune|Node:sop/DW_OpenVDBPrune]\n\
- [Node:sop/vdbactivate]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Densify::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Densify(net, name, op);
}


SOP_OpenVDB_Densify::SOP_OpenVDB_Densify(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {
struct DensifyOp {
    DensifyOp() {}

    template<typename GridT>
    void operator()(GridT& grid) const
    {
        grid.tree().voxelizeActiveTiles(/*threaded=*/true);
    }
};
}


OP_ERROR
SOP_OpenVDB_Densify::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = this->matchGroup(*gdp, evalStdString("group", time));

        // Construct a functor to process grids of arbitrary type.
        const DensifyOp densifyOp;

        UT_AutoInterrupt progress("Densifying VDBs");

        // Process each VDB primitive that belongs to the selected group.
        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {
            if (progress.wasInterrupted()) {
                throw std::runtime_error("processing was interrupted");
            }

            GU_PrimVDB* vdbPrim = *it;
            GEOvdbProcessTypedGridTopology(*vdbPrim, densifyOp);
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
