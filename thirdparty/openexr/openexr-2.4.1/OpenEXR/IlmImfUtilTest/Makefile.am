##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

check_PROGRAMS = IlmImfUtilTest

IlmImfUtilTest_SOURCES = main.cpp \
	testFlatImage.h testFlatImage.cpp \
	testDeepImage.h testDeepImage.cpp \
	testIO.h testIO.cpp tmpDir.h

AM_CPPFLAGS = -I$(top_builddir)  \
	   -I$(top_srcdir)/IlmImf \
	   -I$(top_srcdir)/IlmImfUtil \
	   -I$(top_srcdir)/config \
	   @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

LDADD = -L$(top_builddir)/IlmImf \
	-L$(top_builddir)/IlmImfUtil \
	@ILMBASE_LDFLAGS@ $(ILMBASE_LIBS) \
	-lIlmImfUtil -lIlmImf $(ZLIB_LIBS)

TESTS = IlmImfUtilTest

EXTRA_DIST = CMakeLists.txt
