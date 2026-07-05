aux_source_directories(SOURCES "" .)

vor_reset_warning_level()
aux_source_directories(SOURCES "" ./third_party/ANN)

vor_add_executable(test_nn_search ${SOURCES})
target_link_libraries(test_nn_search geogram)

set_target_properties(test_nn_search PROPERTIES FOLDER "GEOGRAM/Tests")

