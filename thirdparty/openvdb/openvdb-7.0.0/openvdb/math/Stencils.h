// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @author Ken Museth
///
/// @file Stencils.h
///
/// @brief Defines various finite difference stencils by means of the
///        "curiously recurring template pattern" on a BaseStencil
///        that caches stencil values and stores a ValueAccessor for
///        fast lookup.

#ifndef OPENVDB_MATH_STENCILS_HAS_BEEN_INCLUDED
#define OPENVDB_MATH_STENCILS_HAS_BEEN_INCLUDED

#include <algorithm>
#include <vector>                          // for std::vector
#include <bitset>                          // for std::bitset
#include <openvdb/math/Math.h>             // for Pow2, needed by WENO and Godunov
#include <openvdb/Types.h>                 // for Real
#include <openvdb/math/Coord.h>            // for Coord
#include <openvdb/math/FiniteDifference.h> // for WENO5 and GodunovsNormSqrd
#include <openvdb/tree/ValueAccessor.h>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace math {


////////////////////////////////////////

template<typename DerivedType, typename GridT, bool IsSafe>
class BaseStencil
{
public:
    typedef GridT                                       GridType;
    typedef typename GridT::TreeType                    TreeType;
    typedef typename GridT::ValueType                   ValueType;
    typedef tree::ValueAccessor<const TreeType, IsSafe> AccessorType;
    typedef std::vector<ValueType>                      BufferType;
    typedef typename BufferType::iterator               IterType;

    /// @brief Initialize the stencil buffer with the values of voxel (i, j, k)
    /// and its neighbors.
    /// @param ijk Index coordinates of stencil center
    inline void moveTo(const Coord& ijk)
    {
        mCenter = ijk;
        mValues[0] = mAcc.getValue(ijk);
        static_cast<DerivedType&>(*this).init(mCenter);
    }

    /// @brief Initialize the stencil buffer with the values of voxel (i, j, k)
    /// and its neighbors. The method also takes a value of the center
    /// element of the stencil, assuming it is already known.
    /// @param ijk Index coordinates of stnecil center
    /// @param centerValue Value of the center element of the stencil
    inline void moveTo(const Coord& ijk, const ValueType& centerValue)
    {
        mCenter = ijk;
        mValues[0] = centerValue;
        static_cast<DerivedType&>(*this).init(mCenter);
    }

    /// @brief Initialize the stencil buffer with the values of voxel
    /// (x, y, z) and its neighbors.
    ///
    /// @note This version is slightly faster than the one above, since
    /// the center voxel's value is read directly from the iterator.
    template<typename IterType>
    inline void moveTo(const IterType& iter)
    {
        mCenter = iter.getCoord();
        mValues[0] = *iter;
        static_cast<DerivedType&>(*this).init(mCenter);
    }

    /// @brief Initialize the stencil buffer with the values of voxel (x, y, z)
    /// and its neighbors.
    /// @param xyz Floating point voxel coordinates of stencil center
    /// @details This method will check to see if it is necessary to
    /// update the stencil based on the cached index coordinates of
    /// the center point.
    template<typename RealType>
    inline void moveTo(const Vec3<RealType>& xyz)
    {
        Coord ijk = Coord::floor(xyz);
        if (ijk != mCenter) this->moveTo(ijk);
    }

    /// @brief Return the value from the stencil buffer with linear
    /// offset pos.
    ///
    /// @note The default (@a pos = 0) corresponds to the first element
    /// which is typically the center point of the stencil.
    inline const ValueType& getValue(unsigned int pos = 0) const
    {
        assert(pos < mValues.size());
        return mValues[pos];
    }

    /// @brief Return the value at the specified location relative to the center of the stencil
    template<int i, int j, int k>
    inline const ValueType& getValue() const
    {
        return mValues[static_cast<const DerivedType&>(*this).template pos<i,j,k>()];
    }

    /// @brief Set the value at the specified location relative to the center of the stencil
    template<int i, int j, int k>
    inline void setValue(const ValueType& value)
    {
        mValues[static_cast<const DerivedType&>(*this).template pos<i,j,k>()] = value;
    }

    /// @brief Return the size of the stencil buffer.
    inline int size() { return mValues.size(); }

    /// @brief Return the median value of the current stencil.
    inline ValueType median() const
    {
        BufferType tmp(mValues);//local copy
        assert(!tmp.empty());
        size_t midpoint = (tmp.size() - 1) >> 1;
        // Partially sort the vector until the median value is at the midpoint.
        std::nth_element(tmp.begin(), tmp.begin() + midpoint, tmp.end());
        return tmp[midpoint];
    }

    /// @brief Return the mean value of the current stencil.
    inline ValueType mean() const
    {
        ValueType sum = 0.0;
        for (int n = 0, s = int(mValues.size()); n < s; ++n) sum += mValues[n];
        return sum / ValueType(mValues.size());
    }

    /// @brief Return the smallest value in the stencil buffer.
    inline ValueType min() const
    {
        IterType iter = std::min_element(mValues.begin(), mValues.end());
        return *iter;
    }

    /// @brief Return the largest value in the stencil buffer.
    inline ValueType max() const
    {
        IterType iter = std::max_element(mValues.begin(), mValues.end());
        return *iter;
    }

    /// @brief Return the coordinates of the center point of the stencil.
    inline const Coord& getCenterCoord() const { return mCenter; }

    /// @brief Return the value at the center of the stencil
    inline const ValueType& getCenterValue() const { return mValues[0]; }

    /// @brief Return true if the center of the stencil intersects the
    /// iso-contour specified by the isoValue
    inline bool intersects(const ValueType &isoValue = zeroVal<ValueType>()) const
    {
        const bool less = this->getValue< 0, 0, 0>() < isoValue;
        return (less  ^  (this->getValue<-1, 0, 0>() < isoValue)) ||
               (less  ^  (this->getValue< 1, 0, 0>() < isoValue)) ||
               (less  ^  (this->getValue< 0,-1, 0>() < isoValue)) ||
               (less  ^  (this->getValue< 0, 1, 0>() < isoValue)) ||
               (less  ^  (this->getValue< 0, 0,-1>() < isoValue)) ||
               (less  ^  (this->getValue< 0, 0, 1>() < isoValue))  ;
    }

    /// @brief Return true a bit-mask where the 6 bits indicates if the
    /// center of the stencil intersects the iso-contour specified by the isoValue.
    ///
    /// @note There are 2^6 = 64 different possible cases, including no intersections!
    ///
    /// @details The ordering of bit mask is ( -x, +x, -y, +y, -z, +z ), so to
    /// check if there is an intersection in -y use mask.test(2) where mask is
    /// ther return value from this function. To check if there are any
    /// intersections use mask.any(), and for no intersections use mask.none().
    /// To count the number of intersections use mask.count().
    inline std::bitset<6> intersectionMask(const ValueType &isoValue = zeroVal<ValueType>()) const
    {
        std::bitset<6> mask;
        const bool less =   this->getValue< 0, 0, 0>() < isoValue;
        mask[0] = less  ^  (this->getValue<-1, 0, 0>() < isoValue);
        mask[1] = less  ^  (this->getValue< 1, 0, 0>() < isoValue);
        mask[2] = less  ^  (this->getValue< 0,-1, 0>() < isoValue);
        mask[3] = less  ^  (this->getValue< 0, 1, 0>() < isoValue);
        mask[4] = less  ^  (this->getValue< 0, 0,-1>() < isoValue);
        mask[5] = less  ^  (this->getValue< 0, 0, 1>() < isoValue);
        return mask;
    }

    /// @brief Return a const reference to the grid from which this
    /// stencil was constructed.
    inline const GridType& grid() const { return *mGrid; }

    /// @brief Return a const reference to the ValueAccessor
    /// associated with this Stencil.
    inline const AccessorType& accessor() const { return mAcc; }

protected:
    // Constructor is protected to prevent direct instantiation.
    BaseStencil(const GridType& grid, int size)
        : mGrid(&grid)
        , mAcc(grid.tree())
        , mValues(size)
        , mCenter(Coord::max())
    {
    }

    const GridType* mGrid;
    AccessorType    mAcc;
    BufferType      mValues;
    Coord           mCenter;

}; // BaseStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the seven point stencil
    template<int i, int j, int k> struct SevenPt {};
    template<> struct SevenPt< 0, 0, 0> { enum { idx = 0 }; };
    template<> struct SevenPt< 1, 0, 0> { enum { idx = 1 }; };
    template<> struct SevenPt< 0, 1, 0> { enum { idx = 2 }; };
    template<> struct SevenPt< 0, 0, 1> { enum { idx = 3 }; };
    template<> struct SevenPt<-1, 0, 0> { enum { idx = 4 }; };
    template<> struct SevenPt< 0,-1, 0> { enum { idx = 5 }; };
    template<> struct SevenPt< 0, 0,-1> { enum { idx = 6 }; };

}


template<typename GridT, bool IsSafe = true>
class SevenPointStencil: public BaseStencil<SevenPointStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef SevenPointStencil<GridT, IsSafe>  SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe> BaseType;
public:
    typedef GridT                             GridType;
    typedef typename GridT::TreeType          TreeType;
    typedef typename GridT::ValueType         ValueType;

    static const int SIZE = 7;

    SevenPointStencil(const GridT& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return SevenPt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        BaseType::template setValue<-1, 0, 0>(mAcc.getValue(ijk.offsetBy(-1, 0, 0)));
        BaseType::template setValue< 1, 0, 0>(mAcc.getValue(ijk.offsetBy( 1, 0, 0)));

        BaseType::template setValue< 0,-1, 0>(mAcc.getValue(ijk.offsetBy( 0,-1, 0)));
        BaseType::template setValue< 0, 1, 0>(mAcc.getValue(ijk.offsetBy( 0, 1, 0)));

        BaseType::template setValue< 0, 0,-1>(mAcc.getValue(ijk.offsetBy( 0, 0,-1)));
        BaseType::template setValue< 0, 0, 1>(mAcc.getValue(ijk.offsetBy( 0, 0, 1)));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// SevenPointStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the eight point box stencil
    template<int i, int j, int k> struct BoxPt {};
    template<> struct BoxPt< 0, 0, 0> { enum { idx = 0 }; };
    template<> struct BoxPt< 0, 0, 1> { enum { idx = 1 }; };
    template<> struct BoxPt< 0, 1, 1> { enum { idx = 2 }; };
    template<> struct BoxPt< 0, 1, 0> { enum { idx = 3 }; };
    template<> struct BoxPt< 1, 0, 0> { enum { idx = 4 }; };
    template<> struct BoxPt< 1, 0, 1> { enum { idx = 5 }; };
    template<> struct BoxPt< 1, 1, 1> { enum { idx = 6 }; };
    template<> struct BoxPt< 1, 1, 0> { enum { idx = 7 }; };
}

