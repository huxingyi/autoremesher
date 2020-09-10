# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

FindLog4cplus
-------------

Find Log4cplus include dirs and libraries

Use this module by invoking find_package with the form::

  find_package(Log4cplus
    [version] [EXACT]      # Minimum or EXACT version
    [REQUIRED]             # Fail with error if Log4cplus is not found
    )


IMPORTED Targets
^^^^^^^^^^^^^^^^

``Log4cplus::Log4cplus``
  This module defines IMPORTED target Log4cplus::log4cplus, if Log4cplus has been
  found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Log4cplus_FOUND``
  True if the system has the Log4cplus library.
``Log4cplus_VERSION``
  The version of the Log4cplus library which was found.
``Log4cplus_INCLUDE_DIRS``
  Include directories needed to use Log4cplus.
``Log4cplus_LIBRARIES``
  Libraries needed to link to Log4cplus.
``Log4cplus_LIBRARY_DIRS``
  Log4cplus library directories.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Log4cplus_INCLUDE_DIR``
  The directory containing ``log4cplus/version.h``.
``Log4cplus_LIBRARY``
  The path to the Log4cplus library.

Hints
^^^^^

Instead of explicitly setting the cache variables, the following variables
may be provided to tell this module where to look.

``Log4cplus_ROOT``
  Preferred installation prefix.
``LOG4CPLUS_INCLUDEDIR``
  Preferred include directory e.g. <prefix>/include
``LOG4CPLUS_LIBRARYDIR``
  Preferred library directory e.g. <prefix>/lib
``SYSTEM_LIBRARY_PATHS``
  Global list of library paths intended to be searched by and find_xxx call
``LOG4CPLUS_USE_STATIC_LIBS``
  Only search for static log4cplus libraries
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

mark_as_advanced(
  Log4cplus_INCLUDE_DIR
  Log4cplus_LIBRARY
)

