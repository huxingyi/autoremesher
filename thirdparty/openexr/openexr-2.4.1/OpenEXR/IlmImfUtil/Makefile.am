##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libIlmImfUtil.la

libIlmImfUtil_la_SOURCES = \
	ImfImageChannel.h ImfImageChannel.cpp \
	ImfFlatImageChannel.h ImfFlatImageChannel.cpp \
	ImfDeepImageChannel.h ImfDeepImageChannel.cpp \
	ImfSampleCountChannel.h ImfSampleCountChannel.cpp \
	ImfImageLevel.h ImfImageLevel.cpp \
	ImfFlatImageLevel.h ImfFlatImageLevel.cpp \
	ImfDeepImageLevel.h ImfDeepImageLevel.cpp \
	ImfImage.h ImfImage.cpp \
	ImfFlatImage.h ImfFlatImage.cpp \
	ImfDeepImage.h ImfDeepImage.cpp \
	ImfImageIO.h ImfImageIO.cpp \
	ImfFlatImageIO.h ImfFlatImageIO.cpp \
	ImfDeepImageIO.h ImfDeepImageIO.cpp \
	ImfImageDataWindow.h ImfImageDataWindow.cpp \
	ImfImageChannelRenaming.h ImfUtilExport.h

libIlmImfUtil_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ \
			-no-undefined 


if LIB_SUFFIX_EXISTS
libIlmImfUtil_la_LDFLAGS += -release @LIB_SUFFIX@
endif


libIlmImfUtil_la_LIBADD =  -L$(top_builddir)/IlmImf $(ILMBASE_LIBS) -lIlmImf

libIlmImfUtilincludedir = $(includedir)/OpenEXR

EXTRA_DIST = CMakeLists.txt

AM_CPPFLAGS = \
	   -I$(top_builddir) \
	   -I$(top_srcdir)/IlmImf \
	   -I$(top_srcdir)/config \
	   @ILMBASE_CXXFLAGS@ $(ILMBASE_CFLAGS)

