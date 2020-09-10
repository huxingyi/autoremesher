##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

if HAVE_FLTK

bin_PROGRAMS = exrdisplay

AM_CPPFLAGS = @OPENEXR_CXXFLAGS@ \
        @GL_CXXFLAGS@ @CG_CXXFLAGS@ @FLTK_CXXFLAGS@ -I$(top_builddir) \
	@OPENEXR_CTL_CXXFLAGS@

LDADD = @OPENEXR_LDFLAGS@ @OPENEXR_LIBS@ \
        @OPENEXR_CTL_LDFLAGS@ @OPENEXR_CTL_LIBS@ \
        @CG_LDFLAGS@ @FLTK_LDFLAGS@
        
all-local:
	@FLTK_CONFIG@ --post exrdisplay

install-exec-hook:
	@FLTK_CONFIG@ --post $(DESTDIR)$(bindir)/exrdisplay

else
install-exec-hook:

endif


exrdisplay_SOURCES = \
	main.cpp \
	ImageView.cpp \
	ImageView.h \
	loadImage.cpp \
	loadImage.h \
	scaleImage.cpp \
	scaleImage.h \
	applyCtl.cpp \
	applyCtl.h \
	GlWindow3d.h \
	GlWindow3d.cpp

noinst_HEADERS =  \
        ImageView.h \
        loadImage.h \
        scaleImage.h \
        applyCtl.h \
        GlWindow3d.h \
        namespaceAlias.h

EXTRA_DIST = $(exrdisplay_SOURCES)




