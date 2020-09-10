// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SOP_OpenVDB_Points_Delete.cc
///
/// @author Francisco Gochez, Dan Bailey
///
/// @brief Delete points that are members of specific groups

#include <openvdb/openvdb.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/points/PointDelete.h>

#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/PointUtils.h>
#include <openvdb_houdini/Utils.h>
#include <houdini_utils/geometry.h>
#include <houdini_utils/ParmFactory.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>



using namespace openvdb;
using namespace openvdb::points;
using namespace openvdb::math;

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////


class SOP_OpenVDB_Points_Delete: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Points_Delete(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Points_Delete() override = default;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
}; // class SOP_OpenVDB_Points_Delete


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    openvdb::initialize();

    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setHelpText("Specify a subset of the input point data grids to delete from.")
        .setChoiceList(&hutil::PrimGroupMenu));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbpointsgroups", "VDB Points Groups")
        .setHelpText("Specify VDB points groups to delete.")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "invert", "Invert")
        .setDefault(PRMzeroDefaults)
        .setHelpText("Invert point deletion so that points not belonging to any of the "
            "groups will be deleted."));
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "dropgroups", "Drop Points Groups")
        .setDefault(PRMoneDefaults)
        .setHelpText("Drop the VDB points groups that were used for deletion. This option is "
            "ignored if \"invert\" is enabled."));

    //////////
    // Register this operator.

    hvdb::OpenVDBOpFactory("VDB Points Delete",
        SOP_OpenVDB_Points_Delete::factory, parms, *table)
#if UT_VERSION_INT < 0x11050000 // earlier than 17.5.0
        .setNativeName("")
#endif
        .addInput("VDB Points")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Points_Delete::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Delete points that are members of specific groups.\"\"\"\n\
\n\
@overview\n\
\n\
The OpenVDB Points Delete SOP allows deletion of points that are members\n\
of a supplied group(s).\n\
An invert toggle may be enabled to allow deleting points that are not\n\
members of the supplied group(s).\n\
\n\
@related\n\
- [OpenVDB Points Convert|Node:sop/DW_OpenVDBPointsConvert]\n\
- [OpenVDB Points Group|Node:sop/DW_OpenVDBPointsGroup]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}

bool
SOP_OpenVDB_Points_Delete::updateParmsFlags()
{
    const bool invert = evalInt("invert", 0, 0) != 0;

    return enableParm("dropgroups", !invert);
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Points_Delete::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Points_Delete(net, name, op);
}


SOP_OpenVDB_Points_Delete::SOP_OpenVDB_Points_Delete(OP_Network* net,
    const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Points_Delete::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const std::string groups = evalStdString("vdbpointsgroups", context.getTime());

        // early exit if the VDB points group field is empty
        if (groups.empty()) return error();

        UT_AutoInterrupt progress("Processing points group deletion");

        const bool invert = evalInt("invert", 0, context.getTime());
        const bool drop = evalInt("dropgroups", 0, context.getTime());

        // select Houdini primitive groups we wish to use
        hvdb::VdbPrimIterator vdbIt(gdp,
            matchGroup(*gdp, evalStdString("group", context.getTime())));

        for (; vdbIt; ++vdbIt) {
            if (progress.wasInterrupted()) {
                throw std::runtime_error("processing was interrupted");
            }
            GU_PrimVDB* vdbPrim = *vdbIt;

            // Limit the lifetime of our const shared copies so
            // we don't have false-sharing when we go to make the
            // grid unique.
            std::vector<std::string> pointGroups;

            {
                PointDataGrid::ConstPtr inputGrid =
                        openvdb::gridConstPtrCast<PointDataGrid>(vdbPrim->getConstGridPtr());

                // early exit if the grid is of the wrong type
                if (!inputGrid) continue;

                // early exit if the tree is empty
                auto leafIter = inputGrid->tree().cbeginLeaf();
                if (!leafIter) continue;

                // extract names of all selected VDB groups


                // the "exclude groups" parameter to parseNames is not used in this context,
                // so we disregard it by storing it in a temporary variable

                std::vector<std::string> tmp;

                AttributeSet::Descriptor::parseNames(pointGroups, tmp, groups);

                // determine in any of the requested groups are actually present in the tree

                const AttributeSet::Descriptor& descriptor = leafIter->attributeSet().descriptor();
                const bool hasPointsToDrop = std::any_of(pointGroups.begin(), pointGroups.end(),
                    [&descriptor](const std::string& group) { return descriptor.hasGroup(group); });

                if (!hasPointsToDrop) continue;
            }

            // deep copy the VDB tree if it is not already unique
            vdbPrim->makeGridUnique();

            PointDataGrid& outputGrid = UTvdbGridCast<PointDataGrid>(vdbPrim->getGrid());
            deleteFromGroups(outputGrid.tree(), pointGroups, invert, drop);
        }

    } catch (const std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
