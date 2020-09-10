// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file DDA.h
///
/// @author Ken Museth
///
/// @brief Digital Differential Analyzers specialized for VDB.

#ifndef OPENVDB_MATH_DDA_HAS_BEEN_INCLUDED
#define OPENVDB_MATH_DDA_HAS_BEEN_INCLUDED

#include "Coord.h"
#include "Math.h"
#include "Vec3.h"
#include <openvdb/Types.h>
#include <iostream> // for std::ostream
#include <limits> // for std::numeric_limits<Type>::max()
#include <boost/mpl/at.hpp>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace math {

/// @brief A Digital Differential Analyzer specialized for OpenVDB grids
/// @note Conceptually similar to Bresenham's line algorithm applied
/// to a 3D Ray intersecting OpenVDB nodes or voxels. Log2Dim = 0
/// corresponds to a voxel and Log2Dim a tree node of size 2^Log2Dim.
///
/// @note The Ray template class is expected to have the following
/// methods: test(time), t0(), t1(), invDir(), and  operator()(time).
/// See the example Ray class above for their definition.
template<typename RayT, Index Log2Dim = 0>
class DDA
{
public:
    using RealType = typename RayT::RealType;
    using RealT = RealType;
    using Vec3Type = typename RayT::Vec3Type;
    using Vec3T = Vec3Type;

    /// @brief uninitialized constructor
    DDA() {}

    DDA(const RayT& ray) { this->init(ray); }

    DDA(const RayT& ray, RealT startTime) { this->init(ray, startTime); }

    DDA(const RayT& ray, RealT startTime, RealT maxTime) { this->init(ray, startTime, maxTime); }

    inline void init(const RayT& ray, RealT startTime, RealT maxTime)
    {
        assert(startTime <= maxTime);
        static const int DIM = 1 << Log2Dim;
        mT0 = startTime;
        mT1 = maxTime;
        const Vec3T &pos = ray(mT0), &dir = ray.dir(), &inv = ray.invDir();
        mVoxel = Coord::floor(pos) & (~(DIM-1));
        for (int axis = 0; axis < 3; ++axis) {
            if (math::isZero(dir[axis])) {//handles dir = +/- 0
                mStep[axis]  = 0;//dummy value
                mNext[axis]  = std::numeric_limits<RealT>::max();//i.e. disabled!
                mDelta[axis] = std::numeric_limits<RealT>::max();//dummy value
            } else if (inv[axis] > 0) {
                mStep[axis]  = DIM;
                mNext[axis]  = mT0 + (mVoxel[axis] + DIM - pos[axis]) * inv[axis];
                mDelta[axis] = mStep[axis] * inv[axis];
            } else {
                mStep[axis]  = -DIM;
                mNext[axis]  = mT0 + (mVoxel[axis] - pos[axis]) * inv[axis];
                mDelta[axis] = mStep[axis] * inv[axis];
            }
        }
    }

    inline void init(const RayT& ray) { this->init(ray, ray.t0(), ray.t1()); }

    inline void init(const RayT& ray, RealT startTime) { this->init(ray, startTime, ray.t1()); }

    /// @brief Increment the voxel index to next intersected voxel or node
    /// and returns true if the step in time does not exceed maxTime.
    inline bool step()
    {
        const int stepAxis = static_cast<int>(math::MinIndex(mNext));
        mT0 = mNext[stepAxis];
        mNext[stepAxis]  += mDelta[stepAxis];
        mVoxel[stepAxis] += mStep[stepAxis];
        return mT0 <= mT1;
    }

    /// @brief Return the index coordinates of the next node or voxel
    /// intersected by the ray. If Log2Dim = 0 the return value is the
    /// actual signed coordinate of the voxel, else it is the origin
    /// of the corresponding VDB tree node or tile.
    /// @note Incurs no computational overhead.
    inline const Coord& voxel() const { return mVoxel; }

    /// @brief Return the time (parameterized along the Ray) of the
    /// first hit of a tree node of size 2^Log2Dim.
    /// @details This value is initialized to startTime or ray.t0()
    /// depending on the constructor used.
    /// @note Incurs no computational overhead.
    inline RealType time() const { return mT0; }

    /// @brief Return the maximum time (parameterized along the Ray).
    inline RealType maxTime() const { return mT1; }

    /// @brief Return the time (parameterized along the Ray) of the
    /// second (i.e. next) hit of a tree node of size 2^Log2Dim.
    /// @note Incurs a (small) computational overhead.
    inline RealType next() const { return math::Min(mT1, mNext[0], mNext[1], mNext[2]); }

