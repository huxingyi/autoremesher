// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @author Ken Museth
///
/// @file LevelSetMeasure.h

#ifndef OPENVDB_TOOLS_LEVELSETMEASURE_HAS_BEEN_INCLUDED
#define OPENVDB_TOOLS_LEVELSETMEASURE_HAS_BEEN_INCLUDED

#include <openvdb/math/Math.h>
#include <openvdb/Types.h>
#include <openvdb/Grid.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/tree/ValueAccessor.h>
#include <openvdb/math/FiniteDifference.h>
#include <openvdb/math/Operators.h>
#include <openvdb/math/Stencils.h>
#include <openvdb/util/NullInterrupter.h>
#include <boost/math/constants/constants.hpp>//for Pi
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>
#include <tbb/parallel_invoke.h>
#include <type_traits>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace tools {

/// @brief Return the surface area of a narrow-band level set.
///
/// @param grid          a scalar, floating-point grid with one or more disjoint,
///                      closed level set surfaces
/// @param useWorldSpace if true the area is computed in
///                      world space units, else in voxel units.
///
/// @throw TypeError if @a grid is not scalar or not floating-point or not a level set or empty.
template<class GridType>
inline Real
levelSetArea(const GridType& grid, bool useWorldSpace = true);

/// @brief Return the volume of a narrow-band level set surface.
///
/// @param grid          a scalar, floating-point grid with one or more disjoint,
///                      closed level set surfaces
/// @param useWorldSpace if true the volume is computed in
///                      world space units, else in voxel units.
///
/// @throw TypeError if @a grid is not scalar or not floating-point or not a level set or empty.
template<class GridType>
inline Real
levelSetVolume(const GridType& grid, bool useWorldSpace = true);

/// @brief Return the Euler Characteristics of a narrow-band level set surface (possibly disconnected).
///
/// @param grid          a scalar, floating-point grid with one or more disjoint,
///                      closed level set surfaces
///
/// @throw TypeError if @a grid is not scalar or not floating-point or not a level set or empty.
template<class GridType>
inline int
levelSetEulerCharacteristic(const GridType& grid);

/// @brief Return the genus of a narrow-band level set surface.
///
/// @param grid          a scalar, floating-point grid with one or more disjoint,
///                      closed level set surfaces
/// @warning The genus is only well defined for a single connected surface
///
/// @throw TypeError if @a grid is not scalar or not floating-point or not a level set or empty.
template<class GridType>
inline int
levelSetGenus(const GridType& grid);

////////////////////////////////////////////////////////////////////////////////////////

/// @brief Smeared-out and continuous Dirac Delta function.
template<typename RealT>
class DiracDelta
{
public:
    // eps is the half-width of the dirac delta function in units of phi
    DiracDelta(RealT eps) : mC(0.5/eps), mD(2*boost::math::constants::pi<RealT>()*mC), mE(eps) {}
    // values of the dirac delta function are in units of one over the units of phi
    inline RealT operator()(RealT phi) const { return math::Abs(phi) > mE ? 0 : mC*(1+cos(mD*phi)); }
private:
    const RealT mC, mD, mE;
};// DiracDelta functor


/// @brief Multi-threaded computation of surface area, volume and
/// average mean-curvature for narrow band level sets.
///
/// @details To reduce the risk of round-off errors (primarily due to
/// catastrophic cancellation) and guarantee determinism during
/// multi-threading this class is implemented using parallel_for, and
/// delayed reduction of a sorted list.
template<typename GridT, typename InterruptT = util::NullInterrupter>
class LevelSetMeasure
{
public:
    using GridType = GridT;
    using TreeType = typename GridType::TreeType;
    using ValueType = typename TreeType::ValueType;
    using ManagerType = typename tree::LeafManager<const TreeType>;

    static_assert(std::is_floating_point<ValueType>::value,
        "level set measure is supported only for scalar, floating-point grids");

    /// @brief Main constructor from a grid
    /// @param grid The level set to be measured.
    /// @param interrupt Optional interrupter.
    /// @throw RuntimeError if the grid is not a level set or if it's empty.
    LevelSetMeasure(const GridType& grid, InterruptT* interrupt = nullptr);

