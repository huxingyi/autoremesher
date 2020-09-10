// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file GU_VDBPointTools.h
/// @author FX R&D OpenVDB team
///
/// @brief Collection of PointIndexGrid helpers for Houdini

#ifndef GU_VDBPOINTTOOLS_H_HAS_BEEN_INCLUDED
#define GU_VDBPOINTTOOLS_H_HAS_BEEN_INCLUDED

#if defined(SESI_OPENVDB)
    #include "GU_Detail.h"
    #include "GU_DetailHandle.h"
    #include "GU_PackedContext.h"
    #include "GU_PackedFragment.h"
    #include "GU_PackedGeometry.h"
    #include "GU_PrimPacked.h"
#else
    #include <GU/GU_Detail.h>
    #include <GU/GU_DetailHandle.h>
    #include <GU/GU_PackedContext.h>
    #include <GU/GU_PackedFragment.h>
    #include <GU/GU_PackedGeometry.h>
    #include <GU/GU_PrimPacked.h>
#endif
#include <GA/GA_ElementGroup.h>
#include <UT/UT_SharedPtr.h>
#include <UT/UT_VectorTypes.h>
#include <UT/UT_Version.h>

#include <openvdb/Platform.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <openvdb/tools/ParticleAtlas.h>
#include <openvdb/tools/PointsToMask.h>

#include <vector>


/// @brief Houdini point attribute wrapper
template<typename VectorType>
struct GU_VDBPointList
{
    using Ptr = UT_SharedPtr<GU_VDBPointList>;
    using ConstPtr = UT_SharedPtr<const GU_VDBPointList>;

    using PosType = VectorType;
    using ScalarType = typename PosType::value_type;

    GU_VDBPointList(const GU_Detail& detail, const GA_PointGroup* group = nullptr)
        : mPositionHandle(detail.getP())
        , mVelocityHandle()
        , mRadiusHandle()
        , mIndexMap(&detail.getP()->getIndexMap())
        , mOffsets()
        , mSize(mIndexMap->indexSize())
    {
        if (group) {
            mSize = group->entries();
            mOffsets.reserve(mSize);

            GA_Offset start, end;
            GA_Range range(*group);
            for (GA_Iterator it = range.begin(); it.blockAdvance(start, end); ) {
                for (GA_Offset off = start; off < end; ++off) {
                    mOffsets.push_back(off);
                }
            }

            getOffset = &GU_VDBPointList::offsetFromGroupMap;
        } else if (mIndexMap->isTrivialMap()) {
            getOffset = &GU_VDBPointList::offsetFromIndexCast;
        } else {
            getOffset = &GU_VDBPointList::offsetFromGeoMap;
        }

        // Bind optional attributes

        GA_ROAttributeRef velRef = detail.findFloatTuple(GA_ATTRIB_POINT, GEO_STD_ATTRIB_VELOCITY, 3);
        if (velRef.isValid()) {
            mVelocityHandle.bind(velRef.getAttribute());
        }

        GA_ROAttributeRef radRef = detail.findFloatTuple(GA_ATTRIB_POINT, GEO_STD_ATTRIB_PSCALE);
        if (radRef.isValid()) {
            mRadiusHandle.bind(radRef.getAttribute());
        }
    }

    static Ptr create(const GU_Detail& detail, const GA_PointGroup* group = nullptr)
    {
        return Ptr(new GU_VDBPointList(detail, group));
    }

    size_t size() const { return mSize; }

    bool hasVelocity() const { return mVelocityHandle.isValid(); }
    bool hasRadius() const { return mRadiusHandle.isValid(); }

    // Index access methods

    void getPos(size_t n, PosType& xyz) const {
        getPosFromOffset((this->*getOffset)(n), xyz);
    }

    void getVelocity(size_t n, PosType& v) const {
        getVelocityFromOffset((this->*getOffset)(n), v);
    }

    void getRadius(size_t n, ScalarType& r) const {
        getRadiusFromOffset((this->*getOffset)(n), r);
    }

    // Offset access methods

    GA_Offset offsetFromIndex(size_t n) const {
        return (this->*getOffset)(n);
    }

    void getPosFromOffset(const GA_Offset offset, PosType& xyz) const {
        const UT_Vector3 data = mPositionHandle.get(offset);
        xyz[0] = ScalarType(data[0]);
        xyz[1] = ScalarType(data[1]);
        xyz[2] = ScalarType(data[2]);
    }

    void getVelocityFromOffset(const GA_Offset offset, PosType& v) const {
        const UT_Vector3 data = mVelocityHandle.get(offset);
        v[0] = ScalarType(data[0]);
        v[1] = ScalarType(data[1]);
        v[2] = ScalarType(data[2]);
    }

