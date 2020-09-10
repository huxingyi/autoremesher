// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file VectorTransformer.h

#ifndef OPENVDB_TOOLS_VECTORTRANSFORMER_HAS_BEEN_INCLUDED
#define OPENVDB_TOOLS_VECTORTRANSFORMER_HAS_BEEN_INCLUDED

#include <openvdb/Types.h>
#include <openvdb/math/Mat4.h>
#include <openvdb/math/Vec3.h>
#include "ValueTransformer.h" // for tools::foreach()
#include <type_traits>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace tools {

/// @brief Apply an affine transform to the voxel values of a vector-valued grid
/// in accordance with the grid's vector type (covariant, contravariant, etc.).
/// @throw TypeError if the grid is not vector-valued
template<typename GridType>
inline void
transformVectors(GridType&, const Mat4d&);


////////////////////////////////////////


// Functors for use with tools::foreach() to transform vector voxel values

struct HomogeneousMatMul
{
    const Mat4d mat;
    HomogeneousMatMul(const Mat4d& _mat): mat(_mat) {}
    template<typename TreeIterT> void operator()(const TreeIterT& it) const
    {
        Vec3d v(*it);
        it.setValue(mat.transformH(v));
    }
};

struct MatMul
{
    const Mat4d mat;
    MatMul(const Mat4d& _mat): mat(_mat) {}
    template<typename TreeIterT>
    void operator()(const TreeIterT& it) const
    {
        Vec3d v(*it);
        it.setValue(mat.transform3x3(v));
    }
};

struct MatMulNormalize
{
    const Mat4d mat;
    MatMulNormalize(const Mat4d& _mat): mat(_mat) {}
    template<typename TreeIterT>
    void operator()(const TreeIterT& it) const
    {
        Vec3d v(*it);
        v = mat.transform3x3(v);
        v.normalize();
        it.setValue(v);
    }
};


//{
/// @cond OPENVDB_VECTOR_TRANSFORMER_INTERNAL

/// @internal This overload is enabled only for scalar-valued grids.
template<typename GridType> inline
typename std::enable_if<!VecTraits<typename GridType::ValueType>::IsVec, void>::type
doTransformVectors(GridType&, const Mat4d&)
{
    OPENVDB_THROW(TypeError, "tools::transformVectors() requires a vector-valued grid");
}

/// @internal This overload is enabled only for vector-valued grids.
template<typename GridType> inline
typename std::enable_if<VecTraits<typename GridType::ValueType>::IsVec, void>::type
doTransformVectors(GridType& grid, const Mat4d& mat)
{
    if (!grid.isInWorldSpace()) return;

    const VecType vecType = grid.getVectorType();
    switch (vecType) {
        case VEC_COVARIANT:
        case VEC_COVARIANT_NORMALIZE:
        {
            Mat4d invmat = mat.inverse();
            invmat = invmat.transpose();

            if (vecType == VEC_COVARIANT_NORMALIZE) {
                foreach(grid.beginValueAll(), MatMulNormalize(invmat));
            } else {
                foreach(grid.beginValueAll(), MatMul(invmat));
            }
            break;
        }

        case VEC_CONTRAVARIANT_RELATIVE:
            foreach(grid.beginValueAll(), MatMul(mat));
            break;

        case VEC_CONTRAVARIANT_ABSOLUTE:
            foreach(grid.beginValueAll(), HomogeneousMatMul(mat));
            break;

        case VEC_INVARIANT:
            break;
    }
}

/// @endcond
//}


template<typename GridType>
inline void
transformVectors(GridType& grid, const Mat4d& mat)
{
    doTransformVectors<GridType>(grid, mat);
}

} // namespace tools
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_TOOLS_VECTORTRANSFORMER_HAS_BEEN_INCLUDED
