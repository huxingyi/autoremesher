##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = exrmaketiled

AM_CPPFLAGS = -I$(top_builddir) \
           -I$(top_srcdir)/IlmImf -I$(top_srcdir)/config \
           @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

LDADD = @ILMBASE_LDFLAGS@ $(ILMBASE_LIBS) \
	$(top_builddir)/IlmImf/libIlmImf.la \
	$(ZLIB_LIBS)

exrmaketiled_SOURCES = main.cpp \
		       Image.h Image.cpp \
		       makeTiled.cpp makeTiled.h \
		       namespaceAlias.h

noinst_HEADERS = Image.h makeTiled.h

EXTRA_DIST = main.cpp \
	     Image.h Image.cpp \
	     makeTiled.cpp makeTiled.h \
	     namespaceAlias.h \
             CMakeLists.txt
