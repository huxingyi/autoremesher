// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file pyVec3Grid.cc
/// @brief Boost.Python wrappers for vector-valued openvdb::Grid types

#include "pyGrid.h"


void exportVec3Grid();


void
exportVec3Grid()
{
    pyGrid::exportGrid<Vec3SGrid>();
#ifdef PY_OPENVDB_WRAP_ALL_GRID_TYPES
    pyGrid::exportGrid<Vec3IGrid>();
    pyGrid::exportGrid<Vec3DGrid>();
#endif
}
