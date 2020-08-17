
vor_reset_warning_level()

if(WIN32)
#  TODO: there are probably some MSVC warnings to 
#  deactivate here...
else()
   add_definitions(-Wno-unused -Wno-sign-compare -Wno-strict-aliasing)
endif()


if(GEOGRAM_WITH_GRAPHICS AND NOT ANDROID)
  
  if(GEOGRAM_WITH_EMSCRIPTEN)
    set(GEOGRAM_USE_BUILTIN_GLFW3 FALSE)
  else()
    if(GEOGRAM_USE_SYSTEM_GLFW3)
        find_package(glfw3 QUIET)
    endif()
    if(glfw3_FOUND)
      message(STATUS "Found GLFW3 installed in the system, using it.")
      set(GEOGRAM_USE_BUILTIN_GLFW3 FALSE PARENT_SCOPE)
    else()
      if(TARGET glfw)
         message(STATUS "Geogram is included in another project that already has GLFW3, using it.")	
      else()
         message(STATUS "Did not find GLFW3 in the system, using built-in GLFW3.")
         set(GEOGRAM_USE_BUILTIN_GLFW3 TRUE PARENT_SCOPE)
         set(GLFW_INSTALL TRUE CACHE BOOL "Generate GLFW installation target")
         add_subdirectory(glfw)
      endif()	 
    endif()
  endif()
  
endif()

# BLAS,LAPACK,CHOLDMOD,SUPERLU and ARPACK
# used as plug-ins by OpenNL

if(GEOGRAM_WITH_LEGACY_NUMERICS)
   add_subdirectory(numerics)
endif()
