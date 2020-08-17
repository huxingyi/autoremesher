aux_source_directories(SOURCES "" .)
vor_add_executable(test_RVC ${SOURCES})
target_link_libraries(test_RVC geogram)

set_target_properties(test_RVC PROPERTIES FOLDER "GEOGRAM/Tests")


