# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(exr2aces main.cpp)
target_link_libraries(exr2aces OpenEXR::IlmImf)
set_target_properties(exr2aces PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS exr2aces DESTINATION ${CMAKE_INSTALL_BINDIR})
