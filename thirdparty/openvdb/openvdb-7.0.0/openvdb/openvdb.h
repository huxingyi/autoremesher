// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_OPENVDB_HAS_BEEN_INCLUDED
#define OPENVDB_OPENVDB_HAS_BEEN_INCLUDED

#include "Platform.h"
#include "Types.h"
#include "Metadata.h"
#include "math/Maps.h"
#include "math/Transform.h"
#include "Grid.h"
#include "tree/Tree.h"
#include "io/File.h"


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {

/// Common tree types
using BoolTree     = tree::Tree4<bool,        5, 4, 3>::Type;
using DoubleTree   = tree::Tree4<double,      5, 4, 3>::Type;
using FloatTree    = tree::Tree4<float,       5, 4, 3>::Type;
using Int32Tree    = tree::Tree4<int32_t,     5, 4, 3>::Type;
using Int64Tree    = tree::Tree4<int64_t,     5, 4, 3>::Type;
using MaskTree     = tree::Tree4<ValueMask,   5, 4, 3>::Type;
using StringTree   = tree::Tree4<std::string, 5, 4, 3>::Type;
using UInt32Tree   = tree::Tree4<uint32_t,    5, 4, 3>::Type;
using Vec2DTree    = tree::Tree4<Vec2d,       5, 4, 3>::Type;
using Vec2ITree    = tree::Tree4<Vec2i,       5, 4, 3>::Type;
using Vec2STree    = tree::Tree4<Vec2s,       5, 4, 3>::Type;
using Vec3DTree    = tree::Tree4<Vec3d,       5, 4, 3>::Type;
using Vec3ITree    = tree::Tree4<Vec3i,       5, 4, 3>::Type;
using Vec3STree    = tree::Tree4<Vec3f,       5, 4, 3>::Type;
using ScalarTree   = FloatTree;
using TopologyTree = MaskTree;
using Vec3dTree    = Vec3DTree;
using Vec3fTree    = Vec3STree;
using VectorTree   = Vec3fTree;

/// Common grid types
using BoolGrid     = Grid<BoolTree>;
using DoubleGrid   = Grid<DoubleTree>;
using FloatGrid    = Grid<FloatTree>;
using Int32Grid    = Grid<Int32Tree>;
using Int64Grid    = Grid<Int64Tree>;
using MaskGrid     = Grid<MaskTree>;
using StringGrid   = Grid<StringTree>;
using Vec3DGrid    = Grid<Vec3DTree>;
using Vec3IGrid    = Grid<Vec3ITree>;
using Vec3SGrid    = Grid<Vec3STree>;
using ScalarGrid   = FloatGrid;
using TopologyGrid = MaskGrid;
using Vec3dGrid    = Vec3DGrid;
using Vec3fGrid    = Vec3SGrid;
using VectorGrid   = Vec3fGrid;

/// Global registration of basic types
OPENVDB_API void initialize();

/// Global deregistration of basic types
OPENVDB_API void uninitialize();

} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_OPENVDB_HAS_BEEN_INCLUDED
