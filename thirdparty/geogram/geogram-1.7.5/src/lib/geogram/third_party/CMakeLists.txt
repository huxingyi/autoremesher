vor_reset_warning_level()

if(WIN32)
  add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
else()
  add_definitions(-Wno-deprecated) # PoissonRecon uses hash map
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions(-Wno-maybe-uninitialized)
  endif()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-Wno-unknown-warning-option -Wno-zero-as-null-pointer-constant)
  endif()
endif()


# Since we are in a CMake "object library" (i.e.,
# a set of object files compiled with same options),
# the automatically generated geogram_EXPORTS
# pre-processor definition of the parent library
# is not automatically inherited (this is why
# I set it manually)
if(BUILD_SHARED_LIBS)
   add_definitions(-Dgeogram_EXPORTS)
   if(WIN32)
# For Windows, nothing to do, because
# LUA_DL_DLL is by-default activated in luaconf.h
   else()
      add_definitions(-DLUA_USE_DLOPEN)   
   endif()
endif()

aux_source_directories(SOURCES "Source Files"               .)
aux_source_directories(SOURCES "Source Files\\LM6"          LM7)
aux_source_directories(SOURCES "Source Files\\rply"         rply)
aux_source_directories(SOURCES "Source Files\\shewchuk"     shewchuk)
aux_source_directories(SOURCES "Source Files\\zlib"         zlib)
aux_source_directories(SOURCES "Source Files\\gzstream"     gzstream)
aux_source_directories(SOURCES "Source Files\\PoissonRecon" PoissonRecon)
aux_source_directories(SOURCES "Source Files\\xatlas"       xatlas)

if(GEOGRAM_WITH_LUA)
   aux_source_directories(SOURCES "Source Files\\lua"          lua)
endif()

if(GEOGRAM_WITH_HLBFGS)
    aux_source_directories(SOURCES "Source Files\\HLBFGS"       HLBFGS)
endif()

if(GEOGRAM_WITH_TETGEN)
    aux_source_directories(SOURCES "Source Files\\tetgen" tetgen)
endif()

if(GEOGRAM_WITH_TRIANGLE)
    aux_source_directories(SOURCES "Source Files\\triangle" triangle)
endif()

add_library(geogram_third_party OBJECT ${SOURCES})

if(ANDROID)
  target_include_directories(geogram_third_party PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
  message(STATUS "building for Android")
endif()  


set_target_properties(
    geogram_third_party PROPERTIES
    FOLDER "GEOGRAM"
)
