# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

FindOpenEXR
-----------

Find OpenEXR include dirs and libraries

Use this module by invoking find_package with the form::

  find_package(OpenEXR
    [version] [EXACT]      # Minimum or EXACT version
    [REQUIRED]             # Fail with error if OpenEXR is not found
    [COMPONENTS <libs>...] # OpenEXR libraries by their canonical name
                           # e.g. "IlmImf" for "libIlmImf"
    )

IMPORTED Targets
^^^^^^^^^^^^^^^^

``OpenEXR::IlmImf``
  The IlmImf library target.
``OpenEXR::IlmImfUtil``
  The IlmImfUtil library target.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpenEXR_FOUND``
  True if the system has the OpenEXR library.
``OpenEXR_VERSION``
  The version of the OpenEXR library which was found.
``OpenEXR_INCLUDE_DIRS``
  Include directories needed to use OpenEXR.
``OpenEXR_LIBRARIES``
  Libraries needed to link to OpenEXR.
``OpenEXR_LIBRARY_DIRS``
  OpenEXR library directories.
``OpenEXR_DEFINITIONS``
  Definitions to use when compiling code that uses OpenEXR.
``OpenEXR_{COMPONENT}_FOUND``
  True if the system has the named OpenEXR component.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``OpenEXR_INCLUDE_DIR``
  The directory containing ``OpenEXR/config-auto.h``.
``OpenEXR_{COMPONENT}_LIBRARY``
  Individual component libraries for OpenEXR
``OpenEXR_{COMPONENT}_DLL``
  Individual component dlls for OpenEXR on Windows.

Hints
^^^^^

Instead of explicitly setting the cache variables, the following variables
may be provided to tell this module where to look.

``OpenEXR_ROOT``
  Preferred installation prefix.
``OPENEXR_INCLUDEDIR``
  Preferred include directory e.g. <prefix>/include
``OPENEXR_LIBRARYDIR``
  Preferred library directory e.g. <prefix>/lib
``SYSTEM_LIBRARY_PATHS``
  Global list of library paths intended to be searched by and find_xxx call
``OPENEXR_USE_STATIC_LIBS``
  Only search for static openexr libraries
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

mark_as_advanced(
  OpenEXR_INCLUDE_DIR
  OpenEXR_LIBRARY
)

set(_FIND_OPENEXR_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_OPENEXR_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

set(_OPENEXR_COMPONENT_LIST
  IlmImf
  IlmImfUtil
)

if(OpenEXR_FIND_COMPONENTS)
  set(OPENEXR_COMPONENTS_PROVIDED TRUE)
  set(_IGNORED_COMPONENTS "")
  foreach(COMPONENT ${OpenEXR_FIND_COMPONENTS})
    if(NOT ${COMPONENT} IN_LIST _OPENEXR_COMPONENT_LIST)
      list(APPEND _IGNORED_COMPONENTS ${COMPONENT})
    endif()
  endforeach()

  if(_IGNORED_COMPONENTS)
    message(STATUS "Ignoring unknown components of OpenEXR:")
    foreach(COMPONENT ${_IGNORED_COMPONENTS})
      message(STATUS "  ${COMPONENT}")
    endforeach()
    list(REMOVE_ITEM OpenEXR_FIND_COMPONENTS ${_IGNORED_COMPONENTS})
  endif()
else()
  set(OPENEXR_COMPONENTS_PROVIDED FALSE)
  set(OpenEXR_FIND_COMPONENTS ${_OPENEXR_COMPONENT_LIST})
endif()

# Set _OPENEXR_ROOT based on a user provided root var. Xxx_ROOT and ENV{Xxx_ROOT}
# are prioritised over the legacy capitalized XXX_ROOT variables for matching
# CMake 3.12 behaviour
# @todo  deprecate -D and ENV OPENEXR_ROOT from CMake 3.12
if(OpenEXR_ROOT)
  set(_OPENEXR_ROOT ${OpenEXR_ROOT})
elseif(DEFINED ENV{OpenEXR_ROOT})
  set(_OPENEXR_ROOT $ENV{OpenEXR_ROOT})
elseif(OPENEXR_ROOT)
  set(_OPENEXR_ROOT ${OPENEXR_ROOT})
elseif(DEFINED ENV{OPENEXR_ROOT})
  set(_OPENEXR_ROOT $ENV{OPENEXR_ROOT})
endif()

# Additionally try and use pkconfig to find OpenEXR

if(NOT DEFINED PKG_CONFIG_FOUND)
  find_package(PkgConfig)
endif()
pkg_check_modules(PC_OpenEXR QUIET OpenEXR)

# ------------------------------------------------------------------------
#  Search for OpenEXR include DIR
# ------------------------------------------------------------------------

set(_OPENEXR_INCLUDE_SEARCH_DIRS "")
list(APPEND _OPENEXR_INCLUDE_SEARCH_DIRS
  ${OPENEXR_INCLUDEDIR}
  ${_OPENEXR_ROOT}
  ${PC_OpenEXR_INCLUDEDIR}
  ${SYSTEM_LIBRARY_PATHS}
)

# Look for a standard OpenEXR header file.
find_path(OpenEXR_INCLUDE_DIR OpenEXRConfig.h
  ${_FIND_OPENEXR_ADDITIONAL_OPTIONS}
  PATHS ${_OPENEXR_INCLUDE_SEARCH_DIRS}
  PATH_SUFFIXES  include/OpenEXR OpenEXR
)

if(EXISTS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h")
  # Get the EXR version information from the config header
  file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h"
    _openexr_version_major_string REGEX "#define OPENEXR_VERSION_MAJOR "
  )
  string(REGEX REPLACE "#define OPENEXR_VERSION_MAJOR" ""
    _openexr_version_major_string "${_openexr_version_major_string}"
  )
  string(STRIP "${_openexr_version_major_string}" OpenEXR_VERSION_MAJOR)

  file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h"
     _openexr_version_minor_string REGEX "#define OPENEXR_VERSION_MINOR "
  )
  string(REGEX REPLACE "#define OPENEXR_VERSION_MINOR" ""
    _openexr_version_minor_string "${_openexr_version_minor_string}"
  )
  string(STRIP "${_openexr_version_minor_string}" OpenEXR_VERSION_MINOR)

  unset(_openexr_version_major_string)
  unset(_openexr_version_minor_string)

  set(OpenEXR_VERSION ${OpenEXR_VERSION_MAJOR}.${OpenEXR_VERSION_MINOR})
