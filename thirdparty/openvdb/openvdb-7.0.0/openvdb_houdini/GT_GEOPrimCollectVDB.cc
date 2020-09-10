// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c) Side Effects Software Inc.
 *
 * Produced by:
 *      Side Effects Software Inc
 *      123 Front Street West, Suite 1401
 *      Toronto, Ontario
 *      Canada   M5J 2M2
 *      416-504-9876
 *
 * NAME:        GT_GEOPrimCollectVDB.h (GT Library, C++)
 *
 * COMMENTS:
 */

#include "GT_GEOPrimCollectVDB.h"
#include "UT_VDBUtils.h"

#include <GT/GT_DANumeric.h>
#include <GT/GT_GEOPrimCollect.h>
#include <GT/GT_GEOPrimitive.h>
#include <GT/GT_Handles.h>
#include <GT/GT_PrimCurveMesh.h>
#include <GU/GU_DetailHandle.h>

#include "GEO_PrimVDB.h"
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Version.h>

#include <openvdb/openvdb.h>
#include <openvdb/Grid.h>

using namespace openvdb_houdini;

void
GT_GEOPrimCollectVDB::registerPrimitive(const GA_PrimitiveTypeId &id)
{
    new GT_GEOPrimCollectVDB(id);
}

namespace
{

class gt_RefineVDB
{
public:
    gt_RefineVDB(
            const GU_Detail &gdp,
            const GT_GEOOffsetList &vdb_offsets)
        : myGdp(gdp)
        , myVDBOffsets(vdb_offsets)
        , myPos(new GT_Real32Array(0, 3, GT_TYPE_POINT))
        , myPosHandle(myPos)
    {
    }
    gt_RefineVDB(
            const gt_RefineVDB &task,
            UT_Split)
        : myGdp(task.myGdp)
        , myVDBOffsets(task.myVDBOffsets)
        , myPos(new GT_Real32Array(0, 3, GT_TYPE_POINT))
        , myPosHandle(myPos)
    {
    }

    enum { NPTS = 8 };

    void
    appendBox(openvdb::Vec3s corners[NPTS])
    {
        myVertexCounts.append(NPTS * 2);
        myPos->append(corners[0].asPointer()); // 0
        myPos->append(corners[1].asPointer()); // 1
        myPos->append(corners[2].asPointer()); // 2
        myPos->append(corners[3].asPointer()); // 3
        myPos->append(corners[0].asPointer()); // 4
        myPos->append(corners[4].asPointer()); // 5
        myPos->append(corners[5].asPointer()); // 6
        myPos->append(corners[6].asPointer()); // 7
        myPos->append(corners[7].asPointer()); // 8
        myPos->append(corners[4].asPointer()); // 9
        myPos->append(corners[5].asPointer()); // 10
        myPos->append(corners[1].asPointer()); // 11
        myPos->append(corners[2].asPointer()); // 12
        myPos->append(corners[6].asPointer()); // 13
        myPos->append(corners[7].asPointer()); // 14
        myPos->append(corners[3].asPointer()); // 15
    }

    template <typename GridT>
    void
    processGrid(const GridT &grid, int /*dummy*/)
    {
        using namespace openvdb;
        typedef typename GridT::TreeType TreeT;
        typedef typename TreeT::LeafCIter LeafCIter;
        typedef typename TreeT::LeafNodeType LeafNodeType;

        const openvdb::math::Transform &xform = grid.transform();

        bool appended = false;
        for (LeafCIter iter = grid.tree().cbeginLeaf(); iter; ++iter)
        {
            LeafNodeType const * const leaf = iter.getLeaf();
            const Vec3d half(0.5);
            Vec3d bbox_pos[2];

            /// Nodes are rendered as cell-centered (0.5 voxel dilated)
            /// AABBox in world space
            bbox_pos[0] = leaf->origin() - half;
            bbox_pos[1] = leaf->origin().offsetBy(leaf->dim() - 1) + half;

            Vec3s corners[NPTS];
            Coord lut[NPTS] = {
                Coord(0, 0, 0),
                Coord(0, 0, 1),
                Coord(1, 0, 1),
                Coord(1, 0, 0),
                Coord(0, 1, 0),
                Coord(0, 1, 1),
                Coord(1, 1, 1),
                Coord(1, 1, 0),
            };
            for (int i = 0; i < NPTS; i++)
            {
                Vec3d pt(bbox_pos[lut[i][0]].x(),
                         bbox_pos[lut[i][1]].y(),
                         bbox_pos[lut[i][2]].z());
                corners[i] = xform.indexToWorld(pt);
            }
            appendBox(corners);
            appended = true;
        }

        if (!appended)
        {
            const int NPTS = 6;
            openvdb::Vec3s lines[NPTS];

            lines[0].init(-0.5, 0.0, 0.0);
            lines[1].init( 0.5, 0.0, 0.0);
            lines[2].init( 0.0,-0.5, 0.0);
            lines[3].init( 0.0, 0.5, 0.0);
            lines[4].init( 0.0, 0.0,-0.5);
            lines[5].init( 0.0, 0.0, 0.5);

            for (int i = 0; i < NPTS; i++)
                lines[i] = xform.indexToWorld(lines[i]);

            for (int i = 0; i < NPTS; i += 2)
            {
                myVertexCounts.append(2);
                myPos->append(lines[i].asPointer());
                myPos->append(lines[i+1].asPointer());
            }
        }
    }