    /// @brief Re-initialize using the specified grid.
    /// @param grid The level set to be measured.
    /// @throw RuntimeError if the grid is not a level set or if it's empty.
    void init(const GridType& grid);

    /// @brief Destructor
    virtual ~LevelSetMeasure() {}

     /// @return the grain-size used for multi-threading
    int getGrainSize() const { return mGrainSize; }

    /// @brief Set the grain-size used for multi-threading.
    /// @note A grain size of 0 or less disables multi-threading!
    void setGrainSize(int grainsize) { mGrainSize = grainsize; }

    /// @brief Compute the surface area of the level set.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real area(bool useWorldUnits = true);

    /// @brief Compute the volume of the level set surface.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real volume(bool useWorldUnits = true);

    /// @brief Compute the total mean curvature of the level set surface.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real totMeanCurvature(bool useWorldUnits = true);

    /// @brief Compute the total gaussian curvature of the level set surface.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real totGaussianCurvature(bool useWorldUnits = true);

    /// @brief Compute the average mean curvature of the level set surface.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real avgMeanCurvature(bool useWorldUnits = true) {return this->totMeanCurvature(useWorldUnits) / this->area(useWorldUnits);}

    /// @brief Compute the average gaussian curvature of the level set surface.
    /// @param useWorldUnits Specifies if the result is in world or voxel units.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    Real avgGaussianCurvature(bool useWorldUnits = true) {return this->totGaussianCurvature(useWorldUnits) / this->area(useWorldUnits); }

    /// @brief Compute the Euler characteristic of the level set surface.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    int eulerCharacteristic();

    /// @brief Compute the genus of the level set surface.
    /// @warning The genus is only well defined for a single connected surface.
    /// @note Performs internal caching so only the initial call incurs actual computation.
    int genus() { return 1 - this->eulerCharacteristic()/2;}

private:

    using LeafT = typename TreeType::LeafNodeType;
    using VoxelCIterT = typename LeafT::ValueOnCIter;
    using LeafRange = typename ManagerType::LeafRange;
    using LeafIterT = typename LeafRange::Iterator;
    using ManagerPtr = std::unique_ptr<ManagerType>;
    using BufferPtr  = std::unique_ptr<double[]>;

    // disallow copy construction and copy by assignment!
    LevelSetMeasure(const LevelSetMeasure&);// not implemented
    LevelSetMeasure& operator=(const LevelSetMeasure&);// not implemented

    const GridType *mGrid;
    ManagerPtr      mLeafs;
    BufferPtr       mBuffer;
    InterruptT     *mInterrupter;
    double          mDx, mArea, mVolume, mTotMeanCurvature, mTotGausCurvature;
    int             mGrainSize;
    bool            mUpdateArea, mUpdateCurvature;

    // @brief Return false if the process was interrupted
    bool checkInterrupter();

    struct MeasureArea
    {
        MeasureArea(LevelSetMeasure* parent) : mParent(parent), mStencil(*mParent->mGrid)
        {
            if (parent->mInterrupter) parent->mInterrupter->start("Measuring area and volume of level set");
            if (parent->mGrainSize>0) {
                tbb::parallel_for(parent->mLeafs->leafRange(parent->mGrainSize), *this);
            } else {
                (*this)(parent->mLeafs->leafRange());
            }
            tbb::parallel_invoke([&](){parent->mArea   = parent->reduce(0);},
                                 [&](){parent->mVolume = parent->reduce(1)/3.0;});
            parent->mUpdateArea = false;
            if (parent->mInterrupter) parent->mInterrupter->end();
        }
        MeasureArea(const MeasureArea& other) : mParent(other.mParent), mStencil(*mParent->mGrid) {}
        void operator()(const LeafRange& range) const;
        LevelSetMeasure* mParent;
        mutable math::GradStencil<GridT, false> mStencil;
    };// MeasureArea

