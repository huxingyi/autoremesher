# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

FindOpenVDB
-----------

Find OpenVDB include dirs, libraries and settings

Use this module by invoking find_package with the form::

  find_package(OpenVDB
    [version] [EXACT]      # Minimum or EXACT version
    [REQUIRED]             # Fail with error if OpenVDB is not found
    [COMPONENTS <libs>...] # OpenVDB libraries by their canonical name
                           # e.g. "openvdb" for "libopenvdb"
    )

IMPORTED Targets
^^^^^^^^^^^^^^^^

``OpenVDB::openvdb``
  The core openvdb library target.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpenVDB_FOUND``
  True if the system has the OpenVDB library.
``OpenVDB_VERSION``
  The version of the OpenVDB library which was found.
``OpenVDB_INCLUDE_DIRS``
  Include directories needed to use OpenVDB.
``OpenVDB_LIBRARIES``
  Libraries needed to link to OpenVDB.
``OpenVDB_LIBRARY_DIRS``
  OpenVDB library directories.
``OpenVDB_DEFINITIONS``
  Definitions to use when compiling code that uses OpenVDB.
``OpenVDB_{COMPONENT}_FOUND``
  True if the system has the named OpenVDB component.
``OpenVDB_USES_BLOSC``
  True if the OpenVDB Library has been built with blosc support
``OpenVDB_USES_LOG4CPLUS``
  True if the OpenVDB Library has been built with log4cplus support
``OpenVDB_USES_EXR``
  True if the OpenVDB Library has been built with openexr support
``OpenVDB_ABI``
  Set if this module was able to determine the ABI number the located
  OpenVDB Library was built against. Unset otherwise.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``OpenVDB_INCLUDE_DIR``
  The directory containing ``openvdb/version.h``.
``OpenVDB_{COMPONENT}_LIBRARY``
  Individual component libraries for OpenVDB

Hints
^^^^^

Instead of explicitly setting the cache variables, the following variables
may be provided to tell this module where to look.

``OpenVDB_ROOT``
  Preferred installation prefix.
``OPENVDB_INCLUDEDIR``
  Preferred include directory e.g. <prefix>/include
``OPENVDB_LIBRARYDIR``
  Preferred library directory e.g. <prefix>/lib
``SYSTEM_LIBRARY_PATHS``
  Global list of library paths intended to be searched by and find_xxx call
``OPENVDB_USE_STATIC_LIBS``
  Only search for static openvdb libraries
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# Include utility functions for version information
include(${CMAKE_CURRENT_LIST_DIR}/OpenVDBUtils.cmake)

mark_as_advanced(
  OpenVDB_INCLUDE_DIR
  OpenVDB_LIBRARY
)

