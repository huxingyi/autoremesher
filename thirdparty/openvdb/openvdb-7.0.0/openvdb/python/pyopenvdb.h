// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file pyopenvdb.h
///
/// @brief Glue functions for access to pyOpenVDB objects from C++ code
/// @details Use these functions in your own Python function implementations
/// to extract an OpenVDB grid from or wrap a grid in a @c PyObject.
/// For example (using Boost.Python),
/// @code
/// #include <openvdb.h>
/// #include <pyopenvdb.h>
/// #include <boost/python.hpp>
///
/// // Implementation of a Python function that processes pyOpenVDB grids
/// boost::python::object
/// processGrid(boost::python::object inObj)
/// {
///     boost::python::object outObj;
///     try {
///         // Extract an OpenVDB grid from the input argument.
///         if (openvdb::GridBase::Ptr grid =
///             pyopenvdb::getGridFromPyObject(inObj))
///         {
///             grid = grid->deepCopyGrid();
///
///             // Process the grid...
///
///             // Wrap the processed grid in a PyObject.
///             outObj = pyopenvdb::getPyObjectFromGrid(grid);
///         }
///     } catch (openvdb::TypeError& e) {
///         PyErr_Format(PyExc_TypeError, e.what());
///         boost::python::throw_error_already_set();
///     }
///     return outObj;
/// }
///
/// BOOST_PYTHON_MODULE(mymodule)
/// {
///     openvdb::initialize();
///
///     // Definition of a Python function that processes pyOpenVDB grids
///     boost::python::def(/*name=*/"processGrid", &processGrid, /*argname=*/"grid");
/// }
/// @endcode
/// Then, from Python,
/// @code
/// import openvdb
/// import mymodule
///
/// grid = openvdb.read('myGrid.vdb', 'MyGrid')
/// grid = mymodule.processGrid(grid)
/// openvdb.write('myProcessedGrid.vdb', [grid])
/// @endcode

#ifndef PYOPENVDB_HAS_BEEN_INCLUDED
#define PYOPENVDB_HAS_BEEN_INCLUDED

#include <boost/python.hpp>
#include "openvdb/Grid.h"


namespace pyopenvdb {

//@{
/// @brief Return a pointer to the OpenVDB grid held by the given Python object.
/// @throw openvdb::TypeError if the Python object is not one of the pyOpenVDB grid types.
///     (See the Python module's GridTypes global variable for the list of supported grid types.)
openvdb::GridBase::Ptr getGridFromPyObject(PyObject*);
openvdb::GridBase::Ptr getGridFromPyObject(const boost::python::object&);
//@}

/// @brief Return a new Python object that holds the given OpenVDB grid.
/// @return @c None if the given grid pointer is null.
/// @throw openvdb::TypeError if the grid is not of a supported type.
///     (See the Python module's GridTypes global variable for the list of supported grid types.)
boost::python::object getPyObjectFromGrid(const openvdb::GridBase::Ptr&);

} // namespace pyopenvdb

#endif // PYOPENVDB_HAS_BEEN_INCLUDED