    struct MeasureCurvatures
    {
        MeasureCurvatures(LevelSetMeasure* parent) : mParent(parent), mStencil(*mParent->mGrid)
        {
            if (parent->mInterrupter) parent->mInterrupter->start("Measuring curvatures of level set");
            if (parent->mGrainSize>0) {
                tbb::parallel_for(parent->mLeafs->leafRange(parent->mGrainSize), *this);
            } else {
                (*this)(parent->mLeafs->leafRange());
            }
            tbb::parallel_invoke([&](){parent->mTotMeanCurvature = parent->reduce(0);},
                                 [&](){parent->mTotGausCurvature = parent->reduce(1);});
            parent->mUpdateCurvature = false;
            if (parent->mInterrupter) parent->mInterrupter->end();
        }
        MeasureCurvatures(const MeasureCurvatures& other) : mParent(other.mParent), mStencil(*mParent->mGrid) {}
        void operator()(const LeafRange& range) const;
        LevelSetMeasure* mParent;
        mutable math::CurvatureStencil<GridT, false> mStencil;
    };// MeasureCurvatures

    double reduce(int offset)
    {
        double *first = mBuffer.get() + offset*mLeafs->leafCount(), *last = first + mLeafs->leafCount();
        tbb::parallel_sort(first, last);// mitigates catastrophic cancellation
        Real sum = 0.0;
        while(first != last) sum += *first++;
        return sum;
    }

}; // end of LevelSetMeasure class


template<typename GridT, typename InterruptT>
inline
LevelSetMeasure<GridT, InterruptT>::LevelSetMeasure(const GridType& grid, InterruptT* interrupt)
    : mInterrupter(interrupt)
    , mGrainSize(1)
{
    this->init(grid);
}

template<typename GridT, typename InterruptT>
inline void
LevelSetMeasure<GridT, InterruptT>::init(const GridType& grid)
{
    if (!grid.hasUniformVoxels()) {
         OPENVDB_THROW(RuntimeError,
             "The transform must have uniform scale for the LevelSetMeasure to function");
    }
    if (grid.getGridClass() != GRID_LEVEL_SET) {
        OPENVDB_THROW(RuntimeError,
            "LevelSetMeasure only supports level sets;"
            " try setting the grid class to \"level set\"");
    }
    if (grid.empty()) {
        OPENVDB_THROW(RuntimeError,
            "LevelSetMeasure does not support empty grids;");
    }
    mGrid = &grid;
    mDx = grid.voxelSize()[0];
    mLeafs  = std::make_unique<ManagerType>(mGrid->tree());
    mBuffer = std::make_unique<double[]>(2*mLeafs->leafCount());
    mUpdateArea = mUpdateCurvature = true;
}

template<typename GridT, typename InterruptT>
inline Real
LevelSetMeasure<GridT, InterruptT>::area(bool useWorldUnits)
{
    if (mUpdateArea) {MeasureArea m(this);};
    double area = mArea;
    if (useWorldUnits) area *= math::Pow2(mDx);
    return area;
}

template<typename GridT, typename InterruptT>
inline Real
LevelSetMeasure<GridT, InterruptT>::volume(bool useWorldUnits)
{
    if (mUpdateArea) {MeasureArea m(this);};
    double volume = mVolume;
    if (useWorldUnits) volume *= math::Pow3(mDx) ;
    return volume;
}

template<typename GridT, typename InterruptT>
inline Real
LevelSetMeasure<GridT, InterruptT>::totMeanCurvature(bool useWorldUnits)
{
    if (mUpdateCurvature) {MeasureCurvatures m(this);};
    return mTotMeanCurvature * (useWorldUnits ? mDx : 1);
}

template<typename GridT, typename InterruptT>
inline Real
LevelSetMeasure<GridT, InterruptT>::totGaussianCurvature(bool)
{
    if (mUpdateCurvature) {MeasureCurvatures m(this);};
    return mTotGausCurvature;
}