    /// @brief Print information about this DDA for debugging.
    /// @param os    a stream to which to write textual information.
    void print(std::ostream& os = std::cout) const
    {
        os << "Dim=" << (1<<Log2Dim) << " time=" << mT0 << " next()="
            << this->next() << " voxel=" << mVoxel << " next=" << mNext
            << " delta=" << mDelta << " step=" << mStep << std::endl;
    }

private:
    RealT mT0, mT1;
    Coord mVoxel, mStep;
    Vec3T mDelta, mNext;
}; // class DDA

/// @brief Output streaming of the Ray class.
/// @note Primarily intended for debugging.
template<typename RayT, Index Log2Dim>
inline std::ostream& operator<<(std::ostream& os, const DDA<RayT, Log2Dim>& dda)
{
    os << "Dim="     << (1<<Log2Dim) << " time="  << dda.time()
       << " next()=" << dda.next()   << " voxel=" << dda.voxel();
    return os;
}

/////////////////////////////////////////// LevelSetHDDA ////////////////////////////////////////////


/// @brief Helper class that implements Hierarchical Digital Differential Analyzers
/// and is specialized for ray intersections with level sets
template<typename TreeT, int NodeLevel>
struct LevelSetHDDA
{
    using ChainT = typename TreeT::RootNodeType::NodeChainType;
    using NodeT = typename boost::mpl::at<ChainT, boost::mpl::int_<NodeLevel> >::type;

    template <typename TesterT>
    static bool test(TesterT& tester)
    {
        math::DDA<typename TesterT::RayT, NodeT::TOTAL> dda(tester.ray());
        do {
            if (tester.template hasNode<NodeT>(dda.voxel())) {
                tester.setRange(dda.time(), dda.next());
                if (LevelSetHDDA<TreeT, NodeLevel-1>::test(tester)) return true;
            }
        } while(dda.step());
        return false;
    }
};

/// @brief Specialization of Hierarchical Digital Differential Analyzer
/// class that intersects a ray against the voxels of a level set
template<typename TreeT>
struct LevelSetHDDA<TreeT, -1>
{
    template <typename TesterT>
    static bool test(TesterT& tester)
    {
        math::DDA<typename TesterT::RayT, 0> dda(tester.ray());
        tester.init(dda.time());
        do { if (tester(dda.voxel(), dda.next())) return true; } while(dda.step());
        return false;
    }
};

//////////////////////////////////////////// VolumeHDDA /////////////////////////////////////////////

/// @brief Helper class that implements Hierarchical Digital Differential Analyzers
/// for ray intersections against a generic volume.
///
/// @details The template argument ChildNodeLevel specifies the entry
/// upper node level used for the hierarchical ray-marching. The final
/// lowest level is always the leaf node level, i.e. not the voxel level!
template <typename TreeT, typename RayT, int ChildNodeLevel>
class VolumeHDDA
{
public:

    using ChainT = typename TreeT::RootNodeType::NodeChainType;
    using NodeT = typename boost::mpl::at<ChainT, boost::mpl::int_<ChildNodeLevel> >::type;
    using TimeSpanT = typename RayT::TimeSpan;

    VolumeHDDA() {}

    template <typename AccessorT>
    TimeSpanT march(RayT& ray, AccessorT &acc)
    {
        TimeSpanT t(-1, -1);
        if (ray.valid()) this->march(ray, acc, t);
        return t;
    }

    /// ListType is a list of RayType::TimeSpan and is required to
    /// have the two methods: clear() and push_back(). Thus, it could
    /// be std::vector<typename RayType::TimeSpan> or
    /// std::deque<typename RayType::TimeSpan>.
    template <typename AccessorT, typename ListT>
    void hits(RayT& ray, AccessorT &acc, ListT& times)
    {
        TimeSpanT t(-1,-1);
        times.clear();
        this->hits(ray, acc, times, t);
        if (t.valid()) times.push_back(t);
    }

private:

    friend class VolumeHDDA<TreeT, RayT, ChildNodeLevel+1>;

    template <typename AccessorT>
    bool march(RayT& ray, AccessorT &acc, TimeSpanT& t)
    {
        mDDA.init(ray);
        do {
            if (acc.template probeConstNode<NodeT>(mDDA.voxel()) != nullptr) {//child node
                ray.setTimes(mDDA.time(), mDDA.next());
                if (mHDDA.march(ray, acc, t)) return true;//terminate
            } else if (acc.isValueOn(mDDA.voxel())) {//hit an active tile
                if (t.t0<0) t.t0 = mDDA.time();//this is the first hit so set t0
            } else if (t.t0>=0) {//hit an inactive tile after hitting active values
                t.t1 = mDDA.time();//set end of active ray segment
                if (t.valid()) return true;//terminate
                t.set(-1, -1);//reset to an empty and invalid time-span
            }
        } while (mDDA.step());
        if (t.t0>=0) t.t1 = mDDA.maxTime();
        return false;
    }