set(_FIND_OPENVDB_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_OPENVDB_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

set(_OPENVDB_COMPONENT_LIST
  openvdb
)

if(OpenVDB_FIND_COMPONENTS)
  set(OPENVDB_COMPONENTS_PROVIDED TRUE)
  set(_IGNORED_COMPONENTS "")
  foreach(COMPONENT ${OpenVDB_FIND_COMPONENTS})
    if(NOT ${COMPONENT} IN_LIST _OPENVDB_COMPONENT_LIST)
      list(APPEND _IGNORED_COMPONENTS ${COMPONENT})
    endif()
  endforeach()

  if(_IGNORED_COMPONENTS)
    message(STATUS "Ignoring unknown components of OpenVDB:")
    foreach(COMPONENT ${_IGNORED_COMPONENTS})
      message(STATUS "  ${COMPONENT}")
    endforeach()
    list(REMOVE_ITEM OpenVDB_FIND_COMPONENTS ${_IGNORED_COMPONENTS})
  endif()
else()
  set(OPENVDB_COMPONENTS_PROVIDED FALSE)
  set(OpenVDB_FIND_COMPONENTS ${_OPENVDB_COMPONENT_LIST})
endif()

# Set _OPENVDB_ROOT based on a user provided root var. Xxx_ROOT and ENV{Xxx_ROOT}
# are prioritised over the legacy capitalized XXX_ROOT variables for matching
# CMake 3.12 behaviour
# @todo  deprecate -D and ENV OPENVDB_ROOT from CMake 3.12
if(OpenVDB_ROOT)
  set(_OPENVDB_ROOT ${OpenVDB_ROOT})
elseif(DEFINED ENV{OpenVDB_ROOT})
  set(_OPENVDB_ROOT $ENV{OpenVDB_ROOT})
elseif(OPENVDB_ROOT)
  set(_OPENVDB_ROOT ${OPENVDB_ROOT})
elseif(DEFINED ENV{OPENVDB_ROOT})
  set(_OPENVDB_ROOT $ENV{OPENVDB_ROOT})
endif()

# Additionally try and use pkconfig to find OpenVDB

if(NOT DEFINED PKG_CONFIG_FOUND)
  find_package(PkgConfig)
endif()
pkg_check_modules(PC_OpenVDB QUIET OpenVDB)

# This CMake module supports being called from external packages AND from
# within the OpenVDB repository for building openvdb components with the
# core library build disabled. Determine where we are being called from:
#
# (repo structure = <root>/cmake/FindOpenVDB.cmake)
# (inst structure = <root>/lib/cmake/OpenVDB/FindOpenVDB.cmake)

get_filename_component(_DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

if(${_DIR_NAME} STREQUAL "cmake")
  # Called from root repo for openvdb components
elseif(${_DIR_NAME} STREQUAL "OpenVDB")
  # Set the install variable to track directories if this is being called from
  # an installed location and from another package. The expected installation
  # directory structure is:
  #  <root>/lib/cmake/OpenVDB/FindOpenVDB.cmake
  #  <root>/include
  #  <root>/bin
  get_filename_component(_IMPORT_PREFIX ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
  get_filename_component(_IMPORT_PREFIX ${_IMPORT_PREFIX} DIRECTORY)
  get_filename_component(_IMPORT_PREFIX ${_IMPORT_PREFIX} DIRECTORY)
  set(_OPENVDB_INSTALL ${_IMPORT_PREFIX})
  list(APPEND _OPENVDB_ROOT ${_OPENVDB_INSTALL})
endif()

unset(_DIR_NAME)
unset(_IMPORT_PREFIX)

# ------------------------------------------------------------------------
#  Search for OpenVDB include DIR
# ------------------------------------------------------------------------

set(_OPENVDB_INCLUDE_SEARCH_DIRS "")
list(APPEND _OPENVDB_INCLUDE_SEARCH_DIRS
  ${OPENVDB_INCLUDEDIR}
  ${_OPENVDB_ROOT}
  ${PC_OpenVDB_INCLUDE_DIRS}
  ${SYSTEM_LIBRARY_PATHS}
)

# Look for a standard OpenVDB header file.
find_path(OpenVDB_INCLUDE_DIR openvdb/version.h
  ${_FIND_OPENVDB_ADDITIONAL_OPTIONS}
  PATHS ${_OPENVDB_INCLUDE_SEARCH_DIRS}
  PATH_SUFFIXES include
)

OPENVDB_VERSION_FROM_HEADER("${OpenVDB_INCLUDE_DIR}/openvdb/version.h"
  VERSION OpenVDB_VERSION
  MAJOR   OpenVDB_MAJOR_VERSION
  MINOR   OpenVDB_MINOR_VERSION
  PATCH   OpenVDB_PATCH_VERSION
)

# ------------------------------------------------------------------------
#  Search for OPENVDB lib DIR
# ------------------------------------------------------------------------

set(_OPENVDB_LIBRARYDIR_SEARCH_DIRS "")

# Append to _OPENVDB_LIBRARYDIR_SEARCH_DIRS in priority order

list(APPEND _OPENVDB_LIBRARYDIR_SEARCH_DIRS
  ${OPENVDB_LIBRARYDIR}
  ${_OPENVDB_ROOT}
  ${PC_OpenVDB_LIBRARY_DIRS}
  ${SYSTEM_LIBRARY_PATHS}
)

# Build suffix directories

set(OPENVDB_PATH_SUFFIXES
  lib64
  lib
)

# Static library setup
if(UNIX AND OPENVDB_USE_STATIC_LIBS)
  set(_OPENVDB_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

set(OpenVDB_LIB_COMPONENTS "")

foreach(COMPONENT ${OpenVDB_FIND_COMPONENTS})
  set(LIB_NAME ${COMPONENT})
  find_library(OpenVDB_${COMPONENT}_LIBRARY ${LIB_NAME}
    ${_FIND_OPENVDB_ADDITIONAL_OPTIONS}
    PATHS ${_OPENVDB_LIBRARYDIR_SEARCH_DIRS}
    PATH_SUFFIXES ${OPENVDB_PATH_SUFFIXES}
  )
  list(APPEND OpenVDB_LIB_COMPONENTS ${OpenVDB_${COMPONENT}_LIBRARY})

  if(OpenVDB_${COMPONENT}_LIBRARY)
    set(OpenVDB_${COMPONENT}_FOUND TRUE)
  else()
    set(OpenVDB_${COMPONENT}_FOUND FALSE)
  endif()
endforeach()

if(UNIX AND OPENVDB_USE_STATIC_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_OPENVDB_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
  unset(_OPENVDB_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)
endif()

# ------------------------------------------------------------------------
#  Cache and set OPENVDB_FOUND
# ------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVDB
  FOUND_VAR OpenVDB_FOUND
  REQUIRED_VARS
    OpenVDB_INCLUDE_DIR
    OpenVDB_LIB_COMPONENTS
  VERSION_VAR OpenVDB_VERSION
  HANDLE_COMPONENTS
)

# ------------------------------------------------------------------------
#  Determine ABI number
# ------------------------------------------------------------------------

# Set the ABI number the library was built against. Uses vdb_print

if(_OPENVDB_INSTALL)
  OPENVDB_ABI_VERSION_FROM_PRINT(
    "${_OPENVDB_INSTALL}/bin/vdb_print"
    ABI OpenVDB_ABI
  )
else()
  # Try and find vdb_print from the include path
  OPENVDB_ABI_VERSION_FROM_PRINT(
    "${OpenVDB_INCLUDE_DIR}/../bin/vdb_print"
    ABI OpenVDB_ABI
  )
endif()

if(NOT OpenVDB_FIND_QUIET)
  if(NOT OpenVDB_ABI)
    message(WARNING "Unable to determine OpenVDB ABI version from OpenVDB "
      "installation. The library major version \"${OpenVDB_MAJOR_VERSION}\" "
      "will be inferred. If this is not correct, use "
      "add_definitions(-DOPENVDB_ABI_VERSION_NUMBER=N)"
    )
  else()
    message(STATUS "OpenVDB ABI Version: ${OpenVDB_ABI}")
  endif()
endif()

# ------------------------------------------------------------------------
#  Handle OpenVDB dependencies
# ------------------------------------------------------------------------

# Add standard dependencies

find_package(IlmBase REQUIRED COMPONENTS Half)
find_package(TBB REQUIRED COMPONENTS tbb)
find_package(ZLIB REQUIRED)

if(NOT OPENVDB_USE_STATIC_LIBS)
  # @note  Both of these must be set for Boost 1.70 (VFX2020) to link against
  #        boost shared libraries (more specifically libraries built with -fPIC).
  #        http://boost.2283326.n4.nabble.com/CMake-config-scripts-broken-in-1-70-td4708957.html
  #        https://github.com/boostorg/boost_install/commit/160c7cb2b2c720e74463865ef0454d4c4cd9ae7c
  set(BUILD_SHARED_LIBS ON)
  set(Boost_USE_STATIC_LIBS OFF)
endif()

find_package(Boost REQUIRED COMPONENTS iostreams system)

# Use GetPrerequisites to see which libraries this OpenVDB lib has linked to
# which we can query for optional deps. This basically runs ldd/otoll/objdump
# etc to track deps. We could use a vdb_config binary tools here to improve
# this process

include(GetPrerequisites)

set(_EXCLUDE_SYSTEM_PREREQUISITES 1)
set(_RECURSE_PREREQUISITES 0)
set(_OPENVDB_PREREQUISITE_LIST)

get_prerequisites(${OpenVDB_openvdb_LIBRARY}
  _OPENVDB_PREREQUISITE_LIST
  ${_EXCLUDE_SYSTEM_PREREQUISITES}
  ${_RECURSE_PREREQUISITES}
  ""
  "${SYSTEM_LIBRARY_PATHS}"
)

unset(_EXCLUDE_SYSTEM_PREREQUISITES)
unset(_RECURSE_PREREQUISITES)

# As the way we resolve optional libraries relies on library file names, use
# the configuration options from the main CMakeLists.txt to allow users
# to manually identify the requirements of OpenVDB builds if they know them.

set(OpenVDB_USES_BLOSC ${USE_BLOSC})
set(OpenVDB_USES_LOG4CPLUS ${USE_LOG4CPLUS})
set(OpenVDB_USES_EXR ${USE_EXR})

# Search for optional dependencies

foreach(PREREQUISITE ${_OPENVDB_PREREQUISITE_LIST})
  set(_HAS_DEP)
  get_filename_component(PREREQUISITE ${PREREQUISITE} NAME)

  string(FIND ${PREREQUISITE} "blosc" _HAS_DEP)
  if(NOT ${_HAS_DEP} EQUAL -1)
    set(OpenVDB_USES_BLOSC ON)
  endif()

  string(FIND ${PREREQUISITE} "log4cplus" _HAS_DEP)
  if(NOT ${_HAS_DEP} EQUAL -1)
    set(OpenVDB_USES_LOG4CPLUS ON)
  endif()

  string(FIND ${PREREQUISITE} "IlmImf" _HAS_DEP)
  if(NOT ${_HAS_DEP} EQUAL -1)
    set(OpenVDB_USES_EXR ON)
  endif()
endforeach()

unset(_OPENVDB_PREREQUISITE_LIST)
unset(_HAS_DEP)

if(OpenVDB_USES_BLOSC)
  find_package(Blosc REQUIRED)
endif()

if(OpenVDB_USES_LOG4CPLUS)
  find_package(Log4cplus REQUIRED)
endif()

if(OpenVDB_USES_EXR)
  find_package(IlmBase REQUIRED)
  find_package(OpenEXR REQUIRED)
endif()

if(UNIX)
  find_package(Threads REQUIRED)
endif()

# Set deps. Note that the order here is important. If we're building against
# Houdini 17.5 we must include OpenEXR and IlmBase deps first to ensure the
# users chosen namespaced headers are correctly prioritized. Otherwise other
# include paths from shared installs (including houdini) may pull in the wrong
# headers

set(_OPENVDB_VISIBLE_DEPENDENCIES
  Boost::iostreams
  Boost::system
  IlmBase::Half
)

set(_OPENVDB_DEFINITIONS)
if(OpenVDB_ABI)
  list(APPEND _OPENVDB_DEFINITIONS "-DOPENVDB_ABI_VERSION_NUMBER=${OpenVDB_ABI}")
endif()

if(OpenVDB_USES_EXR)
  list(APPEND _OPENVDB_VISIBLE_DEPENDENCIES
    IlmBase::IlmThread
    IlmBase::Iex
    IlmBase::Imath
    OpenEXR::IlmImf
  )
  list(APPEND _OPENVDB_DEFINITIONS "-DOPENVDB_TOOLS_RAYTRACER_USE_EXR")
endif()

if(OpenVDB_USES_LOG4CPLUS)
  list(APPEND _OPENVDB_VISIBLE_DEPENDENCIES Log4cplus::log4cplus)
  list(APPEND _OPENVDB_DEFINITIONS "-DOPENVDB_USE_LOG4CPLUS")
endif()

list(APPEND _OPENVDB_VISIBLE_DEPENDENCIES
  TBB::tbb
)
if(UNIX)
  list(APPEND _OPENVDB_VISIBLE_DEPENDENCIES
    Threads::Threads
  )
endif()

set(_OPENVDB_HIDDEN_DEPENDENCIES)

if(OpenVDB_USES_BLOSC)
  list(APPEND _OPENVDB_HIDDEN_DEPENDENCIES Blosc::blosc)
endif()

list(APPEND _OPENVDB_HIDDEN_DEPENDENCIES ZLIB::ZLIB)

# ------------------------------------------------------------------------
#  Configure imported target
# ------------------------------------------------------------------------

set(OpenVDB_LIBRARIES
  ${OpenVDB_LIB_COMPONENTS}
)
set(OpenVDB_INCLUDE_DIRS ${OpenVDB_INCLUDE_DIR})

set(OpenVDB_DEFINITIONS)
list(APPEND OpenVDB_DEFINITIONS "${PC_OpenVDB_CFLAGS_OTHER}")
list(APPEND OpenVDB_DEFINITIONS "${_OPENVDB_DEFINITIONS}")
list(REMOVE_DUPLICATES OpenVDB_DEFINITIONS)

set(OpenVDB_LIBRARY_DIRS "")
foreach(LIB ${OpenVDB_LIB_COMPONENTS})
  get_filename_component(_OPENVDB_LIBDIR ${LIB} DIRECTORY)
  list(APPEND OpenVDB_LIBRARY_DIRS ${_OPENVDB_LIBDIR})
endforeach()
list(REMOVE_DUPLICATES OpenVDB_LIBRARY_DIRS)

foreach(COMPONENT ${OpenVDB_FIND_COMPONENTS})
  if(NOT TARGET OpenVDB::${COMPONENT})
    add_library(OpenVDB::${COMPONENT} UNKNOWN IMPORTED)
    set_target_properties(OpenVDB::${COMPONENT} PROPERTIES
      IMPORTED_LOCATION "${OpenVDB_${COMPONENT}_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${OpenVDB_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${OpenVDB_INCLUDE_DIR}"
      IMPORTED_LINK_DEPENDENT_LIBRARIES "${_OPENVDB_HIDDEN_DEPENDENCIES}" # non visible deps
      INTERFACE_LINK_LIBRARIES "${_OPENVDB_VISIBLE_DEPENDENCIES}" # visible deps (headers)
      INTERFACE_COMPILE_FEATURES cxx_std_14
   )
  endif()
endforeach()

unset(_OPENVDB_DEFINITIONS)
unset(_OPENVDB_VISIBLE_DEPENDENCIES)
unset(_OPENVDB_HIDDEN_DEPENDENCIES)
