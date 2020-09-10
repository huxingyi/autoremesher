// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c) Side Effects Software Inc.
 *
 * Produced by:
 *      Side Effects Software Inc
 *      477 Richmond Street West
 *      Toronto, Ontario
 *      Canada   M5V 3E7
 *      416-504-9876
 *
 * NAME:        GU_PrimVDB.C ( GU Library, C++)
 *
 * COMMENTS:    Definitions for utility functions of vdb.
 */

#include <UT/UT_Version.h>

#if defined(SESI_OPENVDB) || defined(SESI_OPENVDB_PRIM)

#include "GU_PrimVDB.h"

#include "GT_GEOPrimCollectVDB.h"
#include <GU/GU_ConvertParms.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimPolySoup.h>
#include <GU/GU_PrimVolume.h>
#include <GU/GU_RayIntersect.h>

#include <GEO/GEO_AttributeHandleList.h>
#include <GEO/GEO_Closure.h>
#include <GEO/GEO_WorkVertexBuffer.h>

#include <GA/GA_AIFTuple.h>
#include <GA/GA_AttributeFilter.h>
#include <GA/GA_ElementWrangler.h>
#include <GA/GA_Handle.h>
#include <GA/GA_PageHandle.h>
#include <GA/GA_PageIterator.h>
#include <GA/GA_SplittableRange.h>

#include <UT/UT_Debug.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Lock.h>
#include <UT/UT_MemoryCounter.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_UniquePtr.h>

#include <UT/UT_Singleton.h>

#include <UT/UT_StopWatch.h>

#include <SYS/SYS_Inline.h>
#include <SYS/SYS_Types.h>
#include <SYS/SYS_TypeTraits.h>

#include <openvdb/tools/VolumeToMesh.h>

#include <hboost/function.hpp>

#include <openvdb/tools/SignedFloodFill.h>

#include <algorithm>
#include <vector>

#include <stddef.h>


#define TIMING_DEF \
    UT_StopWatch timer; \
    if (verbose) timer.start();
#define TIMING_LOG(msg) \
    if (verbose) { \
        printf(msg ": %f ms\n", 1000*timer.stop()); \
        fflush(stdout); \
        timer.start(); \
    }


GA_PrimitiveDefinition *GU_PrimVDB::theDefinition = 0;

GU_PrimVDB*
GU_PrimVDB::build(GU_Detail *gdp, bool append_points)
{
#ifndef SESI_OPENVDB
    // This is only necessary as a stop gap measure until we have the
    // registration code split out properly.
    if (!GU_PrimVDB::theDefinition)
        GU_PrimVDB::registerMyself(&GUgetFactory());

    GU_PrimVDB* primvdb = (GU_PrimVDB *)gdp->appendPrimitive(GU_PrimVDB::theTypeId());

#else

    GU_PrimVDB* primvdb = UTverify_cast<GU_PrimVDB *>(gdp->appendPrimitive(GEO_PRIMVDB));
    primvdb->assignVertex(gdp->appendVertex(), true);

#endif

    if (append_points) {
        GEO_Primitive *prim = primvdb;
        const GA_Size npts = primvdb->getVertexCount();
        GA_Offset startptoff = gdp->appendPointBlock(npts);
        for (GA_Size i = 0; i < npts; i++) {
            prim->setPointOffset(i, startptoff+i);
        }
    }
    return primvdb;
}

GU_PrimVDB*
GU_PrimVDB::buildFromGridAdapter(GU_Detail& gdp, void* gridPtr,
    const GEO_PrimVDB* src, const char* name)
{
    // gridPtr is assumed to point to an openvdb::vX_Y_Z::GridBase::Ptr, for
    // some version X.Y.Z of OpenVDB that may be newer than the one with which
    // libHoudiniGEO.so was built.  This is safe provided that GridBase and
    // its member objects are ABI-compatible between the two OpenVDB versions.
    openvdb::GridBase::Ptr grid =
        *static_cast<openvdb::GridBase::Ptr*>(gridPtr);
    if (!grid)
        return nullptr;

    GU_PrimVDB* vdb = GU_PrimVDB::build(&gdp);
    if (vdb != nullptr) {
        if (src != nullptr) {
            // Copy the source primitive's attributes to this primitive,
            // then transfer those attributes to this grid's metadata.
            vdb->copyAttributesAndGroups(*src, /*copyGroups=*/true);
            GU_PrimVDB::createMetadataFromGridAttrs(*grid, *vdb, gdp);

            // Copy the source's visualization options.
            GEO_VolumeOptions   visopt = src->getVisOptions();
            vdb->setVisualization(visopt.myMode, visopt.myIso, visopt.myDensity);
        }

        // Ensure that certain metadata exists (grid name, grid class, etc.).
        if (name != nullptr) grid->setName(name);
        grid->removeMeta("value_type");
        grid->insertMeta("value_type", openvdb::StringMetadata(grid->valueType()));
        // For each of the following, force any existing metadata's value to be
        // one of the supported values. Note the careful 3 statement sequences
        // so that it works with type mismatches.
        openvdb::GridClass grid_class = grid->getGridClass();
        grid->removeMeta(openvdb::GridBase::META_GRID_CLASS);
        grid->setGridClass(grid_class);
        openvdb::VecType vec_type = grid->getVectorType();
        grid->removeMeta(openvdb::GridBase::META_VECTOR_TYPE);
        grid->setVectorType(vec_type);
        bool is_in_world_space = grid->isInWorldSpace();
        grid->removeMeta(openvdb::GridBase::META_IS_LOCAL_SPACE);
        grid->setIsInWorldSpace(is_in_world_space);
        bool save_as_half = grid->saveFloatAsHalf();
        grid->removeMeta(openvdb::GridBase::META_SAVE_HALF_FLOAT);
        grid->setSaveFloatAsHalf(save_as_half);

        // Transfer the grid's metadata to primitive attributes.
        GU_PrimVDB::createGridAttrsFromMetadata(*vdb, *grid, gdp);

        vdb->setGrid(*grid);

        // If we had no source, have to set options to reasonable
        // defaults.
        if (src == nullptr)
        {
            if (grid->getGridClass() == openvdb::GRID_LEVEL_SET)
            {
                vdb->setVisualization(GEO_VOLUMEVIS_ISO,
                                      vdb->getVisIso(), vdb->getVisDensity());
            }
            else
            {
                vdb->setVisualization(GEO_VOLUMEVIS_SMOKE,
                                      vdb->getVisIso(), vdb->getVisDensity());
            }
        }
    }
    return vdb;
}

int64
GU_PrimVDB::getMemoryUsage() const
{
    int64 mem = sizeof(*this);
    mem += GEO_PrimVDB::getBaseMemoryUsage();
    return mem;
}

void
GU_PrimVDB::countMemory(UT_MemoryCounter &counter) const
{
    counter.countUnshared(sizeof(*this));
    GEO_PrimVDB::countBaseMemory(counter);
}

namespace // anonymous
{

class gu_VolumeMax
{
public:
    gu_VolumeMax(
            const UT_VoxelArrayReadHandleF &vox,
            UT_AutoInterrupt &progress)
        : myVox(vox)
        , myProgress(progress)
        , myMax(std::numeric_limits<float>::min())
    {
    }
    gu_VolumeMax(const gu_VolumeMax &other, UT_Split)
        : myVox(other.myVox)
        , myProgress(other.myProgress)
        // NOTE: other.myMax could be half written-to while this
        //       constructor is being called, so don't use its
        //       value here.  Initialize myMax as in the main
        //       constructor.
        , myMax(std::numeric_limits<float>::min())
    {
    }

    void operator()(const UT_BlockedRange<int> &range)
    {
        uint8 bcnt = 0;

        for (int i = range.begin(); i != range.end(); ++i) {
            float   min_value;
            float   max_value;

            myVox->getLinearTile(i)->findMinMax(min_value, max_value);
            myMax = SYSmax(myMax, max_value);

            if (!bcnt++ && myProgress.wasInterrupted())
                break;
        }
    }

    void join(const gu_VolumeMax &other)
    {
        myMax = std::max(myMax, other.myMax);
    }

    float findMax()
    {
        UTparallelReduce(UT_BlockedRange<int>(0, myVox->numTiles()), *this);
        return myMax;
    }

private:
    const UT_VoxelArrayReadHandleF &    myVox;
    UT_AutoInterrupt &                  myProgress;
    float                               myMax;
};

class gu_ConvertToVDB
{
public:
    gu_ConvertToVDB(
            const UT_VoxelArrayReadHandleF &vox,
            float background,
            UT_AutoInterrupt &progress,
            bool activateInsideSDF
            )
        : myVox(vox)
        , myGrid(openvdb::FloatGrid::create(background))
        , myProgress(progress)
        , myActivateInsideSDF(activateInsideSDF)
    {
    }
    gu_ConvertToVDB(const gu_ConvertToVDB &other, UT_Split)
        : myVox(other.myVox)
        , myGrid(openvdb::FloatGrid::create(other.myGrid->background()))
        , myProgress(other.myProgress)
        , myActivateInsideSDF(other.myActivateInsideSDF)
    {
    }

    openvdb::FloatGrid::Ptr run()
    {
        using namespace openvdb;

        UTparallelReduce(UT_BlockedRange<int>(0, myVox->numTiles()), *this);

        // Check if the VDB grid can be made empty
        openvdb::Coord dim = myGrid->evalActiveVoxelDim();
        if (dim[0] == 1 && dim[1] == 1 && dim[2] == 1) {
            openvdb::Coord ijk = myGrid->evalActiveVoxelBoundingBox().min();
            float value = myGrid->tree().getValue(ijk);
            if (openvdb::math::isApproxEqual<float>(value, myGrid->background())) {
                    myGrid->clear();
            }
        }

        return myGrid;
    }