    /// ListType is a list of RayType::TimeSpan and is required to
    /// have the two methods: clear() and push_back(). Thus, it could
    /// be std::vector<typename RayType::TimeSpan> or
    /// std::deque<typename RayType::TimeSpan>.
    template <typename AccessorT, typename ListT>
    void hits(RayT& ray, AccessorT &acc, ListT& times, TimeSpanT& t)
    {
        mDDA.init(ray);
        do {
            if (acc.template probeConstNode<NodeT>(mDDA.voxel()) != nullptr) {//child node
                ray.setTimes(mDDA.time(), mDDA.next());
                mHDDA.hits(ray, acc, times, t);
            } else if (acc.isValueOn(mDDA.voxel())) {//hit an active tile
                if (t.t0<0) t.t0 = mDDA.time();//this is the first hit so set t0
            } else if (t.t0>=0) {//hit an inactive tile after hitting active values
                t.t1 = mDDA.time();//set end of active ray segment
                if (t.valid()) times.push_back(t);
                t.set(-1,-1);//reset to an empty and invalid time-span
            }
        } while (mDDA.step());
        if (t.t0>=0) t.t1 = mDDA.maxTime();
    }

    math::DDA<RayT, NodeT::TOTAL> mDDA;
    VolumeHDDA<TreeT, RayT, ChildNodeLevel-1> mHDDA;
};

/// @brief Specialization of Hierarchical Digital Differential Analyzer
/// class that intersects against the leafs or tiles of a generic volume.
template <typename TreeT, typename RayT>
class VolumeHDDA<TreeT, RayT, 0>
{
public:

    using LeafT = typename TreeT::LeafNodeType;
    using TimeSpanT = typename RayT::TimeSpan;

    VolumeHDDA() {}

    template <typename AccessorT>
    TimeSpanT march(RayT& ray, AccessorT &acc)
    {
        TimeSpanT t(-1, -1);
        if (ray.valid()) this->march(ray, acc, t);
        return t;
    }

    template <typename AccessorT, typename ListT>
    void hits(RayT& ray, AccessorT &acc, ListT& times)
    {
        TimeSpanT t(-1,-1);
        times.clear();
        this->hits(ray, acc, times, t);
        if (t.valid()) times.push_back(t);
    }

private:

    friend class VolumeHDDA<TreeT, RayT, 1>;

    template <typename AccessorT>
    bool march(RayT& ray, AccessorT &acc, TimeSpanT& t)
    {
        mDDA.init(ray);
        do {
            if (acc.template probeConstNode<LeafT>(mDDA.voxel()) ||
                acc.isValueOn(mDDA.voxel())) {//hit a leaf or an active tile
                if (t.t0<0) t.t0 = mDDA.time();//this is the first hit
            } else if (t.t0>=0) {//hit an inactive tile after hitting active values
                t.t1 = mDDA.time();//set end of active ray segment
                if (t.valid()) return true;//terminate
                t.set(-1, -1);//reset to an empty and invalid time-span
            }
        } while (mDDA.step());
        if (t.t0>=0) t.t1 = mDDA.maxTime();
        return false;
    }

    template <typename AccessorT, typename ListT>
    void hits(RayT& ray, AccessorT &acc, ListT& times, TimeSpanT& t)
    {
        mDDA.init(ray);
        do {
            if (acc.template probeConstNode<LeafT>(mDDA.voxel()) ||
                acc.isValueOn(mDDA.voxel())) {//hit a leaf or an active tile
                if (t.t0<0) t.t0 = mDDA.time();//this is the first hit
            } else if (t.t0>=0) {//hit an inactive tile after hitting active values
                t.t1 = mDDA.time();//set end of active ray segment
                if (t.valid()) times.push_back(t);
                t.set(-1, -1);//reset to an empty and invalid time-span
            }
        } while (mDDA.step());
        if (t.t0>=0) t.t1 = mDDA.maxTime();
    }
    math::DDA<RayT, LeafT::TOTAL> mDDA;
};

} // namespace math
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_MATH_DDA_HAS_BEEN_INCLUDED
