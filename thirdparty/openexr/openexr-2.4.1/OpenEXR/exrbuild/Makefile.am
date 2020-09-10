##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = exrbuild

AM_CPPFLAGS = -I$(top_builddir) \
	              -I$(top_srcdir)/IlmImf -I$(top_srcdir)/config \
		                 @ILMBASE_CXXFLAGS@

LDADD = @ILMBASE_LDFLAGS@ @ILMBASE_LIBS@ \
	        $(top_builddir)/IlmImf/libIlmImf.la \
		        -lz

exrbuild_SOURCES = exrbuild.cpp

