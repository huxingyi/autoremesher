# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(exrheader main.cpp)
target_link_libraries(exrheader OpenEXR::IlmImf)
set_target_properties(exrheader PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS exrheader DESTINATION ${CMAKE_INSTALL_BINDIR})
