# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

OpenVDBHoudiniSetup
-------------------

Wraps the call the FindPackage ( Houdini ) for OpenVDB builds. This
ensures that all dependencies that are included with a Houdini
distribution are configured to load from that installation.

This CMake searches for the HoudiniConfig.cmake module provided by
SideFX to configure the OpenVDB Houdini base and DSO libraries. Users
can provide paths to the location of their Houdini Installation by
setting HOUDINI_ROOT either as an environment variable or by passing it
to CMake. This module also reads the value of $HFS, usually set by
sourcing the Houdini Environment. Note that as long as you provide a
path to your Houdini Installation you do not need to source the
Houdini Environment.

Use this module by invoking include with the form::

  include ( OpenVDBHoudiniSetup )

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Houdini_FOUND``
  True if the system has Houdini installed.
``Houdini_VERSION``
  The version of the Houdini which was found.
``OPENVDB_HOUDINI_ABI``
  The ABI version that Houdini uses for it's own OpenVDB installation.

A variety of variables will also be set from HoudiniConfig.cmake.

Additionally, the following values are set for all dependent OpenVDB
builds, ensuring they link against the correct libraries. This may
overwrite user provided values.

``ZLIB_ROOT``
``ZLIB_LIBRARY``
``OPENEXR_INCLUDEDIR``
``OPENEXR_LIBRARYDIR``
``ILMBASE_INCLUDEDIR``
``ILMBASE_LIBRARYDIR``
``TBB_INCLUDEDIR``
``TBB_LIBRARYDIR``
``BLOSC_INCLUDEDIR``
``BLOSC_LIBRARYDIR``
``JEMALLOC_LIBRARYDIR``

Hints
^^^^^

Instead of explicitly setting the cache variables, the following variables
may be provided to tell this module where to look.

``ENV{HFS}``
  Preferred installation prefix.
``Houdini_ROOT``
  Preferred installation prefix.
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

# Find the Houdini installation and use Houdini's CMake to initialize
# the Houdini lib

cmake_minimum_required(VERSION 3.3)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

