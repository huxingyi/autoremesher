##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libImath.la

libImath_la_SOURCES = ImathMatrixAlgo.cpp ImathVec.cpp \
		      ImathColorAlgo.cpp ImathExc.cpp ImathFun.cpp \
		      ImathColorAlgo.h ImathMatrixAlgo.h ImathVec.h \
		      ImathShear.h ImathFun.h ImathBox.h ImathBoxAlgo.h \
		      ImathEuler.h ImathExc.h ImathLimits.h ImathLine.h \
		      ImathLineAlgo.h ImathMatrix.h ImathPlane.h \
		      ImathSphere.h ImathVecAlgo.h ImathQuat.h \
		      ImathFrustum.h ImathMath.h ImathGL.h \
		      ImathColor.h ImathRandom.h ImathRoots.h \
		      ImathHalfLimits.h ImathInterval.h ImathGLU.h \
		      ImathFrame.h ImathPlatform.h \
		      ImathRandom.cpp ImathInt64.h \
		      ImathFrustumTest.h

libImath_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ -no-undefined
if LIB_SUFFIX_EXISTS
libImath_la_LDFLAGS += -release @LIB_SUFFIX@
endif


libImath_la_LIBADD = ../Iex/libIex.la

libImathincludedir = $(includedir)/OpenEXR

libImathinclude_HEADERS = ImathColorAlgo.h ImathMatrixAlgo.h ImathVec.h \
			  ImathShear.h ImathFun.h ImathBox.h ImathBoxAlgo.h \
			  ImathEuler.h ImathExc.h ImathLimits.h ImathLine.h \
			  ImathLineAlgo.h ImathMatrix.h ImathPlane.h \
			  ImathSphere.h ImathVecAlgo.h ImathQuat.h \
			  ImathFrustum.h ImathMath.h ImathGL.h \
			  ImathColor.h ImathRandom.h ImathRoots.h \
			  ImathHalfLimits.h ImathInterval.h ImathGLU.h \
			  ImathFrame.h ImathPlatform.h ImathInt64.h \
                          ImathNamespace.h ImathForward.h ImathExport.h \
                          ImathFrustumTest.h

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Iex -I$(top_srcdir)/Half \
	   -I$(top_srcdir)/config

EXTRA_DIST = CMakeLists.txt
