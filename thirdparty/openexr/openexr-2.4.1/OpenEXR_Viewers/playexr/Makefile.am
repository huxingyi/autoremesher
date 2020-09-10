##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

if HAVE_CG

bin_PROGRAMS = playexr

playexr_CPPFLAGS = $(AM_CPPFLAGS) 

AM_CPPFLAGS = \
	@OPENEXR_CTL_CXXFLAGS@ \
	@GL_CXXFLAGS@ \
	@CG_CXXFLAGS@

LDADD =	\
	@OPENEXR_CTL_LDFLAGS@ \
	@CG_LDFLAGS@ \
	@OPENEXR_CTL_LIBS@




playexr_SOURCES = \
	main.cpp \
	playExr.cpp \
	playExr.h \
	fileNameForFrame.cpp \
	fileNameForFrame.h \
	FileReadingThread.cpp \
	FileReadingThread.h \
	ImageBuffers.cpp \
	ImageBuffers.h \
	Timer.cpp \
	Timer.h \
	ctlToLut.cpp \
	ctlToLut.h \
	osDependent.cpp \
	osDependent.h 

endif

noinst_HEADERS =  \
	playExr.h \
	fileNameForFrame.h \
	FileReadingThread.h \
	ImageBuffers.h \
	Timer.h \
	ctlToLut.cpp \
	ctlToLut.h \
	osDependent.h

EXTRA_DIST = \
	$(playexr_SOURCES)
