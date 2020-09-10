##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

if BUILD_IMFFUZZTEST
check_PROGRAMS = IlmImfFuzzTest
endif

IlmImfFuzzTest_SOURCES = fuzzFile.cpp fuzzFile.h main.cpp tmpDir.h \
			 testFuzzScanLines.cpp testFuzzScanLines.h \
			 testFuzzDeepScanLines.cpp testFuzzDeepScanLines.h \
			 testFuzzDeepTiles.cpp testFuzzDeepTiles.h \
			 testFuzzTiles.cpp testFuzzTiles.h 


AM_CPPFLAGS = -I$(top_builddir)  \
	   -I$(top_srcdir)/IlmImf \
	   -I$(top_srcdir)/config \
	   @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

LDADD = -L$(top_builddir)/IlmImf \
	@ILMBASE_LDFLAGS@ $(ILMBASE_LIBS) \
	-lIlmImf $(ZLIB_LIBS)

if BUILD_IMFFUZZTEST
TESTS = IlmImfFuzzTest
endif

EXTRA_DIST = CMakeLists.txt
