##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = exrmultiview

AM_CPPFLAGS = -I$(top_builddir) \
           -I$(top_srcdir)/IlmImf -I$(top_srcdir)/config \
           @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

LDADD = @ILMBASE_LDFLAGS@ $(ILMBASE_LIBS) \
	$(top_builddir)/IlmImf/libIlmImf.la \
	$(ZLIB_LIBS)

exrmultiview_SOURCES = main.cpp  \
		       Image.h Image.cpp \
		       makeMultiView.cpp makeMultiView.h \
		       namespaceAlias.h

noinst_HEADERS = Image.h makeMultiView.h

EXTRA_DIST = main.cpp \
	     Image.h Image.cpp \
	     makeMultiView.h makeMultiView.cpp \
	     namespaceAlias.h \
             CMakeLists.txt
