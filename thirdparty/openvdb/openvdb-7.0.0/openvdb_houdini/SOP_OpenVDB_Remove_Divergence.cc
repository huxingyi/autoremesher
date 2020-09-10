// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Remove_Divergence.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/math/ConjGradient.h> // for JacobiPreconditioner
#include <openvdb/tools/GridOperators.h>
#include <openvdb/tools/LevelSetUtil.h> // for tools::sdfInteriorMask()
#include <openvdb/tools/PoissonSolver.h>
#include <openvdb/tools/Prune.h>

#include <UT/UT_Interrupt.h>
#include <UT/UT_StringArray.h>
#include <UT/UT_Version.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>
#include <GA/GA_Handle.h>
#include <GA/GA_PageIterator.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <sstream>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace {
using ColliderMaskGrid = openvdb::BoolGrid; ///< @todo really should derive from velocity grid
using ColliderBBox = openvdb::BBoxd;
using Coord = openvdb::Coord;

enum ColliderType { CT_NONE, CT_BBOX, CT_STATIC, CT_DYNAMIC };

const int DEFAULT_MAX_ITERATIONS = 10000;
const double DEFAULT_MAX_ERROR = 1.0e-20;
}


////////////////////////////////////////


struct SOP_OpenVDB_Remove_Divergence: public hvdb::SOP_NodeVDB
{
    SOP_OpenVDB_Remove_Divergence(OP_Network*, const char* name, OP_Operator*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned input) const override { return (input > 0); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Names of vector-valued VDBs to be processed")
        .setDocumentation(
            "A subset of vector-valued input VDBs to be processed"
            " (see [specifying volumes|/model/volumes#group])\n\n"
            "VDBs with nonuniform voxels, including frustum grids, are not supported.\n"
            "They should be [resampled|Node:sop/DW_OpenVDBResample]"
            " to have a linear transform with uniform scale."));

    {
        std::ostringstream ostr;
        ostr << "If disabled, limit the pressure solver to "
            << DEFAULT_MAX_ITERATIONS << " iterations.";
        const std::string tooltip = ostr.str();

        parms.add(hutil::ParmFactory(PRM_TOGGLE, "useiterations", "")
            .setDefault(PRMoneDefaults)
            .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
            .setTooltip(tooltip.c_str()));

        parms.add(hutil::ParmFactory(PRM_INT_J, "iterations", "Iterations")
            .setDefault(1000)
            .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 2000)
            .setTooltip("Maximum number of iterations of the pressure solver")
            .setDocumentation(
                ("Maximum number of iterations of the pressure solver\n\n" + tooltip).c_str()));
    }
    {
        std::ostringstream ostr;
        ostr << "If disabled, limit the pressure solver error to "
            << std::setprecision(3) << DEFAULT_MAX_ERROR << ".";
        const std::string tooltip = ostr.str();

        parms.add(hutil::ParmFactory(PRM_TOGGLE, "usetolerance", "")
            .setDefault(PRMoneDefaults)
            .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
            .setTooltip(tooltip.c_str()));

        ostr.str("");
        ostr << "If disabled, limit the pressure solver error to 10<sup>"
            << int(std::log10(DEFAULT_MAX_ERROR)) << "</sup>.";

        parms.add(hutil::ParmFactory(PRM_FLT_J, "tolerance", "Tolerance")
            .setDefault(openvdb::math::Delta<float>::value())
            .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 0.01)
            .setTooltip(
                "The pressure solver is deemed to have converged when\n"
                "the magnitude of the error is less than this tolerance.")
            .setDocumentation(
                ("The pressure solver is deemed to have converged when"
                " the magnitude of the error is less than this tolerance.\n\n"
                + ostr.str()).c_str()));
    }

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usecollider", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "collidertype", "Collider Type")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "bbox",    "Bounding Box",
            "static",  "Static VDB",
            "dynamic", "Dynamic VDB"
        })
        .setDefault("bbox")
        .setTooltip(
"Bounding Box:\n"
"    Use the bounding box of any reference geometry as the collider.\n"
"Static VDB:\n"
"    Treat the active voxels of the named VDB volume as solid, stationary obstacles."
"\nDynamic VDB:\n"
"    If the named VDB volume is vector-valued, treat the values of active voxels\n"
"    as velocities of moving obstacles; otherwise, treat the active voxels as\n"
"    stationary obstacles."
        ));

    parms.add(hutil::ParmFactory(PRM_STRING, "collider", "Collider")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip(
            "Name of the reference VDB volume whose active voxels denote solid obstacles\n\n"
            "If multiple volumes are selected, only the first one will be used."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "invertcollider", "Invert Collider")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "Invert the collider so that active voxels denote empty space\n"
            "and inactive voxels denote solid obstacles."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "pressure", "Output Pressure")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "Output the computed pressure for each input VDB \"v\"\n"
            "as a scalar VDB named \"v_pressure\"."));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Project Non-Divergent",
        SOP_OpenVDB_Remove_Divergence::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBRemoveDivergence")
