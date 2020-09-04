aux_source_directories(SOURCES "" .)
vor_add_executable(test_convex_cell ${SOURCES})
target_link_libraries(test_convex_cell geogram)

set_target_properties(test_convex_cell PROPERTIES FOLDER "GEOGRAM/Tests")