set(_FIND_HOUDINI_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_HOUDINI_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

# Set _HOUDINI_ROOT based on a user provided root var. Xxx_ROOT and ENV{Xxx_ROOT}
# are prioritised over the legacy capitalized XXX_ROOT variables for matching
# CMake 3.12 behaviour
# @todo  deprecate -D and ENV HOUDINI_ROOT from CMake 3.12
if(Houdini_ROOT)
  set(_HOUDINI_ROOT ${Houdini_ROOT})
elseif(DEFINED ENV{Houdini_ROOT})
  set(_HOUDINI_ROOT $ENV{Houdini_ROOT})
elseif(HOUDINI_ROOT)
  set(_HOUDINI_ROOT ${HOUDINI_ROOT})
elseif(DEFINED ENV{HOUDINI_ROOT})
  set(_HOUDINI_ROOT $ENV{HOUDINI_ROOT})
endif()

set(_HOUDINI_ROOT_SEARCH_DIR)

if(_HOUDINI_ROOT)
  list(APPEND _HOUDINI_ROOT_SEARCH_DIR ${_HOUDINI_ROOT})
endif()

if(DEFINED ENV{HFS})
  list(APPEND _HOUDINI_ROOT_SEARCH_DIR $ENV{HFS})
endif()

# ------------------------------------------------------------------------
#  Search for Houdini
# ------------------------------------------------------------------------

set(_HOUDINI_CMAKE_PATH_SUFFIXES)

if(APPLE)
  list(APPEND _HOUDINI_CMAKE_PATH_SUFFIXES
    Frameworks/Houdini.framework/Versions/Current/Resources/toolkit/cmake
    Houdini.framework/Versions/Current/Resources/toolkit/cmake
    Versions/Current/Resources/toolkit/cmake
    Current/Resources/toolkit/cmake
    Resources/toolkit/cmake
  )
endif()

list(APPEND _HOUDINI_CMAKE_PATH_SUFFIXES
  toolkit/cmake
  cmake
)

find_package(Houdini
  ${_FIND_HOUDINI_ADDITIONAL_OPTIONS}
  PATHS ${_HOUDINI_ROOT_SEARCH_DIR}
  PATH_SUFFIXES ${_HOUDINI_CMAKE_PATH_SUFFIXES}
  REQUIRED)

# Note that passing MINIMUM_HOUDINI_VERSION into find_package(Houdini) doesn't work
if(NOT Houdini_FOUND)
  message(FATAL_ERROR "Unable to locate Houdini Installation.")
elseif(Houdini_VERSION VERSION_LESS MINIMUM_HOUDINI_VERSION)
  message(FATAL_ERROR "Unsupported Houdini Version ${Houdini_VERSION}. Minimum "
    "supported is ${MINIMUM_HOUDINI_VERSION}."
  )
endif()

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(Houdini
  REQUIRED_VARS _houdini_install_root Houdini_FOUND
  VERSION_VAR Houdini_VERSION
)

# ------------------------------------------------------------------------
#  Add support for older versions of Houdini
# ------------------------------------------------------------------------

if(OPENVDB_FUTURE_DEPRECATION AND FUTURE_MINIMUM_HOUDINI_VERSION)
  if(Houdini_VERSION VERSION_LESS ${FUTURE_MINIMUM_HOUDINI_VERSION})
    message(DEPRECATION "Support for Houdini versions < ${FUTURE_MINIMUM_HOUDINI_VERSION} "
      "is deprecated and will be removed.")
  endif()
endif()

# ------------------------------------------------------------------------
#  Configure imported Houdini target
# ------------------------------------------------------------------------

# Set the relative directory containing Houdini libs and populate an extra list
# of Houdini dependencies for _houdini_create_libraries.

set(_HOUDINI_LIB_DIR)
set(_HOUDINI_EXTRA_LIBRARIES)
set(_HOUDINI_EXTRA_LIBRARY_NAMES)

if(APPLE)
  set(_HOUDINI_LIB_DIR
    Frameworks/Houdini.framework/Versions/Current/Libraries
  )
  list(APPEND _HOUDINI_EXTRA_LIBRARIES
    ${_HOUDINI_LIB_DIR}/libHoudiniRAY.dylib
    ${_HOUDINI_LIB_DIR}/libhboost_regex.dylib
    ${_HOUDINI_LIB_DIR}/libhboost_thread.dylib
  )
else()
  set(_HOUDINI_LIB_DIR dsolib)
  list(APPEND _HOUDINI_EXTRA_LIBRARIES
    ${_HOUDINI_LIB_DIR}/libHoudiniRAY.so
    ${_HOUDINI_LIB_DIR}/libhboost_regex.so
    ${_HOUDINI_LIB_DIR}/libhboost_thread.so
  )
endif()

list(APPEND _HOUDINI_EXTRA_LIBRARY_NAMES
  HoudiniRAY
  hboost_regex
  hboost_thread
)

# Additionally link extra deps

_houdini_create_libraries(
  PATHS ${_HOUDINI_EXTRA_LIBRARIES}
  TARGET_NAMES ${_HOUDINI_EXTRA_LIBRARY_NAMES}
  TYPE SHARED
)

unset(_HOUDINI_EXTRA_LIBRARIES)
unset(_HOUDINI_EXTRA_LIBRARY_NAMES)

# Set Houdini lib and include directories

set(_HOUDINI_INCLUDE_DIR ${_houdini_include_dir})
set(_HOUDINI_LIB_DIR ${_houdini_install_root}/${_HOUDINI_LIB_DIR})

# ------------------------------------------------------------------------
#  Configure dependencies
# ------------------------------------------------------------------------

# Congfigure dependency hints to point to Houdini. Allow for user overriding
# if custom Houdini installations are in use

# ZLIB - FindPackage ( ZLIB) only supports a few path hints. We use
# ZLIB_ROOT to find the zlib includes and explicitly set the path to
# the zlib library

if(NOT ZLIB_ROOT)
  set(ZLIB_ROOT ${_HOUDINI_INCLUDE_DIR})
endif()
if(NOT ZLIB_LIBRARY)
  # Full path to zlib library - FindPackage ( ZLIB)
  find_library(ZLIB_LIBRARY z
    ${_FIND_HOUDINI_ADDITIONAL_OPTIONS}
    PATHS ${_HOUDINI_LIB_DIR}
  )
  if(NOT EXISTS ${ZLIB_LIBRARY})
    message(WARNING "The OpenVDB Houdini CMake setup is unable to locate libz within "
      "the Houdini installation at: ${_HOUDINI_LIB_DIR}. OpenVDB may not build correctly."
    )
  endif()
endif()

# TBB

if(NOT TBB_INCLUDEDIR)
  set(TBB_INCLUDEDIR ${_HOUDINI_INCLUDE_DIR})
endif()
if(NOT TBB_LIBRARYDIR)
  set(TBB_LIBRARYDIR ${_HOUDINI_LIB_DIR})
endif()

# Blosc

if(NOT BLOSC_INCLUDEDIR)
  set(BLOSC_INCLUDEDIR ${_HOUDINI_INCLUDE_DIR})
endif()
if(NOT BLOSC_LIBRARYDIR)
  set(BLOSC_LIBRARYDIR ${_HOUDINI_LIB_DIR})
endif()

# Jemalloc

if(NOT JEMALLOC_LIBRARYDIR)
  set(JEMALLOC_LIBRARYDIR ${_HOUDINI_LIB_DIR})
endif()

# OpenEXR

if(NOT OPENEXR_INCLUDEDIR)
  set(OPENEXR_INCLUDEDIR ${_HOUDINI_INCLUDE_DIR})
endif()
if(NOT OPENEXR_LIBRARYDIR)
  set(OPENEXR_LIBRARYDIR ${_HOUDINI_LIB_DIR})
endif()

# IlmBase

if(NOT ILMBASE_INCLUDEDIR)
  set(ILMBASE_INCLUDEDIR ${_HOUDINI_INCLUDE_DIR})
endif()
if(NOT ILMBASE_LIBRARYDIR)
  set(ILMBASE_LIBRARYDIR ${_HOUDINI_LIB_DIR})
endif()

# Boost - currently must be provided as VDB is not fully configured to
# use Houdini's namespaced hboost

unset(_HOUDINI_INCLUDE_DIR)
unset(_HOUDINI_LIB_DIR)

# Versions of Houdini >= 17.5 have some namespaced libraries (IlmBase/OpenEXR).
# Add the required suffix as part of the cmake lib suffix searches

if(APPLE)
  list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES "_sidefx.dylib")
