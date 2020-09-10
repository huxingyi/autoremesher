// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Advect.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Level set advection SOP

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/tools/LevelSetAdvect.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/VolumeAdvect.h>

#include <UT/UT_Version.h>
#include <UT/UT_Interrupt.h>
#include <GA/GA_PageIterator.h>
#include <GU/GU_PrimPoly.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>

#include <hboost/algorithm/string/join.hpp>

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace { struct AdvectionParms; }


class SOP_OpenVDB_Advect: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Advect(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Advect() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;
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
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("VDB grid(s) to advect.")
        .setDocumentation(
            "A subset of VDBs in the first input to move using the velocity field"
            " (see [specifying volumes|/model/volumes#group])"));

    // Velocity grid
    parms.add(hutil::ParmFactory(PRM_STRING, "velgroup", "Velocity")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("Velocity grid")
        .setDocumentation(
            "The name of a VDB primitive in the second input to use as"
            " the velocity field (see [specifying volumes|/model/volumes#group])\n\n"
            "This must be a vector-valued VDB primitive."
            " You can use the [Vector Merge node|Node:sop/DW_OpenVDBVectorMerge]"
            " to turn a `vel.[xyz]` triple into a single primitive."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "respectclass", "Respect Grid Class")
        .setDefault(PRMoneDefaults)
        .setTooltip("If disabled, advect level sets using general advection scheme.")
        .setDocumentation(
            "When this option is disabled, all VDBs will use a general numerical"
            " advection scheme, otherwise level set VDBs will be advected using"
            " a spatial finite-difference scheme."));

    // Advect: timestep
    parms.add(hutil::ParmFactory(PRM_FLT, "timestep", "Timestep")
        .setDefault(1, "1.0/$FPS")
        .setDocumentation(
            "Number of seconds of movement to apply to the input points\n\n"
            "The default is `1/$FPS` (one frame's worth of time)."
            " You can use negative values to move the points backwards through"
            " the velocity field."));

    parms.add(hutil::ParmFactory(PRM_HEADING, "general", "General Advection")
         .setDocumentation(
             "These control how VDBs that are not level sets are moved through the velocity field."
             " If the grid class is not being respected, all grids will be advected"
             " using general advection regardless of whether they are level sets or not."));

    // SubSteps
    parms.add(hutil::ParmFactory(PRM_INT_J, "substeps", "Substeps")
         .setDefault(1)
         .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
         .setTooltip(
             "The number of substeps per integration step.\n"
             "The only reason to increase it above its default value of one"
             " is to reduce the memory footprint from dilations--likely at the cost"
             " of more smoothing!")
         .setDocumentation(
            "The number of substeps per integration step\n\n"
            "The only reason to increase this above its default value of one is to reduce"
            " the memory footprint from dilations&mdash;likely at the cost of more smoothing."));

    // Advection Scheme
    parms.add(hutil::ParmFactory(PRM_STRING, "advection", "Advection Scheme")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "semi",   "Semi-Lagrangian",
            "mid",    "Mid-Point",
            "rk3",    "3rd order Runge-Kutta",
            "rk4",    "4th order Runge-Kutta",
            "mac",    "MacCormack",
            "bfecc",  "BFECC"
        })
        .setDefault("semi")
        .setTooltip("Set the numerical advection scheme."));

    // Limiter Scheme
    parms.add(hutil::ParmFactory(PRM_STRING, "limiter", "Limiter Scheme")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none",     "No limiter",
            "clamp",    "Clamp to extrema",
            "revert",   "Revert to 1st order"
        })
        .setDefault("revert")
        .setTooltip(
            "Set the limiter scheme used to stabilize the second-order"
            " MacCormack and BFECC schemes."));

    parms.add(hutil::ParmFactory(PRM_HEADING, "advectionHeading", "Level Set Advection")
        .setDocumentation(
            "These control how level set VDBs are moved through the velocity field."
            " If the grid class is not being respected, these options are not used."));

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
                "How accurately the gradients of the signed distance field are computed\n\n"
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
                "How accurately time is evolved within the timestep\n\n"
                "The later choices are more accurate but take more time."));
    }

    parms.add(hutil::ParmFactory(PRM_HEADING, "renormheading", "Renormalization"));

    parms.add(hutil::ParmFactory(PRM_INT_J, "normsteps", "Steps")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("The number of normalizations performed after each CFL iteration.")
        .setDocumentation(
            "After advection, a signed distance field will often no longer contain correct"
            " distances.  A number of renormalization passes can be performed between"
            " every substep to convert it back into a proper signed distance field."));

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
                "How accurately the gradients of the signed distance field are computed\n\n"
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
            .setDefault(items[0])
            .setTooltip("Set the temporal integration scheme.")
            .setDocumentation(
                "How accurately time is evolved within the renormalization stage\n\n"
                "The later choices are more accurate but take more time."));
    }

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT, "beginTime", "Begin time"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT, "endTime", "Time step"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "lsGroup", "Group"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "densityGroup", "Group"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "renormHeading", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "velGroup", "Velocity"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "advectSpatial", "Spatial Scheme")
        .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS)));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "advectTemporal", "Temporal Scheme")
        .setDefault(temporalIntegrationSchemeToString(TVD_RK2)));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "normSteps", "Renormalization Steps")
        .setDefault(PRMthreeDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "renormSpatial", "Spatial Renormalization")
        .setDefault(biasedGradientSchemeToString(HJWENO5_BIAS)));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "renormTemporal", "Temporal Renormalization")
        .setDefault(temporalIntegrationSchemeToString(TemporalIntegrationScheme(0))));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Advect",
        SOP_OpenVDB_Advect::factory, parms, *table)
        .setNativeName("vdbadvectsdf")
        .setObsoleteParms(obsoleteParms)
        .addInput("VDBs to Advect")
        .addInput("Velocity VDB")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Advect::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Move VDBs in the input geometry along a VDB velocity field.\"\"\"\n\
\n\
@overview\n\
The OpenVDB Advect operation will advect VDB volumes according to\n\
a velocity field defined in a vector VDB.\n\
\n\
@animation Animating advection\n\
\n\
*This node is not a feedback loop*.\n\
\n\
It moves the fields it finds in the input geometry.\n\
It _cannot_ modify the fields over time.\n\
(That is, if you hook this node up to do advection and press play,\n\
the fields will not animate.)\n\
\n\
To set up a feedback loop, where the advection at each frame affects\n\
the advected field from the previous frame, do one of the following:\n\
* Do the advection inside a [SOP Solver|Node:sop/solver].\n\
* Set the __Time Step__ to `$T`\n\
\n\
  This will cause the node to recalculate, _at every frame_, the path\n\
  of every particle through _every previous frame_ to get the current one.\n\
  This is obviously not very practical.\n\
\n\
@related\n\
- [OpenVDB Advect Points|Node:sop/DW_OpenVDBAdvectPoints]\n\
- [OpenVDB Morph Level Set|Node:sop/DW_OpenVDBMorphLevelSet]\n\
- [Node:sop/vdbadvectsdf]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Advect::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Advect(net, name, op);
}