    void operator()(const UT_BlockedRange<int> &range)
    {
        using namespace openvdb;

        FloatGrid &             grid = *myGrid.get();
        const float             background = grid.background();
        const UT_VoxelArrayF &  vox = *myVox;
        uint8                   bcnt = 0;

        FloatGrid::Accessor acc = grid.getAccessor();

        for (int i = range.begin(); i != range.end(); ++i) {

            const UT_VoxelTile<float> & tile = *vox.getLinearTile(i);
            Coord                       org;
            Coord                       dim;

            vox.linearTileToXYZ(i, org[0], org[1], org[2]);
            org[0] *= TILESIZE; org[1] *= TILESIZE; org[2] *= TILESIZE;
            dim[0] = tile.xres(); dim[1] = tile.yres(); dim[2] = tile.zres();

            if (tile.isConstant()) {
                CoordBBox   bbox(org, org + dim.offsetBy(-1));
                float       value = tile(0, 0, 0);

                if (!SYSisEqual(value, background) &&
                    (myActivateInsideSDF || !SYSisEqual(value, -background))) {
                    grid.fill(bbox, value);
                }
            } else {
                openvdb::Coord ijk;
                for (ijk[2] = 0; ijk[2] < dim[2]; ++ijk[2]) {
                    for (ijk[1] = 0; ijk[1] < dim[1]; ++ijk[1]) {
                        for (ijk[0] = 0; ijk[0] < dim[0]; ++ijk[0]) {
                            float value = tile(ijk[0], ijk[1], ijk[2]);
                            if (!SYSisEqual(value, background) &&
                                (myActivateInsideSDF || !SYSisEqual(value, -background))) {
                                Coord pos = ijk.offsetBy(org[0], org[1], org[2]);
                                acc.setValue(pos, value);
                            }
                        }
                    }
                }
            }

            if (!bcnt++ && myProgress.wasInterrupted())
                break;
        }
    }

    void join(const gu_ConvertToVDB &other)
    {
        if (myProgress.wasInterrupted())
            return;
        UT_IF_ASSERT(int old_count = myGrid->activeVoxelCount();)
        UT_IF_ASSERT(int other_count = other.myGrid->activeVoxelCount();)
        myGrid->merge(*other.myGrid);
        UT_ASSERT(myGrid->activeVoxelCount() == old_count + other_count);
    }

private:
    const UT_VoxelArrayReadHandleF &    myVox;
    openvdb::FloatGrid::Ptr             myGrid;
    UT_AutoInterrupt &                  myProgress;
    bool                                myActivateInsideSDF;

}; // class gu_ConvertToVDB

} // namespace anonymous

GU_PrimVDB *
GU_PrimVDB::buildFromPrimVolume(
        GU_Detail &geo,
        const GEO_PrimVolume &vol,
        const char *name,
        const bool flood_sdf,
        const bool prune,
        const float tolerance,
        const bool activate_inside_sdf)
{
    using namespace openvdb;

    UT_AutoInterrupt            progress("Converting to VDB");
    UT_VoxelArrayReadHandleF    vox = vol.getVoxelHandle();

    float background;
    if (vol.isSDF())
    {
        gu_VolumeMax max_op(vox, progress);
        background = max_op.findMax();
        if (progress.wasInterrupted())
            return nullptr;
    }
    else
    {
        if (vol.getBorder() == GEO_VOLUMEBORDER_CONSTANT)
            background = vol.getBorderValue();
        else
            background = 0.0;
    }

    // When flood-filling SDFs, the inactive interior voxels will be set to
    // -background.  In that case we can avoid activating all inside voxels
    // that already have that value, maintaining the narrow band (if any) of the
    // original native volume.  For non-SDF we always activate interior voxels.
    gu_ConvertToVDB converter(vox, background, progress,
                              activate_inside_sdf || !flood_sdf || !vol.isSDF());
    FloatGrid::Ptr grid = converter.run();
    if (progress.wasInterrupted())
        return nullptr;

    if (vol.isSDF())
        grid->setGridClass(GridClass(GRID_LEVEL_SET));
    else
        grid->setGridClass(GridClass(GRID_FOG_VOLUME));

    if (prune) {
        grid->pruneGrid(tolerance);
    }

    if (flood_sdf && vol.isSDF()) {
        // only call signed flood fill on SDFs
        openvdb::tools::signedFloodFill(grid->tree());
    }

    GU_PrimVDB *prim_vdb = buildFromGrid(geo, grid, nullptr, name);
    if (!prim_vdb)
        return nullptr;
    int rx, ry, rz;
    vol.getRes(rx, ry, rz);
    prim_vdb->setSpaceTransform(vol.getSpaceTransform(), UT_Vector3R(rx,ry,rz));
    prim_vdb->setVisualization(
                vol.getVisualization(), vol.getVisIso(), vol.getVisDensity());
    return prim_vdb;
}

// Copy the exclusive bbox [start,end) from myVox into acc
static void
guCopyVoxelBBox(
        const UT_VoxelArrayReadHandleF &vox,
        openvdb::FloatGrid::Accessor &acc,
        openvdb::Coord start, openvdb::Coord end)
{
    openvdb::Coord c;
    for (c[0] = start[0] ; c[0] < end[0]; c[0]++) {
        for (c[1] = start[1] ; c[1] < end[1]; c[1]++) {
            for (c[2] = start[2] ; c[2] < end[2]; c[2]++) {
                float value = vox->getValue(c[0], c[1], c[2]);
                acc.setValueOnly(c, value);
            }
        }
    }
}

void
GU_PrimVDB::expandBorderFromPrimVolume(const GEO_PrimVolume &vol, int pad)
{
    using namespace openvdb;

    UT_AutoInterrupt                progress("Add inactive VDB border");
    const UT_VoxelArrayReadHandleF  vox(vol.getVoxelHandle());
    const Coord                     res(vox->getXRes(),
                                        vox->getYRes(),
                                        vox->getZRes());
    GridBase &                      base = getGrid();
    FloatGrid &                     grid = UTvdbGridCast<FloatGrid>(base);
    FloatGrid::Accessor             acc = grid.getAccessor();

    // For simplicity, we overdraw the edges and corners
    for (int axis = 0; axis < 3; axis++) {

        if (progress.wasInterrupted())
            return;

        openvdb::Coord beg(-pad, -pad, -pad);
        openvdb::Coord end = res.offsetBy(+pad);

        beg[axis] = -pad;
        end[axis] = 0;
        guCopyVoxelBBox(vox, acc, beg, end);

        beg[axis] = res[axis];
        end[axis] = res[axis] + pad;
        guCopyVoxelBBox(vox, acc, beg, end);
    }
}

// The following code is for HDK only
#ifndef SESI_OPENVDB
// Static callback for our factory.
static GA_Primitive*
gu_newPrimVDB(GA_Detail &detail, GA_Offset offset,
        const GA_PrimitiveDefinition &)
{
    return new GU_PrimVDB(static_cast<GU_Detail *>(&detail), offset);
}

static GA_Primitive*
gaPrimitiveMergeConstructor(const GA_MergeMap &map,
                            GA_Detail &dest_detail,
                            GA_Offset dest_offset,
                            const GA_Primitive &src_prim)
{
    return new GU_PrimVDB(map, dest_detail, dest_offset, static_cast<const GU_PrimVDB &>(src_prim));
}

static UT_Lock theInitPrimDefLock;

void
GU_PrimVDB::registerMyself(GA_PrimitiveFactory *factory)
{
    // Ignore double registration
    if (theDefinition) return;

    UT_Lock::Scope lock(theInitPrimDefLock);

    if (theDefinition) return;

#if defined(__ICC)
    // Disable ICC "assignment to static variable" warning,
    // since the assignment to theDefinition is mutex-protected.
    __pragma(warning(disable:1711));
#endif

    theDefinition = factory->registerDefinition("VDB",
        gu_newPrimVDB, GA_FAMILY_NONE);

#if defined(__ICC)
    __pragma(warning(default:1711));
#endif

    if (!theDefinition) {
        if (!factory->lookupDefinition("VDB")) {
            //std::cerr << "WARNING: failed to register GU_PrimVDB\n";
        }
        return;
    }

    theDefinition->setLabel("Sparse Volumes (VDBs)");
    theDefinition->setHasLocalTransform(true);
    theDefinition->setMergeConstructor(&gaPrimitiveMergeConstructor);
    registerIntrinsics(*theDefinition);

    // Register the GT tesselation too (now we know what type id we have)
    openvdb_houdini::GT_GEOPrimCollectVDB::registerPrimitive(theDefinition->getId());
}
#endif

GEO_Primitive *
GU_PrimVDB::convertToNewPrim(
        GEO_Detail &dst_geo,
        GU_ConvertParms &parms,
        fpreal adaptivity,
        bool split_disjoint_volumes,
        bool &success) const
{
    GEO_Primitive *     prim = nullptr;

    const GA_PrimCompat::TypeMask parmType = parms.toType();

    success = false;
    if (parmType == GEO_PrimTypeCompat::GEOPRIMPOLY) {
        prim = convertToPoly(dst_geo, parms, adaptivity,
                             /*polysoup*/false, success);
    } else if (parmType == GEO_PrimTypeCompat::GEOPRIMPOLYSOUP) {
        prim = convertToPoly(dst_geo, parms, adaptivity,
                             /*polysoup*/true, success);
    } else if (parmType == GEO_PrimTypeCompat::GEOPRIMVOLUME) {
        prim = convertToPrimVolume(dst_geo, parms, split_disjoint_volumes);
        if (prim)
            success = true;
    }

    return prim;
}

