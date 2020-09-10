// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Ray.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Performs geometry projections using level set ray intersections or closest point queries.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/Utils.h>

#include <openvdb/tools/RayIntersector.h>
#include <openvdb/tools/VolumeToSpheres.h> // for ClosestSurfacePoint

#include <GA/GA_PageHandle.h>
#include <GA/GA_SplittableRange.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>

#include <hboost/algorithm/string/join.hpp>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////


class SOP_OpenVDB_Ray: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Ray(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Ray() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


////////////////////////////////////////

void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDB grids to process.")
        .setDocumentation(
            "A subset of VDBs to process (see [specifying volumes|/model/volumes#group])"));

    // Method
    parms.add(hutil::ParmFactory(PRM_ORD, "method", "Method")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Projection method")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "rayintersection",  "Ray Intersection",
            "closestpoint",     "Closest Surface Point"
        }));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isovalue", "Isovalue")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, -1.0, PRM_RANGE_UI, 1.0)
        .setTooltip(
            "The voxel value that defines the surface\n\n"
            "Zero works for signed distance fields, while fog volumes require"
            " a larger positive value (0.5 is a good initial guess)."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "dotrans", "Transform")
        .setDefault(PRMoneDefaults)
        .setTooltip("If enabled, transform the intersected geometry."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "scale", "Scale")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_UI, 0, PRM_RANGE_UI, 1)
        .setTooltip("Specify the amount by which to scale the intersected geometry."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "putdist", "Store Distances")
        .setTooltip(
            "Create a point attribute giving the distance to the"
            " collision surface or to the closest surface point."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "lookfar", "Intersect Farthest Surface")
        .setTooltip("Use the farthest intersection point instead of the closest."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "reverserays", "Reverse Rays")
        .setTooltip("Make rays fire in the direction opposite to the normals."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "bias", "Bias")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, 0, PRM_RANGE_UI, 1)
        .setTooltip("Offset the starting position of the rays."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "creategroup", "Create Ray Hit Group")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, create a point group to hold all successful intersections"));

    parms.add(hutil::ParmFactory(PRM_STRING, "hitgrp", "Ray Hit Group")
        .setDefault("rayHitGroup")
        .setTooltip("Point group name"));

    //////////

    hvdb::OpenVDBOpFactory("VDB Ray", SOP_OpenVDB_Ray::factory, parms, *table)
        .addInput("points")
        .addInput("level set grids")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Ray::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Project geometry onto a level set VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node performs geometry projections using level set ray intersections\n\
or closest point queries.\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


OP_Node*
SOP_OpenVDB_Ray::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Ray(net, name, op);
}


