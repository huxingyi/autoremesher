##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

# tell autoconf to include the m4 macros in the /m4 directory
# (an alternative to the acinclude.m4 mechanism)
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = DtexToExr

INCLUDES = @OPENEXR_CXXFLAGS@ @PRMANSDK_CXXFLAGS@

LDADD = @OPENEXR_LDFLAGS@ @OPENEXR_LIBS@ @PRMANSDK_LDFLAGS@ @PRMANSDK_LIBS@ -lz

DtexToExr_SOURCES = PxDeepUtils.cpp DtexToExr.cpp 





