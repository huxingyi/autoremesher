# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

pyilmbase_define_module(iex
  LIBNAME PyIex
  PRIV_EXPORT PYIEX_BUILD
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  LIBSOURCE
    PyIex.cpp
  MODSOURCE
    iexmodule.cpp
  HEADERS
    PyIex.h
    PyIexExport.h
    PyIexTypeTranslator.h
  DEPENDENCIES
    IlmBase::Iex IlmBase::IexMath
  )
