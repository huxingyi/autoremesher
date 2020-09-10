// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Rebuild_Level_Set.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Rebuild level sets or fog volumes.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/LevelSetRebuild.h>

#include <UT/UT_Interrupt.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_SharedFunc.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <hboost/algorithm/string/join.hpp>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////


class SOP_OpenVDB_Rebuild_Level_Set: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Rebuild_Level_Set(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Rebuild_Level_Set() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;
};


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    //////////
    // Conversion settings

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDB grids to be processed\n"
            "(scalar, floating-point grids only)")
        .setDocumentation(
            "A subset of the scalar, floating-point input VDBs to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isovalue", "Isovalue")
        .setRange(PRM_RANGE_UI, -1, PRM_RANGE_UI, 1)
        .setTooltip("The isovalue that defines the implicit surface"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "worldunits", "Use World Space Units")
        .setTooltip("If enabled, specify the width of the narrow band in world units."));

    // Voxel unit narrow-band width {
    parms.add(hutil::ParmFactory(PRM_FLT_J, "exteriorBandWidth", "Exterior Band Voxels")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("Specify the width of the exterior (distance >= 0) portion of the narrow band. "
            "(3 voxel units is optimal for level set operations.)")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "interiorBandWidth", "Interior Band Voxels")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("Specify the width of the interior (distance < 0) portion of the narrow band. "
            "(3 voxel units is optimal for level set operations.)")
        .setDocumentation(nullptr));
    // }

    // World unit narrow-band width {
    parms.add(hutil::ParmFactory(PRM_FLT_J, "exteriorBandWidthWS", "Exterior Band")
        .setDefault(0.1)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 10)
        .setTooltip("Specify the width of the exterior (distance >= 0) portion of the narrow band.")
        .setDocumentation(
            "Specify the width of the exterior (_distance_ => 0) portion of the narrow band."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "interiorBandWidthWS",  "Interior Band")
        .setDefault(0.1)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 10)
        .setTooltip("Specify the width of the interior (distance < 0) portion of the narrow band.")
        .setDocumentation(
            "Specify the width of the interior (_distance_ < 0) portion of the narrow band."));
    // }

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fillinterior", "Fill Interior")
        .setTooltip(
            "If enabled, extract signed distances for all interior voxels.\n\n"
            "This operation densifies the interior of the surface and requires"
            " a closed, watertight surface."));

    //////////
    // Obsolete parameters

    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "halfbandwidth", "Half-Band Width")
        .setDefault(PRMthreeDefaults));


    //////////
    // Register this operator.

    hvdb::OpenVDBOpFactory("VDB Rebuild SDF",
        SOP_OpenVDB_Rebuild_Level_Set::factory, parms, *table)
        .setNativeName("")
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBRebuildLevelSet")
#endif
        .setObsoleteParms(obsoleteParms)
        .addInput("VDB grids to process")
        .setVerb(SOP_NodeVerb::COOK_INPLACE,
            []() { return new SOP_OpenVDB_Rebuild_Level_Set::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Repair level sets represented by VDB volumes.\"\"\"\n\
\n\
@overview\n\
\n\
Certain operations on a level set volume can cause the signed distances\n\
to its zero crossing to become invalid.\n\
This node restores proper distances by surfacing the level set with\n\
a polygon mesh and then converting the mesh back to a level set.\n\
As such, it can repair more badly damaged level sets than can the\n\
[OpenVDB Renormalize Level Set|Node:sop/DW_OpenVDBRenormalizeLevelSet] node.\n\
\n\
@related\n\
- [OpenVDB Offset Level Set|Node:sop/DW_OpenVDBOffsetLevelSet]\n\
- [OpenVDB Renormalize Level Set|Node:sop/DW_OpenVDBRenormalizeLevelSet]\n\
- [OpenVDB Smooth Level Set|Node:sop/DW_OpenVDBSmoothLevelSet]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Rebuild_Level_Set::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Rebuild_Level_Set(net, name, op);
}


SOP_OpenVDB_Rebuild_Level_Set::SOP_OpenVDB_Rebuild_Level_Set(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


void
SOP_OpenVDB_Rebuild_Level_Set::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;
    const fpreal time = CHgetEvalTime();

    PRM_Parm* parm = obsoleteParms->getParmPtr("halfbandwidth");

    if (parm && !parm->isFactoryDefault()) {
        const fpreal voxelWidth = obsoleteParms->evalFloat("halfbandwidth", 0, time);
        setFloat("exteriorBandWidth", 0, time, voxelWidth);
        setFloat("interiorBandWidth", 0, time, voxelWidth);
    }

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


////////////////////////////////////////


// Enable/disable or show/hide parameters in the UI.
bool
SOP_OpenVDB_Rebuild_Level_Set::updateParmsFlags()
{
    bool changed = false;

    const bool fillInterior = bool(evalInt("fillinterior", 0, 0));
    changed |= enableParm("interiorBandWidth", !fillInterior);
    changed |= enableParm("interiorBandWidthWS", !fillInterior);

    const bool worldUnits = bool(evalInt("worldunits", 0, 0));

    changed |= setVisibleState("interiorBandWidth", !worldUnits);
    changed |= setVisibleState("interiorBandWidthWS", worldUnits);

    changed |= setVisibleState("exteriorBandWidth", !worldUnits);
    changed |= setVisibleState("exteriorBandWidthWS", worldUnits);

    return changed;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Rebuild_Level_Set::Cache::cookVDBSop(
    OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = this->matchGroup(*gdp, evalStdString("group", time));

        // Get other UI parameters.

        const bool fillInterior = bool(evalInt("fillinterior", 0, time));
        const bool worldUnits = bool(evalInt("worldunits", 0, time));

        float exBandWidthVoxels = float(evalFloat("exteriorBandWidth", 0, time));
        float inBandWidthVoxels = fillInterior ? std::numeric_limits<float>::max() :
                                    float(evalFloat("interiorBandWidth", 0, time));

        float exBandWidthWorld = float(evalFloat("exteriorBandWidthWS", 0, time));
        float inBandWidthWorld = fillInterior ? std::numeric_limits<float>::max() :
                                    float(evalFloat("interiorBandWidthWS", 0, time));

        const float iso = float(evalFloat("isovalue", 0, time));

        hvdb::Interrupter boss("Rebuilding Level Set Grids");

        std::vector<std::string> skippedGrids;

        // Process each VDB primitive that belongs to the selected group.
        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

            if (boss.wasInterrupted()) break;

            GU_PrimVDB* vdbPrim = *it;

            float exWidth = exBandWidthVoxels, inWidth = inBandWidthVoxels;

            if (worldUnits) {
                const float voxelSize = float(vdbPrim->getGrid().voxelSize()[0]);

                exWidth = exBandWidthWorld / voxelSize;
                if (!fillInterior) inWidth = inBandWidthWorld / voxelSize;

                if (exWidth < 1.0f || inWidth < 1.0f) {
                    exWidth = std::max(exWidth, 1.0f);
                    inWidth = std::max(inWidth, 1.0f);
                    std::string s = it.getPrimitiveNameOrIndex().toStdString();
                    s += " - band width is smaller than one voxel.";
                    addWarning(SOP_MESSAGE, s.c_str());
                }
            }

            // Process floating point grids.

            if (vdbPrim->getStorageType() == UT_VDB_FLOAT) {

                openvdb::FloatGrid& grid = UTvdbGridCast<openvdb::FloatGrid>(vdbPrim->getGrid());

                vdbPrim->setGrid(*openvdb::tools::levelSetRebuild(
                    grid, iso, exWidth, inWidth, /*xform=*/nullptr, &boss));

                const GEO_VolumeOptions& visOps = vdbPrim->getVisOptions();
                vdbPrim->setVisualization(GEO_VOLUMEVIS_ISO, visOps.myIso, visOps.myDensity);

            } else if (vdbPrim->getStorageType() == UT_VDB_DOUBLE) {

                openvdb::DoubleGrid& grid = UTvdbGridCast<openvdb::DoubleGrid>(vdbPrim->getGrid());

                vdbPrim->setGrid(*openvdb::tools::levelSetRebuild(
                    grid, iso, exWidth, inWidth, /*xform=*/nullptr, &boss));

                const GEO_VolumeOptions& visOps = vdbPrim->getVisOptions();
                vdbPrim->setVisualization(GEO_VOLUMEVIS_ISO, visOps.myIso, visOps.myDensity);
            } else {
                skippedGrids.push_back(it.getPrimitiveNameOrIndex().toStdString());
            }
        }

        if (!skippedGrids.empty()) {
            std::string s = "The following non-floating-point grids were skipped: " +
                hboost::algorithm::join(skippedGrids, ", ") + ".";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (boss.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "Process was interrupted.");
        }

        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
