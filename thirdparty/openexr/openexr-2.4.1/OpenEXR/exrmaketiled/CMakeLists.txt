# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(exrmaketiled 
  makeTiled.cpp
  main.cpp
  Image.cpp
)
target_link_libraries(exrmaketiled OpenEXR::IlmImf)
set_target_properties(exrmaketiled PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS exrmaketiled DESTINATION ${CMAKE_INSTALL_BINDIR})
