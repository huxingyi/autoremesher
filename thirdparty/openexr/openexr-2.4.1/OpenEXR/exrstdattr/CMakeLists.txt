# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(exrstdattr main.cpp)
target_link_libraries(exrstdattr OpenEXR::IlmImf)
set_target_properties(exrstdattr PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS exrstdattr DESTINATION ${CMAKE_INSTALL_BINDIR})