SOP_OpenVDB_Ray::SOP_OpenVDB_Ray(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


bool
SOP_OpenVDB_Ray::updateParmsFlags()
{
    bool changed = false;

    bool rayintersection = evalInt("method", 0, 0) == 0;

    changed |= enableParm("isovalue", !rayintersection);

    changed |= enableParm("lookfar", rayintersection);
    changed |= enableParm("reverserays", rayintersection);
    changed |= enableParm("creategroup", rayintersection);
    changed |= enableParm("bias", rayintersection);

    changed |= enableParm("scale", bool(evalInt("dotrans", 0, 0)));

    bool creategroup = evalInt("creategroup", 0, 0);
    changed |= enableParm("hitgrp", creategroup && rayintersection);

    return changed;
}


////////////////////////////////////////

template<typename GridType>
class IntersectPoints
{
public:
    IntersectPoints(
        const GU_Detail& gdp,
        const UT_Vector3Array& pointNormals,
        const GridType& grid,
        UT_Vector3Array& positions,
        UT_FloatArray& distances,
        std::vector<char>& intersections,
        bool keepMaxDist = false,
        bool reverseRays = false,
        double scale = 1.0,
        double bias = 0.0)
    : mGdp(gdp)
    , mPointNormals(pointNormals)
    , mIntersector(grid)
    , mPositions(positions)
    , mDistances(distances)
    , mIntersections(intersections)
    , mKeepMaxDist(keepMaxDist)
    , mReverseRays(reverseRays)
    , mScale(scale)
    , mBias(bias)
    {
    }

    void operator()(const GA_SplittableRange &range) const
    {
        GA_Offset start, end;
        GA_Index pointIndex;
        using RayT = openvdb::math::Ray<double>;
        openvdb::Vec3d eye, dir, intersection;

        const bool doScaling = !openvdb::math::isApproxEqual(mScale, 1.0);
        const bool offsetRay = !openvdb::math::isApproxEqual(mBias, 0.0);

        GA_ROPageHandleV3 points(mGdp.getP());

        // Iterate over blocks
        for (GA_Iterator it(range); it.blockAdvance(start, end); ) {

            points.setPage(start);

            // Point Offsets
            for (GA_Offset pointOffset = start; pointOffset < end; ++pointOffset) {

                const UT_Vector3& pos = points.value(pointOffset);

                eye[0] = double(pos.x());
                eye[1] = double(pos.y());
                eye[2] = double(pos.z());

                pointIndex = mGdp.pointIndex(pointOffset);

                const UT_Vector3& normal = mPointNormals(pointIndex);

                dir[0] = double(normal.x());
                dir[1] = double(normal.y());
                dir[2] = double(normal.z());

                if (mReverseRays) dir = -dir;

                RayT ray((offsetRay ? (eye + dir * mBias) : eye), dir);

                if (!mIntersector.intersectsWS(ray, intersection)) {

                    if (!mIntersections[pointIndex]) mPositions(pointIndex) = pos;
                    continue;
                }

                float distance = float((intersection - eye).length());

                if ((!mKeepMaxDist && mDistances(pointIndex) > distance) ||
                    (mKeepMaxDist && mDistances(pointIndex) < distance)) {

                    mDistances(pointIndex) = distance;

                    UT_Vector3& position = mPositions(pointIndex);


                    if (doScaling) intersection = eye + dir * mScale * double(distance);

                    position.x() = float(intersection[0]);
                    position.y() = float(intersection[1]);
                    position.z() = float(intersection[2]);
                }

                mIntersections[pointIndex] = 1;
            }
        }

    }

private:
    const GU_Detail& mGdp;
    const UT_Vector3Array& mPointNormals;
    openvdb::tools::LevelSetRayIntersector<GridType> mIntersector;
    UT_Vector3Array& mPositions;
    UT_FloatArray& mDistances;
    std::vector<char>& mIntersections;
    const bool mKeepMaxDist, mReverseRays;
    const double mScale, mBias;
};


template<typename GridT, typename InterrupterT>
inline void
closestPoints(const GridT& grid, float isovalue, const GU_Detail& gdp,
    UT_FloatArray& distances, UT_Vector3Array* positions, InterrupterT& boss)
{
    std::vector<openvdb::Vec3R> tmpPoints(distances.entries());

    GA_ROHandleV3 points = GA_ROHandleV3(gdp.getP());

    for (size_t n = 0, N = tmpPoints.size(); n < N; ++n) {
        const UT_Vector3 pos = points.get(gdp.pointOffset(n));
        tmpPoints[n][0] = pos.x();
        tmpPoints[n][1] = pos.y();
        tmpPoints[n][2] = pos.z();
    }

    std::vector<float> tmpDistances;

    const bool transformPoints = (positions != nullptr);

    auto closestPoint = openvdb::tools::ClosestSurfacePoint<GridT>::create(grid, isovalue, &boss);
    if (!closestPoint) return;

    if (transformPoints) closestPoint->searchAndReplace(tmpPoints, tmpDistances);
    else closestPoint->search(tmpPoints, tmpDistances);

    for (size_t n = 0, N = tmpDistances.size(); n < N; ++n) {
        if (tmpDistances[n] < distances(n)) {
            distances(n) = tmpDistances[n];
            if (transformPoints) {
                UT_Vector3& pos = (*positions)(n);

                pos.x() = float(tmpPoints[n].x());
                pos.y() = float(tmpPoints[n].y());
                pos.z() = float(tmpPoints[n].z());
            }
        }
    }
}


class ScalePositions
{
public:
    ScalePositions(
        const GU_Detail& gdp,
        UT_Vector3Array& positions,
        UT_FloatArray& distances,
        double scale = 1.0)
    : mGdp(gdp)
    , mPositions(positions)
    , mDistances(distances)
    , mScale(scale)
    {
    }

    void operator()(const GA_SplittableRange &range) const
    {
        GA_Offset start, end;
        GA_Index pointIndex;
        UT_Vector3 dir;

        GA_ROPageHandleV3 points(mGdp.getP());

        // Iterate over blocks
        for (GA_Iterator it(range); it.blockAdvance(start, end); ) {

            points.setPage(start);

            // Point Offsets
            for (GA_Offset pointOffset = start; pointOffset < end; ++pointOffset) {

                pointIndex = mGdp.pointIndex(pointOffset);

                const UT_Vector3& point = points.value(pointOffset);
                UT_Vector3& pos = mPositions(pointIndex);

                dir = pos - point;
                dir.normalize();

                pos = point + dir * mDistances(pointIndex) * mScale;
            }
        }

    }

private:
    const GU_Detail& mGdp;
    UT_Vector3Array& mPositions;
    UT_FloatArray& mDistances;
    const double mScale;
};

////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Ray::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Computing VDB ray intersections");

        const GU_Detail* vdbGeo = inputGeo(1);
        if (vdbGeo == nullptr) return error();

        // Get the group of grids to surface.
        const GA_PrimitiveGroup* group = matchGroup(*vdbGeo, evalStdString("group", time));
        hvdb::VdbPrimCIterator vdbIt(vdbGeo, group);

        if (!vdbIt) {
            addWarning(SOP_MESSAGE, "No VDB grids found.");
            return error();
        }

        // Eval attributes
        const bool keepMaxDist = bool(evalInt("lookfar", 0, time));
        const bool reverseRays = bool(evalInt("reverserays", 0, time));
        const bool rayIntersection = evalInt("method", 0, time) == 0;
        const double scale = double(evalFloat("scale", 0, time));
        const double bias = double(evalFloat("bias", 0, time));
        const float isovalue = float(evalFloat("isovalue", 0, time));

        UT_Vector3Array pointNormals;

        GA_ROAttributeRef attributeRef = gdp->findPointAttribute("N");
        if (attributeRef.isValid()) {
            gdp->getAttributeAsArray(
                attributeRef.getAttribute(), gdp->getPointRange(), pointNormals);
        } else {
            gdp->normal(pointNormals, /*use_internaln=*/false);
        }


        const size_t numPoints = gdp->getNumPoints();

        UT_Vector3Array positions(numPoints);

        std::vector<char> intersections(numPoints);

        const double limit = std::numeric_limits<double>::max();
        UT_FloatArray distances;
        distances.appendMultiple(
            float((keepMaxDist && rayIntersection) ? -limit : limit), numPoints);

        std::vector<std::string> skippedGrids;

        for (; vdbIt; ++vdbIt) {
            if (boss.wasInterrupted()) break;

            if (vdbIt->getGrid().getGridClass() == openvdb::GRID_LEVEL_SET &&
                vdbIt->getGrid().type() == openvdb::FloatGrid::gridType()) {

                openvdb::FloatGrid::ConstPtr gridPtr =
                    openvdb::gridConstPtrCast<openvdb::FloatGrid>(vdbIt->getGridPtr());

                if (rayIntersection) {
                    IntersectPoints<openvdb::FloatGrid> op(
                        *gdp, pointNormals, *gridPtr, positions, distances,
                        intersections, keepMaxDist, reverseRays, scale, bias);
                    UTparallelFor(GA_SplittableRange(gdp->getPointRange()), op);
                } else {
                    closestPoints(*gridPtr, isovalue, *gdp, distances, &positions, boss);
                }

            } else {
                skippedGrids.push_back(vdbIt.getPrimitiveNameOrIndex().toStdString());
                continue;
            }
        }

        if (bool(evalInt("dotrans", 0, time))) { // update point positions

            if (!rayIntersection && !openvdb::math::isApproxEqual(scale, 1.0)) {
                UTparallelFor(GA_SplittableRange(gdp->getPointRange()),
                    ScalePositions(*gdp, positions, distances, scale));
            }

            gdp->setPos3FromArray(gdp->getPointRange(), positions);
        }

        if (bool(evalInt("putdist", 0, time))) { // add distance attribute

            GA_RWAttributeRef aRef = gdp->findPointAttribute("dist");
            if (!aRef.isValid()) {
                aRef = gdp->addFloatTuple(GA_ATTRIB_POINT, "dist", 1, GA_Defaults(0.0));
            }
            gdp->setAttributeFromArray(aRef.getAttribute(), gdp->getPointRange(), distances);
        }

        if (rayIntersection && bool(evalInt("creategroup", 0, time))) { // group intersecting points

            const auto groupStr = evalStdString("hitgrp", time);
            if (!groupStr.empty()) {
                GA_PointGroup *pointGroup = gdp->findPointGroup(groupStr.c_str());
                if (!pointGroup) pointGroup = gdp->newPointGroup(groupStr.c_str());

                for (size_t n = 0; n < numPoints; ++n) {
                    if (intersections[n]) pointGroup->addIndex(n);
                }
            }
        }

        if (!skippedGrids.empty()) {
            std::string s = "Only level set grids are supported, the following "
                "were skipped: '" + hboost::algorithm::join(skippedGrids, ", ") + "'.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (boss.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "Process was interrupted");
        }

        boss.end();
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
