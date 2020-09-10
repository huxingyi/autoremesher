##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/config

lib_LTLIBRARIES = libHalf.la

libHalf_la_SOURCES = half.cpp half.h halfFunction.h halfLimits.h

libHalf_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ -no-undefined
if LIB_SUFFIX_EXISTS
libHalf_la_LDFLAGS += -release @LIB_SUFFIX@
endif

libHalfincludedir = $(includedir)/OpenEXR

libHalfinclude_HEADERS = half.h halfFunction.h halfLimits.h halfExport.h

# these are used to build eLut.h and toFloat.h dynamically
EXTRA_DIST = eLut.cpp toFloat.cpp CMakeLists.txt

CLEANFILES = eLut eLut.h toFloat toFloat.h

eLut_SOURCES = eLut.cpp

toFloat_SOURCES = toFloat.cpp

eLut.h:	eLut
	./eLut > eLut.h

toFloat.h: toFloat
	   ./toFloat > toFloat.h

BUILT_SOURCES = eLut.h toFloat.h

noinst_PROGRAMS = eLut toFloat
