# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[

  CMake Configuration for OpenVDB

  The OpenVDB CMake build system generates targets depending on the
  enabled components. It is designed for out of source CMake generation
  (a build location for CMake to write to will be required). For example,
  from the root of the repository:

    mkdir build
    cd build
    cmake ../

  Depending on the components you choose to build, a number of optional
  and required dependencies are expected. See the dependency documentation
  for more information:

    https://www.openvdb.org/documentation/doxygen/dependencies.html

  And the documentation on building OpenVDB for more in depth installation
  instructions:

    https://www.openvdb.org/documentation/doxygen/build.html

  This CMakeLists file provides most available options for configuring the
  build and installation of all OpenVDB components. By default the core
  library and the vdb_print binary are enabled.

  Note that various packages have inbuilt CMake module support. See the
  CMake documentation for more ZLib, Doxygen, OpenGL, Boost and Python
  controls:

    https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html

  OpenVDB's CMake supports building the various components of against a
  prior installation of OpenVDB.

#]=======================================================================]

# note: cmake_minimum_required must be called before project commands to
#  ensure policy scope is set up correctly
cmake_minimum_required(VERSION 3.3)
project(OpenVDB)

###### OpenVDB Build/Component Options

include(CMakeDependentOption)
include(GNUInstallDirs)

# todo epydoc and pdflatex
option(OPENVDB_BUILD_CORE "Enable the core OpenVDB library. Both static and shared versions are enabled by default" ON)
option(OPENVDB_BUILD_BINARIES "Enable the vdb binaries. Only vdb_print is enabled by default" ON)
option(OPENVDB_BUILD_PYTHON_MODULE "Build the pyopenvdb Python module" OFF)
option(OPENVDB_BUILD_UNITTESTS "Build the OpenVDB unit tests" OFF)
option(OPENVDB_BUILD_DOCS "Build the OpenVDB documentation" OFF)
option(OPENVDB_BUILD_HOUDINI_PLUGIN "Build the Houdini plugin" OFF)
option(OPENVDB_INSTALL_HOUDINI_PYTHONRC [=[Install a Houdini startup script that sets
the visibilty of OpenVDB nodes and their native equivalents.]=] OFF)
option(OPENVDB_BUILD_MAYA_PLUGIN "Build the Maya plugin" OFF)
option(OPENVDB_ENABLE_RPATH "Build with RPATH information" ON)
option(OPENVDB_CXX_STRICT "Enable or disable pre-defined compiler warnings" OFF)
option(OPENVDB_CODE_COVERAGE "Enable code coverage. This also overrides CMAKE_BUILD_TYPE to Debug" OFF)
cmake_dependent_option(OPENVDB_INSTALL_CMAKE_MODULES
  "Install the provided OpenVDB CMake modules when building the core library"
  ON "OPENVDB_BUILD_CORE" OFF)

option(USE_HOUDINI [=[
Build the library against a Houdini installation. Turns on automatically if OPENVDB_BUILD_HOUDINI_PLUGIN is enabled.
When enabled, you do not need to provide dependency locations for TBB, Blosc, IlmBase and OpenEXR. Boost must be
provided. IlmBase/OpenEXR can optionally be provided if Houdini Version >= 17.5.]=] OFF)
option(USE_MAYA [=[
Build the library against a Maya installation. Turns on automatically if OPENVDB_BUILD_MAYA_PLUGIN is enabled.
When enabled, you do not need to provide dependency locations for TBB. All other dependencies must be provided.]=] OFF)
option(USE_BLOSC [=[
Use blosc while building openvdb components. If OPENVDB_BUILD_CORE is OFF, CMake attempts to query the located
openvdb build configuration to decide on blosc support. You may set this to on to force blosc to be used if you
know it to be required.]=] ON)
option(USE_LOG4CPLUS [=[
Use log4cplus while building openvdb components. If OPENVDB_BUILD_CORE is OFF, CMake attempts to query the
located openvdb build configuration to decide on log4cplus support. You may set this to on to force log4cplus
to be used if you know it to be required.]=] OFF)
option(USE_EXR [=[
Use OpenEXR while building the core openvdb library. If OPENVDB_BUILD_CORE is OFF, CMake attempts to query the located
openvdb build configuration to decide on OpenEXR support. You may set this to on to force OpenEXR to be used if you
know it to be required.]=] OFF)
cmake_dependent_option(OPENVDB_DISABLE_BOOST_IMPLICIT_LINKING
  "Disable the implicit linking of Boost libraries on Windows" ON "WIN32" OFF)
