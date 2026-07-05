# Find Geogram
# ------------
#
# Find Geogram include dirs and libraries
#
# This module defines the following variables:
#
#   Geogram_FOUND        - True if geogram has been found.
#   Geogram::geogram     - Imported target for the main Geogram library.
#   Geogram::geogram_gfx - Imported target for Geogram graphics library.
#
# This module reads hints about the Geogram location from the following
# environment variables:
#
#   GEOGRAM_INSTALL_PREFIX - Directory where Geogram is installed.
#
# Authors: Jeremie Dumas
#          Pierre Moulon
#          Bruno Levy

set (GEOGRAM_SEARCH_PATHS
  ${GEOGRAM_INSTALL_PREFIX}                
  "$ENV{GEOGRAM_INSTALL_PREFIX}"
  "/usr/local/"
  "$ENV{PROGRAMFILES}/Geogram"
  "$ENV{PROGRAMW6432}/Geogram"
)

set (GEOGRAM_SEARCH_PATHS_SYSTEM
  "/usr/lib"
  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
)

find_path (GEOGRAM_INCLUDE_DIR
                geogram/basic/common.h
                PATHS ${GEOGRAM_SEARCH_PATHS}
                PATH_SUFFIXES include/geogram1
)

find_library (GEOGRAM_LIBRARY
                NAMES geogram
                PATHS ${GEOGRAM_SEARCH_PATHS}
                PATH_SUFFIXES lib
)

find_library (GEOGRAM_GFX_LIBRARY
                NAMES geogram_gfx
                PATHS ${GEOGRAM_SEARCH_PATHS}
                PATH_SUFFIXES lib
)

# This one we search in both Geogram search path and
# system search path since it may be already installed
# in the system
find_library (GEOGRAM_GLFW3_LIBRARY
                NAMES glfw3 glfw geogram_glfw3 glfw3dll glfwdll
                PATHS ${GEOGRAM_SEARCH_PATHS} ${GEOGRAM_SEARCH_PATHS_SYSTEM}
                PATH_SUFFIXES lib
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GEOGRAM DEFAULT_MSG GEOGRAM_LIBRARY GEOGRAM_INCLUDE_DIR
)

# Create an imported target for Geogram 
If (GEOGRAM_FOUND)
  
        set(GEOGRAM_INSTALL_PREFIX ${GEOGRAM_INCLUDE_DIR}/../..)
  
        if (NOT TARGET Geogram::geogram)
                add_library (Geogram::geogram UNKNOWN IMPORTED)

                # Interface include directory
                set_target_Properties(Geogram::geogram PROPERTIES
                  INTERFACE_INCLUDE_DIRECTORIES "${GEOGRAM_INCLUDE_DIR}"
                )

                # Link to library file
                Set_Target_Properties(Geogram::geogram PROPERTIES
                  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                  IMPORTED_LOCATION "${GEOGRAM_LIBRARY}"
                )
        endif ()

        if (NOT TARGET Geogram::geogram_gfx)
                add_library (Geogram::geogram_gfx UNKNOWN IMPORTED)

                set_target_properties(Geogram::geogram_gfx PROPERTIES
                  INTERFACE_LINK_LIBRARIES ${GEOGRAM_GLFW3_LIBRARY}
                )

                # Interface include directory
                set_target_properties(Geogram::geogram_gfx PROPERTIES
                  INTERFACE_INCLUDE_DIRECTORIES "${GEOGRAM_INCLUDE_DIR}"
                )

                # Link to library file
                set_target_properties(Geogram::geogram_gfx PROPERTIES
                  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                  IMPORTED_LOCATION "${GEOGRAM_GFX_LIBRARY}"
                )
                
        endif ()

        
endif ()

# Hide variables from the default CMake-Gui options
mark_as_advanced (GEOGRAM_LIBRARY GEOGRAM_GFX_LIBRARY GEOGRAM_INCLUDE_DIR)

# Some specific settings for Windows

if(WIN32)

  # Default mode for Windows uses static libraries. Use this variable to
  # link with geogram compiled as DLLs.
  set(VORPALINE_BUILD_DYNAMIC FALSE CACHE BOOL "Installed Geogram uses DLLs")

  # remove warning for multiply defined symbols (caused by multiple
  # instanciations of STL templates)
  add_definitions(/wd4251)

  # remove all unused stuff from windows.h
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-DVC_EXTRALEAN)

  # do not define a min() and a max() macro, breaks
  # std::min() and std::max() !!
  add_definitions(-DNOMINMAX )

  # we want M_PI etc...
  add_definitions(-D_USE_MATH_DEFINES)

  if(NOT VORPALINE_BUILD_DYNAMIC) 
      # If we use static library, we link with the static C++ runtime.
      foreach(config ${CMAKE_CONFIGURATION_TYPES})
         string(TOUPPER ${config} config)
         string(REPLACE /MD /MT CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
         string(REPLACE /MD /MT CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}}")
      endforeach()
  endif()

endif()    