#endif
        .addInput("Velocity field VDBs")
        .addOptionalInput("Optional collider VDB or geometry")
        .setVerb(SOP_NodeVerb::COOK_INPLACE,
            []() { return new SOP_OpenVDB_Remove_Divergence::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Remove divergence from VDB velocity fields.\"\"\"\n\
\n\
@overview\n\
\n\
A vector-valued VDB volume can represent a velocity field.\n\
When particles flow through the field, they might either expand\n\
from a voxel or collapse into a voxel.\n\
These source/sink behaviors indicate divergence in the field.\n\
\n\
This node computes a new vector field that is close to the input\n\
but has no divergence.\n\
This can be used to condition velocity fields to limit particle creation,\n\
creating more realistic flows.\n\
\n\
If the optional collider volume is provided, the output velocity field\n\
will direct flow around obstacles (i.e., active voxels) in that volume.\n\
The collider itself may be a velocity field, in which case the obstacles\n\
are considered to be moving with the given velocities.\n\
\n\
Combined with the [OpenVDB Advect Points|Node:sop/DW_OpenVDBAdvectPoints]\n\
node and a [Solver|Node:sop/solver] node for feedback, this node\n\
can be used to build a simple FLIP solver.\n\
\n\
@related\n\
- [OpenVDB Advect Points|Node:sop/DW_OpenVDBAdvectPoints]\n\
- [Node:sop/solver]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


bool
SOP_OpenVDB_Remove_Divergence::updateParmsFlags()
{
    bool changed = false;
    const bool useCollider = evalInt("usecollider", 0, 0);
    changed |= enableParm("collidertype", useCollider);
    changed |= enableParm("invertcollider", useCollider);
    changed |= enableParm("collider", useCollider && (evalStdString("collidertype", 0) != "bbox"));
    changed |= enableParm("iterations", bool(evalInt("useiterations", 0, 0)));
    changed |= enableParm("tolerance", bool(evalInt("usetolerance", 0, 0)));
    return changed;
}


OP_Node*
SOP_OpenVDB_Remove_Divergence::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Remove_Divergence(net, name, op);
}


SOP_OpenVDB_Remove_Divergence::SOP_OpenVDB_Remove_Divergence(
    OP_Network* net, const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

struct SolverParms {
    SolverParms()
        : invertCollider(false)
        , colliderType(CT_NONE)
        , iterations(1)
        , absoluteError(-1.0)
        , outputState(openvdb::math::pcg::terminationDefaults<double>())
        , interrupter(nullptr)
    {}

    hvdb::GridPtr velocityGrid;
    hvdb::GridCPtr colliderGrid;
    hvdb::GridPtr pressureGrid;
    hvdb::GridCPtr domainMaskGrid;
    ColliderBBox colliderBBox;
    bool invertCollider;
    ColliderType colliderType;
    int iterations;
    double absoluteError;
    openvdb::math::pcg::State outputState;
    hvdb::Interrupter* interrupter;
};


////////////////////////////////////////


/// @brief Functor to extract an interior mask from a level set grid
/// of arbitrary floating-point type
struct LevelSetMaskOp
{
    template<typename GridType>
    void operator()(const GridType& grid) { outputGrid = openvdb::tools::sdfInteriorMask(grid); }

    hvdb::GridPtr outputGrid;
};


/// @brief Functor to extract a topology mask from a grid of arbitrary type
struct ColliderMaskOp
{
    template<typename GridType>
    void operator()(const GridType& grid)
    {
        if (mask) {
            mask->topologyUnion(grid);
            mask->setTransform(grid.transform().copy());
        }
    }

    ColliderMaskGrid::Ptr mask;
};


////////////////////////////////////////


/// @brief Generic grid accessor
/// @details This just wraps a const accessor to a collider grid, but
/// it changes the behavior of the copy constructor for thread safety.
template<typename GridType>
class GridConstAccessor
{
public:
    using ValueType = typename GridType::ValueType;

    explicit GridConstAccessor(const SolverParms& parms):
        mAcc(static_cast<const GridType&>(*parms.colliderGrid).getConstAccessor())
    {}
    explicit GridConstAccessor(const GridType& grid): mAcc(grid.getConstAccessor()) {}

    // When copying, create a new, empty accessor, to avoid a data race
    // with the existing accessor, which might be updating on another thread.
    GridConstAccessor(const GridConstAccessor& other): mAcc(other.mAcc.tree()) {}

    bool isValueOn(const Coord& ijk) const { return mAcc.isValueOn(ijk); }
    const ValueType& getValue(const Coord& ijk) const { return mAcc.getValue(ijk); }
    bool probeValue(const Coord& ijk, ValueType& val) const { return mAcc.probeValue(ijk, val); }

private:
    GridConstAccessor& operator=(const GridConstAccessor&);

    typename GridType::ConstAccessor mAcc;
}; // class GridConstAccessor

using ColliderMaskAccessor = GridConstAccessor<ColliderMaskGrid>;


/// @brief Bounding box accessor
class BBoxConstAccessor
{
public:
    using ValueType = double;

    explicit BBoxConstAccessor(const SolverParms& parms):
        mBBox(parms.velocityGrid->transform().worldToIndexNodeCentered(parms.colliderBBox)) {}
    BBoxConstAccessor(const BBoxConstAccessor& other): mBBox(other.mBBox) {}

    // Voxels outside the bounding box are solid, i.e., active.
    bool isValueOn(const Coord& ijk) const { return !mBBox.isInside(ijk); }
    ValueType getValue(const Coord&) const { return ValueType(0); }
    bool probeValue(const Coord& ijk, ValueType& v) const { v=ValueType(0); return isValueOn(ijk); }

private:
    BBoxConstAccessor& operator=(const BBoxConstAccessor&);

    const openvdb::CoordBBox mBBox;
}; // class BBoxConstAccessor


////////////////////////////////////////


/// @brief Functor to compute pressure projection in parallel over leaf nodes
template<typename TreeType>
struct PressureProjectionOp
{
    using LeafNodeType = typename TreeType::LeafNodeType;
    using ValueType = typename TreeType::ValueType;

    PressureProjectionOp(SolverParms& parms, LeafNodeType** velNodes,
        const LeafNodeType** gradPressureNodes, bool staggered)
        : mVelocityNodes(velNodes)
        , mGradientOfPressureNodes(gradPressureNodes)
        , mVoxelSize(parms.velocityGrid->transform().voxelSize()[0])
        , mStaggered(staggered)
    {
    }

    void operator()(const tbb::blocked_range<size_t>& range) const
    {
        using ElementType = typename ValueType::value_type;

        // Account for voxel size here, instead of in the Poisson solve.
        const ElementType scale = ElementType((mStaggered ? 1.0 : 4.0) * mVoxelSize * mVoxelSize);

        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
            LeafNodeType& velocityNode = *mVelocityNodes[n];
            ValueType* velocityData = velocityNode.buffer().data();
            const ValueType* gradientOfPressureData = mGradientOfPressureNodes[n]->buffer().data();

            for (typename LeafNodeType::ValueOnIter it = velocityNode.beginValueOn(); it; ++it) {
                const openvdb::Index pos = it.pos();
                velocityData[pos] -= scale * gradientOfPressureData[pos];
            }
        }
    }

    LeafNodeType* const * const mVelocityNodes;
    LeafNodeType const * const * const mGradientOfPressureNodes;
    const double mVoxelSize;
    const bool mStaggered;
}; // class PressureProjectionOp


////////////////////////////////////////


/// @brief Functor for use with Tree::modifyValue() to set a single element
/// of a vector-valued voxel
template<typename VectorType>
struct SetVecElemOp
{
    using ValueType = typename VectorType::ValueType;

    SetVecElemOp(int axis_, ValueType value_): axis(axis_), value(value_) {}
    void operator()(VectorType& v) const { v[axis] = value; }

    const int axis;
    const ValueType value;
};


/// @brief Functor to correct the velocities of voxels adjacent to solid obstacles
template<typename VelocityGridType>
class CorrectCollisionVelocityOp
{
public:
    using VectorType = typename VelocityGridType::ValueType;
    using VectorElementType = typename VectorType::ValueType;
    using MaskGridType = typename VelocityGridType::template ValueConverter<bool>::Type;
    using MaskTreeType = typename MaskGridType::TreeType;

    explicit CorrectCollisionVelocityOp(SolverParms& parms): mParms(&parms)
    {
        const MaskGridType& domainMaskGrid =
            static_cast<const MaskGridType&>(*mParms->domainMaskGrid);
        typename MaskTreeType::Ptr interiorMask(
            new MaskTreeType(domainMaskGrid.tree(), /*background=*/false, openvdb::TopologyCopy()));
        mBorderMask.reset(new MaskTreeType(*interiorMask));
        openvdb::tools::erodeVoxels(*interiorMask, /*iterations=*/1, openvdb::tools::NN_FACE);
        mBorderMask->topologyDifference(*interiorMask);
    }

    template<typename ColliderGridType>
    void operator()(const ColliderGridType&)
    {
        GridConstAccessor<ColliderGridType> collider(
            static_cast<const ColliderGridType&>(*mParms->colliderGrid));
        correctVelocity(collider);
    }

    template<typename ColliderAccessorType>
    void correctVelocity(const ColliderAccessorType& collider)
    {
        using ColliderValueType = typename ColliderAccessorType::ValueType;

        VelocityGridType& velocityGrid = static_cast<VelocityGridType&>(*mParms->velocityGrid);

        typename VelocityGridType::Accessor velocity = velocityGrid.getAccessor();

        const bool invert = mParms->invertCollider;

        switch (mParms->colliderType) {
        case CT_NONE:
            break;

        case CT_BBOX:
        case CT_STATIC:
            // For each border voxel of the velocity grid...
            /// @todo parallelize
            for (typename MaskTreeType::ValueOnCIter it = mBorderMask->cbeginValueOn(); it; ++it) {
                const Coord ijk = it.getCoord();

                // If the neighbor in a certain direction is a stationary obstacle,
                // set the border voxel's velocity in that direction to zero.

                // x direction
                if ((collider.isValueOn(ijk.offsetBy(-1, 0, 0)) != invert)
                    || (collider.isValueOn(ijk.offsetBy(1, 0, 0)) != invert))
                {
                    velocity.modifyValue(ijk, SetVecElemOp<VectorType>(0, 0));
                }
                // y direction
                if ((collider.isValueOn(ijk.offsetBy(0, -1, 0)) != invert)
                    || (collider.isValueOn(ijk.offsetBy(0, 1, 0)) != invert))
                {
                    velocity.modifyValue(ijk, SetVecElemOp<VectorType>(1, 0));
                }
                // z direction
                if ((collider.isValueOn(ijk.offsetBy(0, 0, -1)) != invert)
                    || (collider.isValueOn(ijk.offsetBy(0, 0, 1)) != invert))
                {
                    velocity.modifyValue(ijk, SetVecElemOp<VectorType>(2, 0));
                }
            }
            break;

        case CT_DYNAMIC:
            // For each border voxel of the velocity grid...
            /// @todo parallelize
            for (typename MaskTreeType::ValueOnCIter it = mBorderMask->cbeginValueOn(); it; ++it) {
                const Coord ijk = it.getCoord();

                ColliderValueType colliderVal;

                // If the neighbor in a certain direction is a moving obstacle,
                // set the border voxel's velocity in that direction to the
                // obstacle's velocity in that direction.
                for (int axis = 0; axis <= 2; ++axis) { // 0:x, 1:y, 2:z
                    Coord neighbor = ijk;
                    neighbor[axis] -= 1;
                    if (collider.probeValue(neighbor, colliderVal) != invert) {
                        // Copy or create a Vec3 from the collider value and extract one of
                        // its components.
                        // (Since the collider is dynamic, ColliderValueType must be a Vec3 type,
                        // but this code has to compile for all ColliderGridTypes.)
                        VectorElementType colliderVelocity = VectorType(colliderVal)[axis];
                        velocity.modifyValue(ijk,
                            SetVecElemOp<VectorType>(axis, colliderVelocity));
                    } else {
                        neighbor = ijk;
                        neighbor[axis] += 1;
                        if (collider.probeValue(neighbor, colliderVal) != invert) {
                            VectorElementType colliderVelocity = VectorType(colliderVal)[axis];
                            velocity.modifyValue(ijk,
                                SetVecElemOp<VectorType>(axis, colliderVelocity));
                        }
                    }
                }
            }
            break;
        } // switch (mParms->colliderType)
    }

private:
    SolverParms* mParms;
    typename MaskTreeType::Ptr mBorderMask;
}; // class CorrectCollisionVelocityOp


////////////////////////////////////////


//{
// Boundary condition functors

/// @brief Functor specifying boundary conditions for the Poisson solver
/// when exterior voxels may be either solid (and possibly in motion) or empty
template<typename VelocityGridType, typename ColliderAccessorType>
class ColliderBoundaryOp
{
public:
    using VectorType = typename VelocityGridType::ValueType;

    explicit ColliderBoundaryOp(const SolverParms& parms)
        : mVelocity(static_cast<VelocityGridType&>(*parms.velocityGrid).getConstAccessor())
        , mCollider(parms)
        , mInvert(parms.invertCollider)
        , mDynamic(parms.colliderType == CT_DYNAMIC)
        , mInvVoxelSize(0.5 / (parms.velocityGrid->voxelSize()[0])) // assumes uniform voxels
    {}

    ColliderBoundaryOp(const ColliderBoundaryOp& other)
        // Give this op new, empty accessors, to avoid data races with
        // the other op's accessors, which might be updating on another thread.
        : mVelocity(other.mVelocity.tree())
        , mCollider(other.mCollider)
        , mInvert(other.mInvert)
        , mDynamic(other.mDynamic)
        , mInvVoxelSize(other.mInvVoxelSize)
    {}

    void operator()(const Coord& ijk, const Coord& ijkNeighbor, double& rhs, double& diag) const
    {
        // Voxels outside both the velocity field and the collider
        // are considered to be empty (unless the collider is inverted).
        // Voxels outside the velocity field and inside the collider
        // are considered to be solid.
        if (mCollider.isValueOn(ijkNeighbor) == mInvert) {
            // The exterior neighbor is empty (i.e., zero), so just adjust the center weight.
            diag -= 1;
        } else {
            const VectorType& velocity = mVelocity.getValue(ijkNeighbor);
            double delta = 0.0;
            if (mDynamic) { // exterior neighbor is a solid obstacle with nonzero velocity
                const openvdb::Vec3d colliderVelocity(mCollider.getValue(ijkNeighbor));
                if (ijkNeighbor[0] < ijk[0]) { delta +=  velocity[0] - colliderVelocity[0]; }
                if (ijkNeighbor[0] > ijk[0]) { delta -= (velocity[0] - colliderVelocity[0]); }
                if (ijkNeighbor[1] < ijk[1]) { delta +=  velocity[1] - colliderVelocity[1]; }
                if (ijkNeighbor[1] > ijk[1]) { delta -= (velocity[1] - colliderVelocity[1]); }
                if (ijkNeighbor[2] < ijk[2]) { delta +=  velocity[2] - colliderVelocity[2]; }
                if (ijkNeighbor[2] > ijk[2]) { delta -= (velocity[2] - colliderVelocity[2]); }
            } else { // exterior neighbor is a stationary solid obstacle
                if (ijkNeighbor[0] < ijk[0]) { delta += velocity[0]; }
                if (ijkNeighbor[0] > ijk[0]) { delta -= velocity[0]; }
                if (ijkNeighbor[1] < ijk[1]) { delta += velocity[1]; }
                if (ijkNeighbor[1] > ijk[1]) { delta -= velocity[1]; }
                if (ijkNeighbor[2] < ijk[2]) { delta += velocity[2]; }
                if (ijkNeighbor[2] > ijk[2]) { delta -= velocity[2]; }
            }
            rhs += delta * mInvVoxelSize;
            // Note: no adjustment to the center weight (diag).
        }
    }

private:
    // Disable assignment (due to const members).
    ColliderBoundaryOp& operator=(const ColliderBoundaryOp&);

    typename VelocityGridType::ConstAccessor mVelocity; // accessor to the velocity grid
    ColliderAccessorType mCollider; // accessor to the collider
    const bool mInvert; // invert the collider?
    const bool mDynamic; // is the collider moving?
    const double mInvVoxelSize;
}; // class ColliderBoundaryOp

//}


////////////////////////////////////////


/// @brief Main solver routine
template<typename VectorGridType, typename ColliderGridType, typename BoundaryOpType>
inline bool
removeDivergenceWithColliderGrid(SolverParms& parms, const BoundaryOpType& boundaryOp)
{
    using VectorTreeType = typename VectorGridType::TreeType;
    using VectorLeafNodeType = typename VectorTreeType::LeafNodeType;
    using VectorType = typename VectorGridType::ValueType;
    using VectorElementType = typename VectorType::ValueType;

    using ScalarGrid = typename VectorGridType::template ValueConverter<VectorElementType>::Type;
    using ScalarTree = typename ScalarGrid::TreeType;

    using MaskGridType = typename VectorGridType::template ValueConverter<bool>::Type;

    VectorGridType& velocityGrid = static_cast<VectorGridType&>(*parms.velocityGrid);

    const bool staggered = ((velocityGrid.getGridClass() == openvdb::GRID_STAGGERED)
        && (openvdb::VecTraits<VectorType>::Size == 3));

    // Compute the divergence of the incoming velocity field.
    /// @todo Consider neighboring collider velocities at border voxels?
    openvdb::tools::Divergence<VectorGridType> divergenceOp(velocityGrid);
    typename ScalarGrid::ConstPtr divGrid = divergenceOp.process();

    parms.outputState = openvdb::math::pcg::terminationDefaults<VectorElementType>();
    parms.outputState.iterations = parms.iterations;
    parms.outputState.absoluteError = (parms.absoluteError >= 0.0 ?
        parms.absoluteError : DEFAULT_MAX_ERROR);
    parms.outputState.relativeError = 0.0;

    using PCT = openvdb::math::pcg::JacobiPreconditioner<openvdb::tools::poisson::LaplacianMatrix>;

    // Solve for pressure using Poisson's equation.
    typename ScalarTree::Ptr pressure;
    if (parms.colliderType == CT_NONE) {
        pressure = openvdb::tools::poisson::solveWithBoundaryConditionsAndPreconditioner<PCT>(
            divGrid->tree(), boundaryOp, parms.outputState, *parms.interrupter, staggered);
    } else {
        // Create a domain mask by clipping the velocity grid's topology against the collider's.
        // Pressure will be computed only where the domain mask is active.
        MaskGridType* domainMaskGrid = new MaskGridType(*divGrid); // match input grid's topology
        parms.domainMaskGrid.reset(domainMaskGrid);
        if (parms.colliderType == CT_BBOX) {
            if (parms.invertCollider) {
                // Solve for pressure only outside the bounding box.
                const openvdb::CoordBBox colliderISBBox =
                    velocityGrid.transform().worldToIndexNodeCentered(parms.colliderBBox);
                domainMaskGrid->fill(colliderISBBox, false, false);
            } else {
                // Solve for pressure only inside the bounding box.
                domainMaskGrid->clipGrid(parms.colliderBBox);
            }
        } else {
            const ColliderGridType& colliderGrid =
                static_cast<const ColliderGridType&>(*parms.colliderGrid);
            if (parms.invertCollider) {
                // Solve for pressure only inside the collider.
                domainMaskGrid->topologyIntersection(colliderGrid);
            } else {
                // Solve for pressure only outside the collider.
                domainMaskGrid->topologyDifference(colliderGrid);
            }
        }
        pressure = openvdb::tools::poisson::solveWithBoundaryConditionsAndPreconditioner<PCT>(
            divGrid->tree(), domainMaskGrid->tree(), boundaryOp, parms.outputState,
                *parms.interrupter, staggered);
    }

    // Store the computed pressure grid.
    parms.pressureGrid = ScalarGrid::create(pressure);
    parms.pressureGrid->setTransform(velocityGrid.transform().copy());
    {
        std::string name = parms.velocityGrid->getName();
        if (!name.empty()) name += "_";
        name += "pressure";
        parms.pressureGrid->setName(name);
    }

    // Compute the gradient of the pressure.
    openvdb::tools::Gradient<ScalarGrid> gradientOp(static_cast<ScalarGrid&>(*parms.pressureGrid));
    typename VectorGridType::Ptr gradientOfPressure = gradientOp.process();

    // Compute pressure projection in parallel over leaf nodes.
    {
        // Pressure (and therefore the gradient of the pressure) is computed only where
        // the domain mask is active, but the gradient and velocity grid topologies must match
        // so that pressure projection can be computed in parallel over leaf nodes (see below).
        velocityGrid.tree().voxelizeActiveTiles();
        gradientOfPressure->topologyUnion(velocityGrid);
        gradientOfPressure->topologyIntersection(velocityGrid);
        openvdb::tools::pruneInactive(gradientOfPressure->tree());

        std::vector<VectorLeafNodeType*> velNodes;
        velocityGrid.tree().getNodes(velNodes);

        std::vector<const VectorLeafNodeType*> gradNodes;
        gradNodes.reserve(velNodes.size());
        gradientOfPressure->tree().getNodes(gradNodes);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, velNodes.size()),
            PressureProjectionOp<VectorTreeType>(parms, &velNodes[0], &gradNodes[0], staggered));
    }

    if (parms.colliderType != CT_NONE) {
        // When obstacles are present, the Poisson solve returns a divergence-free
        // velocity field in the interior of the input grid, but border voxels
        // need to be adjusted manually to match neighboring collider velocities.
        CorrectCollisionVelocityOp<VectorGridType> op(parms);
        if (parms.colliderType == CT_BBOX) {
            op.correctVelocity(BBoxConstAccessor(parms));
        } else {
            UTvdbProcessTypedGridTopology(
                UTvdbGetGridType(*parms.colliderGrid), *parms.colliderGrid, op);
        }
    }

    return parms.outputState.success;
}