option(USE_CCACHE "Build using Ccache if found on the path" ON)
option(DISABLE_DEPENDENCY_VERSION_CHECKS [=[
Disable minimum version checks for OpenVDB dependencies. It is strongly recommended that this remains disabled.
Consider updating your dependencies where possible if encountering minimum requirement CMake errors.]=] OFF)
option(DISABLE_CMAKE_SEARCH_PATHS [=[
Disable CMakes default system search paths when locating dependencies. When enabled, CMake will fall back to
its default system search routine if it cannot find a dependency with the provided settings. When disabled, only
paths provided through the Xxx_ROOT, supported XXX_INCLUDEDIR/XXX_LIBRARYDIR variables or the SYSTEM_LIBRARY_PATHS
list will be searched.]=] OFF)
option(OPENVDB_USE_DEPRECATED_ABI "Bypass minimum ABI requirement checks" OFF)
option(OPENVDB_FUTURE_DEPRECATION "Generate messages for upcoming deprecation" OFF)
option(USE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)

set(SYSTEM_LIBRARY_PATHS "" CACHE STRING [=[
A global list of library paths to additionally use into when searching for dependencies.]=])

set(_CONCURRENT_MALLOC_OPTIONS None Auto Jemalloc Tbbmalloc)
if(NOT CONCURRENT_MALLOC)
  set(CONCURRENT_MALLOC Auto CACHE STRING
    "Choose a concurrent malloc library, options are: None Auto Jemalloc Tbbmalloc.
    Although not required, it is strongly recommended to use a concurrent memory allocator.
    Auto is the default and implies Jemalloc, unless USE_MAYA is ON or Jemalloc is unavailable,
    in which case Tbbmalloc is used." FORCE
  )
elseif(NOT ${CONCURRENT_MALLOC} IN_LIST _CONCURRENT_MALLOC_OPTIONS)
  message(WARNING "Unrecognized value for CONCURRENT_MALLOC, using Auto instead.")
  set(CONCURRENT_MALLOC Auto CACHE STRING FORCE)
endif()

set(_OPENVDB_SIMD_OPTIONS None SSE42 AVX)
if(NOT OPENVDB_SIMD)
  set(OPENVDB_SIMD None CACHE STRING
    "Choose whether to enable SIMD compiler flags or not, options are: None SSE42 AVX.
    Although not required, it is strongly recommended to enable SIMD. AVX implies SSE42.
    None is the default." FORCE
  )
elseif(NOT ${OPENVDB_SIMD} IN_LIST _OPENVDB_SIMD_OPTIONS)
  message(WARNING "Unrecognized or unsupported value for OPENVDB_SIMD, "
    "using None instead.")
  set(OPENVDB_SIMD None CACHE STRING FORCE)
endif()

