// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Advect_Points.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief SOP to perform advection of points through a velocity field.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/PointUtils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/PointAdvect.h>
#include <openvdb/points/PointAdvect.h>

#include <GA/GA_PageIterator.h>
#include <GU/GU_PrimPoly.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_UniquePtr.h>

#include <hboost/algorithm/string/case_conv.hpp>
#include <hboost/algorithm/string/trim.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////

// Utilities

namespace {

// Add new items to the *end* of this list, and update NUM_PROPAGATION_TYPES.
enum PropagationType {
    PROPAGATION_TYPE_UNKNOWN = -1,
    PROPAGATION_TYPE_ADVECTION = 0,
    PROPAGATION_TYPE_PROJECTION,
    PROPAGATION_TYPE_CONSTRAINED_ADVECTION
};

enum { NUM_PROPAGATION_TYPES = PROPAGATION_TYPE_CONSTRAINED_ADVECTION + 1 };

std::string
propagationTypeToString(PropagationType pt)
{
    std::string ret;
    switch (pt) {
        case PROPAGATION_TYPE_ADVECTION:             ret = "advection";  break;
        case PROPAGATION_TYPE_PROJECTION:            ret = "projection"; break;
        case PROPAGATION_TYPE_CONSTRAINED_ADVECTION: ret = "cadvection"; break;
        case PROPAGATION_TYPE_UNKNOWN:               ret = "unknown";    break;
    }
    return ret;
}

std::string
propagationTypeToMenuName(PropagationType pt)
{
    std::string ret;
    switch (pt) {
        case PROPAGATION_TYPE_ADVECTION:             ret = "Advection";              break;
        case PROPAGATION_TYPE_PROJECTION:            ret = "Projection";             break;
        case PROPAGATION_TYPE_CONSTRAINED_ADVECTION: ret = "Constrained Advection";  break;
        case PROPAGATION_TYPE_UNKNOWN:               ret = "Unknown";                break;
    }
    return ret;
}

PropagationType
stringToPropagationType(const std::string& s)
{
    PropagationType ret = PROPAGATION_TYPE_UNKNOWN;

    std::string str = s;
    hboost::trim(str);
    hboost::to_lower(str);

    if (str == propagationTypeToString(PROPAGATION_TYPE_ADVECTION)) {
        ret = PROPAGATION_TYPE_ADVECTION;
    } else if (str == propagationTypeToString(PROPAGATION_TYPE_PROJECTION)) {
        ret = PROPAGATION_TYPE_PROJECTION;
    } else if (str == propagationTypeToString(PROPAGATION_TYPE_CONSTRAINED_ADVECTION)) {
        ret = PROPAGATION_TYPE_CONSTRAINED_ADVECTION;
    }

    return ret;
}


// Add new items to the *end* of this list, and update NUM_INTEGRATION_TYPES.
enum IntegrationType {
    INTEGRATION_TYPE_UNKNOWN = -1,
    INTEGRATION_TYPE_FWD_EULER = 0,
    INTEGRATION_TYPE_RK_2ND,
    INTEGRATION_TYPE_RK_3RD,
    INTEGRATION_TYPE_RK_4TH
};

enum { NUM_INTEGRATION_TYPES = INTEGRATION_TYPE_RK_4TH + 1 };

std::string
integrationTypeToString(IntegrationType it)
{
    std::string ret;
    switch (it) {
        case INTEGRATION_TYPE_FWD_EULER:    ret = "fwd euler";  break;
        case INTEGRATION_TYPE_RK_2ND:       ret = "2nd rk";     break;
        case INTEGRATION_TYPE_RK_3RD:       ret = "3rd rk";     break;
        case INTEGRATION_TYPE_RK_4TH:       ret = "4th rk";     break;
        case INTEGRATION_TYPE_UNKNOWN:      ret = "unknown";    break;
    }
    return ret;
}

std::string
integrationTypeToMenuName(IntegrationType it)
{
    std::string ret;
    switch (it) {
        case INTEGRATION_TYPE_FWD_EULER:    ret = "Forward Euler";            break;
        case INTEGRATION_TYPE_RK_2ND:       ret = "Second-Order Runge-Kutta"; break;
        case INTEGRATION_TYPE_RK_3RD:       ret = "Third-Order Runge-Kutta";  break;
        case INTEGRATION_TYPE_RK_4TH:       ret = "Fourth-Order Runge-Kutta"; break;
        case INTEGRATION_TYPE_UNKNOWN:      ret = "Unknown";                  break;
    }
    return ret;
}

IntegrationType
stringToIntegrationType(const std::string& s)
{
    IntegrationType ret = INTEGRATION_TYPE_UNKNOWN;

    std::string str = s;
    hboost::trim(str);
    hboost::to_lower(str);

    if (str == integrationTypeToString(INTEGRATION_TYPE_FWD_EULER)) {
        ret = INTEGRATION_TYPE_FWD_EULER;
    } else if (str == integrationTypeToString(INTEGRATION_TYPE_RK_2ND)) {
        ret = INTEGRATION_TYPE_RK_2ND;
    } else if (str == integrationTypeToString(INTEGRATION_TYPE_RK_3RD)) {
        ret = INTEGRATION_TYPE_RK_3RD;
    } else if (str == integrationTypeToString(INTEGRATION_TYPE_RK_4TH)) {
        ret = INTEGRATION_TYPE_RK_4TH;
    }

    return ret;
}


struct AdvectionParms
{
    AdvectionParms(GU_Detail *outputGeo)
        : mOutputGeo(outputGeo)
        , mPointGeo(nullptr)
        , mPointGroup(nullptr)
        , mOffsetsToSkip()
        , mIncludeGroups()
        , mExcludeGroups()
        , mVelPrim(nullptr)
        , mCptPrim(nullptr)
        , mPropagationType(PROPAGATION_TYPE_ADVECTION)
        , mIntegrationType(INTEGRATION_TYPE_FWD_EULER)
        , mTimeStep(1.0)
        , mIterations(1)
        , mSteps(1)
        , mStaggered(false)
        , mStreamlines(false)
    {
    }