/// @brief Main solver routine in the case of no collider or a bounding box collider
template<typename VectorGridType, typename BoundaryOpType>
inline bool
removeDivergence(SolverParms& parms, const BoundaryOpType& boundaryOp)
{
    return removeDivergenceWithColliderGrid<VectorGridType, VectorGridType>(parms, boundaryOp);
}


/// @brief Functor to invoke the solver with a collider velocity grid of arbitrary vector type
template<typename VelocityGridType>
struct ColliderDispatchOp
{
    SolverParms* parms;
    bool success;

    explicit ColliderDispatchOp(SolverParms& parms_): parms(&parms_) , success(false) {}

    template<typename ColliderGridType>
    void operator()(const ColliderGridType&)
    {
        using ColliderAccessorType = GridConstAccessor<ColliderGridType>;
        ColliderBoundaryOp<VelocityGridType, ColliderAccessorType> boundaryOp(*parms);
        success = removeDivergenceWithColliderGrid<VelocityGridType, ColliderGridType>(
            *parms, boundaryOp);
    }
}; // struct ColliderDispatchOp


/// @brief Invoke the solver for collider inputs of various types (or no collider).
template<typename VelocityGridType>
inline bool
processGrid(SolverParms& parms)
{
    bool success = false;
    switch (parms.colliderType) {
        case CT_NONE:
            // No collider
            success = removeDivergence<VelocityGridType>(
                parms, openvdb::tools::poisson::DirichletBoundaryOp<double>());
            break;
        case CT_BBOX:
            // If collider geometry was supplied, the faces of its bounding box
            // define solid obstacles.
            success = removeDivergence<VelocityGridType>(parms,
                ColliderBoundaryOp<VelocityGridType, BBoxConstAccessor>(parms));
            break;
        case CT_STATIC:
            // If a static collider grid was supplied, its active voxels define solid obstacles.
            success = removeDivergenceWithColliderGrid<VelocityGridType, ColliderMaskGrid>(
                parms, ColliderBoundaryOp<VelocityGridType, ColliderMaskAccessor>(parms));
            break;
        case CT_DYNAMIC:
        {
            // If a dynamic collider grid was supplied, its active values define
            // the velocities of solid obstacles.
            ColliderDispatchOp<VelocityGridType> op(parms);
            success = UTvdbProcessTypedGridVec3(
                UTvdbGetGridType(*parms.colliderGrid), *parms.colliderGrid, op);
            if (success) success = op.success;
            break;
        }
    }
    return success;
}


