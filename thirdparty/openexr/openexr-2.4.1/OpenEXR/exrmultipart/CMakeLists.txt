# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(exrmultipart exrmultipart.cpp)
target_link_libraries(exrmultipart OpenEXR::IlmImf)
set_target_properties(exrmultipart PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS exrmultipart DESTINATION ${CMAKE_INSTALL_BINDIR})
