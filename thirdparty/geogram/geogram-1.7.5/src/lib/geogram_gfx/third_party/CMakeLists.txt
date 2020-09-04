vor_reset_warning_level()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_definitions(-Wno-unknown-warning-option -Wno-zero-as-null-pointer-constant)
endif()


# Since we are in a CMake "object library" (i.e.,
# a set of object files compiled with same options),
# the automatically generated geogram_gfx_EXPORTS
# pre-processor definition of the parent library
# is not automatically inherited (this is why
# I set it manually)
if(BUILD_SHARED_LIBS)
   add_definitions(-Dgeogram_gfx_EXPORTS)
endif()

aux_source_directories(SOURCES "Source Files"            .)
aux_source_directories(SOURCES "Source Files\\glad"      glad)
aux_source_directories(SOURCES "Source Files\\ImGui"     ImGui)
aux_source_directories(SOURCES "Source Files\\ImGuiColorTextEdit" ImGuiColorTextEdit )

if(GEOGRAM_WITH_LUA)
   aux_source_directories(SOURCES "Source Files\\imgui_lua_bindings" imgui_lua_bindings)
endif()

add_library(geogram_gfx_third_party OBJECT ${SOURCES})

set_target_properties(
    geogram_gfx_third_party PROPERTIES
    FOLDER "GEOGRAM"
)

if(ANDROID)
  target_include_directories(geogram_gfx_third_party PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
endif()
