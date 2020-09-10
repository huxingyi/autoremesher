# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

OpenVDBMayaSetup
-------------------

Finds the Maya installation and sets up dependencies for OpenVDB builds.
This ensures that all dependencies that are included with a Maya
distribution are configured to load from that installation.

Use this module by invoking include with the form::

  include ( OpenVDBMayaSetup )

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Maya_FOUND``
  True if the system has Maya installed.
``Maya_VERSION``
  The version of the Maya which was found.

Additionally, the following values are set for all dependent OpenVDB
builds, ensuring they link against the correct libraries. This may
overwrite user provided values.

``TBB_INCLUDEDIR``
``TBB_LIBRARYDIR``

Hints
^^^^^

Instead of explicitly setting the cache variables, the following
variables may be provided to tell this module where to look.

``Maya_ROOT``
  Preferred installation prefix.
``ENV{Maya_ROOT}``
  Preferred installation prefix.
``ENV{MAYA_LOCATION}``
  Preferred installation prefix.
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

# Find the Maya installation and use Maya's CMake to initialize
# the Maya lib

cmake_minimum_required(VERSION 3.3)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

set(_FIND_MAYA_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_MAYA_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

# Set _MAYA_ROOT based on a user provided root var. Xxx_ROOT and ENV{Xxx_ROOT}
# are prioritised over the legacy capitalized XXX_ROOT variables for matching
# CMake 3.12 behaviour
# @todo  deprecate -D and ENV MAYA_ROOT from CMake 3.12
if(Maya_ROOT)
  set(_MAYA_ROOT ${Maya_ROOT})
elseif(DEFINED ENV{Maya_ROOT})
  set(_MAYA_ROOT $ENV{Maya_ROOT})
elseif(MAYA_ROOT)
  set(_MAYA_ROOT ${MAYA_ROOT})
elseif(DEFINED ENV{MAYA_ROOT})
  set(_MAYA_ROOT $ENV{MAYA_ROOT})
endif()

set(_MAYA_ROOT_SEARCH_DIR)
if(_MAYA_ROOT)
  list(APPEND _MAYA_ROOT_SEARCH_DIR ${_MAYA_ROOT})
endif()

# @todo deprecate MAYA_LOCATION? There may be workflows which set this variable
if(DEFINED ENV{MAYA_LOCATION})
  list(APPEND _MAYA_ROOT_SEARCH_DIR $ENV{MAYA_LOCATION})
endif()

# ------------------------------------------------------------------------
#  Search for Maya
# ------------------------------------------------------------------------

find_path(Maya_INCLUDE_DIR maya/MTypes.h
  ${_FIND_MAYA_ADDITIONAL_OPTIONS}
  PATHS ${_MAYA_ROOT_SEARCH_DIR}
  PATH_SUFFIXES include
)

if(NOT EXISTS "${Maya_INCLUDE_DIR}/maya/MTypes.h")
  message(FATAL_ERROR "Unable to locate Maya Installation.")
endif()

# Determine Maya version, including point releases. Currently only works for
# Maya 2016 and onwards so there is no -x64 and -x32 suffixes in the version
file(STRINGS "${Maya_INCLUDE_DIR}/maya/MTypes.h"
  _maya_version_string REGEX "#define MAYA_API_VERSION "
)
string(REGEX REPLACE ".*#define[ \t]+MAYA_API_VERSION[ \t]+([0-9]+).*$" "\\1"
  _maya_version_string "${_maya_version_string}"
)
string(SUBSTRING ${_maya_version_string} 0 4 Maya_MAJOR_VERSION)
string(SUBSTRING ${_maya_version_string} 4 2 Maya_MINOR_VERSION)

if(Maya_MINOR_VERSION LESS 50)
  set(Maya_VERSION ${Maya_MAJOR_VERSION})
else()
  set(Maya_VERSION ${Maya_MAJOR_VERSION}.5)
endif()
unset(_maya_version_string)

# Find required maya libs

set(_MAYA_COMPONENT_LIST
  OpenMaya
  OpenMayaFX
  OpenMayaUI
  Foundation
)

set(Maya_LIBRARY_DIR "")
if(APPLE)
  set(Maya_LIBRARY_DIR ${Maya_INCLUDE_DIR}/../Maya.app/Contents/MacOS/)
else()
  set(Maya_LIBRARY_DIR ${Maya_INCLUDE_DIR}/../lib/)
endif()

set(Maya_LIB_COMPONENTS "")

foreach(COMPONENT ${_MAYA_COMPONENT_LIST})
  find_library(Maya_${COMPONENT}_LIBRARY ${COMPONENT}
    ${_FIND_MAYA_ADDITIONAL_OPTIONS}
    PATHS ${Maya_LIBRARY_DIR}
  )
  list(APPEND Maya_LIB_COMPONENTS ${Maya_${COMPONENT}_LIBRARY})
endforeach()

# ------------------------------------------------------------------------
#  Cache and set Maya_FOUND
# ------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Maya
  FOUND_VAR Maya_FOUND
  REQUIRED_VARS
    Maya_INCLUDE_DIR
    Maya_LIB_COMPONENTS
  VERSION_VAR Maya_VERSION
)

