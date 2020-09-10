##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

# tell autoconf to include the m4 macros in the /m4 directory 
# (an alternative to the acinclude.m4 mechanism)
ACLOCAL_AMFLAGS = -I m4

PYIMATH_SUBDIRS = config PyIex PyImath PyIexTest  PyImathTest PyImathNumpyTest
PYIMATHNUMPY_SUBDIRS = PyImathNumpy

if BUILD_PYIMATHNUMPY
    MAYBE_PYIMATHNUMPY_SUBDIRS = $(PYIMATHNUMPY_SUBDIRS)
endif

SUBDIRS = $(PYIMATH_SUBDIRS) $(MAYBE_PYIMATHNUMPY_SUBDIRS)

DIST_SUBDIRS = \
	$(PYIMATH_SUBDIRS) \
	$(PYIMATHNUMPY_SUBDIRS)

EXTRA_DIST = \
	LICENSE README.md \
	ChangeLog \
	bootstrap \
	pyilmbase.m4 \
	CMakeLists.txt

dist-hook:
	cp -fpR $(abspath $(srcdir)/..)/cmake $(distdir)
	find $(distdir)/cmake -type d ! -perm -700 -exec chmod u+rwx {} \;

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = PyIlmBase.pc

m4datadir = $(datadir)/aclocal
m4data_DATA = pyilmbase.m4

