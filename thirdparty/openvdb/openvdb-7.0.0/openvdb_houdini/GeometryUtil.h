// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file GeometryUtil.h
/// @author FX R&D Simulation team
/// @brief Utility methods and tools for geometry processing

#ifndef OPENVDB_HOUDINI_GEOMETRY_UTIL_HAS_BEEN_INCLUDED
#define OPENVDB_HOUDINI_GEOMETRY_UTIL_HAS_BEEN_INCLUDED

#include <openvdb/openvdb.h>
#include <openvdb/tools/MeshToVolume.h> // for openvdb::tools::MeshToVoxelEdgeData
#include <openvdb/tree/LeafManager.h>
#include <openvdb/util/Util.h> // for openvdb::util::COORD_OFFSETS

#include <GU/GU_Detail.h>
#include <UT/UT_Version.h>

#include <algorithm> // for std::max/min()
#include <memory>
#include <string>
#include <vector>


class GA_SplittableRange;
class OBJ_Camera;
class OP_Context;
class OP_Node;


#ifdef SESI_OPENVDB
    #ifdef OPENVDB_HOUDINI_API
        #undef OPENVDB_HOUDINI_API
        #define OPENVDB_HOUDINI_API
    #endif
#endif


namespace openvdb_houdini {

class Interrupter;


/// Add geometry to the given detail to indicate the extents of a frustum transform.
OPENVDB_HOUDINI_API
void
drawFrustum(GU_Detail&, const openvdb::math::Transform&,
    const UT_Vector3* boxColor, const UT_Vector3* tickColor,
    bool shaded, bool drawTicks = true);


/// Construct a frustum transform from a Houdini camera.
OPENVDB_HOUDINI_API
openvdb::math::Transform::Ptr
frustumTransformFromCamera(
    OP_Node&, OP_Context&, OBJ_Camera&,
    float offset, float nearPlaneDist, float farPlaneDist,
    float voxelDepthSize = 1.0, int voxelCountX = 100);


////////////////////////////////////////


/// @brief Return @c true if the point at the given offset is referenced
/// by primitives from a certain primitive group.
OPENVDB_HOUDINI_API
bool
pointInPrimGroup(GA_Offset ptnOffset, GU_Detail&, const GA_PrimitiveGroup&);


////////////////////////////////////////


/// @brief Convert geometry to quads and triangles.
/// @return a pointer to a new GU_Detail object if the geometry was
/// converted or subdivided, otherwise a null pointer
OPENVDB_HOUDINI_API
std::unique_ptr<GU_Detail>
convertGeometry(const GU_Detail&, std::string& warning, Interrupter*);


////////////////////////////////////////


/// TBB body object for threaded world to voxel space transformation and copy of points
class OPENVDB_HOUDINI_API TransformOp
{
public:
    TransformOp(GU_Detail const * const gdp,
        const openvdb::math::Transform& transform,
        std::vector<openvdb::Vec3s>& pointList);

    void operator()(const GA_SplittableRange&) const;

private:
    GU_Detail const * const mGdp;
    const openvdb::math::Transform& mTransform;
    std::vector<openvdb::Vec3s>* const mPointList;
};


////////////////////////////////////////


/// @brief   TBB body object for threaded primitive copy
/// @details Produces a primitive-vertex index list.
class OPENVDB_HOUDINI_API PrimCpyOp
{
public:
    PrimCpyOp(GU_Detail const * const gdp, std::vector<openvdb::Vec4I>& primList);
    void operator()(const GA_SplittableRange&) const;

private:
    GU_Detail const * const mGdp;
    std::vector<openvdb::Vec4I>* const mPrimList;
};


////////////////////////////////////////


/// @brief   TBB body object for threaded vertex normal generation
/// @details Averages face normals from all similarly oriented primitives,
///          that share the same vertex-point, to maintain sharp features.
class OPENVDB_HOUDINI_API VertexNormalOp
{
public:
    VertexNormalOp(GU_Detail&, const GA_PrimitiveGroup* interiorPrims=nullptr, float angle=0.7f);
    void operator()(const GA_SplittableRange&) const;

private:
    bool isInteriorPrim(GA_Offset primOffset) const
    {
        return mInteriorPrims && mInteriorPrims->containsIndex(
            mDetail.primitiveIndex(primOffset));
    }

