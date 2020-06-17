include (ACGCommon)

include_directories (
  ../../..
)

acg_add_executable (mconvert mconvert.cc)

target_link_libraries (mconvert
  OpenMeshCore
  OpenMeshTools
)

