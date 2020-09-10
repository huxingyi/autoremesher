// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SOP_OpenVDB_Potential_Flow.cc
///
/// @authors Todd Keeler, Dan Bailey

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/Types.h>
#include <openvdb/tools/PotentialFlow.h>
#include <openvdb/tools/TopologyToLevelSet.h>

#include <UT/UT_Interrupt.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace {
    const int DEFAULT_MAX_ITERATIONS = 10000;
    const double DEFAULT_MAX_ERROR = 1.0e-20;
}

// SOP Implementation
struct SOP_OpenVDB_Potential_Flow: public hvdb::SOP_NodeVDB
{
    SOP_OpenVDB_Potential_Flow(OP_Network*, const char* name, OP_Operator*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i == 1); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
}; // SOP_OpenVDB_Potential_Flow


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setHelpText("Specify grids to process")
        .setChoiceList(&hutil::PrimGroupMenuInput1));

    parms.add(hutil::ParmFactory(PRM_STRING, "velocity", "Velocity VDB")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip(
            "Name of the reference VDB volume whose active voxels denote solid obstacles\n\n"
            "If multiple volumes are selected, only the first one will be used."));

    parms.add(hutil::ParmFactory(PRM_STRING, "maskvdbname", "Mask VDB")
        .setHelpText("VDB (from the second input) used to modify the solution domain.")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setDocumentation(
        "A VDB from the second input used to modify the volume where the potential flow"
        " will be solved.  The domain can either be restricted to the VDB input, or excluded"
        " from expanding in the VDB input."));

    parms.add(hutil::ParmFactory(PRM_ORD, "masktype", "Domain Mask Type")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "intersection",  "Intersection",
            "difference",   "Difference"
        })
        .setTooltip("Mode for applying the domain modification mask (second input)")
        .setDocumentation("Modify the constructed domain using the second input VDB mask for"
        " calculating the potential flow velocity.  __Intersection__ causes the created"
        " domain to be restricted to the Mask's active topology.  __Difference__ removes"
        " any overlap between the constructed domain and the topology.  The domain geometry"
        " will likely change the results"));

    parms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", ""));

    {
        std::ostringstream ostr;
        ostr << "If disabled, limit the potential flow solver to "
            << DEFAULT_MAX_ITERATIONS << " iterations.";
        const std::string tooltip = ostr.str();

        parms.add(hutil::ParmFactory(PRM_TOGGLE, "useiterations", "")
            .setDefault(PRMoneDefaults)
            .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
            .setTooltip(tooltip.c_str()));

        parms.add(hutil::ParmFactory(PRM_INT_J, "iterations", "Iterations")
            .setDefault(1000)
            .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 2000)
            .setTooltip("Maximum number of iterations of the potential flow solver")
            .setDocumentation(
            ("Maximum number of iterations of the potential flow solver\n\n" + tooltip).c_str()));
    }

    {
        std::ostringstream ostr;
        ostr << "If disabled, limit the potential flow solver error to "
            << std::setprecision(3) << DEFAULT_MAX_ERROR << ".";
        const std::string tooltip = ostr.str();

        parms.add(hutil::ParmFactory(PRM_TOGGLE, "usetolerance", "")
            .setDefault(PRMoneDefaults)
            .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
            .setTooltip(tooltip.c_str()));

        ostr.str("");
        ostr << "If disabled, limit the potential flow solver error to 10<sup>"
            << int(std::log10(DEFAULT_MAX_ERROR)) << "</sup>.";

        parms.add(hutil::ParmFactory(PRM_FLT_J, "tolerance", "Tolerance")
            .setDefault(openvdb::math::Delta<float>::value())
            .setRange(PRM_RANGE_RESTRICTED, DEFAULT_MAX_ERROR, PRM_RANGE_UI, 1)
            .setTooltip(
                "The potential flow solver is deemed to have converged when\n"
                "the magnitude of the absolute error is less than this tolerance.")
            .setDocumentation(
                ("The potential flow solver is deemed to have converged when"
                " the magnitude of the absolute error is less than this tolerance.\n\n"
                + ostr.str()).c_str()));
    }

    // Toggle between world- and index-space units for offset
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "useworldspace", "Use World Space Units")
        .setDefault(PRMzeroDefaults)
        .setTooltip("If enabled, use world-space units, otherwise use voxels."));

    // Stencil width
    parms.add(hutil::ParmFactory(PRM_INT_J, "dilationvoxels", "Dilation Voxels")
        .setDefault(PRMtenDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 100)
        .setTooltip(
            " The number of voxels to dilate the incoming geometry to create the domain"
            " in which the potential flow will be computed")
        .setDocumentation(
            " The number of voxels to dilate the incoming geometry to create the domain"
            " in which the potential flow will be computed"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "dilation", "Dilation")
        .setDefault(1.0)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 100)
        .setTooltip(
            " The distance in world space units to dilate the incoming geometry to create"
            " the domain in which the potential flow will be computed")
        .setDocumentation(
            " The distance in world space units to dilate the incoming geometry to create"
            " the domain in which the potential flow will be computed"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usebackgroundvelocity", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, a background velocity will be applied regardless of any velocity"
            " VDBs that are also provided. This can be used to create a similar effect"
            " to an object in a wind tunnel."));

    std::vector<fpreal> backgrounddefault{1,0,0};
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "backgroundvelocity", "Background Velocity")
        .setVectorSize(3)
        .setDefault(backgrounddefault)
        .setTooltip("A constant background fluid velocity"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "applybackgroundvelocity",
        "Apply Background Velocity to Flow Field")
        .setDefault(PRMoneDefaults)
        .setTooltip("If enabled, apply the background velocity to the resulting flow field."
            " This can be useful for a simulation where particles are simply advected through a"
            " passive velocity field. In disabling this, a similar result can be achieved by"
            " sampling the velocity and adding to an existing velocity point attribute"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "outputpotential", "Output Potential")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Output the scalar potential"));


    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Potential Flow",
        SOP_OpenVDB_Potential_Flow::factory, parms, *table)