GEO_Primitive *
GU_PrimVDB::convertNew(GU_ConvertParms &parms)
{
    bool success = false;
    return convertToNewPrim(*getParent(), parms,
                /*adaptivity*/0, /*sparse*/false, success);
}

static void
guCopyMesh(
        GEO_Detail& detail,
        openvdb::tools::VolumeToMesh& mesher,
        bool buildpolysoup,
        bool verbose)
{
    TIMING_DEF;

    const openvdb::tools::PointList& points = mesher.pointList();
    openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

    // NOTE: Adaptive meshes consist of tringles and quads.

    // Construct the points
    GA_Size npoints = mesher.pointListSize();
    GA_Offset startpt = detail.appendPointBlock(npoints);
    SYS_STATIC_ASSERT(sizeof(openvdb::tools::PointList::element_type) == sizeof(UT_Vector3));
    GA_RWHandleV3 pthandle(detail.getP());
    pthandle.setBlock(startpt, npoints, (UT_Vector3 *)points.get());

    TIMING_LOG("Copy Points");

    // Construct the array of polygon point numbers
    // NOTE: For quad meshes, the number of quads is about the number of points,
    //       so the number of vertices is about 4*npoints

    GA_Size nquads = 0, ntris = 0;
    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
        nquads += polygons.numQuads();
        ntris += polygons.numTriangles();
    }

    TIMING_LOG("Count Quads and Tris");

    // Don't create anything if nothing to create
    if (!ntris && !nquads)
        return;

    GA_Size nverts = nquads*4 + ntris*3;
    UT_IntArray verts(nverts, nverts);
    GA_Size iquad = 0;
    GA_Size itri = nquads*4;
    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];

        // Copy quads
        for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
            const openvdb::Vec4I& quad = polygons.quad(i);
            verts(iquad++) = quad[0];
            verts(iquad++) = quad[1];
            verts(iquad++) = quad[2];
            verts(iquad++) = quad[3];
        }

        // Copy triangles (adaptive mesh)
        for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
            const openvdb::Vec3I& triangle = polygons.triangle(i);
            verts(itri++) = triangle[0];
            verts(itri++) = triangle[1];
            verts(itri++) = triangle[2];
        }
    }

    TIMING_LOG("Get Quad and Tri Verts");

    GEO_PolyCounts sizelist;
    if (nquads)
        sizelist.append(4, nquads);
    if (ntris)
        sizelist.append(3, ntris);
    if (buildpolysoup)
        GU_PrimPolySoup::build(&detail, startpt, npoints, sizelist, verts.array());
    else
        GU_PrimPoly::buildBlock(&detail, startpt, npoints, sizelist, verts.array());

    TIMING_LOG("Build Polys");
}

namespace {
class gu_VDBNormalsParallel
{
public:
    gu_VDBNormalsParallel(GA_Attribute *p, GA_Attribute *n, const GU_PrimVDB &vdb)
        : myP(p)
        , myN(n)
        , myVDB(vdb)
    {}

    void operator()(const GA_SplittableRange &r) const
    {
        UT_Interrupt *boss = UTgetInterrupt();
        GA_ROPageHandleV3 positions(myP);
        GA_RWPageHandleV3 normals(myN);

        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            if (boss->opInterrupt())
                break;

            const GA_Offset pagefirstoff = pit.getFirstOffsetInPage();
            positions.setPage(pagefirstoff);
            normals.setPage(pagefirstoff);
            GA_Offset start;
            GA_Offset end;
            for (GA_Iterator it = pit.begin(); it.blockAdvance(start, end); )
            {
                myVDB.evalGradients(&normals.value(start), 1,
                                    &positions.value(start), end - start,
                                    /*normalize*/true);
            }
        }
    }
private:
    GA_Attribute *const myP;
    GA_Attribute *const myN;
    const GU_PrimVDB &myVDB;
};
}

GEO_Primitive *
GU_PrimVDB::convertToPoly(
        GEO_Detail &dst_geo,
        GU_ConvertParms &parms,
        fpreal adaptivity,
        bool polysoup,
        bool &success) const
{
    using namespace openvdb;

    UT_AutoInterrupt    progress("Convert VDB to Polygons");
    GA_Detail::OffsetMarker marker(dst_geo);
    bool                verbose = false;

    success = false;

    try
    {
        tools::VolumeToMesh mesher(parms.myOffset, adaptivity);
        UTvdbProcessTypedGridScalar(getStorageType(), getGrid(), mesher);
        if (progress.wasInterrupted())
            return nullptr;
        guCopyMesh(dst_geo, mesher, polysoup, verbose);
        if (progress.wasInterrupted())
            return nullptr;
    }
    catch (std::exception& /*e*/)
    {
        return nullptr;
    }
    GA_Range pointrange(marker.pointRange());
    GA_Range primitiverange(marker.primitiveRange());
    GUconvertCopySingleVertexPrimAttribsAndGroups(
            parms, *getParent(), getMapOffset(), dst_geo,
            primitiverange, pointrange);
    if (progress.wasInterrupted())
        return nullptr;

    // If there was already a point normal attribute, we should compute normals
    // to avoid getting zero default values for the new polygons.
    GA_RWAttributeRef normal_ref = dst_geo.findNormalAttribute(GA_ATTRIB_POINT);
    if (normal_ref.isValid() && !pointrange.isEmpty())
    {
        UTparallelFor(GA_SplittableRange(pointrange),
                      gu_VDBNormalsParallel(dst_geo.getP(), normal_ref.getAttribute(), *this));
        if (progress.wasInterrupted())
            return nullptr;
    }

    // At this point, we have succeeded, marker.numPrimitives() might be 0 if
    // we had an empty VDB.
    success = true;
    if (primitiverange.isEmpty())
        return nullptr;

    return dst_geo.getGEOPrimitive(marker.primitiveBegin());
}

namespace {
class gu_DestroyVDBPrimGuard
{
public:
    gu_DestroyVDBPrimGuard(GU_PrimVDB &vdb)
        : myVDB(vdb)
    {
    }
    ~gu_DestroyVDBPrimGuard()
    {
        myVDB.getDetail().destroyPrimitive(myVDB, /*and_points*/true);
    }
private:
    GU_PrimVDB &myVDB;
};
} // anonymous namespace

/*static*/ void
GU_PrimVDB::convertPrimVolumeToPolySoup(
        GU_Detail &dst_geo,
        const GEO_PrimVolume &src_vol)
{
    using namespace openvdb;
    UT_AutoInterrupt progress("Convert to Polygons");

    GU_PrimVDB &vdb = *buildFromPrimVolume(
                            dst_geo, src_vol, nullptr,
                            /*flood*/false, /*prune*/true, /*tol*/0,
                            /*activate_inside*/true);
    gu_DestroyVDBPrimGuard destroy_guard(vdb);

    if (progress.wasInterrupted())
        return;

    try
    {
        BoolGrid::Ptr mask;
        if (src_vol.getBorder() != GEO_VOLUMEBORDER_CONSTANT)
        {
            Coord res;
            src_vol.getRes(res[0], res[1], res[2]);
            CoordBBox bbox(Coord(0, 0, 0), res.offsetBy(-1)); // inclusive
            if (bbox.hasVolume())
            {
                vdb.expandBorderFromPrimVolume(src_vol, 4);
                if (progress.wasInterrupted())
                    return;
                mask = BoolGrid::create(/*background*/false);
                mask->setTransform(vdb.getGrid().transform().copy());
                mask->fill(bbox, /*foreground*/true);
            }
        }

        tools::VolumeToMesh mesher(src_vol.getVisIso());
        mesher.setSurfaceMask(mask);
        GEOvdbProcessTypedGridScalar(vdb, mesher);
        if (progress.wasInterrupted())
            return;
        guCopyMesh(dst_geo, mesher, /*polysoup*/true, /*verbose*/false);
        if (progress.wasInterrupted())
            return;
    }
    catch (std::exception& /*e*/)
    {
    }
}

namespace // anonymous
{

#define SCALAR_RET(T) \
        typename SYS_EnableIf< SYS_IsArithmetic<T>::value, T >::type

#define NON_SCALAR_RET(T) \
        typename SYS_DisableIf< SYS_IsArithmetic<T>::value, T >::type

/// Houdini Volume wrapper to abstract multiple volumes with a consistent API.
template <int TUPLE_SIZE>
class VoxelArrayVolume
{
public:
    static const int TupleSize = TUPLE_SIZE;

    VoxelArrayVolume(GU_Detail& geo): mGeo(geo)
    {
        for (int i = 0; i < TUPLE_SIZE; i++) {
            mVol[i] = (GU_PrimVolume *)GU_PrimVolume::build(&mGeo);
            mHandle[i] = mVol[i]->getVoxelWriteHandle();
        }
    }

    void
    setSize(const openvdb::Coord &dim)
    {
        for (int i = 0; i < TUPLE_SIZE; i++) {
            mHandle[i]->size(dim[0], dim[1], dim[2]);
        }
    }