template<typename GridT, bool IsSafe = true>
class BoxStencil: public BaseStencil<BoxStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef BoxStencil<GridT, IsSafe>         SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe> BaseType;
public:
    typedef GridT                             GridType;
    typedef typename GridT::TreeType          TreeType;
    typedef typename GridT::ValueType         ValueType;

    static const int SIZE = 8;

    BoxStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return BoxPt<i,j,k>::idx; }

     /// @brief Return true if the center of the stencil intersects the
    /// iso-contour specified by the isoValue
    inline bool intersects(const ValueType &isoValue = zeroVal<ValueType>()) const
    {
        const bool less = mValues[0] < isoValue;
        return (less  ^  (mValues[1] < isoValue)) ||
               (less  ^  (mValues[2] < isoValue)) ||
               (less  ^  (mValues[3] < isoValue)) ||
               (less  ^  (mValues[4] < isoValue)) ||
               (less  ^  (mValues[5] < isoValue)) ||
               (less  ^  (mValues[6] < isoValue)) ||
               (less  ^  (mValues[7] < isoValue))  ;
    }

    /// @brief Return the trilinear interpolation at the normalized position.
    /// @param xyz Floating point coordinate position.
    /// @warning It is assumed that the stencil has already been moved
    /// to the relevant voxel position, e.g. using moveTo(xyz).
    /// @note Trilinear interpolation kernal reads as:
    ///       v000 (1-u)(1-v)(1-w) + v001 (1-u)(1-v)w + v010 (1-u)v(1-w) + v011 (1-u)vw
    ///     + v100 u(1-v)(1-w)     + v101 u(1-v)w     + v110 uv(1-w)     + v111 uvw
    inline ValueType interpolation(const math::Vec3<ValueType>& xyz) const
    {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        const ValueType u = xyz[0] - BaseType::mCenter[0];
        const ValueType v = xyz[1] - BaseType::mCenter[1];
        const ValueType w = xyz[2] - BaseType::mCenter[2];
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END

        assert(u>=0 && u<=1);
        assert(v>=0 && v<=1);
        assert(w>=0 && w<=1);

        ValueType V = BaseType::template getValue<0,0,0>();
        ValueType A = static_cast<ValueType>(V + (BaseType::template getValue<0,0,1>() - V) * w);
        V = BaseType::template getValue< 0, 1, 0>();
        ValueType B = static_cast<ValueType>(V + (BaseType::template getValue<0,1,1>() - V) * w);
        ValueType C = static_cast<ValueType>(A + (B - A) * v);

        V = BaseType::template getValue<1,0,0>();
        A = static_cast<ValueType>(V + (BaseType::template getValue<1,0,1>() - V) * w);
        V = BaseType::template getValue<1,1,0>();
        B = static_cast<ValueType>(V + (BaseType::template getValue<1,1,1>() - V) * w);
        ValueType D = static_cast<ValueType>(A + (B - A) * v);

        return static_cast<ValueType>(C + (D - C) * u);
    }

    /// @brief Return the gradient in world space of the trilinear interpolation kernel.
    /// @param xyz Floating point coordinate position.
    /// @warning It is assumed that the stencil has already been moved
    /// to the relevant voxel position, e.g. using moveTo(xyz).
    /// @note Computed as partial derivatives of the trilinear interpolation kernel:
    ///       v000 (1-u)(1-v)(1-w) + v001 (1-u)(1-v)w + v010 (1-u)v(1-w) + v011 (1-u)vw
    ///     + v100 u(1-v)(1-w)     + v101 u(1-v)w     + v110 uv(1-w)     + v111 uvw
    inline math::Vec3<ValueType> gradient(const math::Vec3<ValueType>& xyz) const
    {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        const ValueType u = xyz[0] - BaseType::mCenter[0];
        const ValueType v = xyz[1] - BaseType::mCenter[1];
        const ValueType w = xyz[2] - BaseType::mCenter[2];
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END

        assert(u>=0 && u<=1);
        assert(v>=0 && v<=1);
        assert(w>=0 && w<=1);

        ValueType D[4]={BaseType::template getValue<0,0,1>()-BaseType::template getValue<0,0,0>(),
                        BaseType::template getValue<0,1,1>()-BaseType::template getValue<0,1,0>(),
                        BaseType::template getValue<1,0,1>()-BaseType::template getValue<1,0,0>(),
                        BaseType::template getValue<1,1,1>()-BaseType::template getValue<1,1,0>()};

        // Z component
        ValueType A = static_cast<ValueType>(D[0] + (D[1]- D[0]) * v);
        ValueType B = static_cast<ValueType>(D[2] + (D[3]- D[2]) * v);
        math::Vec3<ValueType> grad(zeroVal<ValueType>(),
                                   zeroVal<ValueType>(),
                                   static_cast<ValueType>(A + (B - A) * u));

        D[0] = static_cast<ValueType>(BaseType::template getValue<0,0,0>() + D[0] * w);
        D[1] = static_cast<ValueType>(BaseType::template getValue<0,1,0>() + D[1] * w);
        D[2] = static_cast<ValueType>(BaseType::template getValue<1,0,0>() + D[2] * w);
        D[3] = static_cast<ValueType>(BaseType::template getValue<1,1,0>() + D[3] * w);

        // X component
        A = static_cast<ValueType>(D[0] + (D[1] - D[0]) * v);
        B = static_cast<ValueType>(D[2] + (D[3] - D[2]) * v);

        grad[0] = B - A;

        // Y component
        A = D[1] - D[0];
        B = D[3] - D[2];

        grad[1] = static_cast<ValueType>(A + (B - A) * u);

        return BaseType::mGrid->transform().baseMap()->applyIJT(grad, xyz);
    }

private:
    inline void init(const Coord& ijk)
    {
        BaseType::template setValue< 0, 0, 1>(mAcc.getValue(ijk.offsetBy( 0, 0, 1)));
        BaseType::template setValue< 0, 1, 1>(mAcc.getValue(ijk.offsetBy( 0, 1, 1)));
        BaseType::template setValue< 0, 1, 0>(mAcc.getValue(ijk.offsetBy( 0, 1, 0)));
        BaseType::template setValue< 1, 0, 0>(mAcc.getValue(ijk.offsetBy( 1, 0, 0)));
        BaseType::template setValue< 1, 0, 1>(mAcc.getValue(ijk.offsetBy( 1, 0, 1)));
        BaseType::template setValue< 1, 1, 1>(mAcc.getValue(ijk.offsetBy( 1, 1, 1)));
        BaseType::template setValue< 1, 1, 0>(mAcc.getValue(ijk.offsetBy( 1, 1, 0)));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// BoxStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the dense point stencil
    template<int i, int j, int k> struct DensePt {};
    template<> struct DensePt< 0, 0, 0> { enum { idx = 0 }; };

    template<> struct DensePt< 1, 0, 0> { enum { idx = 1 }; };
    template<> struct DensePt< 0, 1, 0> { enum { idx = 2 }; };
    template<> struct DensePt< 0, 0, 1> { enum { idx = 3 }; };

    template<> struct DensePt<-1, 0, 0> { enum { idx = 4 }; };
    template<> struct DensePt< 0,-1, 0> { enum { idx = 5 }; };
    template<> struct DensePt< 0, 0,-1> { enum { idx = 6 }; };

    template<> struct DensePt<-1,-1, 0> { enum { idx = 7 }; };
    template<> struct DensePt< 0,-1,-1> { enum { idx = 8 }; };
    template<> struct DensePt<-1, 0,-1> { enum { idx = 9 }; };

    template<> struct DensePt< 1,-1, 0> { enum { idx = 10 }; };
    template<> struct DensePt< 0, 1,-1> { enum { idx = 11 }; };
    template<> struct DensePt<-1, 0, 1> { enum { idx = 12 }; };

    template<> struct DensePt<-1, 1, 0> { enum { idx = 13 }; };
    template<> struct DensePt< 0,-1, 1> { enum { idx = 14 }; };
    template<> struct DensePt< 1, 0,-1> { enum { idx = 15 }; };

    template<> struct DensePt< 1, 1, 0> { enum { idx = 16 }; };
    template<> struct DensePt< 0, 1, 1> { enum { idx = 17 }; };
    template<> struct DensePt< 1, 0, 1> { enum { idx = 18 }; };
}


template<typename GridT, bool IsSafe = true>
class SecondOrderDenseStencil
    : public BaseStencil<SecondOrderDenseStencil<GridT, IsSafe>, GridT, IsSafe >
{
    typedef SecondOrderDenseStencil<GridT, IsSafe> SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe >     BaseType;
public:
    typedef GridT                                  GridType;
    typedef typename GridT::TreeType               TreeType;
    typedef typename GridType::ValueType           ValueType;

    static const int SIZE = 19;

    SecondOrderDenseStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return DensePt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        mValues[DensePt< 1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,  0,  0));
        mValues[DensePt< 0, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  1,  0));
        mValues[DensePt< 0, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  1));

        mValues[DensePt<-1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,  0,  0));
        mValues[DensePt< 0,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -1,  0));
        mValues[DensePt< 0, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -1));

        mValues[DensePt<-1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, -1,  0));
        mValues[DensePt< 1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, -1,  0));
        mValues[DensePt<-1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,  1,  0));
        mValues[DensePt< 1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,  1,  0));

        mValues[DensePt<-1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-1,  0, -1));
        mValues[DensePt< 1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 1,  0, -1));
        mValues[DensePt<-1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-1,  0,  1));
        mValues[DensePt< 1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 1,  0,  1));

        mValues[DensePt< 0,-1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, -1, -1));
        mValues[DensePt< 0, 1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  1, -1));
        mValues[DensePt< 0,-1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, -1,  1));
        mValues[DensePt< 0, 1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  1,  1));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// SecondOrderDenseStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the dense point stencil
    template<int i, int j, int k> struct ThirteenPt {};
    template<> struct ThirteenPt< 0, 0, 0> { enum { idx = 0 }; };

    template<> struct ThirteenPt< 1, 0, 0> { enum { idx = 1 }; };
    template<> struct ThirteenPt< 0, 1, 0> { enum { idx = 2 }; };
    template<> struct ThirteenPt< 0, 0, 1> { enum { idx = 3 }; };

    template<> struct ThirteenPt<-1, 0, 0> { enum { idx = 4 }; };
    template<> struct ThirteenPt< 0,-1, 0> { enum { idx = 5 }; };
    template<> struct ThirteenPt< 0, 0,-1> { enum { idx = 6 }; };

    template<> struct ThirteenPt< 2, 0, 0> { enum { idx = 7 }; };
    template<> struct ThirteenPt< 0, 2, 0> { enum { idx = 8 }; };
    template<> struct ThirteenPt< 0, 0, 2> { enum { idx = 9 }; };

    template<> struct ThirteenPt<-2, 0, 0> { enum { idx = 10 }; };
    template<> struct ThirteenPt< 0,-2, 0> { enum { idx = 11 }; };
    template<> struct ThirteenPt< 0, 0,-2> { enum { idx = 12 }; };

}