# Top-level location for all openvdb headers
set(OPENVDB_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/openvdb")

###### Deprecated options

option(OPENVDB_BUILD_HOUDINI_SOPS "Build the Houdini plugin  (deprecated - see OPENVDB_BUILD_HOUDINI_PLUGIN)" OFF)
option(OPENVDB_ENABLE_3_ABI_COMPATIBLE "Build with OpenVDB ABI 3 (deprecated - see OPENVDB_ABI_VERSION_NUMBER)" OFF)

# Alias deprecated vars

if(OPENVDB_BUILD_HOUDINI_SOPS)
  # Support for legacy OPENVDB_BUILD_HOUDINI_SOPS variable
  message(DEPRECATION "The OPENVDB_BUILD_HOUDINI_SOPS option is deprecated and will be removed. "
    "Use OPENVDB_BUILD_HOUDINI_PLUGIN.")
  set(OPENVDB_BUILD_HOUDINI_PLUGIN ON)
endif()

if(OPENVDB_ENABLE_3_ABI_COMPATIBLE)
  message(DEPRECATION "OPENVDB_ENABLE_3_ABI_COMPATIBLE is deprecated and will be removed. Use "
    "-D OPENVDB_ABI_VERSION_NUMBER=N, where N is the abi version.")
  if(OPENVDB_ABI_VERSION_NUMBER)
    if(NOT (${OPENVDB_ABI_VERSION_NUMBER} EQUAL 3))
      message(WARNING "OPENVDB_ABI_VERSION_NUMBER holds a different ABI value to "
        "OPENVDB_ENABLE_3_ABI_COMPATIBLE, which will be ignored.")
    endif()
  else()
    # Don't bother setting the docstring as we'll update it later
    set(OPENVDB_ABI_VERSION_NUMBER "3" CACHE STRING "" FORCE)
  endif()
endif()

if(DEFINED USE_SYSTEM_LIBRARY_PATHS)
  message(DEPRECATION "The USE_SYSTEM_LIBRARY_PATHS option is deprecated and will be removed. "
    "Use DISABLE_CMAKE_SEARCH_PATHS.")
  if(USE_SYSTEM_LIBRARY_PATHS)
    set(DISABLE_CMAKE_SEARCH_PATHS OFF)
  else()
    set(DISABLE_CMAKE_SEARCH_PATHS ON)
  endif()
endif()

# Various root level CMake options which are marked as advanced

mark_as_advanced(
  OPENVDB_CXX_STRICT
  OPENVDB_ENABLE_3_ABI_COMPATIBLE
  OPENVDB_ENABLE_RPATH
  USE_HOUDINI
  USE_MAYA
  USE_LOG4CPLUS
  USE_CCACHE
  OPENVDB_BUILD_HOUDINI_SOPS
  DISABLE_DEPENDENCY_VERSION_CHECKS
  DISABLE_CMAKE_SEARCH_PATHS
  OPENVDB_USE_DEPRECATED_ABI
  OPENVDB_FUTURE_DEPRECATION
  CONCURRENT_MALLOC
  OPENVDB_CODE_COVERAGE
  USE_COLORED_OUTPUT
  SYSTEM_LIBRARY_PATHS
  OPENVDB_SIMD
)

# Configure minimum version requirements - some are treated specially and fall
# outside of the DISABLE_DEPENDENCY_VERSION_CHECKS catch

# @note  Blosc version is currently treated as exception which must be adhered
# to. The minimum version must be at least 1.5. Previous versions are incompatible.
set(MINIMUM_BLOSC_VERSION 1.5)
# @note  ABI always enforced so the correct deprecation messages are available.
# OPENVDB_USE_DEPRECATED_ABI should be used to circumvent this
set(MINIMUM_OPENVDB_ABI_VERSION 4)
set(FUTURE_MINIMUM_OPENVDB_ABI_VERSION 5)

if(NOT DISABLE_DEPENDENCY_VERSION_CHECKS)
  # @note  Currently tracking CY2018 of the VFX platform where avaiable
  set(MINIMUM_GCC_VERSION 6.3.1)
  set(MINIMUM_CLANG_VERSION 3.8)
  set(MINIMUM_ICC_VERSION 17)
  set(MINIMUM_MSVC_VERSION 19.10)

  set(MINIMUM_BOOST_VERSION 1.61)
  set(MINIMUM_ILMBASE_VERSION 2.2)
  set(MINIMUM_OPENEXR_VERSION 2.2)
  set(MINIMUM_ZLIB_VERSION 1.2.7)
  set(MINIMUM_TBB_VERSION 2017.0)

  set(MINIMUM_PYTHON_VERSION 2.7)
  set(MINIMUM_NUMPY_VERSION 1.12.1)

  set(MINIMUM_CPPUNIT_VERSION 1.10)
  set(MINIMUM_GLFW_VERSION 3.1)
  set(MINIMUM_LOG4CPLUS_VERSION 1.1.2)
  set(MINIMUM_HOUDINI_VERSION 17.0)

  # These always promote warnings rather than errors
  set(MINIMUM_MAYA_VERSION 2017)
  set(MINIMUM_DOXYGEN_VERSION 1.8.8)
endif()

# VFX 19 deprecations to transition to MINIMUM_* variables in OpenVDB 8.0.0

set(FUTURE_MINIMUM_CMAKE_VERSION 3.12)

# No compiler upgrades planned
#set(FUTURE_MINIMUM_GCC_VERSION 6.3.1)
#set(FUTURE_MINIMUM_ICC_VERSION 17)
#set(FUTURE_MINIMUM_MSVC_VERSION 19.10)

set(FUTURE_MINIMUM_ILMBASE_VERSION 2.3)
set(FUTURE_MINIMUM_OPENEXR_VERSION 2.3)
set(FUTURE_MINIMUM_BOOST_VERSION 1.66)
set(FUTURE_MINIMUM_TBB_VERSION 2018.0)
set(FUTURE_MINIMUM_NUMPY_VERSION 1.14.0)
set(FUTURE_MINIMUM_HOUDINI_VERSION 18.0)

#########################################################################

# General CMake and CXX settings

if(FUTURE_MINIMUM_CMAKE_VERSION)
  if(${CMAKE_VERSION} VERSION_LESS ${FUTURE_MINIMUM_CMAKE_VERSION})
    message(DEPRECATION "Support for CMake versions < ${FUTURE_MINIMUM_CMAKE_VERSION} "
      "is deprecated and will be removed.")
  endif()
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(OPENVDB_ENABLE_RPATH)
  # Configure rpath for installation base on the following:
  # https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# For CMake's find Threads module which brings in pthread - This flag
# forces the compiler -pthread flag vs -lpthread
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

enable_testing()

# Add our cmake modules

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Add backports to support older versions of CMake
#  FindOpenGL.cmake is needed if CMake < 3.8
#  FindNumPy.cmake is needed if CMake < 3.14

if(${CMAKE_VERSION} VERSION_LESS 3.14)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/backports")
endif()

# Add cmake modules to installation command
# @todo fix our glew cmake module

if(OPENVDB_INSTALL_CMAKE_MODULES)
  set(OPENVDB_CMAKE_MODULES
    cmake/FindBlosc.cmake
    cmake/FindCppUnit.cmake
    cmake/FindJemalloc.cmake
    cmake/FindIlmBase.cmake
    cmake/FindLog4cplus.cmake
    cmake/FindOpenEXR.cmake
    cmake/FindOpenVDB.cmake
    cmake/FindTBB.cmake
    cmake/OpenVDBGLFW3Setup.cmake
    cmake/OpenVDBHoudiniSetup.cmake
    cmake/OpenVDBMayaSetup.cmake
    cmake/OpenVDBUtils.cmake
  )
  install(FILES ${OPENVDB_CMAKE_MODULES} DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenVDB)
endif()

# Configure DCC installation if necessary

if(OPENVDB_BUILD_HOUDINI_PLUGIN)
  set(USE_HOUDINI ON)
endif()

if(OPENVDB_BUILD_MAYA_PLUGIN)
  set(USE_MAYA ON)
endif()

# Configure component dependencies by loading the Houdini/Maya setup
# scripts. These also find the Houdini/Maya installations

if(USE_HOUDINI)
  include(OpenVDBHoudiniSetup)
endif()

if(USE_MAYA)
  include(OpenVDBMayaSetup)
endif()

if(OPENVDB_BUILD_DOCS)
  add_subdirectory(doc)
endif()

# Early exit if there's nothing to build

if(NOT (
    OPENVDB_BUILD_CORE OR
    OPENVDB_BUILD_BINARIES OR
    OPENVDB_BUILD_PYTHON_MODULE OR
    OPENVDB_BUILD_UNITTESTS OR
    OPENVDB_BUILD_HOUDINI_PLUGIN OR
    OPENVDB_BUILD_MAYA_PLUGIN)
  )
  return()
endif()

if(USE_MAYA AND USE_HOUDINI)
  # @todo technically this is possible so long as library versions match
  # exactly but it's difficult to validate and dangerous
  message(FATAL_ERROR "Cannot build both Houdini and Maya plugins against "
    "the same core dependencies. Plugins must be compiled separately to "
    "ensure the required DCC dependencies are met."
  )
endif()

#########################################################################

# ccache setup

if(USE_CCACHE)
  find_program(CCACHE_PATH ccache)
  if(CCACHE_PATH)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "Using ccache: ${CCACHE_PATH}")
  endif()
