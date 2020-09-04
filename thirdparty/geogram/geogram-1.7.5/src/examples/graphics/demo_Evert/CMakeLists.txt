include(${PROJECT_SOURCE_DIR}/cmake/opengl.cmake)
set(APP_NAME geogram_demo_Evert)

aux_source_directories(SOURCES "" .)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} geogram_gfx geogram ${GLFW_LIBRARIES})
install_runtime_targets(${APP_NAME})

set_target_properties(${APP_NAME} PROPERTIES FOLDER "GEOGRAM/Examples")
