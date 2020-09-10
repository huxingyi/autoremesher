// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Morph_Level_Set.cc
///
/// @author Ken Museth
///
/// @brief Level set morphing SOP

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/LevelSetMorph.h>

#include <UT/UT_Version.h>

#include <hboost/algorithm/string/join.hpp>

#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////

// Utilities

namespace {

struct MorphingParms {
    MorphingParms()
        : mLSGroup(nullptr)
        , mAdvectSpatial(openvdb::math::UNKNOWN_BIAS)
        , mRenormSpatial(openvdb::math::UNKNOWN_BIAS)
        , mAdvectTemporal(openvdb::math::UNKNOWN_TIS)
        , mRenormTemporal(openvdb::math::UNKNOWN_TIS)
        , mNormCount(1)
        , mTimeStep(0.0)
        , mMinMask(0)
        , mMaxMask(1)
        , mInvertMask(false)
    {
    }

    const GA_PrimitiveGroup*  mLSGroup;
    openvdb::FloatGrid::ConstPtr mTargetGrid;
    openvdb::FloatGrid::ConstPtr mMaskGrid;
    openvdb::math::BiasedGradientScheme mAdvectSpatial, mRenormSpatial;
    openvdb::math::TemporalIntegrationScheme mAdvectTemporal, mRenormTemporal;
    int mNormCount;
    float mTimeStep;
    float mMinMask, mMaxMask;
    bool  mInvertMask;
};

class MorphOp
{
public:
    MorphOp(MorphingParms& parms, hvdb::Interrupter& boss)
        : mParms(&parms)
        , mBoss(&boss)
    {
    }

    void operator()(openvdb::FloatGrid& grid)
    {
        if (mBoss->wasInterrupted()) return;

        openvdb::tools::LevelSetMorphing<openvdb::FloatGrid, hvdb::Interrupter>
            morph(grid, *(mParms->mTargetGrid), mBoss);

        if (mParms->mMaskGrid) {
            morph.setAlphaMask(*(mParms->mMaskGrid));
            morph.setMaskRange(mParms->mMinMask, mParms->mMaxMask);
            morph.invertMask(mParms->mInvertMask);
        }
        morph.setSpatialScheme(mParms->mAdvectSpatial);
        morph.setTemporalScheme(mParms->mAdvectTemporal);
        morph.setTrackerSpatialScheme(mParms->mRenormSpatial);
        morph.setTrackerTemporalScheme(mParms->mRenormTemporal);
        morph.setNormCount(mParms->mNormCount);
        morph.advect(0, mParms->mTimeStep);
    }

private:
    MorphingParms*     mParms;
    hvdb::Interrupter* mBoss;
};

} // namespace


////////////////////////////////////////

// SOP Declaration

class SOP_OpenVDB_Morph_Level_Set: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Morph_Level_Set(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Morph_Level_Set() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i ) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions
    {
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
        OP_ERROR evalMorphingParms(OP_Context&, MorphingParms&);
        bool processGrids(MorphingParms&, hvdb::Interrupter&);
    };

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
    bool updateParmsFlags() override;
};


////////////////////////////////////////


// Build UI and register this operator

