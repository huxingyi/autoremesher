# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

# We require this to get object library link library support and
# combined python 2 + 3 support
if(OPENEXR_BUILD_BOTH_STATIC_SHARED OR ILMBASE_BUILD_BOTH_STATIC_SHARED)
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

# Hint: This can be set to enable custom find_package
# search paths, probably best to set it when configuring
# on the command line to cmake instead of setting it
# here.
###set(CMAKE_PREFIX_PATH "/prefix")

project(OpenEXRMetaProject)

# An "official" way to make this a super-project
# basically overrides the find_package to not find anything
# for stuff we're including locally
set(as_subproject IlmBase OpenEXR)
macro(find_package)
  if(NOT "${ARGV0}" IN_LIST as_subproject)
    _find_package(${ARGV})
  endif()
endmacro()


# If you want to use ctest to configure, build and
# upload the results, cmake has builtin support for
# submitting to CDash, or any server who speaks the
# same protocol
# 
# These settings will need to be set for your environment,
# and then a script such as the example in
#
# cmake/SampleCTestScript.cmake
#
# edited and placed into the CI system, then run:
#
# cmake -S cmake/SampleCTestScript.cmake
#
# [or whatever you name the file you edit]
# 
#set(CTEST_PROJECT_NAME "OpenEXR")
#set(CTEST_NIGHTLY_START_TIME "01:01:01 UTC")
#set(CTEST_DROP_METHOD "http") # there are others...
#set(CTEST_DROP_SITE "open.cdash.org")
#set(CTEST_DROP_LOCATION "/submit.php?project=MyProject")
#set(CTEST_DROP_SITE_CDASH TRUE)

include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

#######################################

# Include these two modules without enable/disable options
add_subdirectory(IlmBase)
add_subdirectory(OpenEXR)

# should this just be always on and we conditionally compile what
# is found and warn otherwise? or error out?
option(PYILMBASE_ENABLE "Enables configuration of the PyIlmBase module" ON)
if(PYILMBASE_ENABLE)
  if (${CMAKE_VERSION} VERSION_LESS "3.12.0")
    message(WARNING ": CMake version ${CMAKE_VERSION} detected, PyIlmBase uses newer features of cmake (>= 3.12), disabling")
  else()
    add_subdirectory(PyIlmBase)
  endif()
endif()

option(OPENEXR_VIEWERS_ENABLE "Enables configuration of the viewers module" ON)
if(OPENEXR_VIEWERS_ENABLE)
  add_subdirectory(OpenEXR_Viewers)
endif()
