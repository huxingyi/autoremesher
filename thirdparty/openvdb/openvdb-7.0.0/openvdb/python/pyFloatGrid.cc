// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file pyFloatGrid.cc
/// @author Peter Cucka
/// @brief Boost.Python wrappers for scalar, floating-point openvdb::Grid types

#include "pyGrid.h"


void exportFloatGrid();


/// Create a Python wrapper for each supported Grid type.
void
exportFloatGrid()
{
    // Add a module-level list that gives the types of all supported Grid classes.
    py::scope().attr("GridTypes") = py::list();

#if defined(PY_OPENVDB_USE_NUMPY) && !defined(PY_OPENVDB_USE_BOOST_PYTHON_NUMPY)
    // Specify that py::numeric::array should refer to the Python type numpy.ndarray
    // (rather than the older Numeric.array).
    py::numeric::array::set_module_and_type("numpy", "ndarray");
#endif

    pyGrid::exportGrid<FloatGrid>();
#ifdef PY_OPENVDB_WRAP_ALL_GRID_TYPES
    pyGrid::exportGrid<DoubleGrid>();
#endif

    py::def("createLevelSetSphere",
        &pyGrid::createLevelSetSphere<FloatGrid>,
        (py::arg("radius"), py::arg("center")=openvdb::Coord(), py::arg("voxelSize")=1.0,
             py::arg("halfWidth")=openvdb::LEVEL_SET_HALF_WIDTH),
        "createLevelSetSphere(radius, center, voxelSize, halfWidth) -> FloatGrid\n\n"
        "Return a grid containing a narrow-band level set representation\n"
        "of a sphere.");
}