    template <class ValueT>
    void
    setVolumeOptions(
            bool is_sdf, const ValueT& background,
            GEO_VolumeVis vismode, fpreal iso, fpreal density,
            SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        if (is_sdf) {
            mVol[0]->setBorder(GEO_VOLUMEBORDER_SDF, background);
            mVol[0]->setVisualization(vismode, iso, density);
        } else {
            mVol[0]->setBorder(GEO_VOLUMEBORDER_CONSTANT, background);
            mVol[0]->setVisualization(vismode, iso, density);
        }
    }
    template <class ValueT>
    void
    setVolumeOptions(
            bool is_sdf, const ValueT& background,
            GEO_VolumeVis vismode, fpreal iso, fpreal density,
            NON_SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        if (is_sdf) {
            for (int i = 0; i < TUPLE_SIZE; i++) {
                mVol[i]->setBorder(GEO_VOLUMEBORDER_SDF, background[i]);
                mVol[i]->setVisualization(vismode, iso, density);
            }
        } else {
            for (int i = 0; i < TUPLE_SIZE; i++) {
                mVol[i]->setBorder(GEO_VOLUMEBORDER_CONSTANT, background[i]);
                mVol[i]->setVisualization(vismode, iso, density);
            }
        }
    }

    void setSpaceTransform(const GEO_PrimVolumeXform& s)
    {
        for (int i = 0; i < TUPLE_SIZE; i++)
            mVol[i]->setSpaceTransform(s);
    }

    int
    numTiles() const
    {
        // Since we create all volumes the same size, we can simply use the
        // first one.
        return mHandle[0]->numTiles();
    }

    template<typename ConstAccessorT>
    void copyToAlignedTile(
            int tile_index,
            ConstAccessorT& src,
            const openvdb::Coord& src_origin);

    template<typename ConstAccessorT>
    void copyToTile(
            int tile_index,
            ConstAccessorT& src,
            const openvdb::Coord& src_origin);

private: // methods

    using VoxelTileF = UT_VoxelTile<fpreal32>;

    template <class ValueT>
    static void
    makeConstant_(VoxelTileF* tiles[TUPLE_SIZE], const ValueT& v,
                 SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        tiles[0]->makeConstant(fpreal32(v));
    }
    template <class ValueT>
    static void
    makeConstant_(VoxelTileF* tiles[TUPLE_SIZE], const ValueT& v,
                 NON_SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        for (int i = 0; i < TUPLE_SIZE; i++)
            tiles[i]->makeConstant(fpreal32(v[i]));
    }

    // Convert a local tile coordinate to a linear offset. This is used instead
    // of UT_VoxelTile::operator()() since we always decompress the tile first.
    SYS_FORCE_INLINE static int
    tileCoordToOffset(const VoxelTileF* tile, const openvdb::Coord& xyz)
    {
        UT_ASSERT_P(xyz[0] >= 0 && xyz[0] < tile->xres());
        UT_ASSERT_P(xyz[1] >= 0 && xyz[1] < tile->yres());
        UT_ASSERT_P(xyz[2] >= 0 && xyz[2] < tile->zres());
        return ((xyz[2] * tile->yres()) + xyz[1]) * tile->xres() + xyz[0];
    }

    // Set the value into tile coord xyz
    template <class ValueT>
    static void
    setTileVoxel(
            const openvdb::Coord& xyz,
            VoxelTileF* tile,
            fpreal32* rawData,
            const ValueT& v,
            int /*i*/,
            SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        rawData[tileCoordToOffset(tile, xyz)] = v;
    }
    template <class ValueT>
    static void
    setTileVoxel(
            const openvdb::Coord& xyz,
            VoxelTileF* tile,
            fpreal32* rawData,
            const ValueT& v,
            int i,
            NON_SCALAR_RET(ValueT)* /*dummy*/ = 0)
    {
        rawData[tileCoordToOffset(tile, xyz)] = v[i];
    }

    template <class ValueT>
    static bool
    compareVoxel(
            const openvdb::Coord& xyz,
            VoxelTileF* tile,
            fpreal32* rawData,
            const ValueT& v,
            int i,
            SCALAR_RET(ValueT) *dummy = 0)
    {
        UT_ASSERT_P(xyz[0] >= 0 && xyz[0] < tile->xres());
        UT_ASSERT_P(xyz[1] >= 0 && xyz[1] < tile->yres());
        UT_ASSERT_P(xyz[2] >= 0 && xyz[2] < tile->zres());
        float vox = (*tile)(xyz[0], xyz[1], xyz[2]);
        return openvdb::math::isApproxEqual<float>(vox, v);
    }
    template <class ValueT>
    static bool
    compareVoxel(
            const openvdb::Coord& xyz,
            VoxelTileF* tile,
            fpreal32* rawData,
            const ValueT& v,
            int i,
            NON_SCALAR_RET(ValueT) *dummy = 0)
    {
        UT_ASSERT_P(xyz[0] >= 0 && xyz[0] < tile->xres());
        UT_ASSERT_P(xyz[1] >= 0 && xyz[1] < tile->yres());
        UT_ASSERT_P(xyz[2] >= 0 && xyz[2] < tile->zres());
        float vox = (*tile)(xyz[0], xyz[1], xyz[2]);
        return openvdb::math::isApproxEqual<float>(vox, v[i]);
    }

    // Check if aligned VDB bbox region is constant
    template<typename ConstAccessorT, typename ValueType>
    static bool
    isAlignedConstantRegion_(
            ConstAccessorT& acc,
            const openvdb::Coord& beg,
            const openvdb::Coord& end,
            const ValueType& const_value)
    {
        using openvdb::math::isApproxEqual;

        using LeafNodeType = typename ConstAccessorT::LeafNodeT;
        const openvdb::Index DIM = LeafNodeType::DIM;

        // The smallest constant tile size in vdb is DIM and the
        // vdb-leaf/hdk-tile coords are aligned.
        openvdb::Coord ijk;
        for (ijk[0] = beg[0]; ijk[0] < end[0]; ijk[0] += DIM) {
            for (ijk[1] = beg[1]; ijk[1] < end[1]; ijk[1] += DIM) {
                for (ijk[2] = beg[2]; ijk[2] < end[2]; ijk[2] += DIM) {

                    if (acc.probeConstLeaf(ijk) != nullptr)
                        return false;

                    ValueType sampleValue = acc.getValue(ijk);
                    if (!isApproxEqual(const_value, sampleValue))
                        return false;
                }
            }
        }
        return true;
    }

    // Copy all data of the aligned leaf node to the tile at origin
    template<typename LeafType>
    static void
    copyAlignedLeafNode_(
            VoxelTileF* tile,
            int tuple_i,
            const openvdb::Coord& origin,
            const LeafType& leaf)
    {
        fpreal32* data = tile->rawData();
        for (openvdb::Index i = 0; i < LeafType::NUM_VALUES; ++i) {
            openvdb::Coord xyz = origin + LeafType::offsetToLocalCoord(i);
            setTileVoxel(xyz, tile, data, leaf.getValue(i), tuple_i);
        }
    }

    // Check if unaligned VDB bbox region is constant.
    // beg_a is beg rounded down to the nearest leaf origin.
    template<typename ConstAccessorT, typename ValueType>
    static bool
    isConstantRegion_(
            ConstAccessorT& acc,
            const openvdb::Coord& beg,
            const openvdb::Coord& end,
            const openvdb::Coord& beg_a,
            const ValueType& const_value)
    {
        using openvdb::math::isApproxEqual;

        using LeafNodeType = typename ConstAccessorT::LeafNodeT;
        const openvdb::Index DIM = LeafNodeType::DIM;
        const openvdb::Index LOG2DIM = LeafNodeType::LOG2DIM;

        UT_ASSERT(beg_a[0] % DIM == 0);
        UT_ASSERT(beg_a[1] % DIM == 0);
        UT_ASSERT(beg_a[2] % DIM == 0);

        openvdb::Coord ijk;
        for (ijk[0] = beg_a[0]; ijk[0] < end[0]; ijk[0] += DIM) {
            for (ijk[1] = beg_a[1]; ijk[1] < end[1]; ijk[1] += DIM) {
                for (ijk[2] = beg_a[2]; ijk[2] < end[2]; ijk[2] += DIM) {

                    const LeafNodeType* leaf = acc.probeConstLeaf(ijk);
                    if (!leaf)
                    {
                        ValueType sampleValue = acc.getValue(ijk);
                        if (!isApproxEqual(const_value, sampleValue))
                            return false;
                        continue;
                    }

                    // Else, we're a leaf node, determine if region is constant
                    openvdb::Coord leaf_beg = ijk;
                    openvdb::Coord leaf_end = ijk + openvdb::Coord(DIM, DIM, DIM);

                    // Clamp the leaf region to the tile bbox
                    leaf_beg.maxComponent(beg);
                    leaf_end.minComponent(end);

                    // Offset into local leaf coordinates
                    leaf_beg -= leaf->origin();
                    leaf_end -= leaf->origin();

                    const ValueType* s0 = &leaf->getValue(leaf_beg[2]);
                    for (openvdb::Int32 x = leaf_beg[0]; x < leaf_end[0]; ++x) {
                        const ValueType* s1 = s0 + (x<<2*LOG2DIM);
                        for (openvdb::Int32 y = leaf_beg[1]; y < leaf_end[1]; ++y) {
                            const ValueType* s2 = s1 + (y<<LOG2DIM);
                            for (openvdb::Int32 z = leaf_beg[2]; z < leaf_end[2]; ++z) {
                                if (!isApproxEqual(const_value, *s2))
                                    return false;
                                s2++;
                            }
                        }
                    }
                }
            }
        }
        return true;
    }