template<typename GridT, bool IsSafe = true>
class ThirteenPointStencil
    : public BaseStencil<ThirteenPointStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef ThirteenPointStencil<GridT, IsSafe> SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe >  BaseType;
public:
    typedef GridT                               GridType;
    typedef typename GridT::TreeType            TreeType;
    typedef typename GridType::ValueType        ValueType;

    static const int SIZE = 13;

    ThirteenPointStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return ThirteenPt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        mValues[ThirteenPt< 2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,  0,  0));
        mValues[ThirteenPt< 1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,  0,  0));
        mValues[ThirteenPt<-1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,  0,  0));
        mValues[ThirteenPt<-2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,  0,  0));

        mValues[ThirteenPt< 0, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  2,  0));
        mValues[ThirteenPt< 0, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  1,  0));
        mValues[ThirteenPt< 0,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -1,  0));
        mValues[ThirteenPt< 0,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -2,  0));

        mValues[ThirteenPt< 0, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  2));
        mValues[ThirteenPt< 0, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  1));
        mValues[ThirteenPt< 0, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -1));
        mValues[ThirteenPt< 0, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -2));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// ThirteenPointStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the 4th-order dense point stencil
    template<int i, int j, int k> struct FourthDensePt {};
    template<> struct FourthDensePt< 0, 0, 0> { enum { idx = 0 }; };

    template<> struct FourthDensePt<-2, 2, 0> { enum { idx = 1 }; };
    template<> struct FourthDensePt<-1, 2, 0> { enum { idx = 2 }; };
    template<> struct FourthDensePt< 0, 2, 0> { enum { idx = 3 }; };
    template<> struct FourthDensePt< 1, 2, 0> { enum { idx = 4 }; };
    template<> struct FourthDensePt< 2, 2, 0> { enum { idx = 5 }; };

    template<> struct FourthDensePt<-2, 1, 0> { enum { idx = 6 }; };
    template<> struct FourthDensePt<-1, 1, 0> { enum { idx = 7 }; };
    template<> struct FourthDensePt< 0, 1, 0> { enum { idx = 8 }; };
    template<> struct FourthDensePt< 1, 1, 0> { enum { idx = 9 }; };
    template<> struct FourthDensePt< 2, 1, 0> { enum { idx = 10 }; };

    template<> struct FourthDensePt<-2, 0, 0> { enum { idx = 11 }; };
    template<> struct FourthDensePt<-1, 0, 0> { enum { idx = 12 }; };
    template<> struct FourthDensePt< 1, 0, 0> { enum { idx = 13 }; };
    template<> struct FourthDensePt< 2, 0, 0> { enum { idx = 14 }; };

    template<> struct FourthDensePt<-2,-1, 0> { enum { idx = 15 }; };
    template<> struct FourthDensePt<-1,-1, 0> { enum { idx = 16 }; };
    template<> struct FourthDensePt< 0,-1, 0> { enum { idx = 17 }; };
    template<> struct FourthDensePt< 1,-1, 0> { enum { idx = 18 }; };
    template<> struct FourthDensePt< 2,-1, 0> { enum { idx = 19 }; };

    template<> struct FourthDensePt<-2,-2, 0> { enum { idx = 20 }; };
    template<> struct FourthDensePt<-1,-2, 0> { enum { idx = 21 }; };
    template<> struct FourthDensePt< 0,-2, 0> { enum { idx = 22 }; };
    template<> struct FourthDensePt< 1,-2, 0> { enum { idx = 23 }; };
    template<> struct FourthDensePt< 2,-2, 0> { enum { idx = 24 }; };


    template<> struct FourthDensePt<-2, 0, 2> { enum { idx = 25 }; };
    template<> struct FourthDensePt<-1, 0, 2> { enum { idx = 26 }; };
    template<> struct FourthDensePt< 0, 0, 2> { enum { idx = 27 }; };
    template<> struct FourthDensePt< 1, 0, 2> { enum { idx = 28 }; };
    template<> struct FourthDensePt< 2, 0, 2> { enum { idx = 29 }; };

    template<> struct FourthDensePt<-2, 0, 1> { enum { idx = 30 }; };
    template<> struct FourthDensePt<-1, 0, 1> { enum { idx = 31 }; };
    template<> struct FourthDensePt< 0, 0, 1> { enum { idx = 32 }; };
    template<> struct FourthDensePt< 1, 0, 1> { enum { idx = 33 }; };
    template<> struct FourthDensePt< 2, 0, 1> { enum { idx = 34 }; };

    template<> struct FourthDensePt<-2, 0,-1> { enum { idx = 35 }; };
    template<> struct FourthDensePt<-1, 0,-1> { enum { idx = 36 }; };
    template<> struct FourthDensePt< 0, 0,-1> { enum { idx = 37 }; };
    template<> struct FourthDensePt< 1, 0,-1> { enum { idx = 38 }; };
    template<> struct FourthDensePt< 2, 0,-1> { enum { idx = 39 }; };

    template<> struct FourthDensePt<-2, 0,-2> { enum { idx = 40 }; };
    template<> struct FourthDensePt<-1, 0,-2> { enum { idx = 41 }; };
    template<> struct FourthDensePt< 0, 0,-2> { enum { idx = 42 }; };
    template<> struct FourthDensePt< 1, 0,-2> { enum { idx = 43 }; };
    template<> struct FourthDensePt< 2, 0,-2> { enum { idx = 44 }; };


    template<> struct FourthDensePt< 0,-2, 2> { enum { idx = 45 }; };
    template<> struct FourthDensePt< 0,-1, 2> { enum { idx = 46 }; };
    template<> struct FourthDensePt< 0, 1, 2> { enum { idx = 47 }; };
    template<> struct FourthDensePt< 0, 2, 2> { enum { idx = 48 }; };

    template<> struct FourthDensePt< 0,-2, 1> { enum { idx = 49 }; };
    template<> struct FourthDensePt< 0,-1, 1> { enum { idx = 50 }; };
    template<> struct FourthDensePt< 0, 1, 1> { enum { idx = 51 }; };
    template<> struct FourthDensePt< 0, 2, 1> { enum { idx = 52 }; };

    template<> struct FourthDensePt< 0,-2,-1> { enum { idx = 53 }; };
    template<> struct FourthDensePt< 0,-1,-1> { enum { idx = 54 }; };
    template<> struct FourthDensePt< 0, 1,-1> { enum { idx = 55 }; };
    template<> struct FourthDensePt< 0, 2,-1> { enum { idx = 56 }; };

    template<> struct FourthDensePt< 0,-2,-2> { enum { idx = 57 }; };
    template<> struct FourthDensePt< 0,-1,-2> { enum { idx = 58 }; };
    template<> struct FourthDensePt< 0, 1,-2> { enum { idx = 59 }; };
    template<> struct FourthDensePt< 0, 2,-2> { enum { idx = 60 }; };

}


template<typename GridT, bool IsSafe = true>
class FourthOrderDenseStencil
    : public BaseStencil<FourthOrderDenseStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef FourthOrderDenseStencil<GridT, IsSafe> SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe >     BaseType;
public:
    typedef GridT                                  GridType;
    typedef typename GridT::TreeType               TreeType;
    typedef typename GridType::ValueType           ValueType;

    static const int SIZE = 61;

    FourthOrderDenseStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return FourthDensePt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        mValues[FourthDensePt<-2, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 2, 0));
        mValues[FourthDensePt<-1, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 2, 0));
        mValues[FourthDensePt< 0, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 0));
        mValues[FourthDensePt< 1, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 2, 0));
        mValues[FourthDensePt< 2, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 2, 0));

        mValues[FourthDensePt<-2, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 1, 0));
        mValues[FourthDensePt<-1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 1, 0));
        mValues[FourthDensePt< 0, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 0));
        mValues[FourthDensePt< 1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 1, 0));
        mValues[FourthDensePt< 2, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 1, 0));

        mValues[FourthDensePt<-2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 0));
        mValues[FourthDensePt<-1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 0));
        mValues[FourthDensePt< 1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 0));
        mValues[FourthDensePt< 2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 0));

        mValues[FourthDensePt<-2,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,-1, 0));
        mValues[FourthDensePt<-1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,-1, 0));
        mValues[FourthDensePt< 0,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 0));
        mValues[FourthDensePt< 1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,-1, 0));
        mValues[FourthDensePt< 2,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,-1, 0));

        mValues[FourthDensePt<-2,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,-2, 0));
        mValues[FourthDensePt<-1,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,-2, 0));
        mValues[FourthDensePt< 0,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 0));
        mValues[FourthDensePt< 1,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,-2, 0));
        mValues[FourthDensePt< 2,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,-2, 0));

        mValues[FourthDensePt<-2, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 2));
        mValues[FourthDensePt<-1, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 2));
        mValues[FourthDensePt< 0, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0, 2));
        mValues[FourthDensePt< 1, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 2));
        mValues[FourthDensePt< 2, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 2));

        mValues[FourthDensePt<-2, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 1));
        mValues[FourthDensePt<-1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 1));
        mValues[FourthDensePt< 0, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0, 1));
        mValues[FourthDensePt< 1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 1));
        mValues[FourthDensePt< 2, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 1));

        mValues[FourthDensePt<-2, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0,-1));
        mValues[FourthDensePt<-1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0,-1));
        mValues[FourthDensePt< 0, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0,-1));
        mValues[FourthDensePt< 1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0,-1));
        mValues[FourthDensePt< 2, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0,-1));

        mValues[FourthDensePt<-2, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0,-2));
        mValues[FourthDensePt<-1, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0,-2));
        mValues[FourthDensePt< 0, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0,-2));
        mValues[FourthDensePt< 1, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0,-2));
        mValues[FourthDensePt< 2, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0,-2));


        mValues[FourthDensePt< 0,-2, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 2));
        mValues[FourthDensePt< 0,-1, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 2));
        mValues[FourthDensePt< 0, 1, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 2));
        mValues[FourthDensePt< 0, 2, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 2));

        mValues[FourthDensePt< 0,-2, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 1));
        mValues[FourthDensePt< 0,-1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 1));
        mValues[FourthDensePt< 0, 1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 1));
        mValues[FourthDensePt< 0, 2, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 1));

        mValues[FourthDensePt< 0,-2,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2,-1));
        mValues[FourthDensePt< 0,-1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1,-1));
        mValues[FourthDensePt< 0, 1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1,-1));
        mValues[FourthDensePt< 0, 2,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2,-1));

        mValues[FourthDensePt< 0,-2,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2,-2));
        mValues[FourthDensePt< 0,-1,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1,-2));
        mValues[FourthDensePt< 0, 1,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1,-2));
        mValues[FourthDensePt< 0, 2,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2,-2));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// FourthOrderDenseStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the dense point stencil
    template<int i, int j, int k> struct NineteenPt {};
    template<> struct NineteenPt< 0, 0, 0> { enum { idx = 0 }; };

    template<> struct NineteenPt< 1, 0, 0> { enum { idx = 1 }; };
    template<> struct NineteenPt< 0, 1, 0> { enum { idx = 2 }; };
    template<> struct NineteenPt< 0, 0, 1> { enum { idx = 3 }; };

    template<> struct NineteenPt<-1, 0, 0> { enum { idx = 4 }; };
    template<> struct NineteenPt< 0,-1, 0> { enum { idx = 5 }; };
    template<> struct NineteenPt< 0, 0,-1> { enum { idx = 6 }; };

    template<> struct NineteenPt< 2, 0, 0> { enum { idx = 7 }; };
    template<> struct NineteenPt< 0, 2, 0> { enum { idx = 8 }; };
    template<> struct NineteenPt< 0, 0, 2> { enum { idx = 9 }; };

    template<> struct NineteenPt<-2, 0, 0> { enum { idx = 10 }; };
    template<> struct NineteenPt< 0,-2, 0> { enum { idx = 11 }; };
    template<> struct NineteenPt< 0, 0,-2> { enum { idx = 12 }; };

    template<> struct NineteenPt< 3, 0, 0> { enum { idx = 13 }; };
    template<> struct NineteenPt< 0, 3, 0> { enum { idx = 14 }; };
    template<> struct NineteenPt< 0, 0, 3> { enum { idx = 15 }; };

    template<> struct NineteenPt<-3, 0, 0> { enum { idx = 16 }; };
    template<> struct NineteenPt< 0,-3, 0> { enum { idx = 17 }; };
    template<> struct NineteenPt< 0, 0,-3> { enum { idx = 18 }; };

}


