# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[

  CMake Configuration for OpenVDB Python bindings

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)
project(OpenVDBPython)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()
include(GNUInstallDirs)

###### OpenVDB Python Options

option(USE_NUMPY [=[
Build the python library with numpy support. Currently requires CMake 3.14.]=] OFF)
option(OPENVDB_PYTHON_WRAP_ALL_GRID_TYPES [=[
Expose (almost) all of the grid types in the python module. Otherwise, only FloatGrid, BoolGrid and
Vec3SGrid will be exposed (see, e.g., exportIntGrid() in python/pyIntGrid.cc). Compiling the Python
module with this ON can be very memory-intensive.]=] OFF)

#########################################################################

message(STATUS "----------------------------------------------------")
message(STATUS "------------ Configuring OpenVDBPython -------------")
message(STATUS "----------------------------------------------------")

##########################################################################

# Collect and configure lib dependencies

if(NOT OPENVDB_BUILD_CORE)
  set(OPENVDB_LIB OpenVDB::openvdb)
else()
  set(OPENVDB_LIB openvdb)
endif()

set(OPENVDB_PYTHON_DEPS)
set(OPENVDB_PYTHON_INCLUDES)

# Small function which mimics basic output (bar components) of
# FindPackageHandleStandardArgs. This is required as we want to ensure
# the minimum python version is MINIMUM_PYTHON_VERSION - however this cannot
# be provided to find_package(Python) with differing major versions. e.g.
# calls to find_package(Python 2.7) fails if python3 is found on the system.
function(OPENVDB_CHECK_PYTHON_VERSION _PY_VERSION _PY_PATH)
  if(_PY_VERSION VERSION_LESS MINIMUM_PYTHON_VERSION)
    message(FATAL_ERROR "Could NOT find Python: Found unsuitable version \"${_PY_VERSION}\""
      "but required is at least \"${MINIMUM_PYTHON_VERSION}\" (found ${_PY_PATH})"
    )
  else()
    message(STATUS "Found Python: ${_PY_PATH}) (found suitable version \"${_PY_VERSION}\", "
      "minimum required is \"${MINIMUM_PYTHON_VERSION}\")"
    )
  endif()
endfunction()

# Configure Python and Numpy. Note that:
#  - find_package(Python Interpreter Development) requires CMake >= 3.12
#  - find_package(Python NumPy) requires CMake >= 3.14
# To ensure consistent versions between components Interpreter, Compiler,
# Development and NumPy, specify all components at the same time when using
# FindPython
# Note that the Interpreter component is only required for the python test
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  # CMake < 3.12
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)

  # Alias variables to newer names
  set(Python_VERSION ${PYTHON_VERSION_STRING})
  set(Python_VERSION_MAJOR ${PYTHON_VERSION_MAJOR})
  set(Python_VERSION_MINOR ${PYTHON_VERSION_MINOR})
  set(Python_EXECUTABLE ${PYTHON_EXECUTABLE})
  set(OPENVDB_PYTHON_DEPS ${PYTHON_LIBRARIES})
  get_filename_component(Python_LIBRARY_DIRS ${OPENVDB_PYTHON_DEPS} DIRECTORY)

  list(APPEND OPENVDB_PYTHON_INCLUDES ${PYTHON_INCLUDE_DIR})

  OPENVDB_CHECK_PYTHON_VERSION(${Python_VERSION} ${Python_EXECUTABLE})

  if(USE_NUMPY)
    # Note: This uses our custom backport in cmake/backports
    find_package(NumPy ${MINIMUM_NUMPY_VERSION} REQUIRED)
    list(APPEND OPENVDB_PYTHON_DEPS Python::NumPy)
  endif()
elseif(${CMAKE_VERSION} VERSION_LESS 3.14)
  # CMake < 3.14
  find_package(Python QUIET COMPONENTS Interpreter Development)
  OPENVDB_CHECK_PYTHON_VERSION(${Python_VERSION} ${Python_EXECUTABLE})
  list(APPEND OPENVDB_PYTHON_DEPS Python::Python)

  if(USE_NUMPY)
    # Note: This uses our custom backport in cmake/backports
    find_package(NumPy ${MINIMUM_NUMPY_VERSION} REQUIRED)
    list(APPEND OPENVDB_PYTHON_DEPS Python::NumPy)
  endif()