    // Copy the leaf node data at the global coord leaf_origin to the local
    // tile region [beg,end)
    template<typename LeafType>
    static void
    copyLeafNode_(
            VoxelTileF* tile,
            int tuple_i,
            const openvdb::Coord& beg,
            const openvdb::Coord& end,
            const openvdb::Coord& leaf_origin,
            const LeafType& leaf)
    {
        using openvdb::Coord;

        fpreal32* data = tile->rawData();

        Coord xyz;
        Coord ijk = leaf_origin;
        for (xyz[2] = beg[2]; xyz[2] < end[2]; ++xyz[2], ++ijk[2]) {
            ijk[1] = leaf_origin[1];
            for (xyz[1] = beg[1]; xyz[1] < end[1]; ++xyz[1], ++ijk[1]) {
                ijk[0] = leaf_origin[0];
                for (xyz[0] = beg[0]; xyz[0] < end[0]; ++xyz[0], ++ijk[0]) {
                    setTileVoxel(xyz, tile, data, leaf.getValue(ijk), tuple_i);
                }
            }
        }
    }

    // Set the local tile region [beg,end) to the same value.
    template <class ValueT>
    static void
    setConstantRegion_(
            VoxelTileF* tile,
            int tuple_i,
            const openvdb::Coord& beg,
            const openvdb::Coord& end,
            const ValueT& value)
    {
        fpreal32* data = tile->rawData();
        openvdb::Coord xyz;
        for (xyz[2] = beg[2]; xyz[2] < end[2]; ++xyz[2]) {
            for (xyz[1] = beg[1]; xyz[1] < end[1]; ++xyz[1]) {
                for (xyz[0] = beg[0]; xyz[0] < end[0]; ++xyz[0]) {
                    setTileVoxel(xyz, tile, data, value, tuple_i);
                }
            }
        }
    }

    void
    getTileCopyData_(
            int tile_index,
            const openvdb::Coord& src_origin,
            VoxelTileF* tiles[TUPLE_SIZE],
            openvdb::Coord& res,
            openvdb::Coord& src_bbox_beg,
            openvdb::Coord& src_bbox_end)
    {
        for (int i = 0; i < TUPLE_SIZE; i++) {
            tiles[i] = mHandle[i]->getLinearTile(tile_index);
        }

        // Since all tiles are the same size, so just use the first one.
        res[0] = tiles[0]->xres();
        res[1] = tiles[0]->yres();
        res[2] = tiles[0]->zres();

        // Define the inclusive coordinate range, in vdb index space.
        // ie. The source bounding box that we will copy from.
        // NOTE: All tiles are the same size, so just use the first handle.
        openvdb::Coord dst;
        mHandle[0]->linearTileToXYZ(tile_index, dst.x(), dst.y(), dst.z());
        dst.x() *= TILESIZE;
        dst.y() *= TILESIZE;
        dst.z() *= TILESIZE;
        src_bbox_beg = src_origin + dst;
        src_bbox_end = src_bbox_beg + res;
    }

private: // data

    GU_Detail& mGeo;
    GU_PrimVolume *mVol[TUPLE_SIZE];
    UT_VoxelArrayWriteHandleF mHandle[TUPLE_SIZE];
};

// Copy the vdb data to the current tile. Assumes full tiles.
template<int TUPLE_SIZE>
template<typename ConstAccessorT>
inline void
VoxelArrayVolume<TUPLE_SIZE>::copyToAlignedTile(
        int tile_index, ConstAccessorT &acc, const openvdb::Coord& src_origin)
{
    using openvdb::Coord;
    using openvdb::CoordBBox;

    using ValueType = typename ConstAccessorT::ValueType;
    using LeafNodeType = typename ConstAccessorT::LeafNodeT;
    const openvdb::Index LEAF_DIM = LeafNodeType::DIM;

    VoxelTileF* tiles[TUPLE_SIZE];
    Coord tile_res;
    Coord beg;
    Coord end;

    getTileCopyData_(tile_index, src_origin, tiles, tile_res, beg, end);

    ValueType const_value = acc.getValue(beg);
    if (isAlignedConstantRegion_(acc, beg, end, const_value)) {

        makeConstant_(tiles, const_value);

    } else { // populate dense tile

        for (int tuple_i = 0; tuple_i < TUPLE_SIZE; tuple_i++) {

            VoxelTileF* tile = tiles[tuple_i];
            tile->makeRawUninitialized();

            Coord ijk;
            for (ijk[0] = beg[0]; ijk[0] < end[0]; ijk[0] += LEAF_DIM) {
                for (ijk[1] = beg[1]; ijk[1] < end[1]; ijk[1] += LEAF_DIM) {
                    for (ijk[2] = beg[2]; ijk[2] < end[2]; ijk[2] += LEAF_DIM) {

                        Coord tile_beg = ijk - beg; // local tile coord
                        Coord tile_end = tile_beg.offsetBy(LEAF_DIM);

                        const LeafNodeType* leaf = acc.probeConstLeaf(ijk);
                        if (leaf != nullptr) {
                            copyAlignedLeafNode_(
                                    tile, tuple_i, tile_beg, *leaf);
                        } else {
                            setConstantRegion_(
                                    tile, tuple_i, tile_beg, tile_end,
                                    acc.getValue(ijk));
                        }
                    }
                }
            }
        }

    } // end populate dense tile
}

template<int TUPLE_SIZE>
template<typename ConstAccessorT>
inline void
VoxelArrayVolume<TUPLE_SIZE>::copyToTile(
        int tile_index, ConstAccessorT &acc, const openvdb::Coord& src_origin)
{
    using openvdb::Coord;
    using openvdb::CoordBBox;

    using ValueType = typename ConstAccessorT::ValueType;
    using LeafNodeType = typename ConstAccessorT::LeafNodeT;
    const openvdb::Index DIM = LeafNodeType::DIM;

    VoxelTileF* tiles[TUPLE_SIZE];
    Coord tile_res;
    Coord beg;
    Coord end;

    getTileCopyData_(tile_index, src_origin, tiles, tile_res, beg, end);

    // a_beg is beg rounded down to the nearest leaf origin
    Coord a_beg(beg[0]&~(DIM-1), beg[1]&~(DIM-1), beg[2]&~(DIM-1));

    ValueType const_value = acc.getValue(a_beg);
    if (isConstantRegion_(acc, beg, end, a_beg, const_value)) {

        makeConstant_(tiles, const_value);

    } else {

        for (int tuple_i = 0; tuple_i < TUPLE_SIZE; tuple_i++) {

            VoxelTileF* tile = tiles[tuple_i];
            tile->makeRawUninitialized();

            Coord ijk;
            for (ijk[0] = a_beg[0]; ijk[0] < end[0]; ijk[0] += DIM) {
                for (ijk[1] = a_beg[1]; ijk[1] < end[1]; ijk[1] += DIM) {
                    for (ijk[2] = a_beg[2]; ijk[2] < end[2]; ijk[2] += DIM) {

                        // Compute clamped local tile coord bbox
                        Coord leaf_beg = ijk;
                        Coord tile_beg = ijk - beg;
                        Coord tile_end = tile_beg.offsetBy(DIM);
                        for (int axis = 0; axis < 3; ++axis) {
                            if (tile_beg[axis] < 0) {
                                tile_beg[axis] = 0;
                                leaf_beg[axis] = beg[axis];
                            }
                            if (tile_end[axis] > tile_res[axis])
                                tile_end[axis] = tile_res[axis];
                        }

                        // Copy the region
                        const LeafNodeType* leaf = acc.probeConstLeaf(leaf_beg);
                        if (leaf != nullptr) {
                            copyLeafNode_(
                                    tile, tuple_i, tile_beg, tile_end,
                                    leaf_beg, *leaf);
                        } else {
                            setConstantRegion_(
                                    tile, tuple_i, tile_beg, tile_end,
                                    acc.getValue(ijk));
                        }
                    }
                }
            }
        }
    }
}

template<typename TreeType, typename VolumeT, bool aligned>
class gu_SparseTreeCopy;

template<typename TreeType, typename VolumeT>
class gu_SparseTreeCopy<TreeType, VolumeT, /*aligned=*/true>
{
public:
    gu_SparseTreeCopy(
            const TreeType& tree,
            VolumeT& volume,
            const openvdb::Coord& src_origin,
            UT_AutoInterrupt& progress
        )
        : mVdbAcc(tree)
        , mVolume(volume)
        , mSrcOrigin(src_origin)
        , mProgress(progress)
    {
    }

    void run(bool threaded = true)
    {
        tbb::blocked_range<int> range(0, mVolume.numTiles());
        if (threaded) tbb::parallel_for(range, *this);
        else (*this)(range);
    }

    void operator()(const tbb::blocked_range<int>& range) const
    {
        uint8 bcnt = 0;
        for (int i = range.begin(); i != range.end(); ++i) {
            mVolume.copyToAlignedTile(i, mVdbAcc, mSrcOrigin);
            if (!bcnt++ && mProgress.wasInterrupted())
                return;
        }
    }

private:
    openvdb::tree::ValueAccessor<const TreeType> mVdbAcc;
    VolumeT& mVolume;
    const openvdb::Coord mSrcOrigin;
    UT_AutoInterrupt& mProgress;
};

template<typename TreeType, typename VolumeT>
class gu_SparseTreeCopy<TreeType, VolumeT, /*aligned=*/false>
{
public:
    gu_SparseTreeCopy(
            const TreeType& tree,
            VolumeT& volume,
            const openvdb::Coord& src_origin,
            UT_AutoInterrupt& progress
        )
        : mVdbAcc(tree)
        , mVolume(volume)
        , mSrcOrigin(src_origin)
        , mProgress(progress)
    {
    }

    void run(bool threaded = true)
    {
        tbb::blocked_range<int> range(0, mVolume.numTiles());
        if (threaded) tbb::parallel_for(range, *this);
        else (*this)(range);
    }

