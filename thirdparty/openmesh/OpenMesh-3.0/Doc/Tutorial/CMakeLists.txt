include (ACGCommon)

include_directories (
  ../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (targetName MyOwnProject)

# collect all header and source files
acg_append_files (headers "*.hh" .)
acg_append_files (sources "*.cc" .)

acg_add_executable (${targetName} ${headers} ${sources})

target_link_libraries (${targetName}
  OpenMeshCore
  OpenMeshTools
)

