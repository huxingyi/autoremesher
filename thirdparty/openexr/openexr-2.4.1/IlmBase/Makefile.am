##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	Half HalfTest \
	Iex IexMath IexTest \
	Imath ImathTest \
	IlmThread \
	config

DIST_SUBDIRS = \
	$(SUBDIRS) 

EXTRA_DIST = \
	AUTHORS ChangeLog LICENSE NEWS \
	README.md \
	bootstrap \
	CMakeLists.txt

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = IlmBase.pc

