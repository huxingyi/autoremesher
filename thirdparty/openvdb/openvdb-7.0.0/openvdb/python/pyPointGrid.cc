// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file pyPointGrid.cc
/// @brief Boost.Python wrappers for point openvdb::Grid types

#include <boost/python.hpp>

#include "pyGrid.h"

namespace py = boost::python;


void exportPointGrid();


void
exportPointGrid()
{
#ifdef PY_OPENVDB_WRAP_ALL_GRID_TYPES
    pyGrid::exportGrid<points::PointDataGrid>();
#endif
}