endif()

# Build type configuration - default to Release if none is set unless
# code coverage is in use, in which case override to Debug

if (OPENVDB_CODE_COVERAGE)
  message(WARNING "Code coverage requires debug mode, setting CMAKE_BUILD_TYPE "
    "to Debug.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
    "Code coverage requires debug mode." FORCE
  )
elseif(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE
  )
endif()
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")

# Code coverage configuration

if(OPENVDB_CODE_COVERAGE)
  # set gcov compile and link flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

  # use .gcno extension instead of .cc.gcno
  set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
endif()

#########################################################################

# Compiler configuration. Add definitions for a number of compiler warnings
# for sub projects and verify version requirements
# @todo  add definitions for Intel.

set(HAS_AVAILABLE_WARNINGS FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS MINIMUM_CLANG_VERSION)
    message(FATAL_ERROR "Insufficient clang++ version. Minimum required is "
      "\"${MINIMUM_CLANG_VERSION}\". Found version \"${CMAKE_CXX_COMPILER_VERSION}\""
    )
  endif()
  if(OPENVDB_CXX_STRICT)
    message(STATUS "Configuring Clang CXX warnings")
    set(HAS_AVAILABLE_WARNINGS TRUE)
    add_compile_options(
      -Werror
      -Wall
      -Wextra
      -Wconversion
      -Wno-sign-conversion
    )
  endif()
  if(USE_COLORED_OUTPUT)
    message(STATUS "Enabling colored compiler output")
    add_compile_options(-fcolor-diagnostics)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS MINIMUM_GCC_VERSION)
    message(FATAL_ERROR "Insufficient g++ version. Minimum required is "
      "\"${MINIMUM_GCC_VERSION}\". Found version \"${CMAKE_CXX_COMPILER_VERSION}\""
    )
  endif()
  if(OPENVDB_FUTURE_DEPRECATION AND FUTURE_MINIMUM_GCC_VERSION)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS FUTURE_MINIMUM_GCC_VERSION)
      message(DEPRECATION "Support for GCC versions < ${FUTURE_MINIMUM_GCC_VERSION} "
        "is deprecated and will be removed.")
    endif()
  endif()
  if(OPENVDB_CXX_STRICT)
    message(STATUS "Configuring GCC CXX warnings")
    set(HAS_AVAILABLE_WARNINGS TRUE)
    add_compile_options(
      -Werror
      -Wall
      -Wextra
      -pedantic
      -Wcast-align
      -Wcast-qual
      -Wconversion
      -Wdisabled-optimization
      -Woverloaded-virtual
    )
  endif()
  if(USE_COLORED_OUTPUT)
    message(STATUS "Enabling colored compiler output")
    add_compile_options(-fdiagnostics-color=always)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS MINIMUM_ICC_VERSION})
    message(FATAL_ERROR "Insufficient ICC version. Minimum required is "
      "\"${MINIMUM_ICC_VERSION}\". Found version \"${CMAKE_CXX_COMPILER_VERSION}\""
    )
  endif()
  if(OPENVDB_FUTURE_DEPRECATION AND FUTURE_MINIMUM_ICC_VERSION)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS FUTURE_MINIMUM_ICC_VERSION)
      message(DEPRECATION "Support for ICC versions < ${FUTURE_MINIMUM_ICC_VERSION} "
        "is deprecated and will be removed.")
    endif()
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(OPENVDB_FUTURE_DEPRECATION AND FUTURE_MINIMUM_MSVC_VERSION)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS FUTURE_MINIMUM_MSVC_VERSION)
      message(DEPRECATION "Support for MSVC versions < ${FUTURE_MINIMUM_MSVC_VERSION} "
        "is deprecated and will be removed.")
    endif()
  endif()
  # Increase the number of sections that an object file can contain
  add_compile_options(/bigobj)
  # Math constants are not included in <cmath> unless _USE_MATH_DEFINES is
  # defined on MSVC
  # https://msdn.microsoft.com/en-us/library/4hwaceh6.aspx
  add_definitions(-D_USE_MATH_DEFINES)
  # Disable the non-portable Windows definitions of min() and max() macros
  add_definitions(-DNOMINMAX)
  # Excludes APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  if(NOT OPENVDB_CXX_STRICT)
    message(STATUS "Suppressing some noisy MSVC CXX warnings, "
      "set OPENVDB_CXX_STRICT=ON to re-enable them.")
    # Conversion from int64_t to long
    add_compile_options(/wd4244)
    # It's not possible to use STL types in DLL interfaces in a portable and
    # reliable way so disable this warning
    add_compile_options(/wd4251)
    # Conversion from size_t to uLong
    add_compile_options(/wd4267)
    # Non dll-interface class used as base for dll-interface class
    add_compile_options(/wd4275)
    # Truncation from 'int' to 'bool'
    add_compile_options(/wd4305)
  endif()
