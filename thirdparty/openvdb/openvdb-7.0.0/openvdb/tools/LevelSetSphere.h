// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
///
/// @file LevelSetSphere.h
///
/// @brief Generate a narrow-band level set of sphere.
///
/// @note By definition a level set has a fixed narrow band width
/// (the half width is defined by LEVEL_SET_HALF_WIDTH in Types.h),
/// whereas an SDF can have a variable narrow band width.

#ifndef OPENVDB_TOOLS_LEVELSETSPHERE_HAS_BEEN_INCLUDED
#define OPENVDB_TOOLS_LEVELSETSPHERE_HAS_BEEN_INCLUDED

#include <openvdb/Grid.h>
#include <openvdb/Types.h>
#include <openvdb/math/Math.h>
#include <openvdb/util/NullInterrupter.h>
#include "SignedFloodFill.h"
#include <type_traits>

#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <thread>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace tools {

/// @brief Return a grid of type @c GridType containing a narrow-band level set
/// representation of a sphere.
///
/// @param radius       radius of the sphere in world units
/// @param center       center of the sphere in world units
/// @param voxelSize    voxel size in world units
/// @param halfWidth    half the width of the narrow band, in voxel units
/// @param interrupt    a pointer adhering to the util::NullInterrupter interface
/// @param threaded     if true multi-threading is enabled (true by default)
///
/// @note @c GridType::ValueType must be a floating-point scalar.
/// @note The leapfrog algorithm employed in this method is best suited
/// for a single large sphere.  For multiple small spheres consider
/// using the faster algorithm in ParticlesToLevelSet.h
template<typename GridType, typename InterruptT>
typename GridType::Ptr
createLevelSetSphere(float radius, const openvdb::Vec3f& center, float voxelSize,
                     float halfWidth = float(LEVEL_SET_HALF_WIDTH),
                     InterruptT* interrupt = nullptr, bool threaded = true);

/// @brief Return a grid of type @c GridType containing a narrow-band level set
/// representation of a sphere.
///
/// @param radius       radius of the sphere in world units
/// @param center       center of the sphere in world units
/// @param voxelSize    voxel size in world units
/// @param halfWidth    half the width of the narrow band, in voxel units
/// @param threaded     if true multi-threading is enabled (true by default)
///
/// @note @c GridType::ValueType must be a floating-point scalar.
/// @note The leapfrog algorithm employed in this method is best suited
/// for a single large sphere.  For multiple small spheres consider
/// using the faster algorithm in ParticlesToLevelSet.h
template<typename GridType>
typename GridType::Ptr
createLevelSetSphere(float radius, const openvdb::Vec3f& center, float voxelSize,
                     float halfWidth = float(LEVEL_SET_HALF_WIDTH), bool threaded = true)
{
    return createLevelSetSphere<GridType, util::NullInterrupter>(radius,center,voxelSize,halfWidth,nullptr,threaded);
}


////////////////////////////////////////


/// @brief Generates a signed distance field (or narrow band level
/// set) to a single sphere.
///
/// @note The leapfrog algorithm employed in this class is best
/// suited for a single large sphere. For multiple small spheres consider
/// using the faster algorithm in tools/ParticlesToLevelSet.h
template<typename GridT, typename InterruptT = util::NullInterrupter>
class LevelSetSphere
{
public:
    using TreeT  = typename GridT::TreeType;
    using ValueT = typename GridT::ValueType;
    using Vec3T  = typename math::Vec3<ValueT>;
    static_assert(std::is_floating_point<ValueT>::value,
        "level set grids must have scalar, floating-point value types");

    /// @brief Constructor
    ///
    /// @param radius radius of the sphere in world units
    /// @param center center of the sphere in world units
    /// @param interrupt pointer to optional interrupter. Use template
    /// argument util::NullInterrupter if no interruption is desired.
    ///
    /// @note If the radius of the sphere is smaller than
    /// 1.5*voxelSize, i.e. the sphere is smaller than the Nyquist
    /// frequency of the grid, it is ignored!
    LevelSetSphere(ValueT radius, const Vec3T &center, InterruptT* interrupt = nullptr)
        : mRadius(radius), mCenter(center), mInterrupt(interrupt)
    {
        if (mRadius<=0) OPENVDB_THROW(ValueError, "radius must be positive");
    }

