##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

if BUILD_IMFEXAMPLES
noinst_PROGRAMS = imfexamples
endif

AM_CPPFLAGS = -I$(top_builddir) \
           -I$(top_srcdir)/IlmImf -I$(top_srcdir)/config \
           @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

LDADD = -L$(top_builddir)/IlmImf \
	@ILMBASE_LDFLAGS@ $(ILMBASE_LIBS) \
	-lIlmImf $(ZLIB_CFLAGS)

imfexamples_SOURCES = main.cpp drawImage.cpp rgbaInterfaceExamples.cpp \
		      rgbaInterfaceTiledExamples.cpp \
		      generalInterfaceExamples.cpp \
		      lowLevelIoExamples.cpp previewImageExamples.cpp \
		      generalInterfaceTiledExamples.cpp \
		      generalInterfaceTiledExamples.h drawImage.h \
		      rgbaInterfaceExamples.h generalInterfaceExamples.h \
		      rgbaInterfaceTiledExamples.h \
		      lowLevelIoExamples.h previewImageExamples.h \
		      namespaceAlias.h

examplesdir = $(docdir)/examples
examples_DATA = $(imfexamples_SOURCES)

imfexamplesdir = $(examplesdir)

EXTRA_DIST = CMakeLists.txt