template<typename GridT, bool IsSafe = true>
class NineteenPointStencil
    : public BaseStencil<NineteenPointStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef NineteenPointStencil<GridT, IsSafe> SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe >  BaseType;
public:
    typedef GridT                               GridType;
    typedef typename GridT::TreeType            TreeType;
    typedef typename GridType::ValueType        ValueType;

    static const int SIZE = 19;

    NineteenPointStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return NineteenPt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        mValues[NineteenPt< 3, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3,  0,  0));
        mValues[NineteenPt< 2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,  0,  0));
        mValues[NineteenPt< 1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,  0,  0));
        mValues[NineteenPt<-1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,  0,  0));
        mValues[NineteenPt<-2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,  0,  0));
        mValues[NineteenPt<-3, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3,  0,  0));

        mValues[NineteenPt< 0, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  3,  0));
        mValues[NineteenPt< 0, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  2,  0));
        mValues[NineteenPt< 0, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,  1,  0));
        mValues[NineteenPt< 0,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -1,  0));
        mValues[NineteenPt< 0,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -2,  0));
        mValues[NineteenPt< 0,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, -3,  0));

        mValues[NineteenPt< 0, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  3));
        mValues[NineteenPt< 0, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  2));
        mValues[NineteenPt< 0, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0,  1));
        mValues[NineteenPt< 0, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -1));
        mValues[NineteenPt< 0, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -2));
        mValues[NineteenPt< 0, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0,  0, -3));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// NineteenPointStencil class


////////////////////////////////////////


namespace { // anonymous namespace for stencil-layout map

    // the 4th-order dense point stencil
    template<int i, int j, int k> struct SixthDensePt { };
    template<> struct SixthDensePt< 0, 0, 0> { enum { idx = 0 }; };

    template<> struct SixthDensePt<-3, 3, 0> { enum { idx = 1 }; };
    template<> struct SixthDensePt<-2, 3, 0> { enum { idx = 2 }; };
    template<> struct SixthDensePt<-1, 3, 0> { enum { idx = 3 }; };
    template<> struct SixthDensePt< 0, 3, 0> { enum { idx = 4 }; };
    template<> struct SixthDensePt< 1, 3, 0> { enum { idx = 5 }; };
    template<> struct SixthDensePt< 2, 3, 0> { enum { idx = 6 }; };
    template<> struct SixthDensePt< 3, 3, 0> { enum { idx = 7 }; };

    template<> struct SixthDensePt<-3, 2, 0> { enum { idx = 8 }; };
    template<> struct SixthDensePt<-2, 2, 0> { enum { idx = 9 }; };
    template<> struct SixthDensePt<-1, 2, 0> { enum { idx = 10 }; };
    template<> struct SixthDensePt< 0, 2, 0> { enum { idx = 11 }; };
    template<> struct SixthDensePt< 1, 2, 0> { enum { idx = 12 }; };
    template<> struct SixthDensePt< 2, 2, 0> { enum { idx = 13 }; };
    template<> struct SixthDensePt< 3, 2, 0> { enum { idx = 14 }; };

    template<> struct SixthDensePt<-3, 1, 0> { enum { idx = 15 }; };
    template<> struct SixthDensePt<-2, 1, 0> { enum { idx = 16 }; };
    template<> struct SixthDensePt<-1, 1, 0> { enum { idx = 17 }; };
    template<> struct SixthDensePt< 0, 1, 0> { enum { idx = 18 }; };
    template<> struct SixthDensePt< 1, 1, 0> { enum { idx = 19 }; };
    template<> struct SixthDensePt< 2, 1, 0> { enum { idx = 20 }; };
    template<> struct SixthDensePt< 3, 1, 0> { enum { idx = 21 }; };

    template<> struct SixthDensePt<-3, 0, 0> { enum { idx = 22 }; };
    template<> struct SixthDensePt<-2, 0, 0> { enum { idx = 23 }; };
    template<> struct SixthDensePt<-1, 0, 0> { enum { idx = 24 }; };
    template<> struct SixthDensePt< 1, 0, 0> { enum { idx = 25 }; };
    template<> struct SixthDensePt< 2, 0, 0> { enum { idx = 26 }; };
    template<> struct SixthDensePt< 3, 0, 0> { enum { idx = 27 }; };


    template<> struct SixthDensePt<-3,-1, 0> { enum { idx = 28 }; };
    template<> struct SixthDensePt<-2,-1, 0> { enum { idx = 29 }; };
    template<> struct SixthDensePt<-1,-1, 0> { enum { idx = 30 }; };
    template<> struct SixthDensePt< 0,-1, 0> { enum { idx = 31 }; };
    template<> struct SixthDensePt< 1,-1, 0> { enum { idx = 32 }; };
    template<> struct SixthDensePt< 2,-1, 0> { enum { idx = 33 }; };
    template<> struct SixthDensePt< 3,-1, 0> { enum { idx = 34 }; };


    template<> struct SixthDensePt<-3,-2, 0> { enum { idx = 35 }; };
    template<> struct SixthDensePt<-2,-2, 0> { enum { idx = 36 }; };
    template<> struct SixthDensePt<-1,-2, 0> { enum { idx = 37 }; };
    template<> struct SixthDensePt< 0,-2, 0> { enum { idx = 38 }; };
    template<> struct SixthDensePt< 1,-2, 0> { enum { idx = 39 }; };
    template<> struct SixthDensePt< 2,-2, 0> { enum { idx = 40 }; };
    template<> struct SixthDensePt< 3,-2, 0> { enum { idx = 41 }; };


    template<> struct SixthDensePt<-3,-3, 0> { enum { idx = 42 }; };
    template<> struct SixthDensePt<-2,-3, 0> { enum { idx = 43 }; };
    template<> struct SixthDensePt<-1,-3, 0> { enum { idx = 44 }; };
    template<> struct SixthDensePt< 0,-3, 0> { enum { idx = 45 }; };
    template<> struct SixthDensePt< 1,-3, 0> { enum { idx = 46 }; };
    template<> struct SixthDensePt< 2,-3, 0> { enum { idx = 47 }; };
    template<> struct SixthDensePt< 3,-3, 0> { enum { idx = 48 }; };


    template<> struct SixthDensePt<-3, 0, 3> { enum { idx = 49 }; };
    template<> struct SixthDensePt<-2, 0, 3> { enum { idx = 50 }; };
    template<> struct SixthDensePt<-1, 0, 3> { enum { idx = 51 }; };
    template<> struct SixthDensePt< 0, 0, 3> { enum { idx = 52 }; };
    template<> struct SixthDensePt< 1, 0, 3> { enum { idx = 53 }; };
    template<> struct SixthDensePt< 2, 0, 3> { enum { idx = 54 }; };
    template<> struct SixthDensePt< 3, 0, 3> { enum { idx = 55 }; };


    template<> struct SixthDensePt<-3, 0, 2> { enum { idx = 56 }; };
    template<> struct SixthDensePt<-2, 0, 2> { enum { idx = 57 }; };
    template<> struct SixthDensePt<-1, 0, 2> { enum { idx = 58 }; };
    template<> struct SixthDensePt< 0, 0, 2> { enum { idx = 59 }; };
    template<> struct SixthDensePt< 1, 0, 2> { enum { idx = 60 }; };
    template<> struct SixthDensePt< 2, 0, 2> { enum { idx = 61 }; };
    template<> struct SixthDensePt< 3, 0, 2> { enum { idx = 62 }; };

    template<> struct SixthDensePt<-3, 0, 1> { enum { idx = 63 }; };
    template<> struct SixthDensePt<-2, 0, 1> { enum { idx = 64 }; };
    template<> struct SixthDensePt<-1, 0, 1> { enum { idx = 65 }; };
    template<> struct SixthDensePt< 0, 0, 1> { enum { idx = 66 }; };
    template<> struct SixthDensePt< 1, 0, 1> { enum { idx = 67 }; };
    template<> struct SixthDensePt< 2, 0, 1> { enum { idx = 68 }; };
    template<> struct SixthDensePt< 3, 0, 1> { enum { idx = 69 }; };