template<typename GridT, typename InterruptT>
inline int
LevelSetMeasure<GridT, InterruptT>::eulerCharacteristic()
{
    const Real x = this->totGaussianCurvature(true) / (2.0*boost::math::constants::pi<Real>());
    return int(math::Round( x ));
}

///////////////////////// PRIVATE METHODS //////////////////////

template<typename GridT, typename InterruptT>
inline bool
LevelSetMeasure<GridT, InterruptT>::checkInterrupter()
{
    if (util::wasInterrupted(mInterrupter)) {
        tbb::task::self().cancel_group_execution();
        return false;
    }
    return true;
}

template<typename GridT, typename InterruptT>
inline void
LevelSetMeasure<GridT, InterruptT>::
MeasureArea::operator()(const LeafRange& range) const
{
    using Vec3T = math::Vec3<ValueType>;
    // computations are performed in index space where dV = 1
    mParent->checkInterrupter();
    const Real invDx = 1.0/mParent->mDx;
    const DiracDelta<Real> DD(1.5);// dirac delta function is 3 voxel units wide
    const size_t leafCount = mParent->mLeafs->leafCount();
    for (LeafIterT leafIter=range.begin(); leafIter; ++leafIter) {
        Real sumA = 0, sumV = 0;//reduce risk of catastrophic cancellation
        for (VoxelCIterT voxelIter = leafIter->cbeginValueOn(); voxelIter; ++voxelIter) {
            const Real dd = DD(invDx * (*voxelIter));
            if (dd > 0.0) {
                mStencil.moveTo(voxelIter);
                const Coord& p = mStencil.getCenterCoord();// in voxel units
                const Vec3T g  = mStencil.gradient();// in world units
                sumA += dd*g.length();// \delta(\phi)*|\nabla\phi|
                sumV += dd*(g[0]*Real(p[0]) + g[1]*Real(p[1]) + g[2]*Real(p[2]));// \delta(\phi)\vec{x}\cdot\nabla\phi
            }
        }
        double* ptr = mParent->mBuffer.get() + leafIter.pos();
        *ptr = sumA;
        ptr += leafCount;
        *ptr = sumV;
    }
}

template<typename GridT, typename InterruptT>
inline void
LevelSetMeasure<GridT, InterruptT>::
MeasureCurvatures::operator()(const LeafRange& range) const
{
    using Vec3T = math::Vec3<ValueType>;
    // computations are performed in index space where dV = 1
    mParent->checkInterrupter();
    const Real dx = mParent->mDx, dx2=dx*dx, invDx = 1.0/dx;
    const DiracDelta<Real> DD(1.5);// dirac delta function is 3 voxel units wide
    ValueType mean, gauss;
    const size_t leafCount = mParent->mLeafs->leafCount();
    for (LeafIterT leafIter=range.begin(); leafIter; ++leafIter) {
        Real sumM = 0, sumG = 0;//reduce risk of catastrophic cancellation
        for (VoxelCIterT voxelIter = leafIter->cbeginValueOn(); voxelIter; ++voxelIter) {
            const Real dd = DD(invDx * (*voxelIter));
            if (dd > 0.0) {
                mStencil.moveTo(voxelIter);
                const Vec3T g  = mStencil.gradient();
                const Real dA  = dd*g.length();// \delta(\phi)*\delta(\phi)
                mStencil.curvatures(mean, gauss);
                sumM += dA*mean*dx;//   \delta(\phi)*\delta(\phi)*MeanCurvature
                sumG += dA*gauss*dx2;// \delta(\phi)*\delta(\phi)*GaussCurvature
            }
        }
        double* ptr = mParent->mBuffer.get() + leafIter.pos();
        *ptr = sumM;
        ptr += leafCount;
        *ptr = sumG;
    }
}

////////////////////////////////////////

//{
/// @cond OPENVDB_LEVEL_SET_MEASURE_INTERNAL

template<class GridT>
inline
typename std::enable_if<std::is_floating_point<typename GridT::ValueType>::value, Real>::type
doLevelSetArea(const GridT& grid, bool useWorldUnits)
{
    LevelSetMeasure<GridT> m(grid);
    return m.area(useWorldUnits);
}

