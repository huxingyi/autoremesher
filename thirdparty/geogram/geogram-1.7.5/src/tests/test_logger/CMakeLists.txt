aux_source_directories(SOURCES "" .)
vor_add_executable(test_logger ${SOURCES})
target_link_libraries(test_logger geogram)

set_target_properties(test_logger PROPERTIES FOLDER "GEOGRAM/Tests")
