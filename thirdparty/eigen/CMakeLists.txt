cmake_minimum_required(VERSION 3.10.0)

#==============================================================================
# CMake Policy issues.
#==============================================================================
# Allow overriding options in a parent project via `set` before including Eigen.
if (POLICY CMP0077)
  cmake_policy (SET CMP0077 NEW)
endif (POLICY CMP0077)

# NOTE Remove setting the policy once the minimum required CMake version is
# increased to at least 3.15. Retain enabling the export to package registry.
if (POLICY CMP0090)
  # The export command does not populate package registry by default
  cmake_policy (SET CMP0090 NEW)
  # Unless otherwise specified, always export to package registry to ensure
  # backwards compatibility.
  if (NOT DEFINED CMAKE_EXPORT_PACKAGE_REGISTRY)
    set (CMAKE_EXPORT_PACKAGE_REGISTRY ON)
  endif (NOT DEFINED CMAKE_EXPORT_PACKAGE_REGISTRY)
endif (POLICY CMP0090)

# Disable warning about find_package(CUDA).
# CUDA language support is lacking for clang as the CUDA compiler
# until at least cmake version 3.18.  Even then, there seems to be
# issues on Windows+Ninja in passing build flags.  Continue using
# the "old" way for now.
if (POLICY CMP0146)
  cmake_policy(SET CMP0146 OLD)
endif ()

# Normalize DESTINATION paths
if (POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif ()

#==============================================================================
# CMake Project.
#==============================================================================

project(Eigen3)

# Remove this block after bumping CMake to v3.21.0
# PROJECT_IS_TOP_LEVEL is defined then by default
if(CMAKE_VERSION VERSION_LESS 3.21.0)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL ON)
  else()
    set(PROJECT_IS_TOP_LEVEL OFF)
  endif()
endif()

#==============================================================================
# Build ON/OFF Settings.
#==============================================================================
# Determine if we should build tests.
include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTING "Enable creation of tests." ON "PROJECT_IS_TOP_LEVEL" OFF)
option(EIGEN_BUILD_TESTING "Enable creation of Eigen tests." ${BUILD_TESTING})
option(EIGEN_LEAVE_TEST_IN_ALL_TARGET "Leaves tests in the all target, needed by ctest for automatic building." OFF)

# Determine if we should build BLAS/LAPACK implementations.
option(EIGEN_BUILD_BLAS "Toggles the building of the Eigen Blas library" ${PROJECT_IS_TOP_LEVEL})
option(EIGEN_BUILD_LAPACK "Toggles the building of the included Eigen LAPACK library" ${PROJECT_IS_TOP_LEVEL})
if (EIGEN_BUILD_BLAS OR EIGEN_BUILD_LAPACK)
  # Determine if we should build shared libraries for BLAS/LAPACK on this platform.
  if (NOT EIGEN_BUILD_SHARED_LIBS)
    get_cmake_property(EIGEN_BUILD_SHARED_LIBS TARGET_SUPPORTS_SHARED_LIBS)
  endif()
endif()

option(EIGEN_BUILD_BTL "Build benchmark suite" OFF)
option(EIGEN_BUILD_SPBENCH "Build sparse benchmark suite" OFF)
# Avoid building docs if included from another project.
# Building documentation requires creating and running executables on the host
# platform.  We shouldn't do this if cross-compiling.
if (PROJECT_IS_TOP_LEVEL AND NOT CMAKE_CROSSCOMPILING)
  set(EIGEN_BUILD_DOC_DEFAULT ON)
endif()
option(EIGEN_BUILD_DOC "Enable creation of Eigen documentation" ${EIGEN_BUILD_DOC_DEFAULT})

option(EIGEN_BUILD_DEMOS "Toggles the building of the Eigen demos" ${PROJECT_IS_TOP_LEVEL})

# Disable pkgconfig only for native Windows builds
if(NOT WIN32 OR NOT CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
  option(EIGEN_BUILD_PKGCONFIG "Build pkg-config .pc file for Eigen" ${PROJECT_IS_TOP_LEVEL})
endif()
option(EIGEN_BUILD_CMAKE_PACKAGE "Enables the creation of EigenConfig.cmake and related files" ${PROJECT_IS_TOP_LEVEL})

if (EIGEN_BUILD_TESTING OR EIGEN_BUILD_BLAS OR EIGEN_BUILD_LAPACK OR EIGEN_BUILT_BTL OR EIGEN_BUILD_BTL OR EIGEN_BUILD_SPBENCH OR EIGEN_BUILD_DOC OR EIGEN_BUILD_DEMOS)
  set(EIGEN_IS_BUILDING_ ON)
endif()

#==============================================================================
# Version Info.
#==============================================================================

# If version information is not provided, automatically parse the version number
# from header files.
file(READ "${PROJECT_SOURCE_DIR}/Eigen/Version" _eigen_version_header)
if (NOT DEFINED EIGEN_WORLD_VERSION)
  string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen_world_version_match "${_eigen_version_header}")
  set(EIGEN_WORLD_VERSION "${CMAKE_MATCH_1}")
