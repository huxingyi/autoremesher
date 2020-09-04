include(${PROJECT_SOURCE_DIR}/cmake/opengl.cmake)

configure_file(geogram_gfx.pc.in ${PROJECT_BINARY_DIR}/geogram_gfx${VORPALINE_VERSION_MAJOR}.pc @ONLY)

add_subdirectory(third_party)

aux_source_directories(SOURCES "Source Files"                .)
aux_source_directories(SOURCES "Source Files\\ImGui_ext"     ImGui_ext)
aux_source_directories(SOURCES "Source Files\\api"           api)
aux_source_directories(SOURCES "Source Files\\basic"         basic)
aux_source_directories(SOURCES "Source Files\\mesh"          mesh)
aux_source_directories(SOURCES "Source Files\\GLUP"          GLUP)
aux_source_directories(SOURCES "Source Files\\GLUP\\shaders" GLUP/shaders)
aux_source_directories(SOURCES "Source Files\\full_screen_effects" full_screen_effects)
aux_source_directories(SOURCES "Source Files\\gui"           gui)

if(GEOGRAM_WITH_LUA)
   aux_source_directories(SOURCES "Source Files\\lua"           lua)
endif()

if(GEOGRAM_USE_SYSTEM_GLFW3)
  add_definitions(-DGEO_USE_SYSTEM_GLFW3)
endif()

add_library(geogram_gfx ${SOURCES} $<TARGET_OBJECTS:geogram_gfx_third_party>)

if(ANDROID)
  target_include_directories(geogram_gfx PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
  message(STATUS "building for Android")
endif()  

set_target_properties(geogram_gfx PROPERTIES
                      VERSION ${VORPALINE_VERSION}
                      SOVERSION ${VORPALINE_VERSION_MAJOR}
		      FOLDER "GEOGRAM")

if(NOT ANDROID)
  target_link_libraries(geogram_gfx glfw)
endif()

if(VORPALINE_BUILD_DYNAMIC)
    # Must provide all symbols at link time
    target_link_libraries(geogram_gfx geogram)
    target_link_libraries(geogram_gfx ${OPENGL_LIBRARIES})
endif()

# Install the library
install_devkit_targets(geogram_gfx)

# Install include files for the standard devkit
install(
    DIRECTORY .
    DESTINATION include/${VORPALINE_INCLUDE_SUBPATH}/geogram_gfx
    COMPONENT devkit
    FILES_MATCHING PATTERN *.h
)

# Install include files for the full devkit
install(
    DIRECTORY .
    DESTINATION include/${VORPALINE_INCLUDE_SUBPATH}/geogram_gfx
    COMPONENT devkit-full
    FILES_MATCHING PATTERN *.h
)

install(
        FILES "${PROJECT_BINARY_DIR}/geogram_gfx${VORPALINE_VERSION_MAJOR}.pc"
        DESTINATION lib${LIB_SUFFIX}/pkgconfig
)


