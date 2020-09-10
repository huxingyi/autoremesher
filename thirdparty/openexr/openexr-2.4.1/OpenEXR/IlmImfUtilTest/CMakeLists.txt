# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(IlmImfUtilTest
  main.cpp
  testFlatImage.cpp
  testDeepImage.cpp
  testIO.cpp
 )
target_link_libraries(IlmImfUtilTest OpenEXR::IlmImfUtil)
set_target_properties(IlmImfUtilTest PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_test(NAME OpenEXR.IlmImfUtil COMMAND $<TARGET_FILE:IlmImfUtilTest>)