set(_FIND_LOG4CPLUS_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_LOG4CPLUS_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

# Set _LOG4CPLUS_ROOT based on a user provided root var. Xxx_ROOT and ENV{Xxx_ROOT}
# are prioritised over the legacy capitalized XXX_ROOT variables for matching
# CMake 3.12 behaviour
# @todo  deprecate -D and ENV LOG4CPLUS_ROOT from CMake 3.12
if(Log4cplus_ROOT)
  set(_LOG4CPLUS_ROOT ${Log4cplus_ROOT})
elseif(DEFINED ENV{Log4cplus_ROOT})
  set(_LOG4CPLUS_ROOT $ENV{Log4cplus_ROOT})
elseif(LOG4CPLUS_ROOT)
  set(_LOG4CPLUS_ROOT ${LOG4CPLUS_ROOT})
elseif(DEFINED ENV{LOG4CPLUS_ROOT})
  set(_LOG4CPLUS_ROOT $ENV{LOG4CPLUS_ROOT})
endif()

# Additionally try and use pkconfig to find log4cplus

if(NOT DEFINED PKG_CONFIG_FOUND)
  find_package(PkgConfig)
endif()
pkg_check_modules(PC_Log4cplus QUIET log4cplus)

# ------------------------------------------------------------------------
#  Search for Log4cplus include DIR
# ------------------------------------------------------------------------

set(_LOG4CPLUS_INCLUDE_SEARCH_DIRS "")
list(APPEND _LOG4CPLUS_INCLUDE_SEARCH_DIRS
  ${LOG4CPLUS_INCLUDEDIR}
  ${_LOG4CPLUS_ROOT}
  ${PC_Log4cplus_INCLUDEDIR}
  ${SYSTEM_LIBRARY_PATHS}
)

# Look for a standard log4cplus header file.
find_path(Log4cplus_INCLUDE_DIR log4cplus/version.h
  ${_FIND_LOG4CPLUS_ADDITIONAL_OPTIONS}
  PATHS ${_LOG4CPLUS_INCLUDE_SEARCH_DIRS}
  PATH_SUFFIXES include
)

if(EXISTS "${Log4cplus_INCLUDE_DIR}/log4cplus/version.h")
  file(STRINGS "${Log4cplus_INCLUDE_DIR}/log4cplus/version.h"
    _log4cplus_version_string REGEX "#define LOG4CPLUS_VERSION LOG4CPLUS_MAKE_VERSION"
  )
  string(REGEX REPLACE "#define LOG4CPLUS_VERSION LOG4CPLUS_MAKE_VERSION\((.*)\).*$" "\\1"
    _log4cplus_version_string "${_log4cplus_version_string}"
  )
  string(REGEX REPLACE "[(]([0-9]+),.*[)].*$" "\\1"
    Log4cplus_MAJOR_VERSION "${_log4cplus_version_string}"
  )
  string(REGEX REPLACE "[(].+, ([0-9]+),.+[)].*$" "\\1"
    Log4cplus_MINOR_VERSION "${_log4cplus_version_string}"
  )
  string(REGEX REPLACE "[(].*,.*, ([0-9]+)[)].*$" "\\1"
    Log4cplus_PATCH_VERSION "${_log4cplus_version_string}"
  )
  unset(_log4cplus_version_string)

  set(Log4cplus_VERSION ${Log4cplus_MAJOR_VERSION}.${Log4cplus_MINOR_VERSION}.${Log4cplus_PATCH_VERSION})
endif()

# ------------------------------------------------------------------------
#  Search for Log4cplus lib DIR
# ------------------------------------------------------------------------

set(_LOG4CPLUS_LIBRARYDIR_SEARCH_DIRS "")
list(APPEND _LOG4CPLUS_LIBRARYDIR_SEARCH_DIRS
  ${LOG4CPLUS_LIBRARYDIR}
  ${_LOG4CPLUS_ROOT}
  ${PC_Log4cplus_LIBDIR}
  ${SYSTEM_LIBRARY_PATHS}
)


if(UNIX AND LOG4CPLUS_USE_STATIC_LIBS)
  set(_LOG4CPLUS_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# Build suffix directories

set(LOG4CPLUS_PATH_SUFFIXES
  lib64
  lib
)

find_library(Log4cplus_LIBRARY log4cplus
  ${_FIND_LOG4CPLUS_ADDITIONAL_OPTIONS}
  PATHS ${_LOG4CPLUS_LIBRARYDIR_SEARCH_DIRS}
  PATH_SUFFIXES ${LOG4CPLUS_PATH_SUFFIXES}
)

if(UNIX AND LOG4CPLUS_USE_STATIC_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_LOG4CPLUS_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
  unset(_LOG4CPLUS_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)
endif()

# ------------------------------------------------------------------------
#  Cache and set Log4cplus_FOUND
# ------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Log4cplus
  FOUND_VAR Log4cplus_FOUND
  REQUIRED_VARS
    Log4cplus_LIBRARY
    Log4cplus_INCLUDE_DIR
  VERSION_VAR Log4cplus_VERSION
)

if(Log4cplus_FOUND)
  set(Log4cplus_LIBRARIES ${Log4cplus_LIBRARY})
  set(Log4cplus_INCLUDE_DIRS ${Log4cplus_INCLUDE_DIR})
  set(Log4cplus_DEFINITIONS ${PC_Log4cplus_CFLAGS_OTHER})

  get_filename_component(Log4cplus_LIBRARY_DIRS ${Log4cplus_LIBRARY} DIRECTORY)

  if(NOT TARGET Log4cplus::log4cplus)
    add_library(Log4cplus::log4cplus UNKNOWN IMPORTED)
    set_target_properties(Log4cplus::log4cplus PROPERTIES
      IMPORTED_LOCATION "${Log4cplus_LIBRARIES}"
      INTERFACE_COMPILE_DEFINITIONS "${Log4cplus_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${Log4cplus_INCLUDE_DIRS}"
    )
  endif()
elseif(Log4cplus_FIND_REQUIRED)
  message(FATAL_ERROR "Unable to find Log4cplus")
endif()
