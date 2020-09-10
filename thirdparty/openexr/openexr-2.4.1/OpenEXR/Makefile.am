##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

# tell autoconf to include the m4 macros in the /m4 directory 
# (an alternative to the acinclude.m4 mechanism)
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = config IlmImf IlmImfUtil IlmImfTest IlmImfUtilTest \
	  IlmImfFuzzTest exrheader exrmaketiled IlmImfExamples doc \
	  exrstdattr exrmakepreview exrenvmap exrmultiview exrmultipart exr2aces

DIST_SUBDIRS = \
	$(SUBDIRS) 

EXTRA_DIST = \
	AUTHORS ChangeLog LICENSE NEWS PATENTS \
	README.md \
	bootstrap openexr.m4 \
	CMakeLists.txt

dist-hook:
	cp -fpR $(abspath $(srcdir)/..)/cmake $(distdir)
	find $(distdir)/cmake -type d ! -perm -700 -exec chmod u+rwx {} \;

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = OpenEXR.pc

m4datadir = $(datadir)/aclocal
m4data_DATA = openexr.m4

