# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

OpenVDBGLFW3Setup
-----------------

Wraps the call the FindPackage ( glfw3 ) for OpenVDB builds. Provides
some extra options for finding the glfw3 installation without polluting
the OpenVDB Binaries cmake.

Use this module by invoking include with the form::

  include ( OpenVDBGLFW3Setup )

IMPORTED Targets
^^^^^^^^^^^^^^^^

``glfw``
  The glfw3 library target.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``glfw3_FOUND``
  True if the system has glfw3 installed.
``glfw3_VERSION``
  The version of the glfw3 library which was found.

Hints
^^^^^

The following variables may be provided to tell this module where to look.

``GLFW3_ROOT``
  Preferred installation prefix.
``SYSTEM_LIBRARY_PATHS``
  Global list of library paths intended to be searched by and find_xxx call
``DISABLE_CMAKE_SEARCH_PATHS``
  Disable CMakes default search paths for find_xxx calls in this module

#]=======================================================================]

# Find the glfw3 installation and use glfw's CMake to initialize
# the glfw lib

cmake_minimum_required(VERSION 3.3)

set(_FIND_GLFW3_ADDITIONAL_OPTIONS "")
if(DISABLE_CMAKE_SEARCH_PATHS)
  set(_FIND_GLFW3_ADDITIONAL_OPTIONS NO_DEFAULT_PATH)
endif()

set(_GLFW3_ROOT_SEARCH_DIR "")

if(GLFW3_ROOT)
  list(APPEND _GLFW3_ROOT_SEARCH_DIR ${GLFW3_ROOT})
else()
  set(_ENV_GLFW_ROOT $ENV{GLFW3_ROOT})
  if(_ENV_GLFW_ROOT)
    list(APPEND _GLFW3_ROOT_SEARCH_DIR ${_ENV_GLFW_ROOT})
  endif()
endif()

# Additionally try and use pkconfig to find glfw, though we only use
# pkg-config to re-direct to the cmake. In other words, glfw's cmake is
# expected to be installed
if(NOT DEFINED PKG_CONFIG_FOUND)
  find_package(PkgConfig)
endif()
pkg_check_modules(PC_glfw3 QUIET glfw3)

if(PC_glfw3_FOUND)
  foreach(DIR ${PC_glfw3_LIBRARY_DIRS})
    list(APPEND _GLFW3_ROOT_SEARCH_DIR ${DIR})
  endforeach()
endif()

list(APPEND _GLFW3_ROOT_SEARCH_DIR ${SYSTEM_LIBRARY_PATHS})

set(_GLFW3_PATH_SUFFIXES "lib/cmake/glfw3" "cmake/glfw3" "glfw3")

# GLFW 3.1 installs CMake modules into glfw instead of glfw3
list(APPEND _GLFW3_PATH_SUFFIXES "lib/cmake/glfw" "cmake/glfw" "glfw")

find_path(GLFW3_CMAKE_LOCATION glfw3Config.cmake
  ${_FIND_GLFW3_ADDITIONAL_OPTIONS}
  PATHS ${_GLFW3_ROOT_SEARCH_DIR}
  PATH_SUFFIXES ${_GLFW3_PATH_SUFFIXES}
)

if(GLFW3_CMAKE_LOCATION)
  if(EXISTS "${GLFW3_CMAKE_LOCATION}/glfw3Targets.cmake")
    include("${GLFW3_CMAKE_LOCATION}/glfw3Targets.cmake")
  elseif(EXISTS "${GLFW3_CMAKE_LOCATION}/glfwTargets.cmake")
    include("${GLFW3_CMAKE_LOCATION}/glfwTargets.cmake")
  endif()
endif()

if(GLFW3_CMAKE_LOCATION)
  list(APPEND CMAKE_PREFIX_PATH "${GLFW3_CMAKE_LOCATION}")
endif()

set(glfw3_FIND_VERSION ${MINIMUM_GLFW_VERSION})
find_package(glfw3 ${MINIMUM_GLFW_VERSION} REQUIRED)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(glfw3
  REQUIRED_VARS glfw3_DIR glfw3_FOUND
  VERSION_VAR glfw3_VERSION
)

unset(glfw3_FIND_VERSION)

# GLFW 3.1 does not export INTERFACE_LINK_LIBRARIES so detect this
# and set the property ourselves
# @todo investigate how this might apply for Mac OSX
if(UNIX)
  get_property(glfw3_HAS_INTERFACE_LINK_LIBRARIES
    TARGET glfw
    PROPERTY INTERFACE_LINK_LIBRARIES
    SET
  )
  if (NOT glfw3_HAS_INTERFACE_LINK_LIBRARIES)
    message(WARNING "GLFW does not have the INTERFACE_LINK_LIBRARIES property "
      "set, so hard-coding to expect a dependency on X11. To use a different "
      "library dependency, consider upgrading to GLFW 3.2+ where this "
      "property is set by the CMake find package module for GLFW.")
    find_package(X11 REQUIRED)
    set_property(TARGET glfw
      PROPERTY INTERFACE_LINK_LIBRARIES
      ${X11_Xrandr_LIB}
      ${X11_Xxf86vm_LIB}
      ${X11_Xcursor_LIB}
      ${X11_Xinerama_LIB}
      ${X11_Xi_LIB}
      ${X11_LIBRARIES}
      ${CMAKE_DL_LIBS}
    )
  endif()
endif()