    template<> struct SixthDensePt<-3, 0,-1> { enum { idx = 70 }; };
    template<> struct SixthDensePt<-2, 0,-1> { enum { idx = 71 }; };
    template<> struct SixthDensePt<-1, 0,-1> { enum { idx = 72 }; };
    template<> struct SixthDensePt< 0, 0,-1> { enum { idx = 73 }; };
    template<> struct SixthDensePt< 1, 0,-1> { enum { idx = 74 }; };
    template<> struct SixthDensePt< 2, 0,-1> { enum { idx = 75 }; };
    template<> struct SixthDensePt< 3, 0,-1> { enum { idx = 76 }; };


    template<> struct SixthDensePt<-3, 0,-2> { enum { idx = 77 }; };
    template<> struct SixthDensePt<-2, 0,-2> { enum { idx = 78 }; };
    template<> struct SixthDensePt<-1, 0,-2> { enum { idx = 79 }; };
    template<> struct SixthDensePt< 0, 0,-2> { enum { idx = 80 }; };
    template<> struct SixthDensePt< 1, 0,-2> { enum { idx = 81 }; };
    template<> struct SixthDensePt< 2, 0,-2> { enum { idx = 82 }; };
    template<> struct SixthDensePt< 3, 0,-2> { enum { idx = 83 }; };


    template<> struct SixthDensePt<-3, 0,-3> { enum { idx = 84 }; };
    template<> struct SixthDensePt<-2, 0,-3> { enum { idx = 85 }; };
    template<> struct SixthDensePt<-1, 0,-3> { enum { idx = 86 }; };
    template<> struct SixthDensePt< 0, 0,-3> { enum { idx = 87 }; };
    template<> struct SixthDensePt< 1, 0,-3> { enum { idx = 88 }; };
    template<> struct SixthDensePt< 2, 0,-3> { enum { idx = 89 }; };
    template<> struct SixthDensePt< 3, 0,-3> { enum { idx = 90 }; };


    template<> struct SixthDensePt< 0,-3, 3> { enum { idx = 91 }; };
    template<> struct SixthDensePt< 0,-2, 3> { enum { idx = 92 }; };
    template<> struct SixthDensePt< 0,-1, 3> { enum { idx = 93 }; };
    template<> struct SixthDensePt< 0, 1, 3> { enum { idx = 94 }; };
    template<> struct SixthDensePt< 0, 2, 3> { enum { idx = 95 }; };
    template<> struct SixthDensePt< 0, 3, 3> { enum { idx = 96 }; };

    template<> struct SixthDensePt< 0,-3, 2> { enum { idx = 97 }; };
    template<> struct SixthDensePt< 0,-2, 2> { enum { idx = 98 }; };
    template<> struct SixthDensePt< 0,-1, 2> { enum { idx = 99 }; };
    template<> struct SixthDensePt< 0, 1, 2> { enum { idx = 100 }; };
    template<> struct SixthDensePt< 0, 2, 2> { enum { idx = 101 }; };
    template<> struct SixthDensePt< 0, 3, 2> { enum { idx = 102 }; };

    template<> struct SixthDensePt< 0,-3, 1> { enum { idx = 103 }; };
    template<> struct SixthDensePt< 0,-2, 1> { enum { idx = 104 }; };
    template<> struct SixthDensePt< 0,-1, 1> { enum { idx = 105 }; };
    template<> struct SixthDensePt< 0, 1, 1> { enum { idx = 106 }; };
    template<> struct SixthDensePt< 0, 2, 1> { enum { idx = 107 }; };
    template<> struct SixthDensePt< 0, 3, 1> { enum { idx = 108 }; };

    template<> struct SixthDensePt< 0,-3,-1> { enum { idx = 109 }; };
    template<> struct SixthDensePt< 0,-2,-1> { enum { idx = 110 }; };
    template<> struct SixthDensePt< 0,-1,-1> { enum { idx = 111 }; };
    template<> struct SixthDensePt< 0, 1,-1> { enum { idx = 112 }; };
    template<> struct SixthDensePt< 0, 2,-1> { enum { idx = 113 }; };
    template<> struct SixthDensePt< 0, 3,-1> { enum { idx = 114 }; };

    template<> struct SixthDensePt< 0,-3,-2> { enum { idx = 115 }; };
    template<> struct SixthDensePt< 0,-2,-2> { enum { idx = 116 }; };
    template<> struct SixthDensePt< 0,-1,-2> { enum { idx = 117 }; };
    template<> struct SixthDensePt< 0, 1,-2> { enum { idx = 118 }; };
    template<> struct SixthDensePt< 0, 2,-2> { enum { idx = 119 }; };
    template<> struct SixthDensePt< 0, 3,-2> { enum { idx = 120 }; };

    template<> struct SixthDensePt< 0,-3,-3> { enum { idx = 121 }; };
    template<> struct SixthDensePt< 0,-2,-3> { enum { idx = 122 }; };
    template<> struct SixthDensePt< 0,-1,-3> { enum { idx = 123 }; };
    template<> struct SixthDensePt< 0, 1,-3> { enum { idx = 124 }; };
    template<> struct SixthDensePt< 0, 2,-3> { enum { idx = 125 }; };
    template<> struct SixthDensePt< 0, 3,-3> { enum { idx = 126 }; };

}


template<typename GridT, bool IsSafe = true>
class SixthOrderDenseStencil
    : public BaseStencil<SixthOrderDenseStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef SixthOrderDenseStencil<GridT, IsSafe> SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe >    BaseType;
public:
    typedef GridT                                 GridType;
    typedef typename GridT::TreeType              TreeType;
    typedef typename GridType::ValueType          ValueType;

    static const int SIZE = 127;

    SixthOrderDenseStencil(const GridType& grid): BaseType(grid, SIZE) {}

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return SixthDensePt<i,j,k>::idx; }

private:
    inline void init(const Coord& ijk)
    {
        mValues[SixthDensePt<-3, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3, 3, 0));
        mValues[SixthDensePt<-2, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 3, 0));
        mValues[SixthDensePt<-1, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 3, 0));
        mValues[SixthDensePt< 0, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3, 0));
        mValues[SixthDensePt< 1, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 3, 0));
        mValues[SixthDensePt< 2, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 3, 0));
        mValues[SixthDensePt< 3, 3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3, 3, 0));

        mValues[SixthDensePt<-3, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3, 2, 0));
        mValues[SixthDensePt<-2, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 2, 0));
        mValues[SixthDensePt<-1, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 2, 0));
        mValues[SixthDensePt< 0, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 0));
        mValues[SixthDensePt< 1, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 2, 0));
        mValues[SixthDensePt< 2, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 2, 0));
        mValues[SixthDensePt< 3, 2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3, 2, 0));

        mValues[SixthDensePt<-3, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3, 1, 0));
        mValues[SixthDensePt<-2, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 1, 0));
        mValues[SixthDensePt<-1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 1, 0));
        mValues[SixthDensePt< 0, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 0));
        mValues[SixthDensePt< 1, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 1, 0));
        mValues[SixthDensePt< 2, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 1, 0));
        mValues[SixthDensePt< 3, 1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3, 1, 0));

        mValues[SixthDensePt<-3, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0, 0));
        mValues[SixthDensePt<-2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 0));
        mValues[SixthDensePt<-1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 0));
        mValues[SixthDensePt< 1, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 0));
        mValues[SixthDensePt< 2, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 0));
        mValues[SixthDensePt< 3, 0, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0, 0));

        mValues[SixthDensePt<-3,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3,-1, 0));
        mValues[SixthDensePt<-2,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,-1, 0));
        mValues[SixthDensePt<-1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,-1, 0));
        mValues[SixthDensePt< 0,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 0));
        mValues[SixthDensePt< 1,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,-1, 0));
        mValues[SixthDensePt< 2,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,-1, 0));
        mValues[SixthDensePt< 3,-1, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3,-1, 0));

        mValues[SixthDensePt<-3,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3,-2, 0));
        mValues[SixthDensePt<-2,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,-2, 0));
        mValues[SixthDensePt<-1,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,-2, 0));
        mValues[SixthDensePt< 0,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 0));
        mValues[SixthDensePt< 1,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,-2, 0));
        mValues[SixthDensePt< 2,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,-2, 0));
        mValues[SixthDensePt< 3,-2, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3,-2, 0));

        mValues[SixthDensePt<-3,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-3,-3, 0));
        mValues[SixthDensePt<-2,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-2,-3, 0));
        mValues[SixthDensePt<-1,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy(-1,-3, 0));
        mValues[SixthDensePt< 0,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3, 0));
        mValues[SixthDensePt< 1,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 1,-3, 0));
        mValues[SixthDensePt< 2,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 2,-3, 0));
        mValues[SixthDensePt< 3,-3, 0>::idx] = mAcc.getValue(ijk.offsetBy( 3,-3, 0));

        mValues[SixthDensePt<-3, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0, 3));
        mValues[SixthDensePt<-2, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 3));
        mValues[SixthDensePt<-1, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 3));
        mValues[SixthDensePt< 0, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0, 3));
        mValues[SixthDensePt< 1, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 3));
        mValues[SixthDensePt< 2, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 3));
        mValues[SixthDensePt< 3, 0, 3>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0, 3));

        mValues[SixthDensePt<-3, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0, 2));
        mValues[SixthDensePt<-2, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 2));
        mValues[SixthDensePt<-1, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 2));
        mValues[SixthDensePt< 0, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0, 2));
        mValues[SixthDensePt< 1, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 2));
        mValues[SixthDensePt< 2, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 2));
        mValues[SixthDensePt< 3, 0, 2>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0, 2));

        mValues[SixthDensePt<-3, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0, 1));
        mValues[SixthDensePt<-2, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0, 1));
        mValues[SixthDensePt<-1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0, 1));
        mValues[SixthDensePt< 0, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0, 1));
        mValues[SixthDensePt< 1, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0, 1));
        mValues[SixthDensePt< 2, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0, 1));
        mValues[SixthDensePt< 3, 0, 1>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0, 1));

        mValues[SixthDensePt<-3, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0,-1));
        mValues[SixthDensePt<-2, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0,-1));
        mValues[SixthDensePt<-1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0,-1));
        mValues[SixthDensePt< 0, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0,-1));
        mValues[SixthDensePt< 1, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0,-1));
        mValues[SixthDensePt< 2, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0,-1));
        mValues[SixthDensePt< 3, 0,-1>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0,-1));

        mValues[SixthDensePt<-3, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0,-2));
        mValues[SixthDensePt<-2, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0,-2));
        mValues[SixthDensePt<-1, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0,-2));
        mValues[SixthDensePt< 0, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0,-2));
        mValues[SixthDensePt< 1, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0,-2));
        mValues[SixthDensePt< 2, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0,-2));
        mValues[SixthDensePt< 3, 0,-2>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0,-2));

        mValues[SixthDensePt<-3, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy(-3, 0,-3));
        mValues[SixthDensePt<-2, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy(-2, 0,-3));
        mValues[SixthDensePt<-1, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy(-1, 0,-3));
        mValues[SixthDensePt< 0, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 0,-3));
        mValues[SixthDensePt< 1, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy( 1, 0,-3));
        mValues[SixthDensePt< 2, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy( 2, 0,-3));
        mValues[SixthDensePt< 3, 0,-3>::idx] = mAcc.getValue(ijk.offsetBy( 3, 0,-3));

        mValues[SixthDensePt< 0,-3, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3, 3));
        mValues[SixthDensePt< 0,-2, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 3));
        mValues[SixthDensePt< 0,-1, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 3));
        mValues[SixthDensePt< 0, 1, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 3));
        mValues[SixthDensePt< 0, 2, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 3));
        mValues[SixthDensePt< 0, 3, 3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3, 3));

        mValues[SixthDensePt< 0,-3, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3, 2));
        mValues[SixthDensePt< 0,-2, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 2));
        mValues[SixthDensePt< 0,-1, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 2));
        mValues[SixthDensePt< 0, 1, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 2));
        mValues[SixthDensePt< 0, 2, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 2));
        mValues[SixthDensePt< 0, 3, 2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3, 2));

        mValues[SixthDensePt< 0,-3, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3, 1));
        mValues[SixthDensePt< 0,-2, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2, 1));
        mValues[SixthDensePt< 0,-1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1, 1));
        mValues[SixthDensePt< 0, 1, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1, 1));
        mValues[SixthDensePt< 0, 2, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2, 1));
        mValues[SixthDensePt< 0, 3, 1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3, 1));

        mValues[SixthDensePt< 0,-3,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3,-1));
        mValues[SixthDensePt< 0,-2,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2,-1));
        mValues[SixthDensePt< 0,-1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1,-1));
        mValues[SixthDensePt< 0, 1,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1,-1));
        mValues[SixthDensePt< 0, 2,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2,-1));
        mValues[SixthDensePt< 0, 3,-1>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3,-1));

        mValues[SixthDensePt< 0,-3,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3,-2));
        mValues[SixthDensePt< 0,-2,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2,-2));
        mValues[SixthDensePt< 0,-1,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1,-2));
        mValues[SixthDensePt< 0, 1,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1,-2));
        mValues[SixthDensePt< 0, 2,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2,-2));
        mValues[SixthDensePt< 0, 3,-2>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3,-2));

        mValues[SixthDensePt< 0,-3,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-3,-3));
        mValues[SixthDensePt< 0,-2,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-2,-3));
        mValues[SixthDensePt< 0,-1,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0,-1,-3));
        mValues[SixthDensePt< 0, 1,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 1,-3));
        mValues[SixthDensePt< 0, 2,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 2,-3));
        mValues[SixthDensePt< 0, 3,-3>::idx] = mAcc.getValue(ijk.offsetBy( 0, 3,-3));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
};// SixthOrderDenseStencil class


