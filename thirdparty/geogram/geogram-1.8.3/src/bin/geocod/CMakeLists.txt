include(${PROJECT_SOURCE_DIR}/cmake/opengl.cmake)
set(APP_NAME geocod)

aux_source_directories(SOURCES "" .)
aux_source_directories(SOURCES "lua" lua)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} geogram_gfx geogram ${GLFW_LIBRARIES})
install_runtime_targets(${APP_NAME})

set_target_properties(${APP_NAME} PROPERTIES FOLDER "GEOGRAM/Programs")
