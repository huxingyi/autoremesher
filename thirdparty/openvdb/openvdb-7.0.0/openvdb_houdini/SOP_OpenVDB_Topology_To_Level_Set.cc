// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Topology_To_Level_Set.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GU_VDBPointTools.h>

#include <openvdb/tools/TopologyToLevelSet.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/points/PointDataGrid.h>

#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <GA/GA_Handle.h>
#include <GA/GA_Types.h>
#include <GA/GA_Iterator.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>

#include <stdexcept>
#include <string>



namespace cvdb = openvdb;
namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Topology_To_Level_Set: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Topology_To_Level_Set(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Topology_To_Level_Set() override {}

    bool updateParmsFlags() override;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
};


////////////////////////////////////////


namespace {

struct Converter
{
    float bandWidthWorld;
    int bandWidthVoxels, closingWidth, dilation, smoothingSteps;
    bool worldSpaceUnits;
    std::string outputName, customName;

    Converter(GU_Detail& geo, hvdb::Interrupter& boss)
        : bandWidthWorld(0)
        , bandWidthVoxels(3)
        , closingWidth(1)
        , dilation(0)
        , smoothingSteps(0)
        , worldSpaceUnits(false)
        , outputName("keep")
        , customName("vdb")
        , mGeoPt(&geo)
        , mBossPt(&boss)
    {
    }

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        int bandWidth = bandWidthVoxels;
        if (worldSpaceUnits) {
            bandWidth = int(openvdb::math::Round(bandWidthWorld / grid.transform().voxelSize()[0]));
        }

        openvdb::FloatGrid::Ptr sdfGrid = openvdb::tools::topologyToLevelSet(
           grid, bandWidth, closingWidth, dilation, smoothingSteps, mBossPt);

        std::string name = grid.getName();
        if (outputName == "append") name += customName;
        else if (outputName == "replace") name = customName;

        hvdb::createVdbPrimitive(*mGeoPt, sdfGrid, name.c_str());
    }

private:
    GU_Detail         * const mGeoPt;
    hvdb::Interrupter * const mBossPt;
}; // struct Converter

} // unnamed namespace


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenu)
        .setTooltip("Specify a subset of the input VDBs to be processed.")
        .setDocumentation(
            "A subset of the input VDB grids to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "outputname", "Output Name")
        .setDefault("keep")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "keep",     "Keep Original Name",
            "append",   "Add Suffix",
            "replace",  "Custom Name",
        })
        .setTooltip("Output VDB naming scheme")
        .setDocumentation(
            "Give the output VDB the same name as the input VDB,"
            " or add a suffix to the input name, or use a custom name."));

    parms.add(hutil::ParmFactory(PRM_STRING, "customname", "Custom Name")
        .setTooltip("The suffix or custom name to be used"));

    /// Narrow-band width {
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "worldspaceunits", "Use World Space for Band")
        .setDocumentation(
            "If enabled, specify the width of the narrow band in world units,"
            " otherwise specify it in voxels.  Voxel units work with all scales of geometry."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "bandwidth", "Half-Band in Voxels")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip(
            "Specify the half width of the narrow band in voxels."
            " Three voxels is optimal for many level set operations."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "bandwidthws", "Half-Band in World")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 10)
        .setTooltip("Specify the half width of the narrow band in world units."));

    /// }

    parms.add(hutil::ParmFactory(PRM_INT_J, "dilation", "Voxel Dilation")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip("Expand the filled voxel region by the specified number of voxels."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "closingwidth", "Closing Width")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip(
            "First expand the filled voxel region, then shrink it by the specified "
            "number of voxels. This causes holes and valleys to be filled."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "smoothingsteps", "Smoothing Steps")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip("Number of smoothing iterations"));


    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "outputName", "Output Name")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "keep",     "Keep Original Name",
            "append",   "Add Suffix",
            "replace",  "Custom Name",
        }));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "customName", "Custom Name"));
    obsoleteParms.add(
        hutil::ParmFactory(PRM_TOGGLE, "worldSpaceUnits", "Use World Space for Band"));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "bandWidth", "Half-Band in Voxels")
        .setDefault(PRMthreeDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "bandWidthWS", "Half-Band in World")
        .setDefault(PRMoneDefaults));


    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Topology to SDF",
        SOP_OpenVDB_Topology_To_Level_Set::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBTopologyToLevelSet")
