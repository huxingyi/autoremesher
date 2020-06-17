include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

acg_add_executable (commandlineDecimater ../decimater.cc)

target_link_libraries (commandlineDecimater
  OpenMeshCore
  OpenMeshTools
)

