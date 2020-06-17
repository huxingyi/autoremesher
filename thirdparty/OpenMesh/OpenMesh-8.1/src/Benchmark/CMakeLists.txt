set (SOURCES "")
list (APPEND SOURCES
	main.cpp
	VectorT_new.cpp
	VectorT_legacy.cpp
        VectorT_dummy_data.cpp
)

add_executable(OMBenchmark ${SOURCES})
set_target_properties(OMBenchmark PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(OMBenchmark benchmark OpenMeshCore)