include (ACGCommon)

include_directories (
  ../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

acg_add_executable (Smoothing smooth.cc)

target_link_libraries (Smoothing
  OpenMeshCore
  OpenMeshTools
)

