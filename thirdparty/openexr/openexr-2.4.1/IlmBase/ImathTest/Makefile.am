##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

check_PROGRAMS = ImathTest

ImathTest_SOURCES = main.cpp testExtractEuler.cpp testExtractSHRT.cpp \
		    testFrustum.cpp testFun.cpp testInvert.cpp \
		    testMatrix.cpp testRandom.cpp testRoots.cpp \
		    testShear.cpp testColor.cpp testColor.h \
		    testExtractEuler.h testExtractSHRT.h testFrustum.h \
		    testFun.h testInvert.h testMatrix.h testRandom.h \
		    testRoots.h testShear.h testQuatSetRotation.cpp \
		    testQuatSetRotation.h testLineAlgo.cpp testLineAlgo.h \
		    testQuatSlerp.cpp testQuatSlerp.h testQuat.cpp \
		    testQuat.h testBoxAlgo.cpp testBoxAlgo.h \
		    testVec.cpp testVec.h testBox.cpp testBox.h \
		    testMiscMatrixAlgo.cpp testMiscMatrixAlgo.h \
		    testProcrustes.cpp testProcrustes.h \
		    testTinySVD.cpp testTinySVD.h \
		    testJacobiEigenSolver.cpp testJacobiEigenSolver.h \
		    testFrustumTest.cpp testFrustumTest.h

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Imath -I$(top_srcdir)/Iex -I$(top_srcdir)/Half \
	   -I$(top_srcdir)/config

LDADD = -L$(top_builddir)/Imath -L$(top_builddir)/Iex -L$(top_builddir)/Half \
	-lImath -lIex -lHalf

TESTS = ImathTest

EXTRA_DIST = CMakeLists.txt