    void
    operator()(const UT_BlockedRange<exint> &range)
    {
        using namespace openvdb;

        for (exint i = range.begin(); i != range.end(); ++i)
        {
            const GEO_Primitive *prim = myGdp.getGEOPrimitive(myVDBOffsets(i));
            const GEO_PrimVDB *vdb = static_cast<const GEO_PrimVDB *>(prim);

            UTvdbCallAllType(vdb->getStorageType(), processGrid, vdb->getGrid(), 0);
        }
    }

    void
    join(const gt_RefineVDB &task)
    {
        myPos->concat(*task.myPos);
        myVertexCounts.concat(task.myVertexCounts);
    }

    const GU_Detail &           myGdp;
    const GT_GEOOffsetList &    myVDBOffsets;
    GT_Real32Array *            myPos;
    GT_DataArrayHandle          myPosHandle;
    GT_GEOOffsetList            myVertexCounts;
};

}


GT_GEOPrimCollectVDB::GT_GEOPrimCollectVDB(const GA_PrimitiveTypeId &id)
    : myId(id)
{
    bind(myId);
}

GT_GEOPrimCollectVDB::~GT_GEOPrimCollectVDB()
{
}

GT_GEOPrimCollectData *
GT_GEOPrimCollectVDB::beginCollecting(
        const GT_GEODetailListHandle &,
        const GT_RefineParms *) const
{
    return new GT_GEOPrimCollectOffsets();
}

GT_PrimitiveHandle
GT_GEOPrimCollectVDB::collect(
        const GT_GEODetailListHandle &/*geometry*/,
        const GEO_Primitive *const* prim_list,
        int /*nsegments*/,
        GT_GEOPrimCollectData *data) const
{
    data->asPointer<GT_GEOPrimCollectOffsets>()->append(prim_list[0]);
    return GT_PrimitiveHandle();
}

GT_PrimitiveHandle
GT_GEOPrimCollectVDB::endCollecting(
        const GT_GEODetailListHandle &g,
        GT_GEOPrimCollectData *data) const
{
    const GT_GEOPrimCollectOffsets &
                offsets = *(data->asPointer<GT_GEOPrimCollectOffsets>());
    const GT_GEOOffsetList &    prims = offsets.getPrimitives();

    if (!prims.entries())
        return GT_PrimitiveHandle();

    GU_DetailHandleAutoReadLock gdl(g->getGeometry(0));
    const GU_Detail* detail = gdl.getGdp();
    gt_RefineVDB task(*detail, prims);

    UTparallelReduce(UT_BlockedRange<exint>(0, prims.entries()), task);

    GT_DataArrayHandle vertex_counts = task.myVertexCounts.allocateArray();

    GT_AttributeListHandle

    vertices = GT_AttributeList::createAttributeList("P", task.myPos);

    return GT_PrimitiveHandle(
                new GT_PrimCurveMesh(
                        GT_BASIS_LINEAR,
                        vertex_counts,
                        vertices.get(),
                        GT_AttributeListHandle(), // uniform
                        GT_AttributeListHandle(), // detail
                        /*wrap*/false));
}