elseif(UNIX)
  list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES "_sidefx.so")
endif()

# ------------------------------------------------------------------------
#  Configure OpenVDB ABI
# ------------------------------------------------------------------------

# Explicitly configure the OpenVDB ABI version depending on the Houdini
# version.

if(Houdini_VERSION VERSION_LESS 17)
  set(OPENVDB_HOUDINI_ABI 4)
elseif(Houdini_VERSION VERSION_LESS 18)
  set(OPENVDB_HOUDINI_ABI 5)
else()
  # Anticipated ABI version for H18
  set(OPENVDB_HOUDINI_ABI 6)
endif()

# ------------------------------------------------------------------------
#  Configure GCC CXX11 ABI
# ------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if((CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 5.1) OR
     (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1))
    message(STATUS "GCC >= 5.1 detected. Configuring GCC CXX11 ABI for Houdini compatibility...")

    execute_process(COMMAND echo "#include <string>"
      COMMAND ${CMAKE_CXX_COMPILER} "-x" "c++" "-E" "-dM" "-"
      COMMAND grep "-F" "_GLIBCXX_USE_CXX11_ABI"
      TIMEOUT 10
      RESULT_VARIABLE QUERIED_GCC_CXX11_ABI_SUCCESS
      OUTPUT_VARIABLE _GCC_CXX11_ABI)

    set(GLIBCXX_USE_CXX11_ABI "UNKNOWN")

    if(NOT QUERIED_GCC_CXX11_ABI_SUCCESS)
      string(FIND ${_GCC_CXX11_ABI} "_GLIBCXX_USE_CXX11_ABI 0" GCC_OLD_CXX11_ABI)
      string(FIND ${_GCC_CXX11_ABI} "_GLIBCXX_USE_CXX11_ABI 1" GCC_NEW_CXX11_ABI)
      if(NOT (${GCC_OLD_CXX11_ABI} EQUAL -1))
        set(GLIBCXX_USE_CXX11_ABI 0)
      endif()
      if(NOT (${GCC_NEW_CXX11_ABI} EQUAL -1))
        set(GLIBCXX_USE_CXX11_ABI 1)
      endif()
    endif()

    # Try and query the Houdini CXX11 ABI. Allow it to be provided by users to
    # override this logic should Houdini's CMake ever change

    if(NOT DEFINED HOUDINI_CXX11_ABI)
      get_target_property(houdini_interface_compile_options
        Houdini INTERFACE_COMPILE_OPTIONS)
      set(HOUDINI_CXX11_ABI "UNKNOWN")
      if("-D_GLIBCXX_USE_CXX11_ABI=0" IN_LIST houdini_interface_compile_options)
        set(HOUDINI_CXX11_ABI 0)
      elseif("-D_GLIBCXX_USE_CXX11_ABI=1" IN_LIST houdini_interface_compile_options)
        set(HOUDINI_CXX11_ABI 1)
      endif()
    endif()

    message(STATUS "  GCC CXX11 ABI     : ${GLIBCXX_USE_CXX11_ABI}")
    message(STATUS "  Houdini CXX11 ABI : ${HOUDINI_CXX11_ABI}")

    if(${HOUDINI_CXX11_ABI} STREQUAL "UNKNOWN")
      message(WARNING "Unable to determine Houdini CXX11 ABI. Assuming newer ABI "
        "has been used.")
      set(HOUDINI_CXX11_ABI 1)
    endif()

    if(${GLIBCXX_USE_CXX11_ABI} EQUAL ${HOUDINI_CXX11_ABI})
      message(STATUS "  Current CXX11 ABI matches Houdini configuration "
        "(_GLIBCXX_USE_CXX11_ABI=${HOUDINI_CXX11_ABI}).")
    else()
      message(WARNING "A potential mismatch was detected between the CXX11 ABI "
        "of GCC and Houdini. The following ABI configuration will be used: "
        "-D_GLIBCXX_USE_CXX11_ABI=${HOUDINI_CXX11_ABI}. See: "
        "https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html and "
        "https://vfxplatform.com/#footnote-gcc6 for more information.")
    endif()

    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=${HOUDINI_CXX11_ABI})
  endif()
endif()