    void operator()(const tbb::blocked_range<int>& range) const
    {
        uint8 bcnt = 0;
        for (int i = range.begin(); i != range.end(); ++i) {
            mVolume.copyToTile(i, mVdbAcc, mSrcOrigin);
            if (!bcnt++ && mProgress.wasInterrupted())
                return;
        }
    }

private:
    openvdb::tree::ValueAccessor<const TreeType> mVdbAcc;
    VolumeT& mVolume;
    const openvdb::Coord mSrcOrigin;
    UT_AutoInterrupt& mProgress;
};

/// @brief Converts an OpenVDB grid into one/three Houdini Volume.
/// @note Vector grids are converted into three Houdini Volumes.
template <class VolumeT>
class gu_ConvertFromVDB
{
public:

    gu_ConvertFromVDB(
            GEO_Detail& dst_geo,
            const GU_PrimVDB& src_vdb,
            bool split_disjoint_volumes,
            UT_AutoInterrupt& progress)
        : mDstGeo(static_cast<GU_Detail&>(dst_geo))
        , mSrcVDB(src_vdb)
        , mSplitDisjoint(split_disjoint_volumes)
        , mProgress(progress)
    {
    }

    template<typename GridT>
    void operator()(const GridT &grid)
    {
        if (mSplitDisjoint) {
            vdbToDisjointVolumes(grid);
        } else {
            using LeafNodeType = typename GridT::TreeType::LeafNodeType;
            const openvdb::Index LEAF_DIM = LeafNodeType::DIM;

            VolumeT volume(mDstGeo);
            openvdb::CoordBBox bbox(grid.evalActiveVoxelBoundingBox());
            bool aligned = (   (bbox.min()[0] % LEAF_DIM) == 0
                            && (bbox.min()[1] % LEAF_DIM) == 0
                            && (bbox.min()[2] % LEAF_DIM) == 0
                            && ((bbox.max()[0]+1) % LEAF_DIM) == 0
                            && ((bbox.max()[1]+1) % LEAF_DIM) == 0
                            && ((bbox.max()[2]+1) % LEAF_DIM) == 0);
            vdbToVolume(grid, bbox, volume, aligned);
        }
    }

    const UT_IntArray& components() const
    {
        return mDstComponents;
    }

private:

    template<typename GridType>
    void vdbToVolume(const GridType& grid, const openvdb::CoordBBox& bbox,
                     VolumeT& volume, bool aligned);

    template<typename GridType>
    void vdbToDisjointVolumes(const GridType& grid);

private:
    GU_Detail&          mDstGeo;
    UT_IntArray         mDstComponents;
    const GU_PrimVDB&   mSrcVDB;
    bool                mSplitDisjoint;
    UT_AutoInterrupt&   mProgress;
};

// Copy the grid's bbox into volume at (0,0,0)
template<typename VolumeT>
template<typename GridType>
void
gu_ConvertFromVDB<VolumeT>::vdbToVolume(
        const GridType& grid,
        const openvdb::CoordBBox& bbox,
        VolumeT& vol,
        bool aligned)
{
    using LeafNodeType = typename GridType::TreeType::LeafNodeType;

    // Creating a Houdini volume with a zero bbox seems to break the transform.
    // (probably related to the bbox derived 'local space')
    openvdb::CoordBBox space_bbox = bbox;
    if (space_bbox.empty())
        space_bbox.resetToCube(openvdb::Coord(0, 0, 0), 1);
    vol.setSize(space_bbox.dim());

    vol.setVolumeOptions(mSrcVDB.isSDF(), grid.background(),
                         mSrcVDB.getVisualization(), mSrcVDB.getVisIso(),
                         mSrcVDB.getVisDensity());
    vol.setSpaceTransform(mSrcVDB.getSpaceTransform(UTvdbConvert(space_bbox)));

    for (int i = 0; i < VolumeT::TupleSize; i++)
        mDstComponents.append(i);

    // Copy the VDB bbox data to voxel array coord (0,0,0).
    SYS_STATIC_ASSERT(LeafNodeType::DIM <= TILESIZE);
    SYS_STATIC_ASSERT((TILESIZE % LeafNodeType::DIM) == 0);
    if (aligned) {
        gu_SparseTreeCopy<typename GridType::TreeType, VolumeT, true>
            copy(grid.tree(), vol, space_bbox.min(), mProgress);
        copy.run();
    } else {
        gu_SparseTreeCopy<typename GridType::TreeType, VolumeT, false>
            copy(grid.tree(), vol, space_bbox.min(), mProgress);
        copy.run();
    }
}

template<typename VolumeT>
template<typename GridType>
void
gu_ConvertFromVDB<VolumeT>::vdbToDisjointVolumes(const GridType& grid)
{
    using TreeType = typename GridType::TreeType;
    using NodeType = typename TreeType::RootNodeType::ChildNodeType;

    std::vector<const NodeType*> nodes;

    typename TreeType::NodeCIter iter = grid.tree().cbeginNode();
    iter.setMaxDepth(1);
    iter.setMinDepth(1);
    for (; iter; ++iter) {
        const NodeType* node = nullptr;
        iter.template getNode<const NodeType>(node);
        if (node) nodes.push_back(node);
    }

    std::vector<openvdb::CoordBBox> nodeBBox(nodes.size());
    for (size_t n = 0, N = nodes.size(); n < N; ++n) {
        nodes[n]->evalActiveBoundingBox(nodeBBox[n], false);
    }

    openvdb::CoordBBox regionA, regionB;

    const int searchDist = int(GridType::TreeType::LeafNodeType::DIM) << 1;

    for (size_t n = 0, N = nodes.size(); n < N; ++n) {
        if (!nodes[n]) continue;

        openvdb::CoordBBox& bbox = nodeBBox[n];

        regionA = bbox;
        regionA.max().offset(searchDist);

        bool expanded = true;

        while (expanded) {

            expanded = false;

            for (size_t i = (n + 1); i < N; ++i) {

                if (!nodes[i]) continue;

                regionB = nodeBBox[i];
                regionB.max().offset(searchDist);

                if (regionA.hasOverlap(regionB)) {

                    nodes[i] = nullptr;
                    expanded = true;

                    bbox.expand(nodeBBox[i]);

                    regionA = bbox;
                    regionA.max().offset(searchDist);
                }
            }
        }

        VolumeT volume(mDstGeo);
        vdbToVolume(grid, bbox, volume, /*aligned*/true);
    }
}

} // namespace anonymous

GEO_Primitive *
GU_PrimVDB::convertToPrimVolume(
        GEO_Detail &dst_geo,
        GU_ConvertParms &parms,
        bool split_disjoint_volumes) const
{
    using namespace openvdb;

    UT_AutoInterrupt    progress("Convert VDB to Volume");
    GA_Detail::OffsetMarker marker(dst_geo);
    UT_IntArray         dst_components;

    bool processed = false;
    { // Try to convert scalar grid
        gu_ConvertFromVDB< VoxelArrayVolume<1> >
            converter(dst_geo, *this, split_disjoint_volumes, progress);
        processed = GEOvdbProcessTypedGridScalar(*this, converter);
    }
    if (!processed) {  // Try to convert vector grid
        gu_ConvertFromVDB< VoxelArrayVolume<3> >
            converter(dst_geo, *this, split_disjoint_volumes, progress);
        processed = GEOvdbProcessTypedGridVec3(*this, converter);
        dst_components = converter.components();
    }

    // Copy attributes from source to dest primitives
    GA_Range pointrange(marker.pointRange());
    GA_Range primitiverange(marker.primitiveRange());
    if (!processed || primitiverange.isEmpty() || progress.wasInterrupted())
        return nullptr;

    GUconvertCopySingleVertexPrimAttribsAndGroups(
            parms, *getParent(), getMapOffset(),
            dst_geo, primitiverange, pointrange);

    // Handle the name attribute if needed
    if (dst_components.entries() > 0)
    {
        GA_ROHandleS src_name(getParent(), GA_ATTRIB_PRIMITIVE, "name");
        GA_RWHandleS dst_name(&dst_geo, GA_ATTRIB_PRIMITIVE, "name");

        if (src_name.isValid() && dst_name.isValid())
        {
            const UT_String name(src_name.get(getMapOffset()));
            if (name.isstring())
            {
                UT_String   full_name(name);
                int         last = name.length() + 1;
                const char  component[] = { 'x', 'y', 'z', 'w' };

                GA_Size nprimitives = primitiverange.getEntries();
                UT_ASSERT(dst_components.entries() == nprimitives);
                full_name += ".x";
                for (int j = 0; j < nprimitives; j++)
                {
                    int i = dst_components(j);
                    if (i < 4)
                        full_name(last) = component[i];
                    else
                        full_name.sprintf("%s%d", (const char *)name, i);
                    // NOTE: This assumes that the offsets are contiguous,
                    //       which is only valid if the converter didn't
                    //       delete anything.
                    dst_name.set(marker.primitiveBegin() + GA_Offset(i),
                                 full_name);
                }
            }
        }
    }

    return dst_geo.getGEOPrimitive(marker.primitiveBegin());
}

GEO_Primitive *
GU_PrimVDB::convert(GU_ConvertParms &parms, GA_PointGroup *usedpts)
{
    bool            success = false;
    GEO_Primitive * prim;

    prim = convertToNewPrim(*getParent(), parms,
                /*adaptivity*/0, /*sparse*/false, success);
    if (success)
    {
        if (usedpts)
            addPointRefToGroup(*usedpts);

        GA_PrimitiveGroup *group = parms.getDeletePrimitives();
        if (group)
            group->add(this);
        else
            getParent()->deletePrimitive(*this, !usedpts);
    }
    return prim;
}

