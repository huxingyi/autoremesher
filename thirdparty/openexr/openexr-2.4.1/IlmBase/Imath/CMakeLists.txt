# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

ilmbase_define_library(Imath
  PRIV_EXPORT IMATH_EXPORTS
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES
    ImathRandom.cpp
    ImathColorAlgo.cpp
    ImathFun.cpp
    ImathVec.cpp
    ImathExc.cpp
    ImathMatrixAlgo.cpp
  HEADERS
    ImathBoxAlgo.h
    ImathBox.h
    ImathColorAlgo.h
    ImathColor.h
    ImathEuler.h
    ImathExc.h
    ImathExport.h
    ImathForward.h
    ImathFrame.h
    ImathFrustum.h
    ImathFrustumTest.h
    ImathFun.h
    ImathGL.h
    ImathGLU.h
    ImathHalfLimits.h
    ImathInt64.h
    ImathInterval.h
    ImathLimits.h
    ImathLineAlgo.h
    ImathLine.h
    ImathMath.h
    ImathMatrixAlgo.h
    ImathMatrix.h
    ImathNamespace.h
    ImathPlane.h
    ImathPlatform.h
    ImathQuat.h
    ImathRandom.h
    ImathRoots.h
    ImathShear.h
    ImathSphere.h
    ImathVecAlgo.h
    ImathVec.h
  DEPENDENCIES
    IlmBase::Half IlmBase::IexMath
)