SOP_OpenVDB_Advect::SOP_OpenVDB_Advect(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


void
SOP_OpenVDB_Advect::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "lsGroup", "group");
    resolveRenamedParm(*obsoleteParms, "densityGroup", "group");
    resolveRenamedParm(*obsoleteParms, "advectSpatial", "advectspatial");
    resolveRenamedParm(*obsoleteParms, "advectTemporal", "advecttemporal");
    resolveRenamedParm(*obsoleteParms, "normSteps", "normsteps");
    resolveRenamedParm(*obsoleteParms, "renormSpatial", "renormspatial");
    resolveRenamedParm(*obsoleteParms, "renormTemporal", "renormtemporal");
    resolveRenamedParm(*obsoleteParms, "velGroup", "velgroup");

    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


////////////////////////////////////////


bool
SOP_OpenVDB_Advect::updateParmsFlags()
{
    bool changed = false;

    const bool respectClass = bool(evalInt("respectclass", 0, 0));

    changed |= enableParm("advectspatial", respectClass);
    changed |= enableParm("advecttemporal", respectClass);
    changed |= enableParm("normsteps", respectClass);
    changed |= enableParm("renormspatial", respectClass);
    changed |= enableParm("renormtemporal", respectClass);

    return changed;
}


////////////////////////////////////////

