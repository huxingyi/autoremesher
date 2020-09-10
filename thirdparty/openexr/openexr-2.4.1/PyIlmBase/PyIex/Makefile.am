##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

pyexec_LTLIBRARIES = iexmodule.la
lib_LTLIBRARIES = libPyIex.la

libPyIex_la_SOURCES = PyIex.cpp 

libPyIexinclude_HEADERS = PyIex.h PyIexExport.h PyIexTypeTranslator.h

libPyIex_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ \
                      -no-undefined
libPyIex_la_LIBADD  = -lz @ILMBASE_LIBS@ @BOOST_PYTHON_LIBS@
libPyIexincludedir  = $(includedir)/OpenEXR

iexmodule_la_SOURCES = iexmodule.cpp
iexmodule_la_LDFLAGS = -avoid-version -module
iexmodule_la_LIBADD  = libPyIex.la @BOOST_PYTHON_LIBS@

noinst_HEADERS = 

AM_CPPFLAGS = @ILMBASE_CXXFLAGS@ \
	   -I$(top_builddir)  \
	   -I$(top_srcdir)/config