else()
  # CMake >= 3.14
  if(USE_NUMPY)
    find_package(Python QUIET REQUIRED COMPONENTS Interpreter Development NumPy)
  else()
    find_package(Python QUIET REQUIRED COMPONENTS Interpreter Development)
  endif()

  OPENVDB_CHECK_PYTHON_VERSION(${Python_VERSION} ${Python_EXECUTABLE})
  list(APPEND OPENVDB_PYTHON_DEPS Python::Python)

  # Check NumPy version
  if(USE_NUMPY AND (Python_NumPy_VERSION VERSION_LESS MINIMUM_NUMPY_VERSION))
      message(FATAL_ERROR "Could NOT find NumPy (Required is at least version "
        "\"${MINIMUM_NUMPY_VERSION}\")"
      )
  elseif(USE_NUMPY)
    message(STATUS "Found NumPy: ${Python_NumPy_INCLUDE_DIRS} (found suitable "
      "version \"${Python_NumPy_VERSION}\", minimum required is "
      "\"${MINIMUM_NUMPY_VERSION}\")"
    )
    list(APPEND OPENVDB_PYTHON_DEPS Python::NumPy)
  endif()
endif()

if(USE_NUMPY)
  if(OPENVDB_FUTURE_DEPRECATION AND FUTURE_MINIMUM_NUMPY_VERSION)
    if(Python_NumPy_VERSION VERSION_LESS FUTURE_MINIMUM_NUMPY_VERSION)
      message(DEPRECATION "Support for NumPy versions < ${FUTURE_MINIMUM_NUMPY_VERSION} "
        "is deprecated and will be removed.")
    endif()
  endif()
endif()

if(TARGET openvdb_shared)
  # @note  Both of these must be set for Boost 1.70 (VFX2020) to link against
  #        boost shared libraries (more specifically libraries built with -fPIC).
  #        http://boost.2283326.n4.nabble.com/CMake-config-scripts-broken-in-1-70-td4708957.html
  #        https://github.com/boostorg/boost_install/commit/160c7cb2b2c720e74463865ef0454d4c4cd9ae7c
  set(BUILD_SHARED_LIBS ON)
  set(Boost_USE_STATIC_LIBS OFF)
endif()

# Boost python handling. Try to find the following named boost python libraries:
# - boost_python{Python_VERSION_MAJOR}${Python_VERSION_MINOR}
# - boost_python{Python_VERSION_MAJOR}
# - boost_python
# Prioritize the version suffixed library, failing if none exist.

find_package(Boost ${MINIMUM_BOOST_VERSION}
  QUIET COMPONENTS python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}
)
find_package(Boost ${MINIMUM_BOOST_VERSION}
  QUIET COMPONENTS python${Python_VERSION_MAJOR}
)
find_package(Boost ${MINIMUM_BOOST_VERSION}
  QUIET COMPONENTS python
)

