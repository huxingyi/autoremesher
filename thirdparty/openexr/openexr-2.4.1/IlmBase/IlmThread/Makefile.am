##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libIlmThread.la

libIlmThread_la_SOURCES = IlmThreadPool.h IlmThread.h \
			  IlmThreadSemaphore.h IlmThreadMutex.h \
			  IlmThreadPool.cpp IlmThread.cpp \
			  IlmThreadSemaphore.cpp IlmThreadMutex.cpp \
			  IlmThreadPosix.cpp IlmThreadSemaphorePosix.cpp \
			  IlmThreadSemaphorePosixCompat.cpp \
			  IlmThreadSemaphoreOSX.cpp \
			  IlmThreadMutexPosix.cpp

libIlmThread_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ -no-undefined
if LIB_SUFFIX_EXISTS
libIlmThread_la_LDFLAGS += -release @LIB_SUFFIX@
endif
libIlmThread_la_LIBADD = ../Iex/libIex.la $(PTHREAD_LIBS)

libIlmThreadincludedir = $(includedir)/OpenEXR

libIlmThreadinclude_HEADERS = IlmThreadPool.h IlmThread.h \
			      IlmThreadSemaphore.h IlmThreadMutex.h \
			      IlmThreadNamespace.h IlmThreadForward.h \
			      IlmThreadExport.h

noinst_HEADERS = 

EXTRA_DIST = $(noinst_HEADERS) IlmThreadMutexWin32.cpp IlmThreadSemaphoreWin32.cpp \
	     IlmThreadWin32.cpp CMakeLists.txt

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Iex -I$(top_srcdir)/config
