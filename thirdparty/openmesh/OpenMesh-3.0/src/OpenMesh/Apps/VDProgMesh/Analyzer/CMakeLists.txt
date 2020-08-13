include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (targetName Analyzer)

# collect all header and source files
set (sources
  ./vdpmanalyzer.cc
)

acg_add_executable (${targetName} ${sources})

target_link_libraries (${targetName}
  OpenMeshCore
  OpenMeshTools
)