    GU_Detail* mOutputGeo;
    const GU_Detail* mPointGeo;
    const GA_PointGroup* mPointGroup;
    std::vector<GA_Offset> mOffsetsToSkip;
    std::vector<std::string> mIncludeGroups;
    std::vector<std::string> mExcludeGroups;
    const GU_PrimVDB *mVelPrim;
    const GU_PrimVDB *mCptPrim;
    PropagationType mPropagationType;
    IntegrationType mIntegrationType;
    double mTimeStep;
    int mIterations, mSteps;
    bool mStaggered, mStreamlines;
};


/// @brief Creates a new line segment for each point in @c ptnGeo
/// @note The lines will only have one node.
void
createNewLines(GU_Detail& geo, const GA_PointGroup* group)
{
    GA_SplittableRange ptnRange(geo.getPointRange(group));
    GA_Offset start, end, pt;

    for (GA_PageIterator pIt = ptnRange.beginPages(); !pIt.atEnd(); ++pIt) {
        for (GA_Iterator it(pIt.begin()); it.blockAdvance(start, end); ) {
            for (GA_Offset i = start; i < end; ++i) {

                pt = geo.appendPointOffset();
                geo.setPos3(pt, geo.getPos3(i));

                GU_PrimPoly& prim = *GU_PrimPoly::build(&geo, 0, GU_POLY_OPEN, 0);
                prim.appendVertex(pt);
            }
        }
    }
}


/// @brief Append a new node to each line.
/// @note The numbers of lines and points have to match.
void
appendLineNodes(GU_Detail& geo, GA_Size firstline, const GU_Detail& ptnGeo)
{
    GA_SplittableRange ptnRange(ptnGeo.getPointRange());
    GA_Offset start, end, pt;

    GA_Size n = firstline, N = geo.getNumPrimitives();


    for (GA_PageIterator pIt = ptnRange.beginPages(); !pIt.atEnd(); ++pIt) {
        for (GA_Iterator it(pIt.begin()); it.blockAdvance(start, end); ) {
            for (GA_Offset i = start; i < end; ++i) {

                pt = geo.appendPointOffset();
                geo.setPos3(pt, ptnGeo.getPos3(i));

                GA_Offset offset = geo.primitiveOffset(n);
                GU_PrimPoly& prim = *static_cast<GU_PrimPoly*>(
                    geo.getPrimitiveList().get(offset));

                prim.appendVertex(pt);

                if (++n == N) break;
            }
            if (n == N) break;
        }
        if (n == N) break;
    }
}


// Threaded closest point projection
template<typename GridType>
class ProjectionOp
{
    using ProjectorType = openvdb::tools::ClosestPointProjector<GridType>;
    using VectorType = typename GridType::ValueType;
    using ElementType = typename VectorType::ValueType;

public:

    ProjectionOp(const GridType& cptGrid, int cptIterations, GU_Detail& geo,
        const std::vector<GA_Offset>& offsetsToSkip, hvdb::Interrupter& boss)
        : mProjector(cptGrid, cptIterations)
        , mGeo(geo)
        , mOffsetsToSkip(offsetsToSkip)
        , mBoss(boss)
    {
    }

    void operator()(const GA_SplittableRange &range) const
    {
        GA_Offset start, end;
        UT_Vector3 p;
        VectorType w;

        for (GA_PageIterator pIt = range.beginPages(); !pIt.atEnd(); ++pIt) {

            if (mBoss.wasInterrupted()) return;

            for (GA_Iterator it(pIt.begin()); it.blockAdvance(start, end); ) {

                if (mBoss.wasInterrupted()) return;

                for (GA_Offset i = start; i < end; ++i) {

                    // skip any offsets requested
                    if (std::binary_search(mOffsetsToSkip.begin(), mOffsetsToSkip.end(), i)) {
                        continue;
                    }

                    p = mGeo.getPos3(i);
                    w[0] = ElementType(p[0]);
                    w[1] = ElementType(p[1]);
                    w[2] = ElementType(p[2]);

                    mProjector.projectToConstraintSurface(w);

                    p[0] = UT_Vector3::value_type(w[0]);
                    p[1] = UT_Vector3::value_type(w[1]);
                    p[2] = UT_Vector3::value_type(w[2]);
                    mGeo.setPos3(i, p);
                }
            }
        }
    }

private:
    ProjectorType mProjector;
    GU_Detail& mGeo;
    const std::vector<GA_Offset>& mOffsetsToSkip;
    hvdb::Interrupter& mBoss;
};


class Projection
{
public:
    Projection(AdvectionParms& parms, hvdb::Interrupter& boss)
        : mParms(parms)
        , mBoss(boss)
    {
    }

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        if (mBoss.wasInterrupted()) return;