// Utilities

namespace {

struct AdvectionParms {
    AdvectionParms()
        : mGroup(nullptr)
        , mAdvectSpatial(openvdb::math::UNKNOWN_BIAS)
        , mRenormSpatial(openvdb::math::UNKNOWN_BIAS)
        , mAdvectTemporal(openvdb::math::UNKNOWN_TIS)
        , mRenormTemporal(openvdb::math::UNKNOWN_TIS)
        , mIntegrator(openvdb::tools::Scheme::SEMI)
        , mLimiter(openvdb::tools::Scheme::NO_LIMITER)
        , mNormCount(1)
        , mSubSteps(1)
        , mTimeStep(0.0)
        , mStaggered(false)
        , mRespectClass(true)
    {
    }

    const GA_PrimitiveGroup *                   mGroup;
    hvdb::Grid::ConstPtr                        mVelocityGrid;
    openvdb::math::BiasedGradientScheme         mAdvectSpatial, mRenormSpatial;
    openvdb::math::TemporalIntegrationScheme    mAdvectTemporal, mRenormTemporal;
    openvdb::tools::Scheme::SemiLagrangian      mIntegrator;
    openvdb::tools::Scheme::Limiter             mLimiter;
    int                                         mNormCount, mSubSteps;
    float                                       mTimeStep;
    bool                                        mStaggered, mRespectClass;
};


template<class VelocityGridT>
class AdvectOp
{
public:
    AdvectOp(AdvectionParms& parms, const VelocityGridT& velGrid, hvdb::Interrupter& boss)
        : mParms(parms)
        , mVelGrid(velGrid)
        , mBoss(boss)
    {
    }

    template<typename GridT, typename SamplerT>
    void process(GridT& grid)
    {
        using FieldT = openvdb::tools::DiscreteField<VelocityGridT, SamplerT>;
        const FieldT field(mVelGrid);

        openvdb::tools::LevelSetAdvection<GridT, FieldT, hvdb::Interrupter>
            advection(grid, field, &mBoss);

        advection.setSpatialScheme(mParms.mAdvectSpatial);
        advection.setTemporalScheme(mParms.mAdvectTemporal);
        advection.setTrackerSpatialScheme(mParms.mRenormSpatial);
        advection.setTrackerTemporalScheme(mParms.mRenormTemporal);
        advection.setNormCount(mParms.mNormCount);

        if (mBoss.wasInterrupted()) return;

        advection.advect(0, mParms.mTimeStep);
    }


    template<typename GridT>
    void operator()(GridT& grid)
    {
        if (mBoss.wasInterrupted()) return;

        if (mParms.mStaggered) process<GridT, openvdb::tools::StaggeredBoxSampler>(grid);
        else process<GridT, openvdb::tools::BoxSampler>(grid);
    }

private:
    AdvectOp(const AdvectOp&);// undefined
    AdvectOp& operator=(const AdvectOp&);// undefined