    void getRadiusFromOffset(const GA_Offset offset, ScalarType& r) const {
        r = ScalarType(mRadiusHandle.get(offset));
    }

private:
    // Disallow copying
    GU_VDBPointList(const GU_VDBPointList&);
    GU_VDBPointList& operator=(const GU_VDBPointList&);

    GA_Offset (GU_VDBPointList::* getOffset)(const size_t) const;

    GA_Offset offsetFromGeoMap(const size_t n) const {
        return mIndexMap->offsetFromIndex(GA_Index(n));
    }

    GA_Offset offsetFromGroupMap(const size_t n) const {
        return mOffsets[n];
    }

    GA_Offset offsetFromIndexCast(const size_t n) const {
        return GA_Offset(n);
    }

    GA_ROHandleV3 mPositionHandle, mVelocityHandle;
    GA_ROHandleF mRadiusHandle;
    GA_IndexMap const * const mIndexMap;
    std::vector<GA_Offset> mOffsets;
    size_t mSize;
}; // GU_VDBPointList


////////////////////////////////////////


// PointIndexGrid utility methods


namespace GU_VDBPointToolsInternal {

template<typename PointArrayType>
struct IndexToOffsetOp {
    IndexToOffsetOp(const PointArrayType& points): mPointList(&points) {}

    template <typename LeafT>
    void operator()(LeafT &leaf, size_t /*leafIndex*/) const {
        typename LeafT::IndexArray& indices = leaf.indices();
        for (size_t n = 0, N = indices.size(); n < N; ++n) {
             indices[n] = static_cast<typename LeafT::ValueType::IntType>(
                 mPointList->offsetFromIndex(GA_Index{indices[n]}));
        }
    }
    PointArrayType const * const mPointList;
};


struct PackedMaskConstructor
{
    PackedMaskConstructor(const std::vector<const GA_Primitive*>& prims,
        const openvdb::math::Transform& xform)
        : mPrims(prims.empty() ? nullptr : &prims.front())
        , mXForm(xform)
        , mMaskGrid(new openvdb::MaskGrid(false))
    {
        mMaskGrid->setTransform(mXForm.copy());
    }

    PackedMaskConstructor(PackedMaskConstructor& rhs, tbb::split)
        : mPrims(rhs.mPrims)
        , mXForm(rhs.mXForm)
        , mMaskGrid(new openvdb::MaskGrid(false))
    {
        mMaskGrid->setTransform(mXForm.copy());
    }

    openvdb::MaskGrid::Ptr getMaskGrid() { return mMaskGrid; }

    void join(PackedMaskConstructor& rhs) { mMaskGrid->tree().topologyUnion(rhs.mMaskGrid->tree()); }

    void operator()(const tbb::blocked_range<size_t>& range)
    {
        GU_PackedContext packedcontext;

        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
            const GA_Primitive *prim = mPrims[n];
            if (!prim || !GU_PrimPacked::isPackedPrimitive(*prim)) continue;

            const GU_PrimPacked * pprim = static_cast<const GU_PrimPacked*>(prim);

            GU_Detail tmpdetail;
            const GU_Detail *detailtouse;

            GU_DetailHandleAutoReadLock readlock(pprim->getPackedDetail(packedcontext));

            UT_Matrix4D mat;
            pprim->getFullTransform4(mat);
            if (mat.isIdentity() && readlock.isValid() && readlock.getGdp()) {
                detailtouse = readlock.getGdp();
            } else {
                pprim->unpackWithContext(tmpdetail, packedcontext);
                detailtouse = &tmpdetail;
            }

            GU_VDBPointList<openvdb::Vec3R>  points(*detailtouse);
            openvdb::MaskGrid::Ptr grid = openvdb::tools::createPointMask(points, mXForm);
            mMaskGrid->tree().topologyUnion(grid->tree());
        }
    }

private:
    GA_Primitive const * const * const  mPrims;
    openvdb::math::Transform            mXForm;
    openvdb::MaskGrid::Ptr              mMaskGrid;
}; // struct PackedMaskConstructor


inline void
getPackedPrimitiveOffsets(const GU_Detail& detail, std::vector<const GA_Primitive*>& primitives)
{
    const GA_Size numPacked = GU_PrimPacked::countPackedPrimitives(detail);

    primitives.clear();
    primitives.reserve(size_t(numPacked));

    if (numPacked != GA_Size(0)) {
        GA_Offset start, end;
        GA_Range range = detail.getPrimitiveRange();
        const GA_PrimitiveList& primList = detail.getPrimitiveList();

        for (GA_Iterator it = range.begin(); it.blockAdvance(start, end); ) {
            for (GA_Offset off = start; off < end; ++off) {

                const GA_Primitive *prim = primList.get(off);

                if (prim && GU_PrimPacked::isPackedPrimitive(*prim)) {
                    primitives.push_back(prim);
                }
            }
        }
    }
}

} // namespace GU_VDBPointToolsInternal