/// @brief Return the given VDB primitive's name in the form "N (NAME)",
/// where N is the primitive's index and NAME is the grid name.
/// @todo Use the VdbPrimCIterator method once it is adopted into the HDK.
inline UT_String
getPrimitiveIndexAndName(const GU_PrimVDB* prim)
{
    UT_String result(UT_String::ALWAYS_DEEP);
    if (prim != nullptr) {
        result.itoa(prim->getMapIndex());
        UT_String name = prim->getGridName();
        result += (" (" + name.toStdString() + ")").c_str();
    }
    return result;
}


inline std::string
joinNames(UT_StringArray& names, const char* lastSep = " and ", const char* sep = ", ")
{
    names.sort();
    UT_String joined;
    names.join(sep, lastSep, joined);
    return "VDB" + (((names.size() == 1) ? " " : "s ") + joined.toStdString());
}

} // unnamed namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Remove_Divergence::Cache::cookVDBSop(
    OP_Context& context)
{
    try {
        const GU_Detail* colliderGeo = inputGeo(1);

        const fpreal time = context.getTime();

        hvdb::Interrupter interrupter("Removing divergence");

        SolverParms parms;
        parms.interrupter = &interrupter;
        parms.iterations = (!evalInt("useiterations", 0, time) ?
            DEFAULT_MAX_ITERATIONS : static_cast<int>(evalInt("iterations", 0, time)));
        parms.absoluteError = (!evalInt("usetolerance", 0, time) ?
            -1.0 : evalFloat("tolerance", 0, time));
        parms.invertCollider = evalInt("invertcollider", 0, time);

        UT_String groupStr;
        evalString(groupStr, "group", 0, time);
        const GA_PrimitiveGroup* group = matchGroup(*gdp, groupStr.toStdString());

        const bool outputPressure = evalInt("pressure", 0, time);

        const bool useCollider = evalInt("usecollider", 0, time);
        const auto colliderTypeStr = evalStdString("collidertype", time);

        UT_StringArray xformMismatchGridNames, nonuniformGridNames;

        // Retrieve either a collider grid or a collider bounding box
        // (or neither) from the reference input.
        if (useCollider && colliderGeo) {
            if (colliderTypeStr == "bbox") {
                // Use the bounding box of the reference geometry as a collider.
                UT_BoundingBox box;
                colliderGeo->computeQuickBounds(box);
                parms.colliderBBox.min() = openvdb::Vec3d(box.xmin(), box.ymin(), box.zmin());
                parms.colliderBBox.max() = openvdb::Vec3d(box.xmax(), box.ymax(), box.zmax());
                parms.colliderType = CT_BBOX;
            } else {
                // Retrieve the collider grid.
                UT_String colliderStr;
                evalString(colliderStr, "collider", 0, time);
                const GA_PrimitiveGroup* colliderGroup = parsePrimitiveGroups(
                    colliderStr.buffer(), GroupCreator(colliderGeo));
                if (hvdb::VdbPrimCIterator colliderIt =
                    hvdb::VdbPrimCIterator(colliderGeo, colliderGroup))
                {
                    if (colliderIt->getConstGrid().getGridClass() == openvdb::GRID_LEVEL_SET) {
                        // If the collider grid is a level set, extract an interior mask from it.
                        LevelSetMaskOp op;
                        if (GEOvdbProcessTypedGridScalar(**colliderIt, op)) {
                            parms.colliderGrid = op.outputGrid;
                        }
                    }
                    if (!parms.colliderGrid) {
                        parms.colliderGrid = colliderIt->getConstGridPtr();
                    }
                    if (parms.colliderGrid
                        && !parms.colliderGrid->constTransform().hasUniformScale())
                    {
                        nonuniformGridNames.append(getPrimitiveIndexAndName(*colliderIt));
                    }
                    if (++colliderIt) {
                        addWarning(SOP_MESSAGE, ("found multiple collider VDBs; using VDB "
                            + getPrimitiveIndexAndName(*colliderIt).toStdString()).c_str());
                    }
                }
                if (!parms.colliderGrid) {
                    if (colliderStr.isstring()) {
                        addError(SOP_MESSAGE,
                            ("collider \"" + colliderStr.toStdString() + "\" not found").c_str());
                    } else {
                        addError(SOP_MESSAGE, "collider VDB not found");
                    }
                    return error();
                }
                if (parms.colliderGrid->empty()) {
                    // An empty collider grid was found; ignore it.
                    parms.colliderGrid.reset();
                }
                if (parms.colliderGrid) {
                    const bool isVec3Grid =
                        (3 == UTvdbGetGridTupleSize(UTvdbGetGridType(*parms.colliderGrid)));
                    if (isVec3Grid && (colliderTypeStr == "dynamic")) {
                        // The collider grid is vector-valued.  Its active values
                        // are the velocities of moving obstacles.
                        parms.colliderType = CT_DYNAMIC;
                    } else {
                        // The active voxels of the collider grid define stationary,
                        // solid obstacles.  Extract a topology mask of those voxels.
                        parms.colliderType = CT_STATIC;
                        ColliderMaskOp op;
                        op.mask = ColliderMaskGrid::create();
                        UTvdbProcessTypedGridTopology(UTvdbGetGridType(*parms.colliderGrid),
                            *parms.colliderGrid, op);
                        parms.colliderGrid = op.mask;
                    }
                }
            }
        }

        int numGridsProcessed = 0;
        std::ostringstream infoStrm;

        // Main loop
        for (hvdb::VdbPrimIterator vdbIt(gdp, group); vdbIt; ++vdbIt) {

            if (interrupter.wasInterrupted()) break;

            const UT_VDBType velocityType = vdbIt->getStorageType();
            if (velocityType == UT_VDB_VEC3F || velocityType == UT_VDB_VEC3D) {
                // Found a vector-valued input grid.
                ++numGridsProcessed;

                vdbIt->makeGridUnique(); // ensure that the grid's tree is not shared
                parms.velocityGrid = vdbIt->getGridPtr();

                const openvdb::math::Transform& xform = parms.velocityGrid->constTransform();

                if (!xform.hasUniformScale()) {
                    nonuniformGridNames.append(getPrimitiveIndexAndName(*vdbIt));
                }
                if (parms.colliderGrid && (parms.colliderGrid->constTransform() != xform)) {
                    // The velocity and collider grid transforms need to match.
                    xformMismatchGridNames.append(getPrimitiveIndexAndName(*vdbIt));
                }

                // Remove divergence.
                bool success = false;
                if (velocityType == UT_VDB_VEC3F) {
                    success = processGrid<openvdb::Vec3SGrid>(parms);
                } else if (velocityType == UT_VDB_VEC3D) {
                    success = processGrid<openvdb::Vec3DGrid>(parms);
                }

                if (!success) {
                    std::ostringstream errStrm;
                    errStrm << "solver failed to converge for VDB "
                        << getPrimitiveIndexAndName(*vdbIt).c_str()
                        << " with error " << parms.outputState.absoluteError;
                    addWarning(SOP_MESSAGE, errStrm.str().c_str());
                } else {
                    if (outputPressure && parms.pressureGrid) {
                        hvdb::createVdbPrimitive(*gdp, parms.pressureGrid);
                    }
                    if (numGridsProcessed > 1) infoStrm << "\n";
                    infoStrm << "solver converged for VDB "
                        << getPrimitiveIndexAndName(*vdbIt).c_str()
                        << " in " << parms.outputState.iterations << " iteration"
                            << (parms.outputState.iterations == 1 ? "" : "s")
                        << " with error " << parms.outputState.absoluteError;
                }
            }
            parms.velocityGrid.reset();
        }

        if (!interrupter.wasInterrupted()) {
            // Report various issues.
            if (numGridsProcessed == 0) {
                addWarning(SOP_MESSAGE, "found no floating-point vector VDBs");
            } else {
                if (nonuniformGridNames.size() > 0) {
                    const std::string names = joinNames(nonuniformGridNames);
                    addWarning(SOP_MESSAGE,
                        ((names + ((nonuniformGridNames.size() == 1) ? " has" : " have"))
                        + " nonuniform voxels and should be resampled").c_str());
                }
                if (xformMismatchGridNames.size() > 0) {
                    const std::string names = joinNames(xformMismatchGridNames, " or ");
                    addWarning(SOP_MESSAGE,
                        ("vector field and collider transforms don't match for " + names).c_str());
                }
                const std::string info = infoStrm.str();
                if (!info.empty()) {
                    addMessage(SOP_MESSAGE, info.c_str());
                }
            }
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