        ProjectionOp<GridType> op(
            grid, mParms.mIterations, *mParms.mOutputGeo, mParms.mOffsetsToSkip, mBoss);
        UTparallelFor(GA_SplittableRange(mParms.mOutputGeo->getPointRange(mParms.mPointGroup)), op);
    }

private:
    AdvectionParms& mParms;
    hvdb::Interrupter& mBoss;
};


// Threaded point advection
template<typename GridType, int IntegrationOrder, bool StaggeredVelocity, bool Constrained = false>
class AdvectionOp
{
    using IntegrationType = openvdb::tools::VelocityIntegrator<GridType, StaggeredVelocity>;
    using ProjectorType =
        openvdb::tools::ClosestPointProjector<GridType>; // Used for constrained advection

    using VectorType = typename GridType::ValueType;
    using ElementType = typename VectorType::ValueType;

public:

    AdvectionOp(const GridType& velocityGrid, GU_Detail& geo,
        const std::vector<GA_Offset>& offsetsToSkip, hvdb::Interrupter& boss,
        double timeStep, GA_ROHandleF traillen, int steps)
        : mVelocityGrid(velocityGrid)
        , mCptGrid(nullptr)
        , mGeo(geo)
        , mOffsetsToSkip(offsetsToSkip)
        , mBoss(boss)
        , mTimeStep(timeStep)
        , mTrailLen(traillen)
        , mSteps(steps)
        , mCptIterations(0)
    {
    }

    AdvectionOp(const GridType& velocityGrid, const GridType& cptGrid, GU_Detail& geo,
        const std::vector<GA_Offset>& offsetsToSkip, hvdb::Interrupter& boss,
        double timeStep, int steps, int cptIterations)
        : mVelocityGrid(velocityGrid)
        , mCptGrid(&cptGrid)
        , mGeo(geo)
        , mOffsetsToSkip(offsetsToSkip)
        , mBoss(boss)
        , mTimeStep(timeStep)
        , mSteps(steps)
        , mCptIterations(cptIterations)
    {
    }

    void operator()(const GA_SplittableRange &range) const
    {
        GA_Offset start, end;
        UT_Vector3 p;
        VectorType w;

        IntegrationType integrator(mVelocityGrid);

        // Constrained-advection compiled out if Constrained == false
        UT_UniquePtr<ProjectorType> projector;
        if (Constrained && mCptGrid != nullptr) {
            projector.reset(new ProjectorType(*mCptGrid, mCptIterations));
        }

        for (GA_PageIterator pIt = range.beginPages(); !pIt.atEnd(); ++pIt) {

            if (mBoss.wasInterrupted()) return;

            for (GA_Iterator it(pIt.begin()); it.blockAdvance(start, end); ) {

                if (mBoss.wasInterrupted()) return;

                for (GA_Offset i = start; i < end; ++i) {

                    // skip any point offsets requested
                    if (std::binary_search(mOffsetsToSkip.begin(), mOffsetsToSkip.end(), i)) {
                        continue;
                    }

                    p = mGeo.getPos3(i);
                    w[0] = ElementType(p[0]);
                    w[1] = ElementType(p[1]);
                    w[2] = ElementType(p[2]);

                    ElementType timestep = static_cast<ElementType>(mTimeStep);
                    if (mTrailLen.isValid()) {
                        timestep *= static_cast<ElementType>(mTrailLen.get(i));
                    }

                    for (int n = 0; n < mSteps; ++n) {
                        integrator.template rungeKutta<IntegrationOrder, VectorType>(timestep, w);

                        if (Constrained) projector->projectToConstraintSurface(w);
                    }

                    p[0] = UT_Vector3::value_type(w[0]);
                    p[1] = UT_Vector3::value_type(w[1]);
                    p[2] = UT_Vector3::value_type(w[2]);
                    mGeo.setPos3(i, p);
                }
            }
        }
    }

private:
    const GridType& mVelocityGrid;
    const GridType* mCptGrid;
    GU_Detail& mGeo;
    const std::vector<GA_Offset>& mOffsetsToSkip;
    hvdb::Interrupter& mBoss;
    double mTimeStep;
    GA_ROHandleF mTrailLen;
    const int mSteps, mCptIterations;
};


class Advection
{
public:
    Advection(AdvectionParms& parms, hvdb::Interrupter& boss)
        : mParms(parms)
        , mBoss(boss)
    {
    }