    /// @return a narrow-band level set of the sphere
    ///
    /// @param voxelSize  Size of voxels in world units
    /// @param halfWidth  Half-width of narrow-band in voxel units
    /// @param threaded   If true multi-threading is enabled (true by default)
    typename GridT::Ptr getLevelSet(ValueT voxelSize, ValueT halfWidth, bool threaded = true)
    {
        mGrid = createLevelSet<GridT>(voxelSize, halfWidth);
        this->rasterSphere(voxelSize, halfWidth, threaded);
        mGrid->setGridClass(GRID_LEVEL_SET);
        return mGrid;
    }

private:
    void rasterSphere(ValueT dx, ValueT w, bool threaded)
    {
        if (!(dx>0.0f)) OPENVDB_THROW(ValueError, "voxel size must be positive");
        if (!(w>1)) OPENVDB_THROW(ValueError, "half-width must be larger than one");

        // Define radius of sphere and narrow-band in voxel units
        const ValueT r0 = mRadius/dx, rmax = r0 + w;

        // Radius below the Nyquist frequency
        if (r0 < 1.5f)  return;

        // Define center of sphere in voxel units
        const Vec3T c(mCenter[0]/dx, mCenter[1]/dx, mCenter[2]/dx);

        // Define bounds of the voxel coordinates
        const int imin=math::Floor(c[0]-rmax), imax=math::Ceil(c[0]+rmax);
        const int jmin=math::Floor(c[1]-rmax), jmax=math::Ceil(c[1]+rmax);
        const int kmin=math::Floor(c[2]-rmax), kmax=math::Ceil(c[2]+rmax);

        // Allocate a ValueAccessor for accelerated random access
        typename GridT::Accessor accessor = mGrid->getAccessor();

        if (mInterrupt) mInterrupt->start("Generating level set of sphere");

        tbb::enumerable_thread_specific<TreeT> pool(mGrid->tree());

        auto kernel = [&](const tbb::blocked_range<int>& r) {
            openvdb::Coord ijk;
            int &i = ijk[0], &j = ijk[1], &k = ijk[2], m=1;
            TreeT &tree = pool.local();
            typename GridT::Accessor acc(tree);
            // Compute signed distances to sphere using leapfrogging in k
            for (i = r.begin(); i <= r.end(); ++i) {
                if (util::wasInterrupted(mInterrupt)) return;
                const auto x2 = math::Pow2(ValueT(i) - c[0]);
                for (j = jmin; j <= jmax; ++j) {
                    const auto x2y2 = math::Pow2(ValueT(j) - c[1]) + x2;
                    for (k = kmin; k <= kmax; k += m) {
                        m = 1;
                        // Distance in voxel units to sphere
                        const auto v = math::Sqrt(x2y2 + math::Pow2(ValueT(k)-c[2]))-r0;
                        const auto d = math::Abs(v);
                        if (d < w) { // inside narrow band
                            acc.setValue(ijk, dx*v);// distance in world units
                        } else { // outside narrow band
                            m += math::Floor(d-w);// leapfrog
                        }
                    }//end leapfrog over k
                }//end loop over j
            }//end loop over i
        };// kernel

        if (threaded) {
            // The code blow is making use of a TLS container to minimize the number of concurrent trees
            // initially populated by tbb::parallel_for and subsequently merged by tbb::parallel_reduce.
            // Experiments have demonstrated this approach to outperform others, including serial reduction
            // and a custom concurrent reduction implementation.
            tbb::parallel_for(tbb::blocked_range<int>(imin, imax, 128), kernel);
            using RangeT = tbb::blocked_range<typename tbb::enumerable_thread_specific<TreeT>::iterator>;
            struct Op {
                const bool mDelete;
                TreeT *mTree;
                Op(TreeT &tree) : mDelete(false), mTree(&tree) {}
                Op(const Op& other, tbb::split) : mDelete(true), mTree(new TreeT(other.mTree->background())) {}
                ~Op() { if (mDelete) delete mTree; }
                void operator()(RangeT &r) { for (auto i=r.begin(); i!=r.end(); ++i) this->merge(*i);}
                void join(Op &other) { this->merge(*(other.mTree)); }
                void merge(TreeT &tree) { mTree->merge(tree, openvdb::MERGE_ACTIVE_STATES); }
            } op( mGrid->tree() );
            tbb::parallel_reduce(RangeT(pool.begin(), pool.end(), 4), op);
        } else {
            kernel(tbb::blocked_range<int>(imin, imax));//serial
            mGrid->tree().merge(*pool.begin(), openvdb::MERGE_ACTIVE_STATES);
        }

        // Define consistent signed distances outside the narrow-band
        tools::signedFloodFill(mGrid->tree(), threaded);

        if (mInterrupt) mInterrupt->end();
    }

    const ValueT        mRadius;
    const Vec3T         mCenter;
    InterruptT*         mInterrupt;
    typename GridT::Ptr mGrid;
};// LevelSetSphere


////////////////////////////////////////


template<typename GridType, typename InterruptT>
typename GridType::Ptr
createLevelSetSphere(float radius, const openvdb::Vec3f& center, float voxelSize,
    float halfWidth, InterruptT* interrupt, bool threaded)
{
    // GridType::ValueType is required to be a floating-point scalar.
    static_assert(std::is_floating_point<typename GridType::ValueType>::value,
        "level set grids must have scalar, floating-point value types");

    using ValueT = typename GridType::ValueType;
    LevelSetSphere<GridType, InterruptT> factory(ValueT(radius), center, interrupt);
    return factory.getLevelSet(ValueT(voxelSize), ValueT(halfWidth), threaded);
}

} // namespace tools
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_TOOLS_LEVELSETSPHERE_HAS_BEEN_INCLUDED