//////////////////////////////////////////////////////////////////////

namespace { // anonymous namespace for stencil-layout map

    // the seven point stencil with a different layout from SevenPt
    template<int i, int j, int k> struct GradPt {};
    template<> struct GradPt< 0, 0, 0> { enum { idx = 0 }; };
    template<> struct GradPt< 1, 0, 0> { enum { idx = 2 }; };
    template<> struct GradPt< 0, 1, 0> { enum { idx = 4 }; };
    template<> struct GradPt< 0, 0, 1> { enum { idx = 6 }; };
    template<> struct GradPt<-1, 0, 0> { enum { idx = 1 }; };
    template<> struct GradPt< 0,-1, 0> { enum { idx = 3 }; };
    template<> struct GradPt< 0, 0,-1> { enum { idx = 5 }; };
}

/// This is a simple 7-point nearest neighbor stencil that supports
/// gradient by second-order central differencing, first-order upwinding,
/// Laplacian, closest-point transform and zero-crossing test.
///
/// @note For optimal random access performance this class
/// includes its own grid accessor.
template<typename GridT, bool IsSafe = true>
class GradStencil : public BaseStencil<GradStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef GradStencil<GridT, IsSafe>         SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe > BaseType;
public:
    typedef GridT                              GridType;
    typedef typename GridT::TreeType           TreeType;
    typedef typename GridType::ValueType       ValueType;

    static const int SIZE = 7;

    GradStencil(const GridType& grid)
        : BaseType(grid, SIZE)
        , mInv2Dx(ValueType(0.5 / grid.voxelSize()[0]))
        , mInvDx2(ValueType(4.0 * mInv2Dx * mInv2Dx))
    {
    }

    GradStencil(const GridType& grid, Real dx)
        : BaseType(grid, SIZE)
        , mInv2Dx(ValueType(0.5 / dx))
        , mInvDx2(ValueType(4.0 * mInv2Dx * mInv2Dx))
    {
    }

    /// @brief Return the norm square of the single-sided upwind gradient
    /// (computed via Godunov's scheme) at the previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType normSqGrad() const
    {
        return mInvDx2 * math::GodunovsNormSqrd(mValues[0] > zeroVal<ValueType>(),
                                                mValues[0] - mValues[1],
                                                mValues[2] - mValues[0],
                                                mValues[0] - mValues[3],
                                                mValues[4] - mValues[0],
                                                mValues[0] - mValues[5],
                                                mValues[6] - mValues[0]);
    }

    /// @brief Return the gradient computed at the previously buffered
    /// location by second order central differencing.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline math::Vec3<ValueType> gradient() const
    {
        return math::Vec3<ValueType>(mValues[2] - mValues[1],
                                     mValues[4] - mValues[3],
                                     mValues[6] - mValues[5])*mInv2Dx;
    }
    /// @brief Return the first-order upwind gradient corresponding to the direction V.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline math::Vec3<ValueType> gradient(const math::Vec3<ValueType>& V) const
    {
        return math::Vec3<ValueType>(
               V[0]>0 ? mValues[0] - mValues[1] : mValues[2] - mValues[0],
               V[1]>0 ? mValues[0] - mValues[3] : mValues[4] - mValues[0],
               V[2]>0 ? mValues[0] - mValues[5] : mValues[6] - mValues[0])*2*mInv2Dx;
    }

    /// Return the Laplacian computed at the previously buffered
    /// location by second-order central differencing.
    inline ValueType laplacian() const
    {
        return mInvDx2 * (mValues[1] + mValues[2] +
                          mValues[3] + mValues[4] +
                          mValues[5] + mValues[6] - 6*mValues[0]);
    }

    /// Return @c true if the sign of the value at the center point of the stencil
    /// is different from the signs of any of its six nearest neighbors.
    inline bool zeroCrossing() const
    {
        const typename BaseType::BufferType& v = mValues;
        return (v[0]>0 ? (v[1]<0 || v[2]<0 || v[3]<0 || v[4]<0 || v[5]<0 || v[6]<0)
                       : (v[1]>0 || v[2]>0 || v[3]>0 || v[4]>0 || v[5]>0 || v[6]>0));
    }

    /// @brief Compute the closest-point transform to a level set.
    /// @return the closest point in index space to the surface
    /// from which the level set was derived.
    ///
    /// @note This method assumes that the grid represents a level set
    /// with distances in world units and a simple affine transfrom
    /// with uniform scaling.
    inline math::Vec3<ValueType> cpt()
    {
        const Coord& ijk = BaseType::getCenterCoord();
        const ValueType d = ValueType(mValues[0] * 0.5 * mInvDx2); // distance in voxels / (2dx^2)
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        const auto value = math::Vec3<ValueType>(   ijk[0] - d*(mValues[2] - mValues[1]),
                                                    ijk[1] - d*(mValues[4] - mValues[3]),
                                                    ijk[2] - d*(mValues[6] - mValues[5]));
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
        return value;
    }

    /// Return linear offset for the specified stencil point relative to its center
    template<int i, int j, int k>
    unsigned int pos() const { return GradPt<i,j,k>::idx; }

private:

    inline void init(const Coord& ijk)
    {
        BaseType::template setValue<-1, 0, 0>(mAcc.getValue(ijk.offsetBy(-1, 0, 0)));
        BaseType::template setValue< 1, 0, 0>(mAcc.getValue(ijk.offsetBy( 1, 0, 0)));

        BaseType::template setValue< 0,-1, 0>(mAcc.getValue(ijk.offsetBy( 0,-1, 0)));
        BaseType::template setValue< 0, 1, 0>(mAcc.getValue(ijk.offsetBy( 0, 1, 0)));

        BaseType::template setValue< 0, 0,-1>(mAcc.getValue(ijk.offsetBy( 0, 0,-1)));
        BaseType::template setValue< 0, 0, 1>(mAcc.getValue(ijk.offsetBy( 0, 0, 1)));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
    const ValueType mInv2Dx, mInvDx2;
}; // GradStencil class

////////////////////////////////////////


