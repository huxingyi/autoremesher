// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "Util.h"
#include <limits>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace util {

const Index32 INVALID_IDX = std::numeric_limits<Index32>::max();

const Coord COORD_OFFSETS[26] =
{
    Coord( 1,  0,  0), /// Voxel-face adjacent neghbours
    Coord(-1,  0,  0), /// 0 to 5
    Coord( 0,  1,  0),
    Coord( 0, -1,  0),
    Coord( 0,  0,  1),
    Coord( 0,  0, -1),
    Coord( 1,  0, -1), /// Voxel-edge adjacent neghbours
    Coord(-1,  0, -1), /// 6 to 17
    Coord( 1,  0,  1),
    Coord(-1,  0,  1),
    Coord( 1,  1,  0),
    Coord(-1,  1,  0),
    Coord( 1, -1,  0),
    Coord(-1, -1,  0),
    Coord( 0, -1,  1),
    Coord( 0, -1, -1),
    Coord( 0,  1,  1),
    Coord( 0,  1, -1),
    Coord(-1, -1, -1), /// Voxel-corner adjacent neghbours
    Coord(-1, -1,  1), /// 18 to 25
    Coord( 1, -1,  1),
    Coord( 1, -1, -1),
    Coord(-1,  1, -1),
    Coord(-1,  1,  1),
    Coord( 1,  1,  1),
    Coord( 1,  1, -1)
};

} // namespace util
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