#endif
        .addInput("VDB Grids")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_GENERATOR,
            []() { return new SOP_OpenVDB_Topology_To_Level_Set::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Create a level set VDB based on the active voxels of another VDB.\"\"\"\n\
\n\
@overview\n\
\n\
This node creates a narrow-band level set VDB that conforms to the active voxels\n\
of the input VDB.  This forms a shell or wrapper that can be used\n\
to conservatively enclose the input volume.\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Topology_To_Level_Set::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Topology_To_Level_Set(net, name, op);
}


SOP_OpenVDB_Topology_To_Level_Set::SOP_OpenVDB_Topology_To_Level_Set(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Topology_To_Level_Set::updateParmsFlags()
{
    bool changed = false;
    const fpreal time = 0;
    const bool wsUnits = bool(evalInt("worldspaceunits", 0, time));

    changed |= enableParm("bandwidth", !wsUnits);
    changed |= enableParm("bandwidthws", wsUnits);

    changed |= setVisibleState("bandwidth", !wsUnits);
    changed |= setVisibleState("bandwidthws", wsUnits);

    const auto outputName = evalStdString("outputname", time);
    changed |= enableParm("customname", (outputName != "keep"));

    return changed;
}


void
SOP_OpenVDB_Topology_To_Level_Set::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    const fpreal time = 0.0;

    if (PRM_Parm* parm = obsoleteParms->getParmPtr("outputName")) {
        if (!parm->isFactoryDefault()) {
            std::string val{"keep"};
            switch (obsoleteParms->evalInt("outputName", 0, time)) {
                case 0: val = "keep"; break;
                case 1: val = "append"; break;
                case 2: val = "replace"; break;
            }
            setString(val.c_str(), CH_STRING_LITERAL, "outputname", 0, time);
        }
    }

    resolveRenamedParm(*obsoleteParms, "customName", "customname");
    resolveRenamedParm(*obsoleteParms, "worldSpaceUnits", "worldspaceunits");
    resolveRenamedParm(*obsoleteParms, "bandWidth", "bandwidth");
    resolveRenamedParm(*obsoleteParms, "bandWidthWS", "bandwidthws");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Topology_To_Level_Set::Cache::cookVDBSop(
    OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        const GU_Detail* inputGeoPt = inputGeo(0);
        if (inputGeoPt == nullptr) return error();

        hvdb::Interrupter boss;

        // Get UI settings

        Converter converter(*gdp, boss);
        converter.worldSpaceUnits = evalInt("worldspaceunits", 0, time) != 0;
        converter.bandWidthWorld = float(evalFloat("bandwidthws", 0, time));
        converter.bandWidthVoxels = static_cast<int>(evalInt("bandwidth", 0, time));
        converter.closingWidth = static_cast<int>(evalInt("closingwidth", 0, time));
        converter.dilation = static_cast<int>(evalInt("dilation", 0, time));
        converter.smoothingSteps = static_cast<int>(evalInt("smoothingsteps", 0, time));
        converter.outputName = evalStdString("outputname", time);
        converter.customName = evalStdString("customname", time);

        // Process VDB primitives

        const GA_PrimitiveGroup* group = matchGroup(*inputGeoPt, evalStdString("group", time));

        hvdb::VdbPrimCIterator vdbIt(inputGeoPt, group);

        if (!vdbIt) {
            addWarning(SOP_MESSAGE, "No VDB grids to process.");
            return error();
        }

        for (; vdbIt; ++vdbIt) {

            if (boss.wasInterrupted()) break;

            const GU_PrimVDB *vdb = *vdbIt;

            if (!GEOvdbProcessTypedGridTopology(*vdb, converter)) {
                if (!GEOvdbProcessTypedGridPoint(*vdb, converter)) {
                    if (vdb->getGrid().isType<cvdb::MaskGrid>()) {
                        cvdb::MaskGrid::ConstPtr grid =
                            cvdb::gridConstPtrCast<cvdb::MaskGrid>(vdb->getGridPtr());
                        converter(*grid);
                    }
                }
            }
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