/// @brief This is a special 19-point stencil that supports optimal fifth-order WENO
/// upwinding, second-order central differencing, Laplacian, and zero-crossing test.
///
/// @note For optimal random access performance this class
/// includes its own grid accessor.
template<typename GridT, bool IsSafe = true>
class WenoStencil: public BaseStencil<WenoStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef WenoStencil<GridT, IsSafe>         SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe > BaseType;
public:
    typedef GridT                              GridType;
    typedef typename GridT::TreeType           TreeType;
    typedef typename GridType::ValueType       ValueType;

    static const int SIZE = 19;

    WenoStencil(const GridType& grid)
        : BaseType(grid, SIZE)
        , mDx2(ValueType(math::Pow2(grid.voxelSize()[0])))
        , mInv2Dx(ValueType(0.5 / grid.voxelSize()[0]))
        , mInvDx2(ValueType(1.0 / mDx2))
    {
    }

    WenoStencil(const GridType& grid, Real dx)
        : BaseType(grid, SIZE)
        , mDx2(ValueType(dx * dx))
        , mInv2Dx(ValueType(0.5 / dx))
        , mInvDx2(ValueType(1.0 / mDx2))
    {
    }

    /// @brief Return the norm-square of the WENO upwind gradient (computed via
    /// WENO upwinding and Godunov's scheme) at the previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType normSqGrad(const ValueType &isoValue = zeroVal<ValueType>()) const
    {
        const typename BaseType::BufferType& v = mValues;
#ifdef DWA_OPENVDB
        // SSE optimized
        const simd::Float4
            v1(v[2]-v[1], v[ 8]-v[ 7], v[14]-v[13], 0),
            v2(v[3]-v[2], v[ 9]-v[ 8], v[15]-v[14], 0),
            v3(v[0]-v[3], v[ 0]-v[ 9], v[ 0]-v[15], 0),
            v4(v[4]-v[0], v[10]-v[ 0], v[16]-v[ 0], 0),
            v5(v[5]-v[4], v[11]-v[10], v[17]-v[16], 0),
            v6(v[6]-v[5], v[12]-v[11], v[18]-v[17], 0),
            dP_m = math::WENO5(v1, v2, v3, v4, v5, mDx2),
            dP_p = math::WENO5(v6, v5, v4, v3, v2, mDx2);

        return mInvDx2 * math::GodunovsNormSqrd(mValues[0] > isoValue, dP_m, dP_p);
#else
        const Real
            dP_xm = math::WENO5(v[ 2]-v[ 1],v[ 3]-v[ 2],v[ 0]-v[ 3],v[ 4]-v[ 0],v[ 5]-v[ 4],mDx2),
            dP_xp = math::WENO5(v[ 6]-v[ 5],v[ 5]-v[ 4],v[ 4]-v[ 0],v[ 0]-v[ 3],v[ 3]-v[ 2],mDx2),
            dP_ym = math::WENO5(v[ 8]-v[ 7],v[ 9]-v[ 8],v[ 0]-v[ 9],v[10]-v[ 0],v[11]-v[10],mDx2),
            dP_yp = math::WENO5(v[12]-v[11],v[11]-v[10],v[10]-v[ 0],v[ 0]-v[ 9],v[ 9]-v[ 8],mDx2),
            dP_zm = math::WENO5(v[14]-v[13],v[15]-v[14],v[ 0]-v[15],v[16]-v[ 0],v[17]-v[16],mDx2),
            dP_zp = math::WENO5(v[18]-v[17],v[17]-v[16],v[16]-v[ 0],v[ 0]-v[15],v[15]-v[14],mDx2);
        return static_cast<ValueType>(
            mInvDx2*math::GodunovsNormSqrd(v[0]>isoValue, dP_xm, dP_xp, dP_ym, dP_yp, dP_zm, dP_zp));
#endif
    }

    /// Return the optimal fifth-order upwind gradient corresponding to the
    /// direction V.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline math::Vec3<ValueType> gradient(const math::Vec3<ValueType>& V) const
    {
        const typename BaseType::BufferType& v = mValues;
        return 2*mInv2Dx * math::Vec3<ValueType>(
            V[0]>0 ? math::WENO5(v[ 2]-v[ 1],v[ 3]-v[ 2],v[ 0]-v[ 3], v[ 4]-v[ 0],v[ 5]-v[ 4],mDx2)
                : math::WENO5(v[ 6]-v[ 5],v[ 5]-v[ 4],v[ 4]-v[ 0], v[ 0]-v[ 3],v[ 3]-v[ 2],mDx2),
            V[1]>0 ? math::WENO5(v[ 8]-v[ 7],v[ 9]-v[ 8],v[ 0]-v[ 9], v[10]-v[ 0],v[11]-v[10],mDx2)
                : math::WENO5(v[12]-v[11],v[11]-v[10],v[10]-v[ 0], v[ 0]-v[ 9],v[ 9]-v[ 8],mDx2),
            V[2]>0 ? math::WENO5(v[14]-v[13],v[15]-v[14],v[ 0]-v[15], v[16]-v[ 0],v[17]-v[16],mDx2)
                : math::WENO5(v[18]-v[17],v[17]-v[16],v[16]-v[ 0], v[ 0]-v[15],v[15]-v[14],mDx2));
    }
    /// Return the gradient computed at the previously buffered
    /// location by second-order central differencing.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline math::Vec3<ValueType> gradient() const
    {
        return mInv2Dx * math::Vec3<ValueType>(mValues[ 4] - mValues[ 3],
                                               mValues[10] - mValues[ 9],
                                               mValues[16] - mValues[15]);
    }

    /// Return the Laplacian computed at the previously buffered
    /// location by second-order central differencing.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType laplacian() const
    {
        return mInvDx2 * (
            mValues[ 3] + mValues[ 4] +
            mValues[ 9] + mValues[10] +
            mValues[15] + mValues[16] - 6*mValues[0]);
    }

    /// Return @c true if the sign of the value at the center point of the stencil
    /// differs from the sign of any of its six nearest neighbors
    inline bool zeroCrossing() const
    {
        const typename BaseType::BufferType& v = mValues;
        return (v[ 0]>0 ? (v[ 3]<0 || v[ 4]<0 || v[ 9]<0 || v[10]<0 || v[15]<0 || v[16]<0)
                        : (v[ 3]>0 || v[ 4]>0 || v[ 9]>0 || v[10]>0 || v[15]>0 || v[16]>0));
    }

private:
    inline void init(const Coord& ijk)
    {
        mValues[ 1] = mAcc.getValue(ijk.offsetBy(-3,  0,  0));
        mValues[ 2] = mAcc.getValue(ijk.offsetBy(-2,  0,  0));
        mValues[ 3] = mAcc.getValue(ijk.offsetBy(-1,  0,  0));
        mValues[ 4] = mAcc.getValue(ijk.offsetBy( 1,  0,  0));
        mValues[ 5] = mAcc.getValue(ijk.offsetBy( 2,  0,  0));
        mValues[ 6] = mAcc.getValue(ijk.offsetBy( 3,  0,  0));

        mValues[ 7] = mAcc.getValue(ijk.offsetBy( 0, -3,  0));
        mValues[ 8] = mAcc.getValue(ijk.offsetBy( 0, -2,  0));
        mValues[ 9] = mAcc.getValue(ijk.offsetBy( 0, -1,  0));
        mValues[10] = mAcc.getValue(ijk.offsetBy( 0,  1,  0));
        mValues[11] = mAcc.getValue(ijk.offsetBy( 0,  2,  0));
        mValues[12] = mAcc.getValue(ijk.offsetBy( 0,  3,  0));

        mValues[13] = mAcc.getValue(ijk.offsetBy( 0,  0, -3));
        mValues[14] = mAcc.getValue(ijk.offsetBy( 0,  0, -2));
        mValues[15] = mAcc.getValue(ijk.offsetBy( 0,  0, -1));
        mValues[16] = mAcc.getValue(ijk.offsetBy( 0,  0,  1));
        mValues[17] = mAcc.getValue(ijk.offsetBy( 0,  0,  2));
        mValues[18] = mAcc.getValue(ijk.offsetBy( 0,  0,  3));
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
    const ValueType mDx2, mInv2Dx, mInvDx2;
}; // WenoStencil class


//////////////////////////////////////////////////////////////////////


template<typename GridT, bool IsSafe = true>
class CurvatureStencil: public BaseStencil<CurvatureStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef CurvatureStencil<GridT, IsSafe>   SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe> BaseType;
public:
    typedef GridT                             GridType;
    typedef typename GridT::TreeType          TreeType;
    typedef typename GridT::ValueType         ValueType;

     static const int SIZE = 19;

    CurvatureStencil(const GridType& grid)
        : BaseType(grid, SIZE)
        , mInv2Dx(ValueType(0.5 / grid.voxelSize()[0]))
        , mInvDx2(ValueType(4.0 * mInv2Dx * mInv2Dx))
    {
    }

    CurvatureStencil(const GridType& grid, Real dx)
        : BaseType(grid, SIZE)
        , mInv2Dx(ValueType(0.5 / dx))
        , mInvDx2(ValueType(4.0 * mInv2Dx * mInv2Dx))
    {
    }

    /// @brief Return the mean curvature at the previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType meanCurvature() const
    {
        Real alpha, normGrad;
        return this->meanCurvature(alpha, normGrad) ?
               ValueType(alpha*mInv2Dx/math::Pow3(normGrad)) : 0;
    }

    /// @brief Return the Gaussian curvature at the previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType gaussianCurvature() const
    {
        Real alpha, normGrad;
        return this->gaussianCurvature(alpha, normGrad) ?
               ValueType(alpha*mInvDx2/math::Pow4(normGrad)) : 0;
    }

    /// @brief Return both the mean and the Gaussian curvature at the
    ///        previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline void curvatures(ValueType &mean, ValueType& gauss) const
    {
        Real alphaM, alphaG, normGrad;
        if (this->curvatures(alphaM, alphaG, normGrad)) {
          mean  = ValueType(alphaM*mInv2Dx/math::Pow3(normGrad));
          gauss = ValueType(alphaG*mInvDx2/math::Pow4(normGrad));
        } else {
          mean = gauss = 0;
        }
    }

    /// Return the mean curvature multiplied by the norm of the
    /// central-difference gradient. This method is very useful for
    /// mean-curvature flow of level sets!
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType meanCurvatureNormGrad() const
    {
        Real alpha, normGrad;
        return this->meanCurvature(alpha, normGrad) ?
               ValueType(alpha*mInvDx2/(2*math::Pow2(normGrad))) : 0;
    }

    /// Return the mean Gaussian multiplied by the norm of the
    /// central-difference gradient.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType gaussianCurvatureNormGrad() const
    {
        Real alpha, normGrad;
        return this->gaussianCurvature(alpha, normGrad) ?
               ValueType(2*alpha*mInv2Dx*mInvDx2/math::Pow3(normGrad)) : 0;
    }

    /// @brief Return both the mean and the Gaussian curvature at the
    ///        previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline void curvaturesNormGrad(ValueType &mean, ValueType& gauss) const
    {
        Real alphaM, alphaG, normGrad;
        if (this->curvatures(alphaM, alphaG, normGrad)) {
          mean  = ValueType(alphaM*mInvDx2/(2*math::Pow2(normGrad)));
          gauss = ValueType(2*alphaG*mInv2Dx*mInvDx2/math::Pow3(normGrad));
        } else {
          mean = gauss = 0;
        }
    }

    /// @brief Return the pair (minimum, maximum) principal curvature at the
    ///        previously buffered location.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline std::pair<ValueType, ValueType> principalCurvatures() const
    {
        std::pair<ValueType, ValueType> pair(0, 0);// min, max
        Real alphaM, alphaG, normGrad;
        if (this->curvatures(alphaM, alphaG, normGrad)) {
          const Real mean = alphaM*mInv2Dx/math::Pow3(normGrad);
          const Real tmp = std::sqrt(mean*mean - alphaG*mInvDx2/math::Pow4(normGrad));
          pair.first  = ValueType(mean - tmp);
          pair.second = ValueType(mean + tmp);
        }
        return pair;// min, max
    }

    /// Return the Laplacian computed at the previously buffered
    /// location by second-order central differencing.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline ValueType laplacian() const
    {
        return mInvDx2 * (
            mValues[1] + mValues[2] +
            mValues[3] + mValues[4] +
            mValues[5] + mValues[6] - 6*mValues[0]);
    }

    /// Return the gradient computed at the previously buffered
    /// location by second-order central differencing.
    ///
    /// @note This method should not be called until the stencil
    /// buffer has been populated via a call to moveTo(ijk).
    inline math::Vec3<ValueType> gradient() const
    {
        return math::Vec3<ValueType>(
            mValues[2] - mValues[1],
            mValues[4] - mValues[3],
            mValues[6] - mValues[5])*mInv2Dx;
    }

