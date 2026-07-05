aux_source_directories(SOURCES "" .)
vor_add_executable(test_HLBFGS ${SOURCES})
target_link_libraries(test_HLBFGS geogram)

set_target_properties(test_HLBFGS PROPERTIES FOLDER "GEOGRAM/Tests")

