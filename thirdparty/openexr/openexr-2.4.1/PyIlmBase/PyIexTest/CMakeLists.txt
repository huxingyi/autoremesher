# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

if(TARGET Python2::Interpreter)
  add_test(PyIlmBase.PyIexTest_Python2
    ${Python2_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/pyIexTest.in
  )
  set_tests_properties(PyIlmBase.PyIexTest_Python2 PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/python${Python2_VERSION_MAJOR}_${Python2_VERSION_MINOR}"
  )
endif()

if(TARGET Python3::Interpreter)
  add_test(PyIlmBase.PyIexTest_Python3
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/pyIexTest.in
  )
  set_tests_properties(PyIlmBase.PyIexTest_Python3 PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/python${Python3_VERSION_MAJOR}_${Python3_VERSION_MINOR}"
  )
endif()