endif()
if (NOT DEFINED EIGEN_MAJOR_VERSION)
  string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen_major_version_match "${_eigen_version_header}")
  set(EIGEN_MAJOR_VERSION "${CMAKE_MATCH_1}")
endif()
if (NOT DEFINED EIGEN_MINOR_VERSION)
  string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen_minor_version_match "${_eigen_version_header}")
  set(EIGEN_MINOR_VERSION "${CMAKE_MATCH_1}")
endif()
if (NOT DEFINED EIGEN_PATCH_VERSION)
  string(REGEX MATCH "define[ \t]+EIGEN_PATCH_VERSION[ \t]+([0-9]+)" _eigen_patch_version_match "${_eigen_version_header}")
  set(EIGEN_PATCH_VERSION "${CMAKE_MATCH_1}")
endif()
if (NOT DEFINED EIGEN_PRERELEASE_VERSION)
  set(EIGEN_PRERELEASE_VERSION "dev")
endif()

# If we are in a git repo, extract a changeset.
if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.git)
  # if the git program is absent or this will leave the EIGEN_GIT_REVNUM string empty,
  # but won't stop CMake.
  execute_process(COMMAND git ls-remote -q ${CMAKE_SOURCE_DIR} HEAD OUTPUT_VARIABLE EIGEN_GIT_OUTPUT)
endif()

# extract the git rev number from the git output...
if(EIGEN_GIT_OUTPUT)
  string(REGEX MATCH "^([0-9;a-f]+).*" EIGEN_GIT_CHANGESET_MATCH "${EIGEN_GIT_OUTPUT}")
  set(EIGEN_GIT_REVNUM "${CMAKE_MATCH_1}")
endif()

if (NOT DEFINED EIGEN_BUILD_VERSION AND DEFINED EIGEN_GIT_REVNUM)
  string(SUBSTRING "${EIGEN_GIT_REVNUM}" 0 8 EIGEN_BUILD_VERSION)
else()
  set(EIGEN_BUILD_VERSION "")
endif()

# The EIGEN_VERSION_NUMBER must be of the form <major.minor.patch>.
# The EIGEN_VERSION_STRING can contain the preprelease/build strings.
set(EIGEN_VERSION_NUMBER "${EIGEN_MAJOR_VERSION}.${EIGEN_MINOR_VERSION}.${EIGEN_PATCH_VERSION}")
set(EIGEN_VERSION_STRING "${EIGEN_VERSION_NUMBER}")
if (NOT "x${EIGEN_PRERELEASE_VERSION}" STREQUAL "x")
  set(EIGEN_VERSION_STRING "${EIGEN_VERSION_STRING}-${EIGEN_PRERELEASE_VERSION}")
endif()
if (NOT "x${EIGEN_BUILD_VERSION}" STREQUAL "x")
  set(EIGEN_VERSION_STRING "${EIGEN_VERSION_STRING}+${EIGEN_BUILD_VERSION}")
endif()


# Generate version file.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/Eigen/Version")

#==============================================================================
# Install Path Configuration.
#==============================================================================

# Unconditionally allow install of targets to support nested dependency
# installations.
#
# Note: projects that depend on Eigen should _probably_ exclude installing
# Eigen by default (e.g. by using EXCLUDE_FROM_ALL when using
# FetchContent_Declare or add_subdirectory) to avoid overwriting a previous
# installation.

include(GNUInstallDirs)
# Backward compatibility support for EIGEN_INCLUDE_INSTALL_DIR
if(EIGEN_INCLUDE_INSTALL_DIR)
  message(WARNING "EIGEN_INCLUDE_INSTALL_DIR is deprecated. Use INCLUDE_INSTALL_DIR instead.")
endif()