if(NOT Maya_FOUND)
  message(FATAL_ERROR "Unable to locate Maya Installation.")
elseif(Maya_VERSION VERSION_LESS MINIMUM_MAYA_VERSION)
  message(WARNING "Unsupported Maya Version ${Maya_VERSION}. Minimum "
    "supported is ${MINIMUM_MAYA_VERSION}."
  )
endif()

# ------------------------------------------------------------------------
#  Configure dependencies
# ------------------------------------------------------------------------

if(NOT TBB_INCLUDEDIR)
  set(TBB_INCLUDEDIR ${Maya_INCLUDE_DIR})
endif()
if(NOT TBB_LIBRARYDIR)
  set(TBB_LIBRARYDIR ${Maya_LIBRARY_DIR})
endif()

# ------------------------------------------------------------------------
#  Configure Maya
# ------------------------------------------------------------------------

set(Maya_LIBRARIES ${Maya_LIB_COMPONENTS})
set(Maya_INCLUDE_DIRS ${Maya_INCLUDE_DIR})
set(Maya_LIBRARY_DIRS ${Maya_LIBRARY_DIR})

if(APPLE)
  set(Maya_DEFINITIONS
    -DMAC_PLUGIN
    -DREQUIRE_IOSTREAM
    -DOSMac_
    -DOSMac_MachO_
    -D_BOOL
  )
elseif(WIN32)
  set(Maya_DEFINITIONS
    -DNOMINMAX
    -DNT_PLUGIN
    -DREQUIRE_IOSTREAM
    -D_USE_MATH_DEFINES
    -D_CRT_SECURE_NO_WARNINGS
  )
else()
  set(Maya_DEFINITIONS
    -D_BOOL
    -DFUNCPROTO
    -DGL_GLEXT_PROTOTYPES=1
    -DREQUIRE_IOSTREAM
    -DUNIX
    -fno-gnu-keywords
    -fno-omit-frame-pointer
    -fno-strict-aliasing
    -funsigned-char
    -Wno-comment
    -Wno-multichar
    -Wno-strict-aliasing
    -m64
    -DBits64_
    -DLINUX
    -DLINUX_64
  )
endif()

# Configure imported targets

if(NOT TARGET Maya)
  add_library(Maya INTERFACE)
  foreach(COMPONENT ${_MAYA_COMPONENT_LIST})
    add_library(Maya::${COMPONENT} UNKNOWN IMPORTED)
    set_target_properties(Maya::${COMPONENT} PROPERTIES
      IMPORTED_LOCATION "${Maya_${COMPONENT}_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${Maya_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${Maya_INCLUDE_DIRS}"
    )
    target_link_libraries(Maya INTERFACE Maya::${COMPONENT})
  endforeach()
endif()

macro(MAYA_SET_LIBRARY_PROPERTIES NAME)
  if(WIN32)
    set_target_properties(${NAME} PROPERTIES
      SUFFIX ".mll"
      PREFIX ""
      LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin"
    )
  elseif(APPLE)
    set_target_properties(${NAME} PROPERTIES
      SUFFIX ".bundle"
      PREFIX ""
    )
  else()
    set_target_properties(${NAME} PROPERTIES
      PREFIX ""
    )
  endif()
endmacro()
