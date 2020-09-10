# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

# We require this to get object library link library support and
# combined python 2 + 3 support
if(OPENEXR_BUILD_BOTH_STATIC_SHARED)
  if (${CMAKE_VERSION} VERSION_LESS "3.12.0")
    message(FATAL_ERROR "CMake 3.12 or newer is required for object library support when building both static and shared libraries")
  endif()
  cmake_minimum_required(VERSION 3.12)
else()
  cmake_minimum_required(VERSION 3.10)
endif()

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# we include this first to parse configure.ac and extract the version
# numbers
include(config/ParseConfigure.cmake)

project(OpenEXR VERSION ${OPENEXR_VERSION} LANGUAGES C CXX)

#######################################
#######################################
# This declares all the configuration variables visible
# in cmake-gui or similar and the rest of the global
# project setup
#
# Please look at this file to see what is configurable
#######################################
#######################################
include(config/OpenEXRSetup.cmake)

# Everyone depends on IlmBase, and we currently rely on
# the version matched with our release
find_package(IlmBase ${OPENEXR_VERSION} EXACT REQUIRED CONFIG)

# generates config headers, package config files
add_subdirectory(config)

# utility function for the repeated boilerplate of defining
# the libraries
include(config/LibraryDefine.cmake)

##########################

add_subdirectory( IlmImf )
add_subdirectory( IlmImfUtil )
add_subdirectory( IlmImfExamples )

##########################
# Tests
##########################
# Can't seem to do EXCLUDE_FROM_ALL on the test executables
# since you can't then create a dependency on the internal
# "test" target such that it is auto built
include(CTest)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory( IlmImfTest )
  add_subdirectory( IlmImfUtilTest )
  add_subdirectory( IlmImfFuzzTest )
endif()

##########################
# Binaries / Utilities
##########################
if(OPENEXR_BUILD_UTILS)
  add_subdirectory( exr2aces )
  add_subdirectory( exrheader )
  add_subdirectory( exrmaketiled )
  add_subdirectory( exrstdattr )
  add_subdirectory( exrmakepreview )
  add_subdirectory( exrenvmap )
  add_subdirectory( exrmultiview )
  add_subdirectory( exrmultipart )
endif()

add_subdirectory(doc)
