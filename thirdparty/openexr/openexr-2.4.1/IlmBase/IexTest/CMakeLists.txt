# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(IexTest
  main.cpp
  testBaseExc.cpp
)

target_link_libraries(IexTest IlmBase::Iex)
set_target_properties(IexTest PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_test(NAME IlmBase.Iex COMMAND $<TARGET_FILE:IexTest>)