void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    using namespace openvdb::math;

    hutil::ParmList parms;

    // Level set grid
    parms.add(hutil::ParmFactory(PRM_STRING, "sourcegroup", "Source")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setDocumentation(
            "A subset of the input level set VDBs to be morphed"
            " (see [specifying volumes|/model/volumes#group])"));

    // Target grid
    parms.add(hutil::ParmFactory(PRM_STRING, "targetgroup", "Target")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setDocumentation(
            "The target level set VDB (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "mask", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Enable/disable the mask.")
        .setDocumentation(nullptr));

    // Alpha grid
    parms.add(hutil::ParmFactory(PRM_STRING, "maskname", "Alpha Mask")
        .setChoiceList(&hutil::PrimGroupMenuInput3)
        .setTooltip(
            "An optional scalar VDB to be used for alpha masking"
            " (see [specifying volumes|/model/volumes#group])\n\n"
            "Voxel values are assumed to be between 0 and 1."));

    parms.add(hutil::ParmFactory(PRM_HEADING, "morphingheading", "Morphing")
        .setDocumentation(
            "These parameters control how the SDF moves from the source to the target."));

    // Advect: timestep
    parms.add(hutil::ParmFactory(PRM_FLT, "timestep", "Timestep")
        .setDefault(1, "1.0/$FPS")
        .setDocumentation(
            "The number of seconds of movement to apply to the input points\n\n"
            "The default is `1/$FPS` (one frame's worth of time).\n\n"
            "TIP:\n"
            "    This parameter can be animated through time using the `$T`\n"
            "    expression. To control how fast the morphing is done, multiply `$T`\n"
            "    by a scale factor. For example, to animate it twice as fast, use\n"
            "    the expression, `$T*2`.\n"));

    // Advect: spatial menu
    {
        std::vector<std::string> items;
        items.push_back(biasedGradientSchemeToString(FIRST_BIAS));
        items.push_back(biasedGradientSchemeToMenuName(FIRST_BIAS));

        items.push_back(biasedGradientSchemeToString(HJWENO5_BIAS));
        items.push_back(biasedGradientSchemeToMenuName(HJWENO5_BIAS));


        parms.add(hutil::ParmFactory(PRM_STRING, "advectspatial", "Spatial Scheme")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS))
            .setTooltip("Set the spatial finite difference scheme.")
            .setDocumentation(
                "How accurately the gradients of the signed distance field\n"
                "are computed during advection\n\n"
                "The later choices are more accurate but take more time."));
    }

    // Advect: temporal menu
    {
        std::vector<std::string> items;
        for (int i = 0; i < NUM_TEMPORAL_SCHEMES; ++i) {
            TemporalIntegrationScheme it = TemporalIntegrationScheme(i);
            items.push_back(temporalIntegrationSchemeToString(it)); // token
            items.push_back(temporalIntegrationSchemeToMenuName(it)); // label
        }

        parms.add(hutil::ParmFactory(PRM_STRING, "advecttemporal", "Temporal Scheme")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(temporalIntegrationSchemeToString(TVD_RK2))
            .setTooltip("Set the temporal integration scheme.")
            .setDocumentation(
                "How accurately time is evolved within each advection step\n\n"
                "The later choices are more accurate but take more time."));
    }

    parms.add(hutil::ParmFactory(PRM_HEADING, "renormheading", "Renormalization")
        .setDocumentation(
            "After morphing the signed distance field, it will often no longer"
            " contain valid distances.  A number of renormalization passes can be"
            " performed to convert it back into a proper signed distance field."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "normsteps", "Steps")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("The number of times to renormalize between each substep"));

    // Renorm: spatial menu
    {
        std::vector<std::string> items;
        items.push_back(biasedGradientSchemeToString(FIRST_BIAS));
        items.push_back(biasedGradientSchemeToMenuName(FIRST_BIAS));

        items.push_back(biasedGradientSchemeToString(HJWENO5_BIAS));
        items.push_back(biasedGradientSchemeToMenuName(HJWENO5_BIAS));

        parms.add(hutil::ParmFactory(PRM_STRING, "renormspatial", "Spatial Scheme")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS))
            .setTooltip("Set the spatial finite difference scheme.")
            .setDocumentation(
                "How accurately the gradients of the signed distance field\n"
                "are computed during renormalization\n\n"
                "The later choices are more accurate but take more time."));
    }

    // Renorm: temporal menu
    {
        std::vector<std::string> items;
        for (int i = 0; i < NUM_TEMPORAL_SCHEMES; ++i) {
            TemporalIntegrationScheme it = TemporalIntegrationScheme(i);
            items.push_back(temporalIntegrationSchemeToString(it)); // token
            items.push_back(temporalIntegrationSchemeToMenuName(it)); // label
        }

        parms.add(hutil::ParmFactory(PRM_STRING, "renormtemporal", "Temporal Scheme")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(temporalIntegrationSchemeToString(TVD_RK2))
            .setTooltip("Set the temporal integration scheme.")
            .setDocumentation(
                "How accurately time is evolved during renormalization\n\n"
                "The later choices are more accurate but take more time."));
    }

    parms.add(hutil::ParmFactory(PRM_HEADING, "maskheading", "Alpha Mask"));

    //Invert mask.
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "invert", "Invert Alpha Mask")
        .setTooltip("Invert the optional mask so that alpha value 0 maps to 1 and 1 maps to 0."));

    // Min mask range
    parms.add(hutil::ParmFactory(PRM_FLT_J, "minmask", "Min Mask Cutoff")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, 0.0, PRM_RANGE_UI, 1.0)
        .setTooltip("Threshold below which to clamp mask values to zero"));

    // Max mask range
    parms.add(hutil::ParmFactory(PRM_FLT_J, "maxmask", "Max Mask Cutoff")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_UI, 0.0, PRM_RANGE_UI, 1.0)
        .setTooltip("Threshold above which to clamp mask values to one"));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT, "beginTime", "Begin time"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT, "endTime", "Time step"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "lsGroup", "Source Level Set"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "targetGroup", "Target"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "maskGroup", "Alpha Mask"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "morphingHeading", "Morphing"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "advectSpatial", "Spatial Scheme")
        .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS)));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "advectTemporal", "Temporal Scheme")
        .setDefault(temporalIntegrationSchemeToString(TVD_RK2)));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "renormHeading", "Renormalization"));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "normSteps", "Steps")
        .setDefault(PRMthreeDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "renormSpatial", "Spatial Scheme")
        .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS)));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "renormTemporal", "Temporal Scheme")
        .setDefault(temporalIntegrationSchemeToString(TVD_RK2)));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "maskHeading", "Alpha Mask"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "minMask", "Min Mask Cutoff")
        .setDefault(PRMzeroDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "maxMask", "Max Mask Cutoff")
        .setDefault(PRMoneDefaults));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Morph SDF",
        SOP_OpenVDB_Morph_Level_Set::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBMorphLevelSet")