    template<typename GridType, int IntegrationOrder, bool StaggeredVelocity>
    void advection(const GridType& velocityGrid)
    {
        if (mBoss.wasInterrupted()) return;

        if (!mParms.mStreamlines) { // Advect points

            GA_ROHandleF traillen_h(mParms.mOutputGeo, GA_ATTRIB_POINT, "traillen");

            AdvectionOp<GridType, IntegrationOrder, StaggeredVelocity>
                op(velocityGrid, *mParms.mOutputGeo, mParms.mOffsetsToSkip,
                   mBoss, mParms.mTimeStep, traillen_h, mParms.mSteps);

            UTparallelFor(
                GA_SplittableRange(mParms.mOutputGeo->getPointRange(mParms.mPointGroup)), op);

        } else { // Advect points and generate streamlines.

            GA_Index firstline = mParms.mOutputGeo->getNumPrimitives();

            GU_Detail geo;
            geo.mergePoints(*mParms.mOutputGeo, mParms.mPointGroup);

            createNewLines(*mParms.mOutputGeo, mParms.mPointGroup);

            for (int n = 0; n < mParms.mSteps; ++n) {

                if (mBoss.wasInterrupted()) return;

                GA_ROHandleF traillen_h(&geo, GA_ATTRIB_POINT, "traillen");

                AdvectionOp<GridType, IntegrationOrder, StaggeredVelocity>
                    op(velocityGrid, geo, mParms.mOffsetsToSkip, mBoss,
                       mParms.mTimeStep, traillen_h, 1);

                UTparallelFor(GA_SplittableRange(geo.getPointRange()), op);

                appendLineNodes(*mParms.mOutputGeo, firstline, geo);
            }
        }
    }

    template<typename GridType, int IntegrationOrder, bool StaggeredVelocity>
    void constrainedAdvection(const GridType& velocityGrid)
    {
        const GridType& cptGrid = static_cast<const GridType&>(mParms.mCptPrim->getGrid());
        using AdvectionOp =
            AdvectionOp<GridType, IntegrationOrder, StaggeredVelocity, /*Constrained*/true>;

        if (mBoss.wasInterrupted()) return;

        if (!mParms.mStreamlines) { // Advect points
            AdvectionOp op(velocityGrid, cptGrid, *mParms.mOutputGeo, mParms.mOffsetsToSkip,
                mBoss, mParms.mTimeStep, mParms.mSteps, mParms.mIterations);

            UTparallelFor(
                GA_SplittableRange(mParms.mOutputGeo->getPointRange(mParms.mPointGroup)), op);

        } else { // Advect points and generate streamlines.

            GA_Index firstline = mParms.mOutputGeo->getNumPrimitives();

            GU_Detail geo;
            geo.mergePoints(*mParms.mOutputGeo, mParms.mPointGroup);

            createNewLines(*mParms.mOutputGeo, mParms.mPointGroup);

            for (int n = 0; n < mParms.mSteps; ++n) {

                if (mBoss.wasInterrupted()) return;

                AdvectionOp op(velocityGrid, cptGrid, geo, mParms.mOffsetsToSkip,
                    mBoss, mParms.mTimeStep, 1, mParms.mIterations);

                UTparallelFor(GA_SplittableRange(geo.getPointRange()), op);

                appendLineNodes(*mParms.mOutputGeo, firstline, geo);
            }
        }
    }

    // Resolves velocity representation and advection type
    template<typename GridType, int IntegrationOrder>
    void resolveAdvection(const GridType& velocityGrid)
    {
        if (mBoss.wasInterrupted()) return;

        if (mParms.mPropagationType == PROPAGATION_TYPE_ADVECTION) {
            if (!mParms.mStaggered) advection<GridType, IntegrationOrder, false>(velocityGrid);
            else advection<GridType, IntegrationOrder, true>(velocityGrid);
        } else if (mParms.mCptPrim != nullptr) { // constrained
            if (!mParms.mStaggered) {
                constrainedAdvection<GridType, IntegrationOrder, false>(velocityGrid);
            } else {
                constrainedAdvection<GridType, IntegrationOrder, true>(velocityGrid);
            }
        }
    }

    template<typename GridType>
    void operator()(const GridType& velocityGrid)
    {
        if (mBoss.wasInterrupted()) return;

        // Resolve integration order
        switch (mParms.mIntegrationType) {
            case INTEGRATION_TYPE_FWD_EULER: resolveAdvection<GridType, 1>(velocityGrid); break;
            case INTEGRATION_TYPE_RK_2ND:    resolveAdvection<GridType, 2>(velocityGrid); break;
            case INTEGRATION_TYPE_RK_3RD:    resolveAdvection<GridType, 3>(velocityGrid); break;
            case INTEGRATION_TYPE_RK_4TH:    resolveAdvection<GridType, 4>(velocityGrid); break;
            case INTEGRATION_TYPE_UNKNOWN: break;
        }
    }

private:
    AdvectionParms&    mParms;
    hvdb::Interrupter& mBoss;
};


