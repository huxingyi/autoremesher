// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file pyIntGrid.cc
/// @brief Boost.Python wrappers for scalar, integer-valued openvdb::Grid types

#include "pyGrid.h"


void exportIntGrid();


void
exportIntGrid()
{
    pyGrid::exportGrid<BoolGrid>();
#ifdef PY_OPENVDB_WRAP_ALL_GRID_TYPES
    pyGrid::exportGrid<Int32Grid>();
    pyGrid::exportGrid<Int64Grid>();
#endif
}