private:
    inline void init(const Coord &ijk)
    {
        mValues[ 1] = mAcc.getValue(ijk.offsetBy(-1,  0,  0));
        mValues[ 2] = mAcc.getValue(ijk.offsetBy( 1,  0,  0));

        mValues[ 3] = mAcc.getValue(ijk.offsetBy( 0, -1,  0));
        mValues[ 4] = mAcc.getValue(ijk.offsetBy( 0,  1,  0));

        mValues[ 5] = mAcc.getValue(ijk.offsetBy( 0,  0, -1));
        mValues[ 6] = mAcc.getValue(ijk.offsetBy( 0,  0,  1));

        mValues[ 7] = mAcc.getValue(ijk.offsetBy(-1, -1,  0));
        mValues[ 8] = mAcc.getValue(ijk.offsetBy( 1, -1,  0));
        mValues[ 9] = mAcc.getValue(ijk.offsetBy(-1,  1,  0));
        mValues[10] = mAcc.getValue(ijk.offsetBy( 1,  1,  0));

        mValues[11] = mAcc.getValue(ijk.offsetBy(-1,  0, -1));
        mValues[12] = mAcc.getValue(ijk.offsetBy( 1,  0, -1));
        mValues[13] = mAcc.getValue(ijk.offsetBy(-1,  0,  1));
        mValues[14] = mAcc.getValue(ijk.offsetBy( 1,  0,  1));

        mValues[15] = mAcc.getValue(ijk.offsetBy( 0, -1, -1));
        mValues[16] = mAcc.getValue(ijk.offsetBy( 0,  1, -1));
        mValues[17] = mAcc.getValue(ijk.offsetBy( 0, -1,  1));
        mValues[18] = mAcc.getValue(ijk.offsetBy( 0,  1,  1));
    }

    inline Real Dx()  const { return 0.5*(mValues[2] - mValues[1]); }// * 1/dx
    inline Real Dy()  const { return 0.5*(mValues[4] - mValues[3]); }// * 1/dx
    inline Real Dz()  const { return 0.5*(mValues[6] - mValues[5]); }// * 1/dx
    inline Real Dxx() const { return mValues[2] - 2 * mValues[0] + mValues[1]; }// * 1/dx2
    inline Real Dyy() const { return mValues[4] - 2 * mValues[0] + mValues[3]; }// * 1/dx2}
    inline Real Dzz() const { return mValues[6] - 2 * mValues[0] + mValues[5]; }// * 1/dx2
    inline Real Dxy() const { return 0.25 * (mValues[10] - mValues[ 8] + mValues[ 7] - mValues[ 9]); }// * 1/dx2
    inline Real Dxz() const { return 0.25 * (mValues[14] - mValues[12] + mValues[11] - mValues[13]); }// * 1/dx2
    inline Real Dyz() const { return 0.25 * (mValues[18] - mValues[16] + mValues[15] - mValues[17]); }// * 1/dx2

    inline bool meanCurvature(Real& alpha, Real& normGrad) const
    {
        // For performance all finite differences are unscaled wrt dx
        const Real Dx = this->Dx(), Dy = this->Dy(), Dz = this->Dz(),
                   Dx2 = Dx*Dx, Dy2 = Dy*Dy, Dz2 = Dz*Dz, normGrad2 = Dx2 + Dy2 + Dz2;
        if (normGrad2 <= math::Tolerance<Real>::value()) {
             alpha = normGrad = 0;
             return false;
        }
        const Real Dxx = this->Dxx(), Dyy = this->Dyy(), Dzz = this->Dzz();
        alpha = Dx2*(Dyy + Dzz) + Dy2*(Dxx + Dzz) + Dz2*(Dxx + Dyy) -
                2*(Dx*(Dy*this->Dxy() + Dz*this->Dxz()) + Dy*Dz*this->Dyz());// * 1/dx^4
        normGrad = std::sqrt(normGrad2); // * 1/dx
        return true;
    }

    inline bool gaussianCurvature(Real& alpha, Real& normGrad) const
    {
        // For performance all finite differences are unscaled wrt dx
        const Real Dx = this->Dx(), Dy = this->Dy(), Dz = this->Dz(),
                   Dx2 = Dx*Dx, Dy2 = Dy*Dy, Dz2 = Dz*Dz, normGrad2 = Dx2 + Dy2 + Dz2;
        if (normGrad2 <= math::Tolerance<Real>::value()) {
             alpha = normGrad = 0;
             return false;
        }
        const Real Dxx = this->Dxx(), Dyy = this->Dyy(), Dzz = this->Dzz(),
                   Dxy = this->Dxy(), Dxz = this->Dxz(), Dyz = this->Dyz();
        alpha = Dx2*(Dyy*Dzz - Dyz*Dyz) + Dy2*(Dxx*Dzz - Dxz*Dxz) + Dz2*(Dxx*Dyy - Dxy*Dxy) +
                2*( Dy*Dz*(Dxy*Dxz - Dyz*Dxx) + Dx*Dz*(Dxy*Dyz - Dxz*Dyy) + Dx*Dy*(Dxz*Dyz - Dxy*Dzz) );// * 1/dx^6
        normGrad  = std::sqrt(normGrad2); // * 1/dx
        return true;
    }
    inline bool curvatures(Real& alphaM, Real& alphaG, Real& normGrad) const
    {
        // For performance all finite differences are unscaled wrt dx
        const Real Dx = this->Dx(), Dy = this->Dy(), Dz = this->Dz(),
                   Dx2 = Dx*Dx, Dy2 = Dy*Dy, Dz2 = Dz*Dz, normGrad2 = Dx2 + Dy2 + Dz2;
        if (normGrad2 <= math::Tolerance<Real>::value()) {
             alphaM = alphaG =normGrad = 0;
             return false;
        }
        const Real Dxx = this->Dxx(), Dyy = this->Dyy(), Dzz = this->Dzz(),
                   Dxy = this->Dxy(), Dxz = this->Dxz(), Dyz = this->Dyz();
        alphaM = Dx2*(Dyy + Dzz) + Dy2*(Dxx + Dzz) + Dz2*(Dxx + Dyy) -
                 2*(Dx*(Dy*Dxy + Dz*Dxz) + Dy*Dz*Dyz);// *1/dx^4
        alphaG = Dx2*(Dyy*Dzz - Dyz*Dyz) + Dy2*(Dxx*Dzz - Dxz*Dxz) + Dz2*(Dxx*Dyy - Dxy*Dxy) +
                 2*( Dy*Dz*(Dxy*Dxz - Dyz*Dxx) + Dx*Dz*(Dxy*Dyz - Dxz*Dyy) + Dx*Dy*(Dxz*Dyz - Dxy*Dzz) );// *1/dx^6
        normGrad  = std::sqrt(normGrad2); // * 1/dx
        return true;
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
    const ValueType mInv2Dx, mInvDx2;
}; // CurvatureStencil class


//////////////////////////////////////////////////////////////////////


/// @brief Dense stencil of a given width
template<typename GridT, bool IsSafe = true>
class DenseStencil: public BaseStencil<DenseStencil<GridT, IsSafe>, GridT, IsSafe>
{
    typedef DenseStencil<GridT, IsSafe>       SelfT;
    typedef BaseStencil<SelfT, GridT, IsSafe> BaseType;
public:
    typedef GridT                             GridType;
    typedef typename GridT::TreeType          TreeType;
    typedef typename GridType::ValueType      ValueType;

    DenseStencil(const GridType& grid, int halfWidth)
        : BaseType(grid, /*size=*/math::Pow3(2 * halfWidth + 1))
        , mHalfWidth(halfWidth)
    {
        assert(halfWidth>0);
    }

    inline const ValueType& getCenterValue() const { return mValues[(mValues.size()-1)>>1]; }

    /// @brief Initialize the stencil buffer with the values of voxel (x, y, z)
    /// and its neighbors.
    inline void moveTo(const Coord& ijk)
    {
        BaseType::mCenter = ijk;
        this->init(ijk);
    }
    /// @brief Initialize the stencil buffer with the values of voxel
    /// (x, y, z) and its neighbors.
    template<typename IterType>
    inline void moveTo(const IterType& iter)
    {
        BaseType::mCenter = iter.getCoord();
        this->init(BaseType::mCenter);
    }

private:
    /// Initialize the stencil buffer centered at (i, j, k).
    /// @warning The center point is NOT at mValues[0] for this DenseStencil!
    inline void init(const Coord& ijk)
    {
        int n = 0;
        for (Coord p=ijk.offsetBy(-mHalfWidth), q=ijk.offsetBy(mHalfWidth); p[0] <= q[0]; ++p[0]) {
            for (p[1] = ijk[1]-mHalfWidth; p[1] <= q[1]; ++p[1]) {
                for (p[2] = ijk[2]-mHalfWidth; p[2] <= q[2]; ++p[2]) {
                    mValues[n++] = mAcc.getValue(p);
                }
            }
        }
    }

    template<typename, typename, bool> friend class BaseStencil; // allow base class to call init()
    using BaseType::mAcc;
    using BaseType::mValues;
    const int mHalfWidth;
};// DenseStencil class


} // end math namespace
} // namespace OPENVDB_VERSION_NAME
} // end openvdb namespace

#endif // OPENVDB_MATH_STENCILS_HAS_BEEN_INCLUDED