template <typename PointDataGridT>
class VDBPointsAdvection
{
public:
    VDBPointsAdvection(PointDataGridT& outputGrid, AdvectionParms& parms, hvdb::Interrupter& boss)
        : mOutputGrid(outputGrid)
        , mParms(parms)
        , mBoss(boss)
    {
    }

    template<typename GridType>
    void operator()(const GridType& velocityGrid)
    {
        if (mBoss.wasInterrupted()) return;

        // note that streamlines are not implemented for VDB Points
        if (mParms.mStreamlines)    return;

        auto leaf = mOutputGrid.constTree().cbeginLeaf();
        if (!leaf)  return;

        openvdb::points::MultiGroupFilter filter(
            mParms.mIncludeGroups, mParms.mExcludeGroups, leaf->attributeSet());
        openvdb::points::advectPoints(mOutputGrid, velocityGrid,
            mParms.mIntegrationType+1, mParms.mTimeStep, mParms.mSteps, filter);
    }

private:
    PointDataGridT&    mOutputGrid;
    AdvectionParms&    mParms;
    hvdb::Interrupter& mBoss;
};

} // namespace


////////////////////////////////////////

// SOP Declaration

class SOP_OpenVDB_Advect_Points: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Advect_Points(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Advect_Points() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i ) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions
    {
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
        bool evalAdvectionParms(OP_Context&, AdvectionParms&);
    }; // class Cache

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

    hutil::ParmList parms;

    // Points to process
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Point Group")
        .setChoiceList(&SOP_Node::pointGroupMenu)
        .setTooltip("A subset of points in the first input to move using the velocity field."));

    // VDB Points advection
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "advectvdbpoints", "Advect VDB Points")
        .setDefault(PRMoneDefaults)
        .setTooltip("Enable/disable advection of VDB Points.")
        .setDocumentation(
            "If enabled, advect the points in a VDB Points grid, otherwise apply advection"
            " only to the Houdini point associated with the VDB primitive.\n\n"
            "The latter is faster to compute but updates the VDB transform only"
            " and not the relative positions of the points within the grid."
            " It is useful primarily when instancing multiple static VDB point sets"
            " onto a dynamically advected Houdini point set."));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbgroup", "VDB Primitive Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("A subset of VDB Points primitives in the first input to move using the velocity field."));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbpointsgroups", "VDB Points Groups")
        .setHelpText("Specify VDB Points groups to advect.")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1));

    // Velocity grid
    parms.add(hutil::ParmFactory(PRM_STRING, "velgroup", "Velocity VDB")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("Velocity grid")
        .setDocumentation(
            "The name of a VDB primitive in the second input to use as"
            " the velocity field (see [specifying volumes|/model/volumes#group])\n\n"
            "This must be a vector-valued VDB primitive."
            " You can use the [Vector Merge node|Node:sop/DW_OpenVDBVectorMerge]"
            " to turn a `vel.[xyz]` triple into a single primitive."));

    // Closest point grid
    parms.add(hutil::ParmFactory(PRM_STRING, "cptgroup", "Closest-Point VDB")
        .setChoiceList(&hutil::PrimGroupMenuInput3)
        .setTooltip("Vector grid that in each voxel stores the closest point on a surface.")
        .setDocumentation(
            "The name of a VDB primitive in the third input to use for"
            " the closest point values (see [specifying volumes|/model/volumes#group])"));

    // Propagation scheme
    {
        std::vector<std::string> items;
        for (int i = 0; i < NUM_PROPAGATION_TYPES; ++i) {
            PropagationType pt = PropagationType(i);
            items.push_back(propagationTypeToString(pt)); // token
            items.push_back(propagationTypeToMenuName(pt)); // label
        }

        parms.add(hutil::ParmFactory(PRM_STRING, "operation", "Operation")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(items[0])
            .setTooltip(
                "Advection: Move the point along the velocity field.\n"
                "Projection: Move the point to the nearest surface point.\n"
                "Constrained Advection: Advect, then project to the nearest surface point.")
            .setDocumentation(
                "How to use the velocity field to move the points\n\n"
                "Advection:\n"
                "    Move each point along the velocity field.\n"
                "Projection:\n"
                "    Move each point to the nearest surface point using the closest point field.\n"
                "Constrained Advection:\n"
                "    Move the along the velocity field, and then project using the"
                "    closest point field. This forces the particles to remain on a surface."));
    }

    // Integration scheme
    {
        std::vector<std::string> items;
        for (int i = 0; i < NUM_INTEGRATION_TYPES; ++i) {
            IntegrationType it = IntegrationType(i);
            items.push_back(integrationTypeToString(it)); // token
            items.push_back(integrationTypeToMenuName(it)); // label
        }

        parms.add(hutil::ParmFactory(PRM_STRING, "integration", "Integration")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDefault(items[0])
            .setTooltip("Lower order means faster performance, "
                "but the points will not follow the velocity field closely.")
            .setDocumentation("Algorithm to use to move the points\n\n"
                "Later options in the list are slower but better follow the velocity field."));
    }

    // Closest point iterations
    parms.add(hutil::ParmFactory(PRM_INT_J, "iterations", "Iterations")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("The interpolation step when sampling nearest points introduces\n"
            "error so that the result of a single sample may not lie exactly\n"
            "on the surface. Multiple iterations help minimize this error.")
        .setDocumentation(
            "Number of times to try projecting to the nearest point on the surface\n\n"
            "Projecting might not move exactly to the surface on the first try."
            " More iterations are slower but give more accurate projection."));

    // Time step
    parms.add(hutil::ParmFactory(PRM_FLT, "timestep", "Timestep")
        .setDefault(1, "1.0/$FPS")
        .setRange(PRM_RANGE_UI, 0, PRM_RANGE_UI, 10)
        .setDocumentation(
            "Number of seconds of movement to apply to the input points\n\n"
            "The default is `1/$FPS` (one frame's worth of time)."
            " You can use negative values to move the points backwards through"
            " the velocity field.\n\n"
            "If the attribute `traillen` is present, it is multiplied by this"
            " time step allowing per-particle variation in trail length."));

    // Steps
    parms.add(hutil::ParmFactory(PRM_INT_J, "steps", "Substeps")
        .setDefault(1)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip("Number of timesteps to take per frame.")
        .setDocumentation(
            "How many times to repeat the advection step\n\n"
            "This will produce a more accurate motion, especially if large"
            " time steps or high velocities are present."));

    // Output streamlines
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "outputstreamlines", "Output Streamlines")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Output the particle path as line segments.")
        .setDocumentation(
            "Generate polylines instead of moving points.\n\n"
            "This is useful for visualizing the effect of the node."
            " It may also be useful for special effects (see also the"
            " [Trail SOP|Node:sop/trail])."));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "staggered", "Staggered Velocities"));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", "Sep"));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep2", "Sep"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "ptnGroup", "Point Group"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "velGroup", "Velocity VDB"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "cptGroup", "Closest-Point VDB"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "propagation", "Operation")
        .setDefault("advection"));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "cptIterations", "Iterations")
        .setDefault(PRMzeroDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT, "timeStep", "Time Step")
        .setDefault(1, "1.0/$FPS"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "outputStreamlines", "Output Streamlines")
        .setDefault(PRMzeroDefaults));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Advect Points",
        SOP_OpenVDB_Advect_Points::factory, parms, *table)
        .setObsoleteParms(obsoleteParms)
        .addInput("Points to Advect")
        .addOptionalInput("Velocity VDB")
        .addOptionalInput("Closest Point VDB")
        .setVerb(SOP_NodeVerb::COOK_DUPLICATE,
            []() { return new SOP_OpenVDB_Advect_Points::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Move points in the input geometry along a VDB velocity field.\"\"\"\n\
\n\
@overview\n\
\n\
This node has different functions based on the value of the __Operation__ parameter.\n\
* Move geometry points according to a VDB velocity field.\n\
* Move points onto a surface using a VDB field storing the nearest surface point at each voxel.\n\
  # Convert the \"sticky\" surface to a VDB SDF using the\n\
    [OpenVDB From Polygons node|Node:sop/DW_OpenVDBFromPolygons].\n\
  # Generate a \"nearest point\" VDB using the \n\
    [OpenVDB Analysis node|Node:sop/DW_OpenVDBAnalysis].\n\
  # Connect the points you want to stick, and the \"nearest point\" field,\n\
    into this node.\n\
* Move geometry points according to a VDB velocity field _and_ stick them\n\
  to a surface using a \"nearest point\" field (combine the first two operations).\n\
  This lets you advect points through a velocity field while keeping them\n\
  stuck to a surface.\n\
\n\
NOTE:\n\
    The `traillen` float attribute can be used to control how far particles\n\
    move on a per-particle basis.\n\
\n\
@animation Animating advection\n\
\n\
*This node is not a feedback loop*.\n\
 It moves the points it finds in the input geometry.  It _cannot_ modify\n\
 the point locations over time.  (That is, if you hook this node up to do advection\n\
 and press play, the points will not animate.)\n\
\n\
To set up a feedback loop, where the advection at each frame affects\n\
 the advected point positions from the previous frame, do one of the following:\n\
\n\
* Do the advection inside a [SOP Solver|Node:sop/solver].\n\
* Set __Substeps__ to `$F` and the __Time Step__ to `$T`\n\
\n\
  This will cause the node to recalculate, _at every frame_, the path\n\
  of every particle through _every previous frame_ to get the current one.\n\
  This is obviously not very practical, however the calculations are fast\n\
  so it may be useful as a quick \"hack\" to animate the advection\n\
  for small numbers of particles.\n\
\n\
@inputs\n\
Points to Advect:\n\
    The points to advect are copied from this input.\n\
Velocity VDB:\n\
    The VDB that stores the velocity at each location\n\
Closest Point VDB:\n\
    The VDB that stores the closest point to each location\n\
\n\
@related\n\
- [OpenVDB Advect|Node:sop/DW_OpenVDBAdvect]\n\
- [OpenVDB From Particles|Node:sop/DW_OpenVDBFromParticles]\n\
- [Node:sop/vdbadvectpoints]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Advect_Points::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "cptGroup", "cptgroup");
    resolveRenamedParm(*obsoleteParms, "cptIterations", "iterations");
    resolveRenamedParm(*obsoleteParms, "outputStreamlines", "outputstreamlines");
    resolveRenamedParm(*obsoleteParms, "propagation", "operation");
    resolveRenamedParm(*obsoleteParms, "ptnGroup", "group");
    resolveRenamedParm(*obsoleteParms, "timeStep", "timestep");
    resolveRenamedParm(*obsoleteParms, "velGroup", "velgroup");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Enable/disable or show/hide parameters in the UI.
bool
SOP_OpenVDB_Advect_Points::updateParmsFlags()
{
    bool changed = false;

    const auto op = stringToPropagationType(evalStdString("operation", 0));

    const bool advectVdbPoints = (0 != evalInt("advectvdbpoints", 0, 0));

    changed |= enableParm("iterations", op != PROPAGATION_TYPE_ADVECTION);
    changed |= enableParm("integration", op != PROPAGATION_TYPE_PROJECTION);
    changed |= enableParm("timestep", op != PROPAGATION_TYPE_PROJECTION);
    changed |= enableParm("steps", op != PROPAGATION_TYPE_PROJECTION);
    changed |= enableParm("outputstreamlines", op != PROPAGATION_TYPE_PROJECTION);
    changed |= enableParm("advectvdbpoints", op == PROPAGATION_TYPE_ADVECTION);
    changed |= enableParm("vdbgroup", (op == PROPAGATION_TYPE_ADVECTION) && advectVdbPoints);
    changed |= enableParm("vdbpointsgroups", (op == PROPAGATION_TYPE_ADVECTION) && advectVdbPoints);

    changed |= setVisibleState("iterations", getEnableState("iterations"));
    changed |= setVisibleState("integration", getEnableState("integration"));
    changed |= setVisibleState("timestep", getEnableState("timestep"));
    changed |= setVisibleState("steps", getEnableState("steps"));
    changed |= setVisibleState("outputstreamlines", getEnableState("outputstreamlines"));
    changed |= setVisibleState("advectvdbpoints", getEnableState("advectvdbpoints"));
    changed |= setVisibleState("vdbgroup", getEnableState("advectvdbpoints"));
    changed |= setVisibleState("vdbpointsgroups", getEnableState("advectvdbpoints"));

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Advect_Points::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Advect_Points(net, name, op);
}


SOP_OpenVDB_Advect_Points::SOP_OpenVDB_Advect_Points(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Advect_Points::Cache::cookVDBSop(OP_Context& context)
{
    try {
        // Evaluate UI parameters
        const fpreal now = context.getTime();

        AdvectionParms parms(gdp);
        if (!evalAdvectionParms(context, parms)) return error();

        const bool advectVdbPoints = (0 != evalInt("advectvdbpoints", 0, now));

        hvdb::Interrupter boss("Processing points");

        if (advectVdbPoints) {
            // build a list of point offsets to skip during Houdini point advection
            for (hvdb::VdbPrimIterator vdbIt(gdp); vdbIt; ++vdbIt) {
                GU_PrimVDB* vdbPrim = *vdbIt;
                parms.mOffsetsToSkip.push_back(vdbPrim->getPointOffset(0));
            }

            // ensure the offsets to skip are sorted to make lookups faster
            std::sort(parms.mOffsetsToSkip.begin(), parms.mOffsetsToSkip.end());

            const std::string vdbGroupStr = evalStdString("vdbgroup", now);
            const GA_PrimitiveGroup* vdbGroup = matchGroup(*parms.mPointGeo, vdbGroupStr);

            for (hvdb::VdbPrimIterator vdbIt(gdp, vdbGroup); vdbIt; ++vdbIt) {
                GU_PrimVDB* vdbPrim = *vdbIt;

                // only process if grid is a PointDataGrid with leaves
                if (!openvdb::gridConstPtrCast<openvdb::points::PointDataGrid>(
                    vdbPrim->getConstGridPtr())) continue;
                auto&& pointDataGrid =
                    UTvdbGridCast<openvdb::points::PointDataGrid>(vdbPrim->getConstGrid());
                auto leafIter = pointDataGrid.tree().cbeginLeaf();
                if (!leafIter) continue;

                // deep copy the VDB tree if it is not already unique
                vdbPrim->makeGridUnique();

                auto&& outputGrid =
                    UTvdbGridCast<openvdb::points::PointDataGrid>(vdbPrim->getGrid());

                switch (parms.mPropagationType) {

                    case PROPAGATION_TYPE_ADVECTION:
                    case PROPAGATION_TYPE_CONSTRAINED_ADVECTION:
                    {
                        VDBPointsAdvection<openvdb::points::PointDataGrid> advection(
                            outputGrid, parms, boss);
                        GEOvdbProcessTypedGridVec3(*parms.mVelPrim, advection);
                        break;
                    }
                    case PROPAGATION_TYPE_PROJECTION: break; // not implemented
                    case PROPAGATION_TYPE_UNKNOWN: break;
                }
            }
        }

        switch (parms.mPropagationType) {

            case PROPAGATION_TYPE_ADVECTION:
            case PROPAGATION_TYPE_CONSTRAINED_ADVECTION:
            {
                Advection advection(parms, boss);
                GEOvdbProcessTypedGridVec3(*parms.mVelPrim, advection);
                break;
            }
            case PROPAGATION_TYPE_PROJECTION:
            {
                Projection projection(parms, boss);
                GEOvdbProcessTypedGridVec3(*parms.mCptPrim, projection);
                break;
            }
            case PROPAGATION_TYPE_UNKNOWN: break;
        }

        if (boss.wasInterrupted()) addWarning(SOP_MESSAGE, "processing was interrupted");
        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}


////////////////////////////////////////


bool
SOP_OpenVDB_Advect_Points::Cache::evalAdvectionParms(
    OP_Context& context, AdvectionParms& parms)
{
    const fpreal now = context.getTime();

    parms.mPointGeo = inputGeo(0);

    if (!parms.mPointGeo) {
        addError(SOP_MESSAGE, "Missing point input");
        return false;
    }

    UT_String ptGroupStr;
    evalString(ptGroupStr, "group", 0, now);

    parms.mPointGroup = parsePointGroups(ptGroupStr, GroupCreator(gdp));

    const bool advectVdbPoints = (0 != evalInt("advectvdbpoints", 0, now));
    if (advectVdbPoints) {
        const std::string groups = evalStdString("vdbpointsgroups", now);

        // Get and parse the vdb points groups
        openvdb::points::AttributeSet::Descriptor::parseNames(
            parms.mIncludeGroups, parms.mExcludeGroups, groups);
    }

    if (!parms.mPointGroup && ptGroupStr.length() > 0) {
        addWarning(SOP_MESSAGE, "Point group not found");
        return false;
    }

    parms.mPropagationType = stringToPropagationType(evalStdString("operation", now));
    if (parms.mPropagationType == PROPAGATION_TYPE_UNKNOWN) {
        addError(SOP_MESSAGE, "Unknown propargation scheme");
        return false;
    }

    if (parms.mPropagationType == PROPAGATION_TYPE_ADVECTION ||
        parms.mPropagationType == PROPAGATION_TYPE_CONSTRAINED_ADVECTION) {

        const GU_Detail* velGeo = inputGeo(1);

        if (!velGeo) {
            addError(SOP_MESSAGE, "Missing velocity grid input");
            return false;
        }

        const GA_PrimitiveGroup* velGroup = matchGroup(*velGeo, evalStdString("velgroup", now));

        hvdb::VdbPrimCIterator it(velGeo, velGroup);
        parms.mVelPrim = *it;

        if (!parms.mVelPrim) {
            addError(SOP_MESSAGE, "Missing velocity grid");
            return false;
        }
        if (parms.mVelPrim->getStorageType() != UT_VDB_VEC3F) {
            addError(SOP_MESSAGE, "Expected velocity grid to be of type Vec3f");
            return false;
        }

        // Check if the velocity grid uses a staggered representation.
        parms.mStaggered =
            parms.mVelPrim->getGrid().getGridClass() == openvdb::GRID_STAGGERED;

        parms.mTimeStep = static_cast<float>(evalFloat("timestep", 0, now));
        parms.mSteps = static_cast<int>(evalInt("steps", 0, now));
        // The underlying code will accumulate, so to make it substeps
        // we need to divide out.
        parms.mTimeStep /= static_cast<float>(parms.mSteps);
        parms.mStreamlines  = bool(evalInt("outputstreamlines", 0, now));

        parms.mIntegrationType = stringToIntegrationType(evalStdString("integration", now));
        if (parms.mIntegrationType == INTEGRATION_TYPE_UNKNOWN) {
            addError(SOP_MESSAGE, "Unknown integration scheme");
            return false;
        }

    }

    if (parms.mPropagationType == PROPAGATION_TYPE_PROJECTION ||
        parms.mPropagationType == PROPAGATION_TYPE_CONSTRAINED_ADVECTION) {

        const GU_Detail* cptGeo = inputGeo(2);

        if (!cptGeo) {
            addError(SOP_MESSAGE, "Missing closest point grid input");
            return false;
        }

        const GA_PrimitiveGroup *cptGroup = matchGroup(*cptGeo, evalStdString("cptgroup", now));

        hvdb::VdbPrimCIterator it(cptGeo, cptGroup);
        parms.mCptPrim = *it;

        if (!parms.mCptPrim) {
            addError(SOP_MESSAGE, "Missing closest point grid");
            return false;
        }
        if (parms.mCptPrim->getStorageType() != UT_VDB_VEC3F) {
            addError(SOP_MESSAGE, "Expected closest point grid to be of type Vec3f");
            return false;
        }

        parms.mIterations = static_cast<int>(evalInt("iterations", 0, now));
    }

    return true;
}