if(TARGET Boost::python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
  message(STATUS "Found boost_python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
  list(APPEND OPENVDB_PYTHON_DEPS Boost::python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
elseif(TARGET Boost::python${Python_VERSION_MAJOR})
  message(STATUS "Found boost_python${Python_VERSION_MAJOR}")
  list(APPEND OPENVDB_PYTHON_DEPS Boost::python${Python_VERSION_MAJOR})
elseif(TARGET Boost::python)
  message(STATUS "Found non-suffixed boost_python, assuming to be python version "
    "\"${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}\" compatible"
  )
  list(APPEND OPENVDB_PYTHON_DEPS Boost::python)
else()
  message(FATAL_ERROR "Unable to find boost_python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}, "
    "boost_python${Python_VERSION_MAJOR} or boost_python."
  )
endif()

# If boost >= 1.65, find the required numpy boost component and link that in.
# Use the same system as above, first trying without the suffix, then with.

if(USE_NUMPY AND (
  (${Boost_VERSION} VERSION_EQUAL 1.65) OR
  (${Boost_VERSION} VERSION_GREATER 1.65)))

  find_package(Boost ${MINIMUM_BOOST_VERSION}
    QUIET COMPONENTS numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR}
  )
  find_package(Boost ${MINIMUM_BOOST_VERSION}
    QUIET COMPONENTS numpy${Python_VERSION_MAJOR}
  )
  find_package(Boost ${MINIMUM_BOOST_VERSION}
    QUIET COMPONENTS numpy
  )

  if(TARGET Boost::numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
    message(STATUS "Found boost_numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
    list(APPEND OPENVDB_PYTHON_DEPS Boost::numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
  elseif(TARGET Boost::numpy${Python_VERSION_MAJOR})
    message(STATUS "Found boost_numpy${Python_VERSION_MAJOR}")
    list(APPEND OPENVDB_PYTHON_DEPS Boost::numpy${Python_VERSION_MAJOR})
  elseif(TARGET Boost::numpy)
    message(STATUS "Found non-suffixed boost_numpy, assuming to be python version "
      "\"${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}\" compatible"
    )
    list(APPEND OPENVDB_PYTHON_DEPS Boost::numpy)
  else()
    message(FATAL_ERROR "Unable to find boost_numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR}, "
      "boost_numpy${Python_VERSION_MAJOR} or boost_numpy."
    )
  endif()
endif()

set(OPENVDB_PYTHON_DEPENDENT_LIBS
  ${OPENVDB_LIB}
  ${OPENVDB_PYTHON_DEPS}
)

##########################################################################

set(OPENVDB_PYTHON_MODULE_SOURCE_FILES
  pyFloatGrid.cc
  pyIntGrid.cc
  pyMetadata.cc
  pyPointGrid.cc
  pyOpenVDBModule.cc
  pyPointGrid.cc
  pyTransform.cc
  pyVec3Grid.cc
)

if(NOT DEFINED PYOPENVDB_INSTALL_DIRECTORY)
  set(PYOPENVDB_INSTALL_DIRECTORY
    ${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}
    CACHE STRING "The directory to install the pyopenvdb.so module."
  )
endif()

add_library(pyopenvdb SHARED
  ${OPENVDB_PYTHON_MODULE_SOURCE_FILES}
)

target_include_directories(pyopenvdb
  SYSTEM PUBLIC ${OPENVDB_PYTHON_INCLUDES}
)

if(OPENVDB_PYTHON_WRAP_ALL_GRID_TYPES)
  target_compile_definitions(pyopenvdb PRIVATE "-DPY_OPENVDB_WRAP_ALL_GRID_TYPES")
endif()

if(USE_NUMPY)
  target_compile_definitions(pyopenvdb PUBLIC "-DPY_OPENVDB_USE_NUMPY")
endif()

target_link_libraries(pyopenvdb
  ${OPENVDB_PYTHON_DEPENDENT_LIBS}
)

set_target_properties(pyopenvdb PROPERTIES
  PREFIX ""  # no 'lib' prefix
)

if(UNIX)
  # must be .so (not .dylib)
  set_target_properties(pyopenvdb PROPERTIES
    SUFFIX ".so"
  )
endif()

if(OPENVDB_ENABLE_RPATH)
  # @todo There is probably a better way to do this for imported targets
  set(RPATHS "")
  list(APPEND RPATHS
    ${Boost_LIBRARY_DIRS}
    ${IlmBase_LIBRARY_DIRS}
    ${Log4cplus_LIBRARY_DIRS}
    ${Blosc_LIBRARY_DIRS}
    ${Tbb_LIBRARY_DIRS}
    ${Python_LIBRARY_DIRS}
  )
  if(OPENVDB_BUILD_CORE)
    list(APPEND RPATHS ${CMAKE_INSTALL_PREFIX}/lib)
  else()
    list(APPEND RPATHS ${OpenVDB_LIBRARY_DIRS})
  endif()

  list(REMOVE_DUPLICATES RPATHS)
  set_target_properties(pyopenvdb
    PROPERTIES INSTALL_RPATH "${RPATHS}"
  )
  unset(RPATHS)
endif()

set(PYTHON_PUBLIC_INCLUDE_NAMES
  pyopenvdb.h
)

install(TARGETS
  pyopenvdb
  DESTINATION
  ${PYOPENVDB_INSTALL_DIRECTORY}
)

install(FILES ${PYTHON_PUBLIC_INCLUDE_NAMES} DESTINATION ${OPENVDB_INSTALL_INCLUDEDIR}/python)

# pytest

add_test(pytest ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/TestOpenVDB.py -v)
set_tests_properties(pytest PROPERTIES ENVIRONMENT "PYTHONPATH=$ENV{PYTHONPATH}:${CMAKE_CURRENT_BINARY_DIR}")


