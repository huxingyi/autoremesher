# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

if(TARGET Python2::Python AND
   TARGET Boost::${PYILMBASE_BOOST_PY2_COMPONENT} AND
   TARGET Python2::IlmBaseNumPy)

  set(moddeps_p2 PyIex PyImath)
  list(TRANSFORM moddeps_p2 APPEND ${PYILMBASE_LIB_PYTHONVER_ROOT}${Python2_VERSION_MAJOR}_${Python2_VERSION_MINOR})

  Python2_add_library(imathnumpy_python2 MODULE
    imathnumpymodule.cpp
  )
  target_link_libraries(imathnumpy_python2
    PRIVATE
      IlmBase::Iex IlmBase::IexMath IlmBase::Imath
      ${moddeps_p2}
      Python2::Python
      Boost::${PYILMBASE_BOOST_PY2_COMPONENT}
      Python2::IlmBaseNumPy
    )
  set_target_properties(imathnumpy_python2 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python${Python2_VERSION_MAJOR}_${Python2_VERSION_MINOR}/"
    LIBRARY_OUTPUT_NAME "imathnumpy"
    DEBUG_POSTFIX ""
  )
endif()

if(TARGET Python3::Python AND
   TARGET Boost::${PYILMBASE_BOOST_PY3_COMPONENT} AND
   TARGET Python3::IlmBaseNumPy)

  set(moddeps_p3 PyIex PyImath)
  list(TRANSFORM moddeps_p3 APPEND ${PYILMBASE_LIB_PYTHONVER_ROOT}${Python3_VERSION_MAJOR}_${Python3_VERSION_MINOR})

  Python3_add_library(imathnumpy_python3 MODULE
    imathnumpymodule.cpp
  )
  target_link_libraries(imathnumpy_python3
    PRIVATE
      IlmBase::Iex IlmBase::IexMath IlmBase::Imath
      ${moddeps_p3}
      Python3::Python
      Boost::${PYILMBASE_BOOST_PY3_COMPONENT}
      Python3::IlmBaseNumPy
    )
  set_target_properties(imathnumpy_python3 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python${Python3_VERSION_MAJOR}_${Python3_VERSION_MINOR}/"
    LIBRARY_OUTPUT_NAME "imathnumpy"
    DEBUG_POSTFIX ""
  )
endif()