////////////////////////////////////////


/// @brief    Utility method to construct a GU_VDBPointList.
/// @details  The GU_VDBPointList is compatible with the PointIndexGrid and ParticleAtals structures.
inline GU_VDBPointList<openvdb::Vec3s>::Ptr
GUvdbCreatePointList(const GU_Detail& detail, const GA_PointGroup* pointGroup = nullptr)
{
    return GU_VDBPointList<openvdb::Vec3s>::create(detail, pointGroup);
}


/// @brief  Utility method to change point indices into Houdini geometry offsets.
/// @note   PointIndexGrid's that store Houdini geometry offsets are not
///         safe to write to disk, offsets are not guaranteed to be immutable
///         under defragmentation operations or I/O.
template<typename PointIndexTreeType, typename PointArrayType>
inline void
GUvdbConvertIndexToOffset(PointIndexTreeType& tree, const PointArrayType& points)
{
    openvdb::tree::LeafManager<PointIndexTreeType> leafnodes(tree);
    leafnodes.foreach(GU_VDBPointToolsInternal::IndexToOffsetOp<PointArrayType>(points));
}


/// @brief    Utility method to construct a PointIndexGrid.
/// @details  The PointIndexGrid supports fast spatial queries for points.
inline openvdb::tools::PointIndexGrid::Ptr
GUvdbCreatePointIndexGrid(
    const openvdb::math::Transform& xform,
    const GU_Detail& detail,
    const GA_PointGroup* pointGroup = nullptr)
{
    GU_VDBPointList<openvdb::Vec3s> points(detail, pointGroup);
    return openvdb::tools::createPointIndexGrid<openvdb::tools::PointIndexGrid>(points, xform);
}


/// @brief    Utility method to construct a PointIndexGrid.
/// @details  The PointIndexGrid supports fast spatial queries for points.
template<typename PointArrayType>
inline openvdb::tools::PointIndexGrid::Ptr
GUvdbCreatePointIndexGrid(const openvdb::math::Transform& xform, const PointArrayType& points)
{
    return openvdb::tools::createPointIndexGrid<openvdb::tools::PointIndexGrid>(points, xform);
}


/// @brief    Utility method to construct a ParticleAtals.
/// @details  The ParticleAtals supports fast spatial queries for particles.
template<typename ParticleArrayType>
inline openvdb::tools::ParticleIndexAtlas::Ptr
GUvdbCreateParticleAtlas(const double minVoxelSize, const ParticleArrayType& particles)
{
    using ParticleIndexAtlas = openvdb::tools::ParticleIndexAtlas;
    ParticleIndexAtlas::Ptr atlas(new ParticleIndexAtlas());

    if (particles.hasRadius()) {
        atlas->construct(particles, minVoxelSize);
    }

    return atlas;
}


/// @brief    Utility method to construct a boolean PointMaskGrid
/// @details  This method supports packed points.
inline openvdb::MaskGrid::Ptr
GUvdbCreatePointMaskGrid(
    const openvdb::math::Transform& xform,
    const GU_Detail& detail,
    const GA_PointGroup* pointGroup = nullptr)
{
    std::vector<const GA_Primitive*> packed;
    GU_VDBPointToolsInternal::getPackedPrimitiveOffsets(detail, packed);

    if (!packed.empty()) {
        GU_VDBPointToolsInternal::PackedMaskConstructor op(packed, xform);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, packed.size()), op);
        return op.getMaskGrid();
    }

    GU_VDBPointList<openvdb::Vec3R> points(detail, pointGroup);
    return openvdb::tools::createPointMask(points, xform);
}


/// @brief  Utility method to construct a PointIndexGrid that stores
///         Houdini geometry offsets.
///
/// @note  PointIndexGrid's that store Houdini geometry offsets are not
///        safe to write to disk, offsets are not guaranteed to be immutable
///        under defragmentation operations or I/O.
inline openvdb::tools::PointIndexGrid::Ptr
GUvdbCreatePointOffsetGrid(
    const openvdb::math::Transform& xform,
    const GU_Detail& detail,
    const GA_PointGroup* pointGroup = nullptr)
{
    GU_VDBPointList<openvdb::Vec3s> points(detail, pointGroup);

    openvdb::tools::PointIndexGrid::Ptr grid =
        openvdb::tools::createPointIndexGrid<openvdb::tools::PointIndexGrid>(points, xform);

    GUvdbConvertIndexToOffset(grid->tree(), points);

    return grid;
}


#endif // GU_VDBPOINTTOOLS_H_HAS_BEEN_INCLUDED
