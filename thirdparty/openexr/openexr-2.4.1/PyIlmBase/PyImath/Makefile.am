##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

pyexec_LTLIBRARIES = imathmodule.la
lib_LTLIBRARIES = libPyImath.la

libPyImath_la_SOURCES = PyImath.cpp \
    PyImathAutovectorize.cpp \
    PyImathBox2Array.cpp \
    PyImathBox3Array.cpp \
    PyImathBox.cpp \
    PyImathColor3.cpp \
    PyImathColor4.cpp \
    PyImathEuler.cpp \
    PyImathFixedArray.cpp \
    PyImathFrustum.cpp \
    PyImathLine.cpp \
    PyImathMatrix33.cpp \
    PyImathMatrix44.cpp \
    PyImathPlane.cpp \
    PyImathQuat.cpp \
    PyImathRandom.cpp \
    PyImathShear.cpp \
    PyImathStringArray.cpp \
    PyImathStringTable.cpp \
    PyImathTask.cpp \
    PyImathUtil.cpp \
    PyImathFixedVArray.cpp \
    PyImathVec2fd.cpp \
    PyImathVec2si.cpp \
    PyImathVec3fd.cpp \
    PyImathVec3siArray.cpp \
    PyImathVec3si.cpp \
    PyImathVec4fd.cpp \
    PyImathVec4siArray.cpp \
    PyImathVec4si.cpp

libPyImathinclude_HEADERS = PyImath.h \
    PyImathAutovectorize.h \
    PyImathBoxArrayImpl.h \
    PyImathBox.h \
    PyImathColor3ArrayImpl.h \
    PyImathColor4Array2DImpl.h \
    PyImathColor4ArrayImpl.h \
    PyImathColor.h \
    PyImathDecorators.h \
    PyImathEuler.h \
    PyImathExport.h \
    PyImathFixedArray2D.h \
    PyImathFixedArray.h \
    PyImathFixedMatrix.h \
    PyImathFrustum.h \
    PyImathLine.h \
    PyImathMathExc.h \
    PyImathMatrix.h \
    PyImathOperators.h \
    PyImathPlane.h \
    PyImathQuat.h \
    PyImathRandom.h \
    PyImathShear.h \
    PyImathStringArray.h \
    PyImathStringArrayRegister.h \
    PyImathStringTable.h \
    PyImathTask.h \
    PyImathUtil.h \
    PyImathFixedVArray.h \
    PyImathVec2Impl.h \
    PyImathVec3ArrayImpl.h \
    PyImathVec3Impl.h \
    PyImathVec4ArrayImpl.h \
    PyImathVec4Impl.h \
    PyImathVec.h \
    PyImathVecOperators.h

libPyImath_la_LDFLAGS = -version-info @LIBTOOL_VERSION@ \
                        -no-undefined
libPyImath_la_LIBADD  = -lz $(top_builddir)/PyIex/libPyIex.la \
                        @ILMBASE_LIBS@ @BOOST_PYTHON_LIBS@
libPyImathincludedir  = $(includedir)/OpenEXR

imathmodule_la_SOURCES = imathmodule.cpp \
    PyImathFun.cpp \
    PyImathBasicTypes.cpp 

imathmodule_la_LDFLAGS = -avoid-version -module
imathmodule_la_LIBADD  = -lPyImath @BOOST_PYTHON_LIBS@

noinst_HEADERS = PyImathFun.h \
    PyImathBasicTypes.h 

AM_CPPFLAGS = @ILMBASE_CXXFLAGS@ \
	   -I$(top_builddir)  \
	   -I$(top_srcdir)/PyIex  \
	   -I$(top_srcdir)/config
