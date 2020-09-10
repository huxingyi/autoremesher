##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/config

lib_LTLIBRARIES = libIex.la

libIex_la_SOURCES = IexThrowErrnoExc.cpp IexBaseExc.cpp IexBaseExc.h \
		    IexErrnoExc.h Iex.h IexMacros.h IexMathExc.h \
		    IexThrowErrnoExc.h

libIex_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ -no-undefined
if LIB_SUFFIX_EXISTS
libIex_la_LDFLAGS += -release @LIB_SUFFIX@
endif

libIexincludedir = $(includedir)/OpenEXR

libIexinclude_HEADERS = IexBaseExc.h IexMathExc.h IexThrowErrnoExc.h \
			IexErrnoExc.h IexMacros.h Iex.h \
                        IexForward.h IexNamespace.h IexExport.h

EXTRA_DIST = CMakeLists.txt
