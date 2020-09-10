##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libIexMath.la

libIexMathincludedir = $(includedir)/OpenEXR

libIexMath_la_SOURCES = IexMathFloatExc.cpp IexMathFpu.cpp
libIexMathinclude_HEADERS = IexMathFloatExc.h IexMathFpu.h IexMathIeeeExc.h

libIexMath_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ -no-undefined
if LIB_SUFFIX_EXISTS
libIexMath_la_LDFLAGS += -release @LIB_SUFFIX@
endif

libIexMath_la_LIBADD = ../Iex/libIex.la

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Iex \
           -I$(top_srcdir)/config

EXTRA_DIST = CMakeLists.txt
