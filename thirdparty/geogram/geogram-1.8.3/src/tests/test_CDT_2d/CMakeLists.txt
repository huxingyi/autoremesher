aux_source_directories(SOURCES "" .)
vor_add_executable(test_CDT_2d ${SOURCES})
target_link_libraries(test_CDT_2d geogram)

set_target_properties(test_CDT_2d PROPERTIES FOLDER "GEOGRAM/Tests")


