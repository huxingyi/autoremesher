# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(eLut eLut.cpp)
target_compile_features(eLut PUBLIC cxx_std_${OPENEXR_CXX_STANDARD})


add_executable(toFloat toFloat.cpp)
target_compile_features(toFloat PUBLIC cxx_std_${OPENEXR_CXX_STANDARD})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/toFloat.h
    ${CMAKE_CURRENT_BINARY_DIR}/eLut.h
  COMMAND $<TARGET_FILE:toFloat> ARGS > ${CMAKE_CURRENT_BINARY_DIR}/toFloat.h
  COMMAND $<TARGET_FILE:eLut> ARGS > ${CMAKE_CURRENT_BINARY_DIR}/eLut.h
  DEPENDS eLut toFloat
)

### Now define the real library

ilmbase_define_library(Half
  PRIV_EXPORT HALF_EXPORTS
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  CURBINDIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/toFloat.h
    ${CMAKE_CURRENT_BINARY_DIR}/eLut.h
    half.cpp
  HEADERS
    half.h
    halfFunction.h
    halfExport.h
    halfLimits.h
)
