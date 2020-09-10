# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

ilmbase_define_library(Iex
  PRIV_EXPORT IEX_EXPORTS
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES
    IexBaseExc.cpp
    IexThrowErrnoExc.cpp
  HEADERS
    IexBaseExc.h
    IexMathExc.h
    IexThrowErrnoExc.h
    IexErrnoExc.h
    IexMacros.h
    Iex.h
    IexNamespace.h
    IexExport.h
    IexForward.h
  )