#endif
        .setObsoleteParms(obsoleteParms)
        .addInput("Source SDF VDBs to Morph")
        .addInput("Target SDF VDB")
        .addOptionalInput("Optional VDB Alpha Mask")
        .setVerb(SOP_NodeVerb::COOK_INPLACE,
            []() { return new SOP_OpenVDB_Morph_Level_Set::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Blend between source and target level set VDBs.\"\"\"\n\
\n\
@overview\n\
\n\
This node advects a source narrow-band signed distance field\n\
towards a target narrow-band signed distance field.\n\
\n\
@related\n\
- [OpenVDB Advect|Node:sop/DW_OpenVDBAdvect]\n\
- [OpenVDB Advect Points|Node:sop/DW_OpenVDBAdvectPoints]\n\
- [Node:sop/vdbmorphsdf]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Morph_Level_Set::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "advectSpatial", "advectspatial");
    resolveRenamedParm(*obsoleteParms, "advectTemporal", "advecttemporal");
    resolveRenamedParm(*obsoleteParms, "lsGroup", "sourcegroup");
    resolveRenamedParm(*obsoleteParms, "maskGroup", "maskname");
    resolveRenamedParm(*obsoleteParms, "maxMask", "maxmask");
    resolveRenamedParm(*obsoleteParms, "minMask", "minmask");
    resolveRenamedParm(*obsoleteParms, "normSteps", "normsteps");
    resolveRenamedParm(*obsoleteParms, "renormSpatial", "renormspatial");
    resolveRenamedParm(*obsoleteParms, "renormTemporal", "renormtemporal");
    resolveRenamedParm(*obsoleteParms, "targetGroup", "targetgroup");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Enable/disable or show/hide parameters in the UI.
bool
SOP_OpenVDB_Morph_Level_Set::updateParmsFlags()
{
    bool changed = false;

    const bool hasMask = (this->nInputs() == 3);
    changed |= enableParm("mask", hasMask);
    const bool useMask = hasMask && bool(evalInt("mask", 0, 0));
    changed |= enableParm("invert", useMask);
    changed |= enableParm("minmask", useMask);
    changed |= enableParm("maxmask", useMask);
    changed |= enableParm("maskname", useMask);

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Morph_Level_Set::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Morph_Level_Set(net, name, op);
}


SOP_OpenVDB_Morph_Level_Set::SOP_OpenVDB_Morph_Level_Set(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Morph_Level_Set::Cache::cookVDBSop(OP_Context& context)
{
    try {
        // Evaluate UI parameters
        MorphingParms parms;
        if (evalMorphingParms(context, parms) >= UT_ERROR_ABORT) return error();

        hvdb::Interrupter boss("Morphing level set");

        processGrids(parms, boss);

        if (boss.wasInterrupted()) addWarning(SOP_MESSAGE, "Process was interrupted");
        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Morph_Level_Set::Cache::evalMorphingParms(
    OP_Context& context, MorphingParms& parms)
{
    const fpreal now = context.getTime();

    parms.mLSGroup = matchGroup(*gdp, evalStdString("sourcegroup", now));

    parms.mTimeStep = static_cast<float>(evalFloat("timestep", 0, now));

    parms.mAdvectSpatial =
        openvdb::math::stringToBiasedGradientScheme(evalStdString("advectspatial", now));
    if (parms.mAdvectSpatial == openvdb::math::UNKNOWN_BIAS) {
        addError(SOP_MESSAGE, "Morph: Unknown biased gradient");
        return UT_ERROR_ABORT;
    }

    parms.mRenormSpatial =
        openvdb::math::stringToBiasedGradientScheme(evalStdString("renormspatial", now));
    if (parms.mRenormSpatial == openvdb::math::UNKNOWN_BIAS) {
        addError(SOP_MESSAGE, "Renorm: Unknown biased gradient");
        return UT_ERROR_ABORT;
    }

    parms.mAdvectTemporal =
        openvdb::math::stringToTemporalIntegrationScheme(evalStdString("advecttemporal", now));
    if (parms.mAdvectTemporal == openvdb::math::UNKNOWN_TIS) {
        addError(SOP_MESSAGE, "Morph: Unknown temporal integration");
        return UT_ERROR_ABORT;
    }

    parms.mRenormTemporal =
        openvdb::math::stringToTemporalIntegrationScheme(evalStdString("renormtemporal", now));
    if (parms.mRenormTemporal == openvdb::math::UNKNOWN_TIS) {
        addError(SOP_MESSAGE, "Renorm: Unknown temporal integration");
        return UT_ERROR_ABORT;
    }

    parms.mNormCount = static_cast<int>(evalInt("normsteps", 0, now));

    const GU_Detail* targetGeo = inputGeo(1);

    if (!targetGeo) {
        addError(SOP_MESSAGE, "Missing target grid input");
        return UT_ERROR_ABORT;
    }

    const GA_PrimitiveGroup* targetGroup =
        matchGroup(*targetGeo, evalStdString("targetgroup", now));

    hvdb::VdbPrimCIterator it(targetGeo, targetGroup);
    if (it) {
        if (it->getStorageType() != UT_VDB_FLOAT) {
            addError(SOP_MESSAGE, "Unrecognized target grid type.");
            return UT_ERROR_ABORT;
        }
        parms.mTargetGrid = hvdb::Grid::constGrid<openvdb::FloatGrid>(it->getConstGridPtr());
    }

    if (!parms.mTargetGrid) {
        addError(SOP_MESSAGE, "Missing target grid");
        return UT_ERROR_ABORT;
    }

    const GU_Detail* maskGeo = evalInt("mask", 0, now) ? inputGeo(2) : nullptr;

    if (maskGeo) {
        const GA_PrimitiveGroup* maskGroup =
            matchGroup(*maskGeo, evalStdString("maskname", now));

        hvdb::VdbPrimCIterator maskIt(maskGeo, maskGroup);
        if (maskIt) {
            if (maskIt->getStorageType() != UT_VDB_FLOAT) {
                addError(SOP_MESSAGE, "Unrecognized alpha mask grid type.");
                return UT_ERROR_ABORT;
            }
            parms.mMaskGrid = hvdb::Grid::constGrid<openvdb::FloatGrid>(maskIt->getConstGridPtr());
        }

        if (!parms.mMaskGrid) {
            addError(SOP_MESSAGE, "Missing alpha mask grid");
            return UT_ERROR_ABORT;
        }
    }

    parms.mMinMask      = static_cast<float>(evalFloat("minmask", 0, now));
    parms.mMaxMask      = static_cast<float>(evalFloat("maxmask", 0, now));
    parms.mInvertMask   = evalInt("invert", 0, now);

    return error();
}


////////////////////////////////////////


bool
SOP_OpenVDB_Morph_Level_Set::Cache::processGrids(
    MorphingParms& parms, hvdb::Interrupter& boss)
{
    MorphOp op(parms, boss);

    std::vector<std::string> skippedGrids, nonLevelSetGrids, narrowBands;

    for (hvdb::VdbPrimIterator it(gdp, parms.mLSGroup); it; ++it) {

        if (boss.wasInterrupted()) break;

        GU_PrimVDB* vdbPrim = *it;

        const openvdb::GridClass gridClass = vdbPrim->getGrid().getGridClass();
        if (gridClass != openvdb::GRID_LEVEL_SET) {
            nonLevelSetGrids.push_back(it.getPrimitiveNameOrIndex().toStdString());
            continue;
        }

        if (vdbPrim->getStorageType() == UT_VDB_FLOAT) {
            vdbPrim->makeGridUnique();
            openvdb::FloatGrid& grid = UTvdbGridCast<openvdb::FloatGrid>(vdbPrim->getGrid());
            if ( grid.background() < float(openvdb::LEVEL_SET_HALF_WIDTH * grid.voxelSize()[0]) ) {
                narrowBands.push_back(it.getPrimitiveNameOrIndex().toStdString());
            }
            op(grid);
        } else {
            skippedGrids.push_back(it.getPrimitiveNameOrIndex().toStdString());
        }
    }

    if (!skippedGrids.empty()) {
        std::string s = "The following non-floating-point grids were skipped: "
            + hboost::algorithm::join(skippedGrids, ", ");
        addWarning(SOP_MESSAGE, s.c_str());
    }

    if (!nonLevelSetGrids.empty()) {
        std::string s = "The following non-level-set grids were skipped: "
            + hboost::algorithm::join(nonLevelSetGrids, ", ");
        addWarning(SOP_MESSAGE, s.c_str());
    }

    if (!narrowBands.empty()) {
        std::string s = "The following grids have a narrow band width that is"
            " less than 3 voxel units: " + hboost::algorithm::join(narrowBands, ", ");
        addWarning(SOP_MESSAGE, s.c_str());
    }

    return true;
}
