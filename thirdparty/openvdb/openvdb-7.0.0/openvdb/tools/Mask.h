// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file Mask.h
///
/// @brief Construct boolean mask grids from grids of arbitrary type

#ifndef OPENVDB_TOOLS_MASK_HAS_BEEN_INCLUDED
#define OPENVDB_TOOLS_MASK_HAS_BEEN_INCLUDED

#include <openvdb/Grid.h>
#include "LevelSetUtil.h" // for tools::sdfInteriorMask()
#include <type_traits> // for std::enable_if, std::is_floating_point


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace tools {

/// @brief Given an input grid of any type, return a new, boolean grid
/// whose active voxel topology matches the input grid's or,
/// if the input grid is a level set, matches the input grid's interior.
/// @param grid      the grid from which to construct a mask
/// @param isovalue  for a level set grid, the isovalue that defines the grid's interior
/// @sa tools::sdfInteriorMask()
template<typename GridType>
inline typename GridType::template ValueConverter<bool>::Type::Ptr
interiorMask(const GridType& grid, const double isovalue = 0.0);


////////////////////////////////////////


namespace mask_internal {

/// @private
template<typename GridType>
struct Traits {
    static const bool isBool = std::is_same<typename GridType::ValueType, bool>::value;
    using BoolGridType = typename GridType::template ValueConverter<bool>::Type;
    using BoolGridPtrType = typename BoolGridType::Ptr;
};


/// @private
template<typename GridType>
inline typename std::enable_if<std::is_floating_point<typename GridType::ValueType>::value,
    typename mask_internal::Traits<GridType>::BoolGridPtrType>::type
doLevelSetInteriorMask(const GridType& grid, const double isovalue)
{
    using GridValueT = typename GridType::ValueType;
    using MaskGridPtrT = typename mask_internal::Traits<GridType>::BoolGridPtrType;

    // If the input grid is a level set (and floating-point), return a mask of its interior.
    if (grid.getGridClass() == GRID_LEVEL_SET) {
        return tools::sdfInteriorMask(grid, static_cast<GridValueT>(isovalue));
    }
    return MaskGridPtrT{};
}


/// @private
// No-op specialization for non-floating-point grids
template<typename GridType>
inline typename std::enable_if<!std::is_floating_point<typename GridType::ValueType>::value,
    typename mask_internal::Traits<GridType>::BoolGridPtrType>::type
doLevelSetInteriorMask(const GridType&, const double /*isovalue*/)
{
    using MaskGridPtrT = typename mask_internal::Traits<GridType>::BoolGridPtrType;
    return MaskGridPtrT{};
}


/// @private
template<typename GridType>
inline typename std::enable_if<mask_internal::Traits<GridType>::isBool,
    typename mask_internal::Traits<GridType>::BoolGridPtrType>::type
doInteriorMask(const GridType& grid, const double /*isovalue*/)
{
    // If the input grid is already boolean, return a copy of it.
    return grid.deepCopy();
}


/// @private
template<typename GridType>
inline typename std::enable_if<!(mask_internal::Traits<GridType>::isBool),
    typename mask_internal::Traits<GridType>::BoolGridPtrType>::type
doInteriorMask(const GridType& grid, const double isovalue)
{
    using MaskGridT = typename mask_internal::Traits<GridType>::BoolGridType;

    // If the input grid is a level set, return a mask of its interior.
    if (auto maskGridPtr = doLevelSetInteriorMask(grid, isovalue)) {
        return maskGridPtr;
    }

    // For any other grid type, return a mask of its active voxels.
    auto maskGridPtr = MaskGridT::create(/*background=*/false);
    maskGridPtr->setTransform(grid.transform().copy());
    maskGridPtr->topologyUnion(grid);
    return maskGridPtr;
}

} // namespace mask_internal


template<typename GridType>
inline typename GridType::template ValueConverter<bool>::Type::Ptr
interiorMask(const GridType& grid, const double isovalue)
{
    return mask_internal::doInteriorMask(grid, isovalue);
}


////////////////////////////////////////

} // namespace tools
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_TOOLS_MASK_HAS_BEEN_INCLUDED
