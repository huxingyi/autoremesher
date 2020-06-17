include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

acg_add_executable (Analyzer vdpmanalyzer.cc)

target_link_libraries (Analyzer
  OpenMeshCore
  OpenMeshTools
)

