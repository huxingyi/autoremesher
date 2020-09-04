aux_source_directories(SOURCES "" .)
vor_add_executable(bench_load ${SOURCES})
target_link_libraries(bench_load geogram)

set_target_properties(bench_load PROPERTIES FOLDER "GEOGRAM/Tests")