endif()

# ------------------------------------------------------------------------
#  Search for OPENEXR lib DIR
# ------------------------------------------------------------------------

set(_OPENEXR_LIBRARYDIR_SEARCH_DIRS "")

# Append to _OPENEXR_LIBRARYDIR_SEARCH_DIRS in priority order

list(APPEND _OPENEXR_LIBRARYDIR_SEARCH_DIRS
  ${OPENEXR_LIBRARYDIR}
  ${_OPENEXR_ROOT}
  ${PC_OpenEXR_LIBDIR}
  ${SYSTEM_LIBRARY_PATHS}
)

# Build suffix directories

set(OPENEXR_PATH_SUFFIXES
  lib64
  lib
)

if(UNIX )
  list(INSERT OPENEXR_PATH_SUFFIXES 0 lib/x86_64-linux-gnu)
endif()

set(_OPENEXR_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

# library suffix handling
if(WIN32)
  list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES
    "-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}.lib"
  )
else()
  if(OPENEXR_USE_STATIC_LIBS)
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES
      "-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}.a"
    )
  else()
    if(APPLE)
      list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES
        "-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}.dylib"
      )
    else()
      list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES
        "-${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}.so"
      )
    endif()
  endif()
endif()

set(OpenEXR_LIB_COMPONENTS "")

foreach(COMPONENT ${OpenEXR_FIND_COMPONENTS})
  find_library(OpenEXR_${COMPONENT}_LIBRARY ${COMPONENT}
    ${_FIND_OPENEXR_ADDITIONAL_OPTIONS}
    PATHS ${_OPENEXR_LIBRARYDIR_SEARCH_DIRS}
    PATH_SUFFIXES ${OPENEXR_PATH_SUFFIXES}
  )
  list(APPEND OpenEXR_LIB_COMPONENTS ${OpenEXR_${COMPONENT}_LIBRARY})

  if(WIN32 AND NOT OPENEXR_USE_STATIC_LIBS)
    set(_OPENEXR_TMP ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    find_library(OpenEXR_${COMPONENT}_DLL ${COMPONENT}
      ${_FIND_OPENEXR_ADDITIONAL_OPTIONS}
      PATHS ${_OPENEXR_LIBRARYDIR_SEARCH_DIRS}
      PATH_SUFFIXES bin
    )
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_OPENEXR_TMP})
    unset(_OPENEXR_TMP)
  endif()

  if(OpenEXR_${COMPONENT}_LIBRARY)
    set(OpenEXR_${COMPONENT}_FOUND TRUE)
  else()
    set(OpenEXR_${COMPONENT}_FOUND FALSE)
  endif()
endforeach()

# reset lib suffix

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_OPENEXR_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
unset(_OPENEXR_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)

# ------------------------------------------------------------------------
#  Cache and set OPENEXR_FOUND
# ------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR
  FOUND_VAR OpenEXR_FOUND
  REQUIRED_VARS
    OpenEXR_INCLUDE_DIR
    OpenEXR_LIB_COMPONENTS
  VERSION_VAR OpenEXR_VERSION
  HANDLE_COMPONENTS
)

if(OpenEXR_FOUND)
  set(OpenEXR_LIBRARIES ${OpenEXR_LIB_COMPONENTS})

  # We have to add both include and include/OpenEXR to the include
  # path in case OpenEXR and IlmBase are installed separately

  set(OpenEXR_INCLUDE_DIRS)
  list(APPEND OpenEXR_INCLUDE_DIRS
    ${OpenEXR_INCLUDE_DIR}/../
    ${OpenEXR_INCLUDE_DIR}
  )
  set(OpenEXR_DEFINITIONS ${PC_OpenEXR_CFLAGS_OTHER})

  set(OpenEXR_LIBRARY_DIRS "")
  foreach(LIB ${OpenEXR_LIB_COMPONENTS})
    get_filename_component(_OPENEXR_LIBDIR ${LIB} DIRECTORY)
    list(APPEND OpenEXR_LIBRARY_DIRS ${_OPENEXR_LIBDIR})
  endforeach()
  list(REMOVE_DUPLICATES OpenEXR_LIBRARY_DIRS)

  # Configure imported target

  foreach(COMPONENT ${OpenEXR_FIND_COMPONENTS})
    if(NOT TARGET OpenEXR::${COMPONENT})
      add_library(OpenEXR::${COMPONENT} UNKNOWN IMPORTED)
      set_target_properties(OpenEXR::${COMPONENT} PROPERTIES
        IMPORTED_LOCATION "${OpenEXR_${COMPONENT}_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${OpenEXR_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIRS}"
      )
    endif()
  endforeach()
elseif(OpenEXR_FIND_REQUIRED)
  message(FATAL_ERROR "Unable to find OpenEXR")
endif()
