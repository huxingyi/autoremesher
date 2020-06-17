cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)


# Set and enforce C++-11 flags
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
set( CMAKE_CXX_STANDARD 11 )

enable_testing()

# Only set project name if OpenMesh is built as stand-alone library
if("${PROJECT_NAME}" STREQUAL "")
    project (OpenMesh)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9")
		message(WARNING "Your version of GCC contains an optimizer bug. Please verify that you do not use -O3!")
		string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE_NEW "${CMAKE_CXX_FLAGS_RELEASE}")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_NEW}" CACHE STRING "" FORCE)
	endif()
endif()

if ( WIN32 )
  # This is the base directory for windows library search used in the finders we ship.
  set(CMAKE_WINDOWS_LIBS_DIR "c:/libs" CACHE STRING "Default Library search dir on windows." )
endif()

if (NOT WIN32 AND NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# add our macro directory to cmake search path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_DEBUG_POSTFIX "d")

# include our cmake files
include (ACGCommon)

acg_get_version ()


# Disable package building when built as an external library
if(${PROJECT_NAME} MATCHES "OpenMesh")
    include(OpenMeshPackage)
endif()

include (ACGOutput)
include(ACGQt)

# ========================================================================
# Definitions
# ========================================================================

if (WIN32)
  add_definitions(
      -D_USE_MATH_DEFINES -DNOMINMAX
      -D_CRT_SECURE_NO_WARNINGS
  )
endif ()

set(NO_DECREMENT_DEPRECATED_WARNINGS OFF CACHE BOOL "Disables all deprecated warnings warning about decrement operations on normal circulators.")
if(NO_DECREMENT_DEPRECATED_WARNINGS)
  add_definitions( -DNO_DECREMENT_DEPRECATED_WARNINGS )
endif()

# ========================================================================
# Windows build style control
# ========================================================================

if ( WIN32 )
  if ( NOT DEFINED OPENMESH_BUILD_SHARED )
    set( OPENMESH_BUILD_SHARED false CACHE BOOL "Build as shared library(DLL)?" )
  endif()
endif()

# ========================================================================
# Add bundle targets here
# ========================================================================
if ( NOT DEFINED BUILD_APPS )
  set( BUILD_APPS true CACHE BOOL "Enable or disable building of apps" )
endif()

# Only call fixbundle, when we are building OpenMesh standalone
if( (${PROJECT_NAME} MATCHES "OpenMesh") AND BUILD_APPS )

  if (WIN32)
    if ( NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles" AND BUILD_APPS )
      add_custom_target (fixbundle ALL
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/fixbundle.win.cmake" )
    endif()
  endif()

  if (APPLE)
    add_custom_target (fixbundle ALL
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/fixbundle.cmake"
    )
  endif()

endif() # project OpenMesh

# ========================================================================
# Call the subdirectories with there projects
# ========================================================================

add_subdirectory (src/OpenMesh/Core)
add_subdirectory (src/OpenMesh/Tools)
add_subdirectory (src/OpenMesh/Apps)

set(OPENMESH_BENCHMARK_DIR CACHE PATH "Source path of benchmark (https://github.com/google/benchmark).")
if (OPENMESH_BENCHMARK_DIR)
	add_subdirectory(${OPENMESH_BENCHMARK_DIR} benchmark)
	add_subdirectory(src/Benchmark)
endif()

# Do not build unit tests when build as external library
if(${PROJECT_NAME} MATCHES "OpenMesh")
    add_subdirectory (src/Unittests)
else()
	# If built as a dependent project simulate effects of
	# successful finder run:
	set (OPENMESH_FOUND true PARENT_SCOPE)
	set (OPENMESH_LIBRARIES OpenMeshCore OpenMeshTools PARENT_SCOPE)
	set (OPENMESH_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)
	set (OPENMESH_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)

	# Also define variables provided by the old legacy finder.
	set (OPENMESH_CORE_LIBRARY OpenMeshCore PARENT_SCOPE)
	set (OPENMESH_TOOLS_LIBRARY OpenMeshTools PARENT_SCOPE)
	set (OPENMESH_TOOLS_LIBRARY OpenMeshTools PARENT_SCOPE)
	set (OPENMESH_LIBRARY OpenMeshCore OpenMeshTools PARENT_SCOPE)
	get_target_property(_OPENMESH_LIBRARY_DIR OpenMeshCore LIBRARY_OUTPUT_DIRECTORY)
	set (OPENMESH_LIBRARY_DIR "${_OPENMESH_LIBRARY_DIR}" CACHE PATH "The directory where the OpenMesh libraries can be found.")
endif()

if ( NOT DEFINED OPENMESH_DOCS )
  set( OPENMESH_DOCS true CACHE BOOL "Enable or disable building of documentation" )
endif()

if (OPENMESH_DOCS)
  add_subdirectory (Doc)
endif()

# ========================================================================
# Bundle generation (Targets exist, now configure them)
# ========================================================================

# Only call fixbundle, when we are building OpenMesh standalone
if(${PROJECT_NAME} MATCHES "OpenMesh")

  if (WIN32 AND  BUILD_APPS )
    # prepare bundle generation cmake file and add a build target for it
    configure_file ("${CMAKE_SOURCE_DIR}/cmake/fixbundle.cmake.win.in"
       "${CMAKE_BINARY_DIR}/fixbundle.win.cmake" @ONLY IMMEDIATE)

    if ( NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles" )
      # let bundle generation depend on all targets
      add_dependencies (fixbundle QtViewer DecimaterGui)
    endif()
 
  endif()
 
  # On apple we do a fixbundle, which is only necessary for the apps and not for the libs
  if (APPLE AND BUILD_APPS)
    # prepare bundle generation cmake file and add a build target for it
    configure_file ("${CMAKE_SOURCE_DIR}/cmake/fixbundle.cmake.in"
       "${CMAKE_BINARY_DIR}/fixbundle.cmake" @ONLY IMMEDIATE)

    # let bundle generation depend on all targets
    add_dependencies (fixbundle DecimaterGui ProgViewer QtViewer SubdividerGui) 

    # Required for Snow leopard, and the latest qt. Then the resources have to be copied
    if ( EXISTS "/opt/local/libexec/qt4-mac/lib/QtGui.framework/Versions/4/Resources/qt_menu.nib" )
        add_custom_command(TARGET OpenMesh POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy_directory "/opt/local/libexec/qt4-mac/lib/QtGui.framework/Versions/4/Resources/qt_menu.nib" 
                                                      "${CMAKE_BINARY_DIR}/Build/Libraries/qt_menu.nib" )
    endif ()
  endif ()

endif()

# ========================================================================

# Generate openmesh.pc file

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "-lOpenMeshCore -lOpenMeshTools")

configure_file("openmesh.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/openmesh.pc" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/openmesh.pc DESTINATION libdata/pkgconfig)

# generate target file

install(EXPORT OpenMeshConfig DESTINATION share/OpenMesh/cmake)

export(TARGETS OpenMeshCore OpenMeshTools FILE OpenMeshConfig.cmake)

# display results
acg_print_configure_header (OPENMESH "OpenMesh")