endif()

if(OPENVDB_CXX_STRICT AND NOT HAS_AVAILABLE_WARNINGS)
  message(WARNING "No available CXX warnings for compiler ${CMAKE_CXX_COMPILER_ID}")
endif()
unset(HAS_AVAILABLE_WARNINGS)

# Configure malloc library. Use Jemalloc for Linux and non-Maya, otherwise Tbbmalloc.
# * On Mac OSX, linking against Jemalloc < 4.3.0 seg-faults with this error:
#     malloc: *** malloc_zone_unregister() failed for 0xaddress
#   Houdini 17.5 and older ships with Jemalloc 3.6.0, so we make Tbbmalloc the default
#   on Mac OSX (https://github.com/jemalloc/jemalloc/issues/420). Later versions of
#   Jemalloc are thought to work, but haven't been tested.
# * On Windows, we follow SideFX's example in using Tbbmalloc due to the challenges
#   of injecting into the Windows runtime to replace the system allocator.

if(CONCURRENT_MALLOC STREQUAL "Auto")
  if(WIN32 OR APPLE OR USE_MAYA)
    set(CONCURRENT_MALLOC "Tbbmalloc")
  else()
    set(CONCURRENT_MALLOC "Jemalloc")
  endif()
endif()

# Configure SIMD. AVX implies SSE 4.2.

