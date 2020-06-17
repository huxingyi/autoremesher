include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

acg_add_executable (commandlineAdaptiveSubdivider ../adaptive_subdivider.cc)

target_link_libraries (commandlineAdaptiveSubdivider
  OpenMeshCore
  OpenMeshTools
)