    const GU_Detail& mDetail;
    const GA_PrimitiveGroup* mInteriorPrims;
    GA_RWHandleV3 mNormalHandle;
    const float mAngle;
};


////////////////////////////////////////


/// TBB body object for threaded sharp feature construction
class OPENVDB_HOUDINI_API SharpenFeaturesOp
{
public:
    using EdgeData = openvdb::tools::MeshToVoxelEdgeData;

    SharpenFeaturesOp(GU_Detail& meshGeo, const GU_Detail& refGeo, EdgeData& edgeData,
        const openvdb::math::Transform& xform, const GA_PrimitiveGroup* surfacePrims = nullptr,
        const openvdb::BoolTree* mask = nullptr);

    void operator()(const GA_SplittableRange&) const;

private:
    GU_Detail& mMeshGeo;
    const GU_Detail& mRefGeo;
    EdgeData& mEdgeData;
    const openvdb::math::Transform& mXForm;
    const GA_PrimitiveGroup* mSurfacePrims;
    const openvdb::BoolTree* mMaskTree;
};


////////////////////////////////////////


/// TBB body object for threaded sharp feature construction
template<typename IndexTreeType, typename BoolTreeType>
class GenAdaptivityMaskOp
{
public:
    using BoolLeafManager = openvdb::tree::LeafManager<BoolTreeType>;

    GenAdaptivityMaskOp(const GU_Detail& refGeo,
        const IndexTreeType& indexTree, BoolLeafManager&, float edgetolerance = 0.0);

    void run(bool threaded = true);

    void operator()(const tbb::blocked_range<size_t>&) const;

private:
    const GU_Detail& mRefGeo;
    const IndexTreeType& mIndexTree;
    BoolLeafManager& mLeafs;
    float mEdgeTolerance;
};


template<typename IndexTreeType, typename BoolTreeType>
GenAdaptivityMaskOp<IndexTreeType, BoolTreeType>::GenAdaptivityMaskOp(const GU_Detail& refGeo,
    const IndexTreeType& indexTree, BoolLeafManager& leafMgr, float edgetolerance)
    : mRefGeo(refGeo)
    , mIndexTree(indexTree)
    , mLeafs(leafMgr)
    , mEdgeTolerance(edgetolerance)
{
    mEdgeTolerance = std::max(0.0f, mEdgeTolerance);
    mEdgeTolerance = std::min(1.0f, mEdgeTolerance);
}


template<typename IndexTreeType, typename BoolTreeType>
void
GenAdaptivityMaskOp<IndexTreeType, BoolTreeType>::run(bool threaded)
{
    if (threaded) {
        tbb::parallel_for(mLeafs.getRange(), *this);
    } else {
        (*this)(mLeafs.getRange());
    }
}


template<typename IndexTreeType, typename BoolTreeType>
void
GenAdaptivityMaskOp<IndexTreeType, BoolTreeType>::operator()(
    const tbb::blocked_range<size_t>& range) const
{
    using IndexAccessorType = typename openvdb::tree::ValueAccessor<const IndexTreeType>;
    IndexAccessorType idxAcc(mIndexTree);

    UT_Vector3 tmpN, normal;
    GA_Offset primOffset;
    int tmpIdx;

    openvdb::Coord ijk, nijk;
    typename BoolTreeType::LeafNodeType::ValueOnIter iter;

    for (size_t n = range.begin(); n < range.end(); ++n) {
        iter = mLeafs.leaf(n).beginValueOn();
        for (; iter; ++iter) {
            ijk = iter.getCoord();

            bool edgeVoxel = false;

            int idx = idxAcc.getValue(ijk);

            primOffset = mRefGeo.primitiveOffset(idx);
            normal = mRefGeo.getGEOPrimitive(primOffset)->computeNormal();

            for (size_t i = 0; i < 18; ++i) {
                nijk = ijk + openvdb::util::COORD_OFFSETS[i];
                if (idxAcc.probeValue(nijk, tmpIdx) && tmpIdx != idx) {
                    primOffset = mRefGeo.primitiveOffset(tmpIdx);
                    tmpN = mRefGeo.getGEOPrimitive(primOffset)->computeNormal();

                    if (normal.dot(tmpN) < mEdgeTolerance) {
                        edgeVoxel = true;
                        break;
                    }
                }
            }

            if (!edgeVoxel) iter.setValueOff();
        }
    }
}


} // namespace openvdb_houdini


////////////////////////////////////////


#endif // OPENVDB_HOUDINI_GEOMETRY_UTIL_HAS_BEEN_INCLUDED