if(OPENVDB_SIMD STREQUAL "AVX")
  add_compile_options(-mavx -msse4.2)
  add_definitions(-DOPENVDB_USE_AVX)
  add_definitions(-DOPENVDB_USE_SSE42)
elseif(OPENVDB_SIMD STREQUAL "SSE42")
  add_compile_options(-msse4.2)
  add_definitions(-DOPENVDB_USE_SSE42)
endif()

#########################################################################

# Configure OpenVDB Library and ABI versions

if(NOT OPENVDB_BUILD_CORE)
  # Find VDB installation and determine lib/abi versions
  find_package(OpenVDB REQUIRED)
  # Check ABI version was found and explicitly error if attempting to build against
  # an incompatible Houdini version
  if(OpenVDB_ABI AND OPENVDB_HOUDINI_ABI)
    if(NOT ${OpenVDB_ABI} EQUAL ${OPENVDB_HOUDINI_ABI})
      message(FATAL_ERROR "Located OpenVDB installation is not ABI compatible with "
        "Houdini Version ${Houdini_VERSION}. Requires ABI ${OPENVDB_HOUDINI_ABI}, found "
        "ABI ${OpenVDB_ABI}.")
    endif()
  endif()
else()
  include("${CMAKE_CURRENT_LIST_DIR}/cmake/OpenVDBUtils.cmake")
  OPENVDB_VERSION_FROM_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/openvdb/version.h"
    VERSION OpenVDB_VERSION
    MAJOR   OpenVDB_MAJOR_VERSION
    MINOR   OpenVDB_MINOR_VERSION
    PATCH   OpenVDB_PATCH_VERSION
  )
  message(STATUS "Configuring for OpenVDB Version ${OpenVDB_VERSION}")
