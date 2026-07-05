include(${PROJECT_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "" .)
add_executable(geobox ${SOURCES})
target_link_libraries(geobox geogram_gfx geogram ${GLFW_LIBRARIES})
install_runtime_targets(geobox)

set_target_properties(geobox PROPERTIES FOLDER "GEOGRAM/Programs")