/*static*/ void
GU_PrimVDB::convertVolumesToVDBs(
        GU_Detail &dst_geo,
        const GU_Detail &src_geo,
        GU_ConvertParms &parms,
        bool flood_sdf,
        bool prune,
        fpreal tolerance,
        bool keep_original,
        bool activate_inside_sdf)
{
    UT_AutoInterrupt progress("Convert");

    const GA_ROHandleS nameHandle(&src_geo, GA_ATTRIB_PRIMITIVE, "name");

    GEO_Primitive *prim;
    GEO_Primitive *next;
    GA_FOR_SAFE_GROUP_PRIMITIVES(&src_geo, parms.primGroup, prim, next)
    {
        if (progress.wasInterrupted())
            break;
        if (prim->getTypeId() != GEO_PRIMVOLUME)
            continue;

        GEO_PrimVolume *vol = UTverify_cast<GEO_PrimVolume*>(prim);
        GA_Offset voloff = vol->getMapOffset();
        GA_Detail::OffsetMarker marker(dst_geo);

        // Get the volume's name, if it has one.
        char const * const volname = (nameHandle.isValid() ? nameHandle.get(voloff) : nullptr);

        GU_PrimVDB *new_prim;
        new_prim = GU_PrimVDB::buildFromPrimVolume(
                dst_geo, *vol, volname, flood_sdf, prune, tolerance,
                activate_inside_sdf);
        if (!new_prim || progress.wasInterrupted())
            break;

        GA_Range pointrange(marker.pointRange());
        GA_Range primitiverange(marker.primitiveRange());
        GUconvertCopySingleVertexPrimAttribsAndGroups(
                parms, src_geo, voloff, dst_geo,
                primitiverange, pointrange);

        if (!keep_original && (&dst_geo == &src_geo))
            dst_geo.deletePrimitive(*vol, /*and points*/true);
    }
}

/*static*/ void
GU_PrimVDB::convertVDBs(
        GU_Detail &dst_geo,
        const GU_Detail &src_geo,
        GU_ConvertParms &parms,
        fpreal adaptivity,
        bool keep_original,
        bool split_disjoint_volumes)
{
    UT_AutoInterrupt    progress("Convert");

    GEO_Primitive *prim;
    GEO_Primitive *next;
    GA_FOR_SAFE_GROUP_PRIMITIVES(&src_geo, parms.primGroup, prim, next)
    {
        if (progress.wasInterrupted())
            break;

        GU_PrimVDB *vdb = dynamic_cast<GU_PrimVDB*>(prim);
        if (vdb == nullptr)
            continue;

        bool success = false;
        (void) vdb->convertToNewPrim(dst_geo, parms, adaptivity,
                                     split_disjoint_volumes, success);
        if (success && !keep_original && (&dst_geo == &src_geo))
            dst_geo.deletePrimitive(*vdb, /*and points*/true);
    }
}

/*static*/ void
GU_PrimVDB::convertVDBs(
        GU_Detail &dst_geo,
        const GU_Detail &src_geo,
        GU_ConvertParms &parms,
        fpreal adaptivity,
        bool keep_original)
{
    convertVDBs(dst_geo, src_geo, parms, adaptivity, keep_original, false);
}

void
GU_PrimVDB::normal(NormalComp& /*output*/) const
{
    // No need here.
}


////////////////////////////////////////


namespace {

template <typename T> struct IsScalarMeta
{ HBOOST_STATIC_CONSTANT(bool, value = true); };

#define DECLARE_VECTOR(METADATA_T) \
    template <> struct IsScalarMeta<METADATA_T> \
    { HBOOST_STATIC_CONSTANT(bool, value = false); }; \
    /**/
DECLARE_VECTOR(openvdb::Vec2IMetadata)
DECLARE_VECTOR(openvdb::Vec2SMetadata)
DECLARE_VECTOR(openvdb::Vec2DMetadata)
DECLARE_VECTOR(openvdb::Vec3IMetadata)
DECLARE_VECTOR(openvdb::Vec3SMetadata)
DECLARE_VECTOR(openvdb::Vec3DMetadata)
#undef DECLARE_VECTOR

template<typename T, typename MetadataT, int I, typename ENABLE = void>
struct MetaTuple
{
    static T get(const MetadataT& meta) {
        return meta.value()[I];
    }
};

template<typename T, typename MetadataT, int I>
struct MetaTuple<T, MetadataT, I, typename SYS_EnableIf< IsScalarMeta<MetadataT>::value >::type>
{
    static T get(const MetadataT& meta) {
        UT_ASSERT(I == 0);
        return meta.value();
    }
};

template<int I>
struct MetaTuple<const char*, openvdb::StringMetadata, I>
{
    static const char* get(const openvdb::StringMetadata& meta) {
    UT_ASSERT(I == 0);
        return meta.value().c_str();
    }
};

template <typename MetadataT> struct MetaAttr;

#define META_ATTR(METADATA_T, STORAGE, TUPLE_T, TUPLE_SIZE) \
    template <> \
    struct MetaAttr<METADATA_T> { \
        using TupleT = TUPLE_T; \
        using RWHandleT = GA_HandleT<TupleT>::RWType; \
        static const int theTupleSize = TUPLE_SIZE; \
        static const GA_Storage theStorage = STORAGE; \
    }; \
    /**/

META_ATTR(openvdb::BoolMetadata,   GA_STORE_INT8,   int8,        1)
META_ATTR(openvdb::FloatMetadata,  GA_STORE_REAL32, fpreal32,    1)
META_ATTR(openvdb::DoubleMetadata, GA_STORE_REAL64, fpreal64,    1)
META_ATTR(openvdb::Int32Metadata,  GA_STORE_INT32,  int32,       1)
META_ATTR(openvdb::Int64Metadata,  GA_STORE_INT64,  int64,       1)
//META_ATTR(openvdb::StringMetadata, GA_STORE_STRING, const char*, 1)
META_ATTR(openvdb::Vec2IMetadata,  GA_STORE_INT32,  int32,       2)
META_ATTR(openvdb::Vec2SMetadata,  GA_STORE_REAL32, fpreal32,    2)
META_ATTR(openvdb::Vec2DMetadata,  GA_STORE_REAL64, fpreal64,    2)
META_ATTR(openvdb::Vec3IMetadata,  GA_STORE_INT32,  int32,       3)
META_ATTR(openvdb::Vec3SMetadata,  GA_STORE_REAL32, fpreal32,    3)
META_ATTR(openvdb::Vec3DMetadata,  GA_STORE_REAL64, fpreal64,    3)

#undef META_ATTR

// Functor for setAttr()
using AttrSettor = std::function<
    void (GEO_Detail&, GA_AttributeOwner, GA_Offset, const char*, const openvdb::Metadata&)>;

template <typename MetadataT>
static void
setAttr(GEO_Detail& geo, GA_AttributeOwner owner, GA_Offset elem,
    const char* name, const openvdb::Metadata& meta_base)
{
    using MetaAttrT = MetaAttr<MetadataT>;
    using RWHandleT = typename MetaAttrT::RWHandleT;
    using TupleT = typename MetaAttrT::TupleT;

    /// @todo If there is an existing attribute with the given name but
    /// a different type, this will replace the old attribute with a new one.
    /// See GA_ReuseStrategy for alternative behaviors.
    GA_RWAttributeRef attrRef = geo.addTuple(MetaAttrT::theStorage, owner, name, MetaAttrT::theTupleSize);
    if (attrRef.isInvalid()) return;

    RWHandleT handle(attrRef.getAttribute());

    const MetadataT& meta = static_cast<const MetadataT&>(meta_base);
    switch (MetaAttrT::theTupleSize) {
    case 3: handle.set(elem, 2, MetaTuple<TupleT,MetadataT,2>::get(meta));
            SYS_FALLTHROUGH;
    case 2: handle.set(elem, 1, MetaTuple<TupleT,MetadataT,1>::get(meta));
            SYS_FALLTHROUGH;
    case 1: handle.set(elem, 0, MetaTuple<TupleT,MetadataT,0>::get(meta));
    }
    UT_ASSERT(MetaAttrT::theTupleSize >= 1 && MetaAttrT::theTupleSize <= 3);
}

/// for Houdini 12.1
template <typename MetadataT>
static void
setStrAttr(GEO_Detail& geo, GA_AttributeOwner owner, GA_Offset elem,
    const char* name, const openvdb::Metadata& meta_base)
{
    GA_RWAttributeRef attrRef = geo.addStringTuple(owner, name, 1);
    if (attrRef.isInvalid()) return;

    GA_RWHandleS handle(attrRef.getAttribute());

    const MetadataT& meta = static_cast<const MetadataT&>(meta_base);
    handle.set(elem, 0, MetaTuple<const char*, MetadataT, 0>::get(meta));
}

class MetaToAttrMap : public std::map<std::string, AttrSettor>
{
public:
    MetaToAttrMap()
    {
        using namespace openvdb;
        // Construct a mapping from OpenVDB metadata types to functions
        // that create attributes of corresponding types.
        (*this)[BoolMetadata::staticTypeName()]   = &setAttr<BoolMetadata>;
        (*this)[FloatMetadata::staticTypeName()]  = &setAttr<FloatMetadata>;
        (*this)[DoubleMetadata::staticTypeName()] = &setAttr<DoubleMetadata>;
        (*this)[Int32Metadata::staticTypeName()]  = &setAttr<Int32Metadata>;
        (*this)[Int64Metadata::staticTypeName()]  = &setAttr<Int64Metadata>;
        (*this)[StringMetadata::staticTypeName()] = &setStrAttr<StringMetadata>;
        (*this)[Vec2IMetadata::staticTypeName()]  = &setAttr<Vec2IMetadata>;
        (*this)[Vec2SMetadata::staticTypeName()]  = &setAttr<Vec2SMetadata>;
        (*this)[Vec2DMetadata::staticTypeName()]  = &setAttr<Vec2DMetadata>;
        (*this)[Vec3IMetadata::staticTypeName()]  = &setAttr<Vec3IMetadata>;
        (*this)[Vec3SMetadata::staticTypeName()]  = &setAttr<Vec3SMetadata>;
        (*this)[Vec3DMetadata::staticTypeName()]  = &setAttr<Vec3DMetadata>;
    }
};


static UT_SingletonWithLock<MetaToAttrMap> sMetaToAttrMap;

} // unnamed namespace