endif()

# Validate the OpenVDB ABI Version. If OpenVDB_ABI is not set, we're either building
# the core library OR the ABI hasn't been deduced from a VDB installation. Use the
# value from OPENVDB_ABI_VERSION_NUMBER, falling back to the lib major version number

if(NOT OpenVDB_ABI)
  if(OPENVDB_ABI_VERSION_NUMBER)
    set(OpenVDB_ABI ${OPENVDB_ABI_VERSION_NUMBER})
  else()
    set(OpenVDB_ABI ${OpenVDB_MAJOR_VERSION})
  endif()
endif()

# From the deduced ABI, check against the required ABI for Houdini (if set).
# Forcefully set the ABI to the required value if necessary - do this after to
# explicitly warn the user if their chosen value is different.

if(OPENVDB_HOUDINI_ABI AND (NOT "${OpenVDB_ABI}" EQUAL "${OPENVDB_HOUDINI_ABI}"))
  message(WARNING "CMake will explicitly set the value of OPENVDB_ABI_VERSION_NUMBER to "
    "${OPENVDB_HOUDINI_ABI} to match the ABI of the target Houdini Version.")
  set(OpenVDB_ABI ${OPENVDB_HOUDINI_ABI})
endif()

if(OpenVDB_ABI LESS MINIMUM_OPENVDB_ABI_VERSION)
  message(FATAL_ERROR "OpenVDB ABI versions earlier than ${MINIMUM_OPENVDB_ABI_VERSION} are "
    "no longer supported.")
endif()

if(FUTURE_MINIMUM_OPENVDB_ABI_VERSION AND OpenVDB_ABI LESS FUTURE_MINIMUM_OPENVDB_ABI_VERSION)
  if(OPENVDB_USE_DEPRECATED_ABI)
    message(DEPRECATION "OpenVDB ABI versions earlier than ${FUTURE_MINIMUM_OPENVDB_ABI_VERSION} "
      "are deprecated and will soon be removed.")
  else()
    message(FATAL_ERROR "OpenVDB ABI versions earlier than ${FUTURE_MINIMUM_OPENVDB_ABI_VERSION} "
      "are deprecated. Set OPENVDB_USE_DEPRECATED_ABI to ON to suppress this error.")
  endif()
  # global target definition
  add_definitions(-DOPENVDB_USE_DEPRECATED_ABI)
endif()

# The ABI is a global target definition, applicable to all components, so just add it
# via add_definitions()

if(OpenVDB_ABI EQUAL 3)
  add_definitions(-DOPENVDB_3_ABI_COMPATIBLE)
endif()

add_definitions(-DOPENVDB_ABI_VERSION_NUMBER=${OpenVDB_ABI})
message(STATUS "Configuring for OpenVDB ABI Version ${OpenVDB_ABI}")

# Always force set as we may need to change it if it's incompatible with Houdini
set(OPENVDB_ABI_VERSION_NUMBER ${OpenVDB_ABI} CACHE STRING [=[
Build for compatibility with version N of the OpenVDB Grid ABI, where N is 3, 4, 5 etc. (some newer features
will be disabled). If OPENVDB_BUILD_CORE is OFF, CMake attempts to query the installed vdb_print binary to
determine the ABI number. You may set this to force a given ABI number.]=] FORCE)

##########################################################################

if(OPENVDB_BUILD_CORE)
  add_subdirectory(openvdb)
endif()

if(OPENVDB_BUILD_PYTHON_MODULE)
  add_subdirectory(openvdb/python)
endif()

if(OPENVDB_BUILD_BINARIES)
  add_subdirectory(openvdb/cmd)
endif()

if(OPENVDB_BUILD_UNITTESTS)
  add_subdirectory(openvdb/unittest)
endif()

if(OPENVDB_BUILD_HOUDINI_PLUGIN)
  add_subdirectory(openvdb_houdini)
endif()

if(OPENVDB_BUILD_MAYA_PLUGIN)
  add_subdirectory(openvdb_maya)
endif()

##########################################################################

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/cmake/Uninstall.cmake
)