template<class GridT>
inline
typename std::enable_if<!std::is_floating_point<typename GridT::ValueType>::value, Real>::type
doLevelSetArea(const GridT&, bool)
{
    OPENVDB_THROW(TypeError,
        "level set area is supported only for scalar, floating-point grids");
}

/// @endcond
//}

template<class GridT>
inline Real
levelSetArea(const GridT& grid, bool useWorldUnits)
{
    return doLevelSetArea<GridT>(grid, useWorldUnits);
}

////////////////////////////////////////

//{
/// @cond OPENVDB_LEVEL_SET_MEASURE_INTERNAL

template<class GridT>
inline
typename std::enable_if<std::is_floating_point<typename GridT::ValueType>::value, Real>::type
doLevelSetVolume(const GridT& grid, bool useWorldUnits)
{
    LevelSetMeasure<GridT> m(grid);
    return m.volume(useWorldUnits);
}

template<class GridT>
inline
typename std::enable_if<!std::is_floating_point<typename GridT::ValueType>::value, Real>::type
doLevelSetVolume(const GridT&, bool)
{
    OPENVDB_THROW(TypeError,
        "level set volume is supported only for scalar, floating-point grids");
}

/// @endcond
//}

template<class GridT>
inline Real
levelSetVolume(const GridT& grid, bool useWorldUnits)
{
    return doLevelSetVolume<GridT>(grid, useWorldUnits);
}

////////////////////////////////////////

//{
/// @cond OPENVDB_LEVEL_SET_MEASURE_INTERNAL

template<class GridT>
inline
typename std::enable_if<std::is_floating_point<typename GridT::ValueType>::value, int>::type
doLevelSetEulerCharacteristic(const GridT& grid)
{
    LevelSetMeasure<GridT> m(grid);
    return m.eulerCharacteristic();
}

template<class GridT>
inline
typename std::enable_if<!std::is_floating_point<typename GridT::ValueType>::value, int>::type
doLevelSetEulerCharacteristic(const GridT&)
{
    OPENVDB_THROW(TypeError,
        "level set euler characteristic is supported only for scalar, floating-point grids");
}

/// @endcond
//}


template<class GridT>
inline int
levelSetEulerCharacteristic(const GridT& grid)
{
    return doLevelSetEulerCharacteristic(grid);
}

////////////////////////////////////////

//{
/// @cond OPENVDB_LEVEL_SET_MEASURE_INTERNAL

template<class GridT>
inline
typename std::enable_if<std::is_floating_point<typename GridT::ValueType>::value, int>::type
doLevelSetEuler(const GridT& grid)
{
    LevelSetMeasure<GridT> m(grid);
    return m.eulerCharacteristics();

}

template<class GridT>
inline
typename std::enable_if<std::is_floating_point<typename GridT::ValueType>::value, int>::type
doLevelSetGenus(const GridT& grid)
{
    LevelSetMeasure<GridT> m(grid);
    return m.genus();
}

template<class GridT>
inline
typename std::enable_if<!std::is_floating_point<typename GridT::ValueType>::value, int>::type
doLevelSetGenus(const GridT&)
{
    OPENVDB_THROW(TypeError,
        "level set genus is supported only for scalar, floating-point grids");
}

/// @endcond
//}

template<class GridT>
inline int
levelSetGenus(const GridT& grid)
{
    return doLevelSetGenus(grid);
}

////////////////////////////////////////////////////////////////////////////////////////

/// @deprecated Use the @a LevelSetMeasure class instead.
template<class GridT>
OPENVDB_DEPRECATED
inline void
levelSetMeasure(const GridT& grid, Real& area, Real& volume, Real& avgCurvature,
                bool useWorldUnits = true)
{
    LevelSetMeasure<GridT> m(grid);
    area = m.area(useWorldUnits);
    volume = m.volume(useWorldUnits);
    avgCurvature = m.avgMeanCurvature(useWorldUnits);
}

} // namespace tools
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_TOOLS_LEVELSETMEASURE_HAS_BEEN_INCLUDED