if(EIGEN_INCLUDE_INSTALL_DIR AND NOT INCLUDE_INSTALL_DIR)
  set(INCLUDE_INSTALL_DIR ${EIGEN_INCLUDE_INSTALL_DIR}
      CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where Eigen header files are installed")
else()
  set(INCLUDE_INSTALL_DIR
      "${CMAKE_INSTALL_INCLUDEDIR}/eigen3"
      CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where Eigen header files are installed"
      )
endif()
set(CMAKEPACKAGE_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/eigen3/cmake"
    CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where Eigen3Config.cmake is installed"
    )
set(PKGCONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/pkgconfig"
    CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where eigen3.pc is installed"
    )

foreach(var INCLUDE_INSTALL_DIR CMAKEPACKAGE_INSTALL_DIR PKGCONFIG_INSTALL_DIR)
  # If an absolute path is specified, make it relative to "{CMAKE_INSTALL_PREFIX}".
  if(IS_ABSOLUTE "${${var}}")
    file(RELATIVE_PATH "${var}" "${CMAKE_INSTALL_PREFIX}" "${${var}}")
  endif()
endforeach()

#==============================================================================
# Eigen Library.
#==============================================================================

# Alias Eigen_*_DIR to Eigen3_*_DIR:
set(Eigen_SOURCE_DIR ${Eigen3_SOURCE_DIR})
set(Eigen_BINARY_DIR ${Eigen3_BINARY_DIR})

# Imported target support
add_library (eigen INTERFACE)
add_library (Eigen3::Eigen ALIAS eigen)
target_include_directories (eigen INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

# Eigen requires at least C++14
target_compile_features (eigen INTERFACE cxx_std_14)

# Export as title case Eigen
set_target_properties (eigen PROPERTIES EXPORT_NAME Eigen)

#==============================================================================
# Install Rule Configuration.
#==============================================================================

install(FILES
  signature_of_eigen3_matrix_library
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
  )

if(EIGEN_BUILD_PKGCONFIG)
    configure_file(eigen3.pc.in eigen3.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/eigen3.pc
        DESTINATION ${PKGCONFIG_INSTALL_DIR})
endif()

install(DIRECTORY Eigen DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
# Replace the "Version" header file with the generated one.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Eigen/Version
    DESTINATION ${INCLUDE_INSTALL_DIR}/Eigen/ COMPONENT Devel)

install(TARGETS eigen EXPORT Eigen3Targets)

if(EIGEN_BUILD_CMAKE_PACKAGE)
  include (CMakePackageConfigHelpers)
  configure_package_config_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Eigen3Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Eigen3Config.cmake
    INSTALL_DESTINATION ${CMAKEPACKAGE_INSTALL_DIR}
    NO_SET_AND_CHECK_MACRO # Eigen does not provide legacy style defines
    NO_CHECK_REQUIRED_COMPONENTS_MACRO # Eigen does not provide components
  )

  set(CVF_VERSION "${EIGEN_VERSION_NUMBER}")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Eigen3ConfigVersion.cmake.in"
                 "Eigen3ConfigVersion.cmake"
                 @ONLY)

  # The Eigen target will be located in the Eigen3 namespace. Other CMake
  # targets can refer to it using Eigen3::Eigen.
  export (TARGETS eigen NAMESPACE Eigen3:: FILE Eigen3Targets.cmake)
  # Export Eigen3 package to CMake registry such that it can be easily found by
  # CMake even if it has not been installed to a standard directory.
  export (PACKAGE Eigen3)

  install (EXPORT Eigen3Targets NAMESPACE Eigen3:: DESTINATION ${CMAKEPACKAGE_INSTALL_DIR})

  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Eigen3Config.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/Eigen3ConfigVersion.cmake
          DESTINATION ${CMAKEPACKAGE_INSTALL_DIR})

  # Add uninstall target
  if(NOT TARGET uninstall AND PROJECT_IS_TOP_LEVEL)
    add_custom_target ( uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EigenUninstall.cmake)
  endif()
endif()

#==============================================================================
# General Build Configuration.
#==============================================================================

# Avoid setting the standard in a parent if unset.
if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD 14 CACHE STRING "Default C++ standard")
  set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard")
  set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "Allow C++ extensions")
endif()

# Guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# Guard against bad build-type strings
if (PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Only try to figure out how to link the math library if we are building something.
# Otherwise, let the parent project deal with dependencies.
if (EIGEN_IS_BUILDING_)
  # Use Eigen's cmake files.
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set(CMAKE_INCLUDE_CURRENT_DIR OFF)

  find_package(StandardMathLibrary)
  set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "")
  if(NOT STANDARD_MATH_LIBRARY_FOUND)
    message(FATAL_ERROR
      "Can't link to the standard math library. Please report to the Eigen developers, telling them about your platform.")
  else()
    if(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
      set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO} ${STANDARD_MATH_LIBRARY}")
    else()
      set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "${STANDARD_MATH_LIBRARY}")
    endif()
  endif()
  if(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
    message(STATUS "Standard libraries to link to explicitly: ${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO}")
  else()
    message(STATUS "Standard libraries to link to explicitly: none")
  endif()

  # Default tests/examples/libraries to row-major.
  option(EIGEN_DEFAULT_TO_ROW_MAJOR "Use row-major as default matrix storage order" OFF)
  if(EIGEN_DEFAULT_TO_ROW_MAJOR)
    add_definitions("-DEIGEN_DEFAULT_TO_ROW_MAJOR")
  endif()
endif()

#==============================================================================
# Test Configuration.
#==============================================================================

if (EIGEN_BUILD_TESTING)
  function(ei_maybe_separate_arguments variable mode args)
    # Use separate_arguments if the input is a single string containing a space.
    # Otherwise, if it is already a list or doesn't have a space, just propagate
    # the original value.  This is to better support multi-argument lists.
    list(LENGTH args list_length)
    if (${list_length} EQUAL 1)
      string(FIND "${args}" " " has_space)
      if (${has_space} GREATER -1)
        separate_arguments(args ${mode} "${args}")
      endif()
    endif()
    set(${variable} ${args} PARENT_SCOPE)
  endfunction(ei_maybe_separate_arguments)

  include(CheckCXXCompilerFlag)
  macro(ei_add_cxx_compiler_flag FLAG)
    string(REGEX REPLACE "-" "" SFLAG1 ${FLAG})
    string(REGEX REPLACE "\\+" "p" SFLAG ${SFLAG1})
    check_cxx_compiler_flag(${FLAG} COMPILER_SUPPORT_${SFLAG})
    if(COMPILER_SUPPORT_${SFLAG})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
  endmacro()

  set(EIGEN_TEST_CUSTOM_LINKER_FLAGS  "" CACHE STRING "Additional linker flags when linking unit tests.")
  set(EIGEN_TEST_CUSTOM_CXX_FLAGS     "" CACHE STRING "Additional compiler flags when compiling unit tests.")
  # Convert space-separated arguments into CMake lists for downstream consumption.
  ei_maybe_separate_arguments(EIGEN_TEST_CUSTOM_LINKER_FLAGS NATIVE_COMMAND "${EIGEN_TEST_CUSTOM_LINKER_FLAGS}")
  ei_maybe_separate_arguments(EIGEN_TEST_CUSTOM_CXX_FLAGS NATIVE_COMMAND "${EIGEN_TEST_CUSTOM_CXX_FLAGS}")

  option(EIGEN_SPLIT_LARGE_TESTS "Split large tests into smaller executables" ON)
  set(EIGEN_TEST_MAX_SIZE "320" CACHE STRING "Maximal matrix/vector size, default is 320")

  # Flags for tests.
  if(NOT MSVC)
    # We assume that other compilers are partly compatible with GNUCC

    # clang outputs some warnings for unknown flags that are not caught by check_cxx_compiler_flag
    # adding -Werror turns such warnings into errors
    check_cxx_compiler_flag("-Werror" COMPILER_SUPPORT_WERROR)
    if(COMPILER_SUPPORT_WERROR)
      set(CMAKE_REQUIRED_FLAGS "-Werror")
    endif()
    ei_add_cxx_compiler_flag("-pedantic")
    ei_add_cxx_compiler_flag("-Wall")
    ei_add_cxx_compiler_flag("-Wextra")
    # ei_add_cxx_compiler_flag("-Weverything") # clang
    ei_add_cxx_compiler_flag("-Wundef")
    ei_add_cxx_compiler_flag("-Wcast-align")
    ei_add_cxx_compiler_flag("-Wchar-subscripts")
    ei_add_cxx_compiler_flag("-Wnon-virtual-dtor")
    ei_add_cxx_compiler_flag("-Wunused-local-typedefs")
    ei_add_cxx_compiler_flag("-Wpointer-arith")
    ei_add_cxx_compiler_flag("-Wwrite-strings")
    ei_add_cxx_compiler_flag("-Wformat-security")
    ei_add_cxx_compiler_flag("-Wshorten-64-to-32")
    ei_add_cxx_compiler_flag("-Wlogical-op")
    ei_add_cxx_compiler_flag("-Wenum-conversion")
    ei_add_cxx_compiler_flag("-Wc++11-extensions")
    ei_add_cxx_compiler_flag("-Wdouble-promotion")
    # ei_add_cxx_compiler_flag("-Wconversion")
    ei_add_cxx_compiler_flag("-Wshadow")
    ei_add_cxx_compiler_flag("-Wno-psabi")
    ei_add_cxx_compiler_flag("-Wno-variadic-macros")
    ei_add_cxx_compiler_flag("-Wno-long-long")
    ei_add_cxx_compiler_flag("-fno-common")
    ei_add_cxx_compiler_flag("-fstrict-aliasing")
    ei_add_cxx_compiler_flag("-wd981")                    # disable ICC's "operands are evaluated in unspecified order" remark
    ei_add_cxx_compiler_flag("-wd2304")                   # disable ICC's "warning #2304: non-explicit constructor with single argument may cause implicit type conversion" produced by -Wnon-virtual-dtor

    # Clang emits warnings about unused flag.
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      ei_add_cxx_compiler_flag("-fno-check-new")
    endif()


    if(ANDROID_NDK)
      ei_add_cxx_compiler_flag("-pie")
      ei_add_cxx_compiler_flag("-fPIE")
    endif()

    set(CMAKE_REQUIRED_FLAGS "")

    option(EIGEN_TEST_SSE2 "Enable/Disable SSE2 in tests/examples" OFF)
    if(EIGEN_TEST_SSE2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
      message(STATUS "Enabling SSE2 in tests/examples")
    endif()

    option(EIGEN_TEST_SSE3 "Enable/Disable SSE3 in tests/examples" OFF)
    if(EIGEN_TEST_SSE3)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
      message(STATUS "Enabling SSE3 in tests/examples")
    endif()

    option(EIGEN_TEST_SSSE3 "Enable/Disable SSSE3 in tests/examples" OFF)
    if(EIGEN_TEST_SSSE3)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
      message(STATUS "Enabling SSSE3 in tests/examples")
    endif()

    option(EIGEN_TEST_SSE4_1 "Enable/Disable SSE4.1 in tests/examples" OFF)
    if(EIGEN_TEST_SSE4_1)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
      message(STATUS "Enabling SSE4.1 in tests/examples")
    endif()

    option(EIGEN_TEST_SSE4_2 "Enable/Disable SSE4.2 in tests/examples" OFF)
    if(EIGEN_TEST_SSE4_2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
      message(STATUS "Enabling SSE4.2 in tests/examples")
    endif()

    option(EIGEN_TEST_AVX "Enable/Disable AVX in tests/examples" OFF)
    if(EIGEN_TEST_AVX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
      message(STATUS "Enabling AVX in tests/examples")
    endif()

    option(EIGEN_TEST_FMA "Enable/Disable FMA in tests/examples" OFF)
    if(EIGEN_TEST_FMA AND NOT EIGEN_TEST_NEON)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
      message(STATUS "Enabling FMA in tests/examples")
    endif()

    option(EIGEN_TEST_AVX2 "Enable/Disable AVX2 in tests/examples" OFF)
    if(EIGEN_TEST_AVX2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
      message(STATUS "Enabling AVX2 in tests/examples")
    endif()

    option(EIGEN_TEST_AVX512 "Enable/Disable AVX512 in tests/examples" OFF)
    if(EIGEN_TEST_AVX512)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f -mfma")
      message(STATUS "Enabling AVX512 in tests/examples")
    endif()

    option(EIGEN_TEST_AVX512DQ "Enable/Disable AVX512DQ in tests/examples" OFF)
    if(EIGEN_TEST_AVX512DQ)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512dq -mfma")
      message(STATUS "Enabling AVX512DQ in tests/examples")
    endif()

    option(EIGEN_TEST_AVX512FP16 "Enable/Disable AVX512-FP16 in tests/examples" OFF)
    if(EIGEN_TEST_AVX512FP16)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f -mfma -mavx512vl -mavx512fp16")
    message(STATUS "Enabling AVX512-FP16 in tests/examples")
    endif()

    option(EIGEN_TEST_F16C "Enable/Disable F16C in tests/examples" OFF)
    if(EIGEN_TEST_F16C)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mf16c")
      message(STATUS "Enabling F16C in tests/examples")
    endif()

    option(EIGEN_TEST_ALTIVEC "Enable/Disable AltiVec in tests/examples" OFF)
    if(EIGEN_TEST_ALTIVEC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maltivec -mabi=altivec")
      message(STATUS "Enabling AltiVec in tests/examples")
    endif()

    option(EIGEN_TEST_VSX "Enable/Disable VSX in tests/examples" OFF)
    if(EIGEN_TEST_VSX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -mvsx")
      message(STATUS "Enabling VSX in tests/examples")
    endif()

    option(EIGEN_TEST_MSA "Enable/Disable MSA in tests/examples" OFF)
    if(EIGEN_TEST_MSA)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmsa")
      message(STATUS "Enabling MSA in tests/examples")
    endif()

   option(EIGEN_TEST_LSX "Enable/Disable LSX in tests/examples" OFF)
   if(EIGEN_TEST_LSX)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlsx")
     message(STATUS "Enabling LSX in tests/examples")
   endif()

    option(EIGEN_TEST_NEON "Enable/Disable Neon in tests/examples" OFF)
    if(EIGEN_TEST_NEON)
      if(EIGEN_TEST_FMA)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-vfpv4")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
      endif()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=hard")
      message(STATUS "Enabling NEON in tests/examples")
    endif()

    option(EIGEN_TEST_NEON64 "Enable/Disable Neon in tests/examples" OFF)
    if(EIGEN_TEST_NEON64)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      message(STATUS "Enabling NEON in tests/examples")
    endif()

    option(EIGEN_TEST_Z13 "Enable/Disable S390X(zEC13) ZVECTOR in tests/examples" OFF)
    if(EIGEN_TEST_Z13)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=z13 -mzvector")
      message(STATUS "Enabling S390X(zEC13) ZVECTOR in tests/examples")
    endif()

    option(EIGEN_TEST_Z14 "Enable/Disable S390X(zEC14) ZVECTOR in tests/examples" OFF)
    if(EIGEN_TEST_Z14)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=z14 -mzvector")
      message(STATUS "Enabling S390X(zEC13) ZVECTOR in tests/examples")
    endif()

    check_cxx_compiler_flag("-fopenmp" COMPILER_SUPPORT_OPENMP)
    if(COMPILER_SUPPORT_OPENMP)
      option(EIGEN_TEST_OPENMP "Enable/Disable OpenMP in tests/examples" OFF)
      if(EIGEN_TEST_OPENMP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
        message(STATUS "Enabling OpenMP in tests/examples")
      endif()
    endif()

  else()
    # C4127 - conditional expression is constant
    # C4714 - marked as __forceinline not inlined (I failed to deactivate it selectively)
    #         We can disable this warning in the unit tests since it is clear that it occurs
    #         because we are oftentimes returning objects that have a destructor or may
    #         throw exceptions - in particular in the unit tests we are throwing extra many
    #         exceptions to cover indexing errors.
    # C4505 - unreferenced local function has been removed (impossible to deactivate selectively)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /wd4127 /wd4505 /wd4714")

    # replace all /Wx by /W4
    string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    check_cxx_compiler_flag("/openmp" COMPILER_SUPPORT_OPENMP)
    if(COMPILER_SUPPORT_OPENMP)
      option(EIGEN_TEST_OPENMP "Enable/Disable OpenMP in tests/examples" OFF)
      if(EIGEN_TEST_OPENMP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
        message(STATUS "Enabling OpenMP in tests/examples")
      endif()
    endif()

    option(EIGEN_TEST_SSE2 "Enable/Disable SSE2 in tests/examples" OFF)
    if(EIGEN_TEST_SSE2)
      if(NOT CMAKE_CL_64)
        # arch is not supported on 64 bit systems, SSE is enabled automatically.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
      endif()
      message(STATUS "Enabling SSE2 in tests/examples")
    endif()

    option(EIGEN_TEST_AVX "Enable/Disable AVX in tests/examples" OFF)
    if(EIGEN_TEST_AVX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
      message(STATUS "Enabling AVX in tests/examples")
    endif()

    option(EIGEN_TEST_FMA "Enable/Disable FMA/AVX2 in tests/examples" OFF)
    option(EIGEN_TEST_AVX2 "Enable/Disable FMA/AVX2 in tests/examples" OFF)
    if((EIGEN_TEST_FMA AND NOT EIGEN_TEST_NEON) OR EIGEN_TEST_AVX2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
      message(STATUS "Enabling FMA/AVX2 in tests/examples")
    endif()

    option(EIGEN_TEST_AVX512 "Enable/Disable AVX512 in tests/examples" OFF)
    option(EIGEN_TEST_AVX512DQ "Enable/Disable AVX512DQ in tests/examples" OFF)
    if(EIGEN_TEST_AVX512 OR EIGEN_TEST_AVX512DQ)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX512")
      message(STATUS "Enabling AVX512 in tests/examples")
    endif()

  endif(NOT MSVC)

  option(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION "Disable explicit vectorization in tests/examples" OFF)
  option(EIGEN_TEST_X87 "Force using X87 instructions. Implies no vectorization." OFF)
  option(EIGEN_TEST_32BIT "Force generating 32bit code." OFF)

  if(EIGEN_TEST_X87)
    set(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION ON)
    if(CMAKE_COMPILER_IS_GNUCXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=387")
      message(STATUS "Forcing use of x87 instructions in tests/examples")
    else()
      message(STATUS "EIGEN_TEST_X87 ignored on your compiler")
    endif()
  endif()

  if(EIGEN_TEST_32BIT)
    if(CMAKE_COMPILER_IS_GNUCXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
      message(STATUS "Forcing generation of 32-bit code in tests/examples")
    else()
      message(STATUS "EIGEN_TEST_32BIT ignored on your compiler")
    endif()
  endif()

  if(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION)
    add_definitions(-DEIGEN_DONT_VECTORIZE=1)
    message(STATUS "Disabling vectorization in tests/examples")
  endif()

  option(EIGEN_TEST_NO_EXPLICIT_ALIGNMENT "Disable explicit alignment (hence vectorization) in tests/examples" OFF)
  if(EIGEN_TEST_NO_EXPLICIT_ALIGNMENT)
    add_definitions(-DEIGEN_DONT_ALIGN=1)
    message(STATUS "Disabling alignment in tests/examples")
  endif()

  option(EIGEN_TEST_NO_EXCEPTIONS "Disables C++ exceptions" OFF)
  if(EIGEN_TEST_NO_EXCEPTIONS)
    ei_add_cxx_compiler_flag("-fno-exceptions")
    message(STATUS "Disabling exceptions in tests/examples")
  endif()

  set(EIGEN_CUDA_CXX_FLAGS "" CACHE STRING "Additional flags to pass to the cuda compiler.")
  set(EIGEN_CUDA_COMPUTE_ARCH 30 CACHE STRING "The CUDA compute architecture(s) to target when compiling CUDA code")

  option(EIGEN_TEST_SYCL "Add Sycl support." OFF)
  if(EIGEN_TEST_SYCL)
    option(EIGEN_SYCL_DPCPP "Use the DPCPP Sycl implementation (DPCPP is default SYCL-Compiler)." ON)
    option(EIGEN_SYCL_TRISYCL "Use the triSYCL Sycl implementation." OFF)
    option(EIGEN_SYCL_ComputeCpp "Use the ComputeCPP Sycl implementation." OFF)

    # Building options
    # https://developer.codeplay.com/products/computecpp/ce/2.11.0/guides/eigen-overview/options-for-building-eigen-sycl
    option(EIGEN_SYCL_USE_DEFAULT_SELECTOR "Use sycl default selector to select the preferred device." OFF)
    option(EIGEN_SYCL_NO_LOCAL_MEM "Build for devices without dedicated shared memory." OFF)
    option(EIGEN_SYCL_LOCAL_MEM "Allow the use of local memory (enabled by default)." ON)
    option(EIGEN_SYCL_LOCAL_THREAD_DIM0 "Set work group size for dimension 0." 16)
    option(EIGEN_SYCL_LOCAL_THREAD_DIM1 "Set work group size for dimension 1." 16)
    option(EIGEN_SYCL_ASYNC_EXECUTION "Allow asynchronous execution (enabled by default)." ON)
    option(EIGEN_SYCL_DISABLE_SKINNY "Disable optimization for tall/skinny matrices." OFF)
    option(EIGEN_SYCL_DISABLE_DOUBLE_BUFFER "Disable double buffer." OFF)
    option(EIGEN_SYCL_DISABLE_SCALAR "Disable scalar contraction." OFF)
    option(EIGEN_SYCL_DISABLE_GEMV "Disable GEMV and create a single kernel to calculate contraction instead." OFF)

    set(EIGEN_SYCL ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-shorten-64-to-32 -Wno-cast-align")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy-with-user-provided-copy -Wno-unused-variable")
    set (CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules" "cmake/Modules/" "${CMAKE_MODULE_PATH}")
    find_package(Threads REQUIRED)
    if(EIGEN_SYCL_TRISYCL)
      message(STATUS "Using triSYCL")
      include(FindTriSYCL)
    elseif(EIGEN_SYCL_ComputeCpp)
      message(STATUS "Using ComputeCPP SYCL")
      include(FindComputeCpp)
      set(COMPUTECPP_DRIVER_DEFAULT_VALUE OFF)
      if (NOT MSVC)
        set(COMPUTECPP_DRIVER_DEFAULT_VALUE ON)
      endif()
      option(COMPUTECPP_USE_COMPILER_DRIVER
              "Use ComputeCpp driver instead of a 2 steps compilation"
              ${COMPUTECPP_DRIVER_DEFAULT_VALUE}
              )
    else() #Default SYCL compiler is DPCPP (EIGEN_SYCL_DPCPP)
      set(DPCPP_SYCL_TARGET "spir64" CACHE STRING "Default target for Intel CPU/GPU")
      message(STATUS "Using DPCPP")
      find_package(DPCPP)
      add_definitions(-DSYCL_COMPILER_IS_DPCPP)
    endif(EIGEN_SYCL_TRISYCL)
    if(EIGEN_DONT_VECTORIZE_SYCL)
      message(STATUS "Disabling SYCL vectorization in tests/examples")
      # When disabling SYCL vectorization, also disable Eigen default vectorization
      add_definitions(-DEIGEN_DONT_VECTORIZE=1)
      add_definitions(-DEIGEN_DONT_VECTORIZE_SYCL=1)
    endif()
  endif()

  include(EigenConfigureTesting)

  if(EIGEN_LEAVE_TEST_IN_ALL_TARGET)
    # CTest automatic test building relies on the "all" target.
    add_subdirectory(test)
    add_subdirectory(failtest)
  else()
    add_subdirectory(test EXCLUDE_FROM_ALL)
    add_subdirectory(failtest EXCLUDE_FROM_ALL)
  endif()

  ei_testing_print_summary()

  if (EIGEN_SPLIT_TESTSUITE)
    ei_split_testsuite("${EIGEN_SPLIT_TESTSUITE}")
  endif()
endif(EIGEN_BUILD_TESTING)

#==============================================================================
# Other Build Configurations.
#==============================================================================
add_subdirectory(unsupported)

if(EIGEN_BUILD_BLAS)
  add_subdirectory(blas)
endif()

if (EIGEN_BUILD_LAPACK)
  add_subdirectory(lapack)
endif()

if(EIGEN_BUILD_DOC)
  add_subdirectory(doc EXCLUDE_FROM_ALL)
endif()

# TODO: consider also replacing EIGEN_BUILD_BTL by a custom target "make btl"?
if(EIGEN_BUILD_BTL)
  add_subdirectory(bench/btl EXCLUDE_FROM_ALL)
endif()

if(NOT WIN32 AND EIGEN_BUILD_SPBENCH)
  add_subdirectory(bench/spbench EXCLUDE_FROM_ALL)
endif()

if (EIGEN_BUILD_DEMOS)
  add_subdirectory(demos EXCLUDE_FROM_ALL)
endif()

if (PROJECT_IS_TOP_LEVEL)
  # must be after test and unsupported, for configuring buildtests.in
  add_subdirectory(scripts EXCLUDE_FROM_ALL)
  configure_file(scripts/cdashtesting.cmake.in cdashtesting.cmake @ONLY)
endif()

#==============================================================================
# Summary.
#==============================================================================

if(PROJECT_IS_TOP_LEVEL)
  string(TOLOWER "${CMAKE_GENERATOR}" cmake_generator_tolower)
  if(cmake_generator_tolower MATCHES "makefile")
    message(STATUS "Available targets (use: make TARGET):")
  else()
    message(STATUS "Available targets (use: cmake --build . --target TARGET):")
  endif()
  message(STATUS "------------+--------------------------------------------------------------")
  message(STATUS "Target      |   Description")
  message(STATUS "------------+--------------------------------------------------------------")
  message(STATUS "install     | Install Eigen. Headers will be installed to:")
  message(STATUS "            |     <CMAKE_INSTALL_PREFIX>/<INCLUDE_INSTALL_DIR>")
  message(STATUS "            |   Using the following values:")
  message(STATUS "            |     CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "            |     INCLUDE_INSTALL_DIR:  ${INCLUDE_INSTALL_DIR}")
  message(STATUS "            |   Change the install location of Eigen headers using:")
  message(STATUS "            |     cmake . -DCMAKE_INSTALL_PREFIX=yourprefix")
  message(STATUS "            |   Or:")
  message(STATUS "            |     cmake . -DINCLUDE_INSTALL_DIR=yourdir")
  message(STATUS "uninstall   | Remove files installed by the install target")
  if (EIGEN_BUILD_DOC)
    message(STATUS "doc         | Generate the API documentation, requires Doxygen & LaTeX")
    message(STATUS "install-doc | Install the API documentation")
  endif()
  if(EIGEN_BUILD_TESTING)
    message(STATUS "check       | Build and run the unit-tests. Read this page:")
    message(STATUS "            |   http://eigen.tuxfamily.org/index.php?title=Tests")
  endif()
  if (EIGEN_BUILD_BLAS)
    message(STATUS "blas        | Build BLAS library (not the same thing as Eigen)")
  endif()
  if (EIGEN_BUILD_LAPACK)
    message(STATUS "lapack      | Build LAPACK subset library (not the same thing as Eigen)")
  endif()
  message(STATUS "------------+--------------------------------------------------------------")
  message(STATUS "")
endif()

message(STATUS "")
message(STATUS "Configured Eigen ${EIGEN_VERSION_STRING}")
message(STATUS "")