    AdvectionParms& mParms;
    const VelocityGridT& mVelGrid;
    hvdb::Interrupter& mBoss;
};


template<typename VelocityGridT, bool StaggeredVelocity>
inline bool
processGrids(GU_Detail* gdp, AdvectionParms& parms, hvdb::Interrupter& boss,
    const std::function<void (const std::string&)>& warningCallback)
{
    using VolumeAdvection =
        openvdb::tools::VolumeAdvection<VelocityGridT, StaggeredVelocity, hvdb::Interrupter>;
    using VelocityGridCPtr = typename VelocityGridT::ConstPtr;

    VelocityGridCPtr velGrid = hvdb::Grid::constGrid<VelocityGridT>(parms.mVelocityGrid);
    if (!velGrid) return false;

    AdvectOp<VelocityGridT> advectLevelSet(parms, *velGrid, boss);

    VolumeAdvection advectVolume(*velGrid, &boss);
    advectVolume.setIntegrator(parms.mIntegrator);
    advectVolume.setLimiter(parms.mLimiter);
    advectVolume.setSubSteps(parms.mSubSteps);


    std::vector<std::string> skippedGrids, doubleGrids;

    for (hvdb::VdbPrimIterator it(gdp, parms.mGroup); it; ++it) {

        if (boss.wasInterrupted()) break;

        GU_PrimVDB* vdbPrim = *it;

        if (parms.mRespectClass && vdbPrim->getGrid().getGridClass() == openvdb::GRID_LEVEL_SET) {

            if (vdbPrim->getStorageType() == UT_VDB_FLOAT) {
                vdbPrim->makeGridUnique();
                auto& grid = UTvdbGridCast<openvdb::FloatGrid>(vdbPrim->getGrid());
                advectLevelSet(grid);
            }
            //else if (vdbPrim->getStorageType() == UT_VDB_DOUBLE) {
            //    vdbPrim->makeGridUnique();
            //    auto& grid = UTvdbGridCast<openvdb::DoubleGrid>(vdbPrim->getGrid());
            //    advectLevelSet(grid);
            //}
            else {
                skippedGrids.push_back(it.getPrimitiveNameOrIndex().toStdString());
            }

        } else {
            switch (vdbPrim->getStorageType()) {

            case UT_VDB_FLOAT:
            {
                const auto& inGrid = UTvdbGridCast<openvdb::FloatGrid>(vdbPrim->getConstGrid());
                auto outGrid = advectVolume.template advect<openvdb::FloatGrid,
                    openvdb::tools::Sampler<1, false>>(inGrid, parms.mTimeStep);
                hvdb::replaceVdbPrimitive(*gdp, outGrid, *vdbPrim);
                break;
            }

            case UT_VDB_DOUBLE:
            {
                const auto& inGrid = UTvdbGridCast<openvdb::DoubleGrid>(vdbPrim->getConstGrid());
                auto outGrid = advectVolume.template advect<openvdb::DoubleGrid,
                    openvdb::tools::Sampler<1, false>>(inGrid, parms.mTimeStep);
                hvdb::replaceVdbPrimitive(*gdp, outGrid, *vdbPrim);
                break;
            }

            case UT_VDB_VEC3F:
            {
                const auto& inGrid = UTvdbGridCast<openvdb::Vec3SGrid>(vdbPrim->getConstGrid());
                auto outGrid = advectVolume.template advect<openvdb::Vec3SGrid,
                    openvdb::tools::Sampler<1, false>>(inGrid, parms.mTimeStep);
                hvdb::replaceVdbPrimitive(*gdp, outGrid, *vdbPrim);
                break;
            }

            default:
                skippedGrids.push_back(it.getPrimitiveNameOrIndex().toStdString());
                break;
            }
        }
    }

    if (!skippedGrids.empty() && warningCallback) {
        std::string s = "The following non-floating-point grids were skipped: "
            + hboost::algorithm::join(skippedGrids, ", ");
        warningCallback(s);
    }

    return true;
} // processGrids()

} // anonymous namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Advect::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal now = context.getTime();

        // Evaluate UI parameters
        AdvectionParms parms;
        {
            parms.mGroup = matchGroup(*gdp, evalStdString("group", now));

            parms.mTimeStep = static_cast<float>(evalFloat("timestep", 0, now));

            parms.mAdvectSpatial =
                openvdb::math::stringToBiasedGradientScheme(evalStdString("advectspatial", now));
            if (parms.mAdvectSpatial == openvdb::math::UNKNOWN_BIAS) {
                throw std::runtime_error{"Advect: Unknown biased gradient"};
            }

            parms.mRenormSpatial = openvdb::math::stringToBiasedGradientScheme(
                evalStdString("renormspatial", now));
            if (parms.mRenormSpatial == openvdb::math::UNKNOWN_BIAS) {
                throw std::runtime_error{"Renorm: Unknown biased gradient"};
            }

            parms.mAdvectTemporal = openvdb::math::stringToTemporalIntegrationScheme(
                evalStdString("advecttemporal", now));
            if (parms.mAdvectTemporal == openvdb::math::UNKNOWN_TIS) {
                throw std::runtime_error{"Advect: Unknown temporal integration"};
            }

            parms.mRenormTemporal = openvdb::math::stringToTemporalIntegrationScheme(
                evalStdString("renormtemporal", now));
            if (parms.mRenormTemporal == openvdb::math::UNKNOWN_TIS) {
                throw std::runtime_error{"Renorm: Unknown temporal integration"};
            }

            parms.mNormCount = static_cast<int>(evalInt("normsteps", 0, now));

            const GU_Detail* velGeo = inputGeo(1);
            if (!velGeo) throw std::runtime_error{"Missing velocity grid input"};

            hvdb::VdbPrimCIterator it{velGeo, matchGroup(*velGeo, evalStdString("velgroup", now))};
            if (it) {
                if (it->getStorageType() != UT_VDB_VEC3F) {
                    throw std::runtime_error{"Unrecognized velocity grid type"};
                }
                parms.mVelocityGrid = it->getConstGridPtr();
            }
            if (!parms.mVelocityGrid) {
                throw std::runtime_error{"Missing velocity grid"};
            }

            parms.mStaggered = parms.mVelocityGrid->getGridClass() == openvdb::GRID_STAGGERED;
            parms.mRespectClass = bool(evalInt("respectclass", 0, now));

            // General advection options

            parms.mSubSteps = static_cast<int>(evalInt("substeps", 0, now));

            {
                const auto str = evalStdString("advection", now);
                if (str == "semi")       { parms.mIntegrator = openvdb::tools::Scheme::SEMI; }
                else if (str == "mid")   { parms.mIntegrator = openvdb::tools::Scheme::MID; }
                else if (str == "rk3")   { parms.mIntegrator = openvdb::tools::Scheme::RK3; }
                else if (str == "rk4")   { parms.mIntegrator = openvdb::tools::Scheme::RK4; }
                else if (str == "mac")   { parms.mIntegrator = openvdb::tools::Scheme::MAC; }
                else if (str == "bfecc") { parms.mIntegrator = openvdb::tools::Scheme::BFECC; }
                else { throw std::runtime_error{"Invalid advection scheme"}; }
            }
            {
                const auto str = evalStdString("limiter", now);
                if (str == "none") {
                    parms.mLimiter = openvdb::tools::Scheme::NO_LIMITER;
                    if (parms.mIntegrator == openvdb::tools::Scheme::MAC) {
                        addWarning(SOP_MESSAGE, "MacCormack is unstable without a limiter");
                    }
                } else if (str == "clamp") {
                    parms.mLimiter = openvdb::tools::Scheme::CLAMP;
                } else if (str == "revert") {
                    parms.mLimiter = openvdb::tools::Scheme::REVERT;
                } else {
                    throw std::runtime_error{"Invalid limiter scheme"};
                }
            }
        }

        hvdb::Interrupter boss("Advecting level set");

        auto warningCallback = [this](const std::string& s) {
            this->addWarning(SOP_MESSAGE, s.c_str());
        };

        if (parms.mStaggered) {
            processGrids<openvdb::Vec3SGrid, true>(gdp, parms, boss, warningCallback);
        } else {
            processGrids<openvdb::Vec3SGrid, false>(gdp, parms, boss, warningCallback);
        }

        if (boss.wasInterrupted()) addWarning(SOP_MESSAGE, "Process was interrupted");
        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