#if UT_VERSION_INT < 0x11050000 // earlier than 17.5.0
        .setNativeName("")
#endif
        .addInput("VDB Surface and optional velocity VDB")
        .addOptionalInput("Optional VDB Mask")
        .setVerb(SOP_NodeVerb::COOK_INPLACE,
            []() { return new SOP_OpenVDB_Potential_Flow::Cache; })
        .setDocumentation(
    "#icon: COMMON/openvdb\n"
    "#tags: vdb\n"
    "\n"
    "\"\"\"Generate Potential Flow VDB velocity field. \"\"\"\n"
    "\n"
    "@overview\n"
    "\n"
    " Potential flow is the non-rotational flow of a fluid around solid moving or deforming"
    " objects constructed only from velocity values on their surface."
    " This flow field is not time-dependent and does not require an input fluid flow from the"
    " previous frame."
    " With the combination of procedural curl noise, this operator can construct fluid"
    " flows around obstacles without simulation dependencies, and therefore allows frames to "
    " be computed in parallel."
    " The potential flow field is generally used to create a flow field that cancels out movement"
    " of fluid into or out of a solid object."
    " A constant surface velocity is given as a parameter to the node, and an additional variable"
    " surface velocity can also be defined via a velocity VDB added to the first input."
    " When both are defined, they are added together on the surface boundary."
    " For objects in three dimensions, the potential flow decays at greater distances to the"
    " boundary.\n\n"
    " The node automatically creates the domain of the flow field by dilating the initial solid"
    " object boundaries."
    " It is up to the user to determine the dilation extent and therefore velocity decay needed"
    " for their application."
    " The primary input is a VDB signed distance field (SDF) on the first input."
    " The resolution and grid transform for the new velocity field will be taken from the input"
    " SDF."
    " If there are multiple SDFs only the first one is used, it is recommended to sample multiple"
    " SDFs into a single one for multiple obstacles."
    " This SDF can be accompanied by a VDB velocity field which will be used to impart the SDF"
    " velocity into the solver."
    " The potential flow created is divergence free by design and has the same velocity on the"
    " boundary as the background velocity.\n\n"
    " The simplest workflow for multiple moving objects is to animate the polygonal geometry and"
    " then create SDFs and velocity VDBs by using the VDB from Polygons node."
    " The output can be fed directly into the first input of the Potential Flow SOP."
    " The second input of the SOP allows a Mask VDB input for modifiying the solution domain"
    " created by the Potential Flow SOP."
    " The created domain can either be restricted to the active voxels of the Mask VDB, or"
    " restricted from creating a domain inside the active voxels."
    " These modes are defined by the respective __Intersection__ or __Difference__ modes on the"
    " parameter toggle"
    );
}

// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Potential_Flow::updateParmsFlags()
{
    bool changed = false;
    const bool worldUnits = bool(evalInt("useworldspace", 0, 0));
    changed |= enableParm("dilationvoxels", !worldUnits);
    changed |= setVisibleState("dilationvoxels", !worldUnits);
    changed |= enableParm("dilation", worldUnits);
    changed |= setVisibleState("dilation", worldUnits);

    const bool hasMask = (2 == nInputs());
    changed |= enableParm("maskvdbname", hasMask);
    changed |= enableParm("masktype", hasMask);
    changed |= enableParm("iterations", bool(evalInt("useiterations", 0, 0)));
    changed |= enableParm("tolerance", bool(evalInt("usetolerance", 0, 0)));
    changed |= enableParm("backgroundvelocity", bool(evalInt("usebackgroundvelocity", 0, 0)));
    changed |= enableParm("applybackgroundvelocity", bool(evalInt("usebackgroundvelocity", 0, 0)));

    return changed;
}


OP_Node*
SOP_OpenVDB_Potential_Flow::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Potential_Flow(net, name, op);
}


SOP_OpenVDB_Potential_Flow::SOP_OpenVDB_Potential_Flow(
    OP_Network* net, const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


namespace {


struct MaskOp
{
    template<typename GridType>
    void operator()(const GridType& grid) { mMaskGrid = openvdb::tools::interiorMask(grid); }

    openvdb::BoolGrid::Ptr mMaskGrid;
};


struct MaskToLevelSetOp
{
    template<typename GridType>
    void operator()(const GridType& grid) { mSdfGrid = openvdb::tools::topologyToLevelSet(grid); }

    openvdb::FloatGrid::Ptr mSdfGrid;
};


template <typename VelGridT, typename MaskGridT>
struct PotentialFlowOp
{
    using GridT = typename VelGridT::template ValueConverter<typename
        VelGridT::ValueType::value_type>::Type;
    using VecT = typename VelGridT::ValueType;
    using ScalarT = typename GridT::ValueType;

    PotentialFlowOp(const openvdb::FloatGrid& solidBoundary,
                    const MaskGridT& domain,
                    const typename VelGridT::ConstPtr& boundaryVelocity,
                    const VecT& backgroundVelocity,
                    const bool applyBackgroundVelocity)
        : mSolidBoundary(solidBoundary)
        , mDomain(domain)
        , mBoundaryVelocity(boundaryVelocity)
        , mBackgroundVelocity(backgroundVelocity)
        , mApplyBackgroundVelocity(applyBackgroundVelocity) { }

    openvdb::math::pcg::State
    process(int iterations, float absoluteError) {

        using namespace openvdb;

        typename VelGridT::Ptr neumann = tools::createPotentialFlowNeumannVelocities(
            mSolidBoundary, mDomain, mBoundaryVelocity, mBackgroundVelocity);

        // create solver state
        math::pcg::State state = math::pcg::terminationDefaults<ScalarT>();

        state.iterations = iterations;
        state.absoluteError = absoluteError;
        state.relativeError = 0.0;

        potential = tools::computeScalarPotential(mDomain, *neumann, state);
        if (mApplyBackgroundVelocity) {
            flowvel = tools::computePotentialFlow(*potential, *neumann, mBackgroundVelocity);
        } else {
            flowvel = tools::computePotentialFlow(*potential, *neumann);
        }

        return state;
    }

    typename VelGridT::Ptr flowvel;
    typename GridT::Ptr potential;

private:
    const openvdb::FloatGrid& mSolidBoundary;
    const MaskGridT& mDomain;
    const typename VelGridT::ConstPtr mBoundaryVelocity;
    const VecT mBackgroundVelocity;
    const bool mApplyBackgroundVelocity;
}; // struct PotentialFlowOp

} // unnamed namespace


OP_ERROR
SOP_OpenVDB_Potential_Flow::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Computing Potential Flow");

        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        const std::string velocity = evalStdString("velocity", time);
        const GA_PrimitiveGroup* velocityGroup = matchGroup(*gdp, velocity);

        // SOP currently only supports float level sets
        using SdfGridT = openvdb::FloatGrid;
        using MaskGridT = openvdb::BoolGrid;

        typename SdfGridT::ConstPtr grid;
        const GU_PrimVDB * velGridPrim(nullptr);

        hvdb::VdbPrimCIterator vdbIt(gdp, group);

        // find the first level set

        for (; vdbIt; ++vdbIt) {

            if (boss.wasInterrupted()) break;

            const openvdb::GridClass gridClass = vdbIt->getGrid().getGridClass();

            if (!grid && vdbIt->getStorageType() == UT_VDB_FLOAT
                    && gridClass == openvdb::GRID_LEVEL_SET) {
                grid = openvdb::gridConstPtrCast<SdfGridT>(vdbIt->getGridPtr());
            }
        }

        // find a vec3 grid for velocity

        if (!velocity.empty()) {
            hvdb::VdbPrimCIterator velocityIt(gdp, velocityGroup);

            for (; velocityIt; ++velocityIt) {
                if (!velGridPrim && (velocityIt->getStorageType() == UT_VDB_VEC3F ||
                                     velocityIt->getStorageType() == UT_VDB_VEC3D)) {
                    velGridPrim = *velocityIt;
                }
            }
        }
        else {
            hvdb::VdbPrimCIterator velocityIt(gdp);

            for (; velocityIt; ++velocityIt) {
                if (!velGridPrim && (velocityIt->getStorageType() == UT_VDB_VEC3F ||
                                     velocityIt->getStorageType() == UT_VDB_VEC3D)) {
                    velGridPrim = *velocityIt;
                }
            }
        }

        // if no level set found, use the topology of the first VDB and turn it into a level set

        if (!grid) {
            for (; vdbIt; ++vdbIt) {

                if (boss.wasInterrupted()) break;

                const GU_PrimVDB *vdb = *vdbIt;

                MaskToLevelSetOp op;
                if (GEOvdbProcessTypedGridTopology(*vdb, op)) {
                    grid = op.mSdfGrid;
                }
                else if (GEOvdbProcessTypedGridPoint(*vdb, op)) {
                    grid = op.mSdfGrid;
                }
            }
        }

        if (grid) {

            // Check for mask input
            const GU_Detail* maskGeo = inputGeo(1);
            MaskGridT::Ptr mask;

            if (maskGeo) {
                const GA_PrimitiveGroup* maskGroup = parsePrimitiveGroups(
                    evalStdString("maskvdbname", time).c_str(), GroupCreator(maskGeo));

                hvdb::VdbPrimCIterator maskIt(maskGeo, maskGroup);
                if (maskIt) {
                    MaskOp op;
                    if (UTvdbProcessTypedGridTopology(maskIt->getStorageType(),
                        maskIt->getGrid(), op)) {
                        mask = op.mMaskGrid;
                    }
                    else if (UTvdbProcessTypedGridPoint(maskIt->getStorageType(),
                        maskIt->getGrid(), op)) {
                        mask = op.mMaskGrid;
                    }
                    else {
                        addWarning(SOP_MESSAGE, "Cannot convert VDB type to mask.");
                    }

                    if (mask && mask->transform() != grid->transform()) {
                        MaskGridT::Ptr resampledMask = mask->copy();
                        resampledMask->setTransform(grid->transform().copy());

                        // resample the mask to match the boundary level set
                        openvdb::tools::resampleToMatch<openvdb::tools::PointSampler>(
                            *mask, *resampledMask);
                    }
                }
            }

            // dilate mask topology by world-space distance
            const bool useWorldSpace = static_cast<int>(evalInt("useworldspace", 0, time)) == 1;

            int dilation;

            if (useWorldSpace) {
                const double dilationDistance(static_cast<float>(evalFloat("dilation", 0, time)));
                dilation = std::max(1, static_cast<int>(dilationDistance / grid->voxelSize()[0]));
            } else {
                dilation = static_cast<int>(evalInt("dilationvoxels", 0, time));
            }

            auto domain = openvdb::tools::createPotentialFlowMask(*grid, dilation);

            if (mask) {
                if (static_cast<int>(evalInt("masktype", 0, time)) == /*intersection*/0) {
                    domain->treePtr()->topologyIntersection(mask->tree());
                } else {
                    domain->treePtr()->topologyDifference(mask->tree());
                }
            }

            const int iterations = (static_cast<int>(evalInt("useiterations", 0, time)) == 1 ?
                static_cast<int>(evalInt("iterations", 0, time)) : DEFAULT_MAX_ITERATIONS);
            const float absoluteError = static_cast<float>(
                static_cast<int>(evalInt("usetolerance", 0, time)) == 1 ?
                evalFloat("tolerance", 0, time) : DEFAULT_MAX_ERROR);

            openvdb::Vec3f backgroundVelocity(0);
            bool applyBackground(false);

            const bool useBackgroundVelocity =
                static_cast<int>(evalInt("usebackgroundvelocity", 0, time)) == 1;

            if (useBackgroundVelocity) {
                backgroundVelocity = openvdb::Vec3f(
                    static_cast<float>(evalFloat("backgroundvelocity", 0, time)),
                    static_cast<float>(evalFloat("backgroundvelocity", 1, time)),
                    static_cast<float>(evalFloat("backgroundvelocity", 2, time)));
                applyBackground = static_cast<int>(
                    evalInt("applybackgroundvelocity", 0, time)) == 1;
            }

            const bool outputPotential = static_cast<int>(
                evalInt("outputpotential", 0, time)) == 1;

            openvdb::math::pcg::State solverState;

            if (velGridPrim && velGridPrim->getStorageType() == UT_VDB_VEC3D) {
                openvdb::Vec3d backgroundVelocityD(
                    backgroundVelocity[0], backgroundVelocity[1], backgroundVelocity[2]);
                openvdb::Vec3dGrid::ConstPtr velGrid =
                    openvdb::gridConstPtrCast<openvdb::Vec3dGrid>(velGridPrim->getGridPtr());
                PotentialFlowOp<openvdb::Vec3dGrid, openvdb::MaskGrid> potentialFlowOp(
                    *grid, *domain, velGrid, backgroundVelocityD, applyBackground);
                solverState = potentialFlowOp.process(iterations, absoluteError);
                hvdb::createVdbPrimitive(*gdp, potentialFlowOp.flowvel, "flowvel");
                if (outputPotential) {
                    hvdb::createVdbPrimitive(*gdp, potentialFlowOp.potential, "potential");
                }
            }
            else {
                openvdb::Vec3fGrid::ConstPtr velGrid;
                if (velGridPrim && velGridPrim->getStorageType() == UT_VDB_VEC3F) {
                    velGrid = openvdb::gridConstPtrCast<openvdb::Vec3fGrid>(
                        velGridPrim->getGridPtr());
                }
                PotentialFlowOp<openvdb::Vec3fGrid, openvdb::MaskGrid> potentialFlowOp(
                    *grid, *domain, velGrid, backgroundVelocity, applyBackground);
                solverState = potentialFlowOp.process(iterations, absoluteError);
                hvdb::createVdbPrimitive(*gdp, potentialFlowOp.flowvel, "flowvel");
                if (outputPotential) {
                    hvdb::createVdbPrimitive(*gdp, potentialFlowOp.potential, "potential");
                }
            }

            if (!solverState.success) {
                std::ostringstream errStrm;
                errStrm << "potential flow failed to converge " <<
                    " with error " << solverState.absoluteError;
                addWarning(SOP_MESSAGE, errStrm.str().c_str());
            }
            else {
                std::ostringstream infoStrm;
                infoStrm << "solver converged in " <<
                    solverState.iterations << " iteration"
                    << (solverState.iterations == 1 ? "" : "s")
                    << " with error " << solverState.absoluteError;
                const std::string info = infoStrm.str();
                if (!info.empty()) {
                    addMessage(SOP_MESSAGE, info.c_str());
                }
            }
        }

        if (!grid && !boss.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "No valid VDB primitives found.");
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