////////////////////////////////////////


void
GU_PrimVDB::syncAttrsFromMetadata()
{
    if (GEO_Detail* detail = this->getParent()) {
        createGridAttrsFromMetadata(*this, this->getConstGrid(), *detail);
    }
}


void
GU_PrimVDB::createGridAttrsFromMetadataAdapter(
        const GEO_PrimVDB& prim,
        const void* gridPtr,
        GEO_Detail& aGdp)
{
    createAttrsFromMetadataAdapter(
        GA_ATTRIB_PRIMITIVE, prim.getMapOffset(), gridPtr, aGdp);
}


void
GU_PrimVDB::createAttrsFromMetadataAdapter(
    GA_AttributeOwner owner,
    GA_Offset element,
    const void* meta_map_ptr,
    GEO_Detail& geo)
{
    // meta_map_ptr is assumed to point to an openvdb::vX_Y_Z::MetaMap, for some
    // version X.Y.Z of OpenVDB that may be newer than the one with which
    // libHoudiniGEO.so was built.  This is safe provided that MetaMap and
    // its member objects are ABI-compatible between the two OpenVDB versions.
    const openvdb::MetaMap& meta_map = *static_cast<const openvdb::MetaMap*>(meta_map_ptr);

    for (openvdb::MetaMap::ConstMetaIterator metaIt = meta_map.beginMeta(),
            metaEnd = meta_map.endMeta(); metaIt != metaEnd; ++metaIt) {

        if (openvdb::Metadata::Ptr meta = metaIt->second) {
            std::string name = metaIt->first;

            UT_String str(name);
            str.toLower();
            str.forceValidVariableName();
            UT_String prefixed(name);
            prefixed.prepend("vdb_");
            if (isIntrinsicMetadata(prefixed))
                continue;

            // If this grid's name is empty and a "name" attribute
            // doesn't already exist, don't create one.
            if (str == "name"
                && meta->typeName() == openvdb::StringMetadata::staticTypeName()
                && meta->str().empty())
            {
                if (!geo.findAttribute(owner, name.c_str())) continue;
            }

            MetaToAttrMap::const_iterator creatorIt =
                sMetaToAttrMap->find(meta->typeName());
            if (creatorIt != sMetaToAttrMap->end()) {
                creatorIt->second(geo, owner, element, name.c_str(), *meta);
            } else {
                /// @todo Add warning:
                // std::string("discarded metadata \"") + name
                //    + "\" of unsupported type " + meta->typeName()
            }
        }
    }
}


void
GU_PrimVDB::createMetadataFromGridAttrsAdapter(
        void* gridPtr,
        const GEO_PrimVDB& prim,
        const GEO_Detail& aGdp)
{
    createMetadataFromAttrsAdapter(
        gridPtr, GA_ATTRIB_PRIMITIVE, prim.getMapOffset(), aGdp);
}

void
GU_PrimVDB::createMetadataFromAttrsAdapter(
    void* meta_map_ptr,
    GA_AttributeOwner owner,
    GA_Offset element,
    const GEO_Detail& geo)
{
    using namespace openvdb;

    // meta_map_ptr is assumed to point to an openvdb::vX_Y_Z::MetaMap, for some
    // version X.Y.Z of OpenVDB that may be newer than the one with which
    // libHoudiniGEO.so was built.  This is safe provided that MetaMap and
    // its member objects are ABI-compatible between the two OpenVDB versions.
    openvdb::MetaMap& meta_map = *static_cast<openvdb::MetaMap*>(meta_map_ptr);

    const GA_AttributeSet& attrs = geo.getAttributes();
    for (GA_AttributeDict::iterator it = attrs.begin(owner, GA_SCOPE_PUBLIC); !it.atEnd(); ++it) {

        if (!it.name()) continue;

        std::string name = it.name();

        UT_String prefixed(name);
        prefixed.prepend("vdb_");
        if (isIntrinsicMetadata(prefixed))
            continue;

        const GA_Attribute* attrib = it.attrib();
        const GA_AIFTuple* tuple = attrib->getAIFTuple();
        const int entries = attrib->getTupleSize();

        switch (attrib->getStorageClass()) {

        case GA_STORECLASS_INT:
            if (!tuple)
                continue;
            switch (entries) {
            case 1:
                meta_map.removeMeta(name);
                if (name.substr(0, 3) == "is_") {
                    // Scalar integer attributes whose names begin with "is_"
                    // are mapped to boolean metadata.
                    if (tuple->getStorage(attrib) == GA_STORE_INT64) {
                        GA_ROHandleT<int64> handle(attrib);
                        meta_map.insertMeta(name, BoolMetadata(
                        handle.get(element) != 0));
                    } else {
                        GA_ROHandleT<int32> handle(attrib);
                        meta_map.insertMeta(name, BoolMetadata(
                        handle.get(element) != 0));
                    }
                } else {
                    if (tuple->getStorage(attrib) == GA_STORE_INT64) {
                        GA_ROHandleT<int64> handle(attrib);
                        meta_map.insertMeta(name, Int64Metadata(
                        handle.get(element)));
                    } else {
                        GA_ROHandleT<int32> handle(attrib);
                        meta_map.insertMeta(name, Int32Metadata(
                        handle.get(element)));
                    }
                }
                break;
            case 2:
                {
                    GA_ROHandleT<UT_Vector2i> handle(attrib);
                    meta_map.removeMeta(name);
                    meta_map.insertMeta(name, Vec2IMetadata(
                    UTvdbConvert(handle.get(element))));
                }
            break;
            case 3:
                {
                    GA_ROHandleT<UT_Vector3i> handle(attrib);
                    meta_map.removeMeta(name);
                    meta_map.insertMeta(name, Vec3IMetadata(
                    UTvdbConvert(handle.get(element))));
                }
            break;
            default:
                {
                    /// @todo Add warning:
                    //std::ostringstream ostr;
                    //ostr << "Skipped int[" << entries << "] metadata attribute \""
                    //    << it.name() << "\" (int tuples of size > 3 are not supported)";
                }
                break;
            }
            break;

        case GA_STORECLASS_FLOAT:
            if (!tuple)
                continue;
            switch (entries) {
            case 1:
                meta_map.removeMeta(name);
                if (tuple->getStorage(attrib) == GA_STORE_REAL64) {
                    GA_ROHandleT<fpreal64> handle(attrib);
                    meta_map.insertMeta(name, DoubleMetadata(
                    handle.get(element)));
                } else {
                    GA_ROHandleT<fpreal32> handle(attrib);
                    meta_map.insertMeta(name, FloatMetadata(
                    handle.get(element)));
                }
                break;
            case 2:
                meta_map.removeMeta(name);
                if (tuple->getStorage(attrib) == GA_STORE_REAL64) {
                    GA_ROHandleT<UT_Vector2D> handle(attrib);
                    meta_map.insertMeta(name, Vec2DMetadata(
                    UTvdbConvert(handle.get(element))));
                } else {
                    GA_ROHandleT<UT_Vector2F> handle(attrib);
                    meta_map.insertMeta(name, Vec2SMetadata(
                    UTvdbConvert(handle.get(element))));
                }
            break;
            case 3:
                meta_map.removeMeta(name);
                if (tuple->getStorage(attrib) == GA_STORE_REAL64) {
                    GA_ROHandleT<UT_Vector3D> handle(attrib);
                    meta_map.insertMeta(name, Vec3DMetadata(
                    UTvdbConvert(handle.get(element))));
                } else {
                    GA_ROHandleT<UT_Vector3F> handle(attrib);
                    meta_map.insertMeta(name, Vec3SMetadata(
                    UTvdbConvert(handle.get(element))));
                }
                break;
            default:
                {
                    /// @todo Add warning:
                    //std::ostringstream ostr;
                    //ostr << "Skipped float[" << entries << "] metadata attribute \""
                    //    << it.name() << "\" (float tuples of size > 3 are not supported)";
                }
                break;
            }
            break;

        case GA_STORECLASS_STRING: {
            GA_ROHandleS handle(attrib);
            if (entries == 1 && handle.isValid()) {
                meta_map.removeMeta(name);
                const char* str = handle.get(element);
                if (!str) str = "";
                meta_map.insertMeta(name, StringMetadata(str));
            } else {
                /// @todo Add warning:
                //std::ostringstream ostr;
                //ostr << "Skipped string[" << entries << "] metadata attribute \""
                //    << it.name() << "\" (string tuples are not supported)";
            }
            break;
        }

        case GA_STORECLASS_INVALID: break;
        case GA_STORECLASS_OTHER: break;
        }
    }
}


////////////////////////////////////////

// Following code is for HDK only
#ifndef SESI_OPENVDB
// This is the usual DSO hook.
extern "C" {
void
newGeometryPrim(GA_PrimitiveFactory *factory)
{
    GU_PrimVDB::registerMyself(factory);
}

} // extern "C"
#endif

#endif // SESI_OPENVDB || SESI_OPENVDB_PRIM
