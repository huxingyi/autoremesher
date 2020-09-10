dnl
dnl SPDX-License-Identifier: BSD-3-Clause
dnl Copyright Contributors to the OpenEXR Project.
dnl

dnl Process this file with autoconf to produce a configure script.
AC_INIT(OpenEXR_Viewers, 2.4.1)
AC_SUBST(OPENEXR_VIEWERS_VERSION, 2.4.1)
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR(playexr/main.cpp)
AM_CONFIG_HEADER(config/OpenEXR_ViewersConfig.h)
AM_INIT_AUTOMAKE(1.6.3)  dnl Require automake 1.6.3 or better
AM_MAINTAINER_MODE


LIBTOOL_CURRENT=24
LIBTOOL_REVISION=1
LIBTOOL_AGE=0
LIBTOOL_VERSION=$LIBTOOL_CURRENT:$LIBTOOL_REVISION:$LIBTOOL_AGE
AC_SUBST(LIBTOOL_VERSION)

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET

dnl
dnl PKGCONFIG preparations
dnl

if test -z "${PKG_CONFIG_PATH}"; then
	PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
fi

export PKG_CONFIG_PATH

LIB64_IF_EXISTS=""
if [[ -e /usr/lib64 ]]; then
   LIB64_IF_EXISTS="-L/usr/lib64"
fi         

AC_MSG_RESULT([LIB64_IF_EXISTS ...   $LIB64_IF_EXISTS])


dnl
dnl get ccflags and libs from openexr and ctl packages
dnl

dnl
dnl get ccflags and libs from other packages in use and check 
dnl whether test programs compile.
dnl OpenEXR is required.  If OpenEXR_CTL is optional.  If it is present, 
dnl the viewer will use ctl.  The compile flag HAVE_CTL_INTERPRETER is set
dnl to 1 if ctl is in use.

dnl
dnl check openexr
dnl
AM_PATH_PKGCONFIG(
   [OPENEXR_CXXFLAGS],
   [OPENEXR_LDFLAGS],
   [OPENEXR_LIBS],
   [OpenEXR],
   [OpenEXR],
   [$LIB64_IF_EXISTS -L/usr/local/lib],
   [-lIlmImf -lImath -lIex -lHalf -lz -lpthread],
   [openexr-prefix])

dnl
dnl check optional OpenexrCTL (see comment above)
dnl
AM_PATH_PKGCONFIG(
   [OPENEXR_CTL_CXXFLAGS],
   [OPENEXR_CTL_LDFLAGS],
   [OPENEXR_CTL_LIBS],
   [OpenEXR_CTL],
   [OpenEXR CTL],
   [$LIB64_IF_EXISTS -L/usr/local/lib],
   [-lIlmImfCtl -lIlmImf -lz -lImath -lHalf -lIex -lIlmThread -lpthread -lIlmCtlSimd -lIlmCtlMath -lIlmCtl],
   [openexrctl-prefix])


dnl Enable and choose c++ standard
AC_ARG_ENABLE(cxxstd,
              AC_HELP_STRING([--enable-cxxstd=14],
                             [enable ISO c++ standard 11/14 [[default=auto]]]),
              [cxxstd="${enableval}"], [cxxstd=14])

if test "${cxxstd}" = 17 ; then
    AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])
	CXXFLAGS="$CXXFLAGS -std=c++17"
else
  if test "${cxxstd}" = 14 ; then
      AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])
  	CXXFLAGS="$CXXFLAGS -std=c++14"
  else
    if test "${cxxstd}" = 11 ; then
      AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])
      CXXFLAGS="$CXXFLAGS -std=c++11"
    else
      if test "${cxxstd}" = 03 ; then
        CXXFLAGS="$CXXFLAGS -std=c++03"
      else
        dnl automatically determine...
        AX_CXX_COMPILE_STDCXX([11], [noext], [optional])
        AX_CXX_COMPILE_STDCXX([14], [noext], [optional])
        AX_CXX_COMPILE_STDCXX([17], [noext], [optional])
        if test "$HAVE_CXX14" = 1 ; then
  	      CXXFLAGS="$CXXFLAGS -std=c++14"
          cxxstd = 14
        else
          if test "$HAVE_CXX11" = 1 ; then
  	        CXXFLAGS="$CXXFLAGS -std=c++11"
            cxxstd = 11
          fi
        fi
      fi
    fi
  fi
fi

dnl --enable-threading
AC_ARG_ENABLE(threading,
              AC_HELP_STRING([--enable-threading],
                             [enable multi-threading [[default=yes]]]),
              [multithread="${enableval}"], [multithread=yes])

if test x$PKG_CONFIG = xno && test "x${multithread}" != xno ; then
    ACX_PTHREAD(
    [
	AC_DEFINE(HAVE_PTHREAD)
	OPENEXR_LIBS="$PTHREAD_LIBS $OPENEXR_LIBS"
	OPENEXR_CXXFLAGS="$OPENEXR_CXXFLAGS $PTHREAD_CFLAGS"
	OPENEXR_CTL_LIBS="$PTHREAD_LIBS $OPENEXR_CTL_LIBS"
	OPENEXR_CTL_CXXFLAGS="$OPENEXR_CTL_CXXFLAGS $PTHREAD_CFLAGS"
	CC="$PTHREAD_CC"
	
	AM_POSIX_SEM()
    ],
    [AC_MSG_ERROR([POSIX thread support required])])
    AC_MSG_NOTICE([multithread true, LIBS = $LIBS, CC = $CC, CXXFLAGS = $CXXFLAGS])
fi


AM_COMPILELINKRUN(
   [OpenEXR],
   [openexrtest],
   [$OPENEXR_CXXFLAGS],
   [$OPENEXR_LDFLAGS],
   [$OPENEXR_LIBS],[[
#include <stdlib.h>
#include <ImfTestFile.h>
#include <ImfNamespace.h>
]],
   [[OPENEXR_IMF_NAMESPACE::isOpenExrFile("notExist");]],
   AC_MSG_RESULT([Compiled and ran OpenEXR test program.]), 
   AC_MSG_ERROR([Could not compile OpenEXR test program.]))



AM_COMPILELINKRUN(
   [OpenEXR_CTL],
   [openexrctltest],
   [$OPENEXR_CTL_CXXFLAGS],
   [$OPENEXR_CTL_LDFLAGS],
   [$OPENEXR_CTL_LIBS],[[
#include <ImfCtlApplyTransforms.h>
#include <CtlSimdInterpreter.h>
#include <ImfHeader.h>
#include <ImfFrameBuffer.h>
#include <ImathBox.h>
#include <ImfNamespace.h>
#include <vector>
#include <cstdlib>
    using namespace Ctl;
    using namespace OPENEXR_IMF_NAMESPACE;
    using namespace Imath;
]],
   [[
    SimdInterpreter interpreter;
    std::vector<std::string> transformNames;
    Box2i transformWindow (V2i (0, 0), V2i (0, 0));
    Header envHeader;
    Header inHeader;
    Header outHeader;
    FrameBuffer inFb;
    FrameBuffer outFb;

    ImfCtl::applyTransforms (interpreter, transformNames, transformWindow,
                             envHeader, inHeader, inFb, outHeader, outFb);
   ]],
   AC_MSG_RESULT([Compiled and ran OpenEXR_CTL test program.]); have_ctl=yes, 
   AC_MSG_RESULT([The OpenEXR_CTL package could not be found.  Will build and install playexr and exrdisplay without ctl support. ]); have_ctl=no)



dnl
dnl if some autoconf expert knows how to get the actual prefix used, please 
dnl tell us.  Until then...
dnl 
if test "x$prefix" != "xNONE"; then
   ACTUAL_PREFIX=$prefix
else
   ACTUAL_PREFIX=/usr/local
fi



[if (test "${have_ctl}" = yes) ; then
   OPENEXR_CTL_CXXFLAGS="$OPENEXR_CTL_CXXFLAGS -DHAVE_CTL_INTERPRETER=1 -DCTL_MODULE_BASE_PATH=\\\"$ACTUAL_PREFIX/lib/CTL\\\""
else
   OPENEXR_CTL_CXXFLAGS="-DHAVE_CTL_INTERPRETER=0 $OPENEXR_CXXFLAGS"
   OPENEXR_CTL_LIBS="-DHAVE_CTL_INTERPRETER=0 $OPENEXR_LIBS"
   OPENEXR_CTL_LDFLAGS="-DHAVE_CTL_INTERPRETER=0 $OPENEXR_LDFLAGS"
fi]

dnl
dnl These are already shared by AM_PATH_PKGCONFIG... now we update them
dnl with the latest changes
AC_SUBST(OPENEXR_CTL_CXXFLAGS)
AC_SUBST(OPENEXR_CTL_LDFLAGS)
AC_SUBST(OPENEXR_CTL_LIBS)


dnl
dnl check for alternate OpenGL headers
dnl
AM_PATH_GL(,)

dnl
dnl check for FLTK
dnl
AM_PATH_FLTK(have_fltk=yes, have_fltk=no)

dnl
dnl check for Cg
dnl
if test "${have_fltk}" = yes; then
  AM_PATH_CG(have_cg=yes, have_cg=no)
  if test "${have_cg}" = yes; then
    AC_DEFINE(HAVE_FRAGMENT_SHADERS)
  fi
else have_cg=no
fi


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

dnl gcc optimization
AC_MSG_CHECKING(for gcc optimization flags)
old_cflags=$CFLAGS
CFLAGS="$CFLAGS -pipe"
AC_TRY_COMPILE([#include <stdio.h>],
[ printf ("hello, world"); ],
[ EXTRA_OPT_CFLAGS="-pipe"],[ EXTRA_OPT_CFLAGS=""])
CFLAGS=$old_cflags
AC_MSG_RESULT([$EXTRA_OPT_CFLAGS])

dnl Platform-specific stuff
case "$host" in
*darwin*) 
  AC_DEFINE(HAVE_DARWIN) 

  dnl OS X universal binary support, requires --disable-dependency-tracking
  AC_ARG_ENABLE(osx-universal-binaries,
	      AC_HELP_STRING([--enable-osx-universal-binaries],
			     [build universal binaries on OS X [[default=no]]]),
	      [build_osxuniversal="${enableval}"], [build_osxuniversal=no])

  if test "${build_osxuniversal}" != no ; then
    if test "$enable_dependency_tracking" != no ; then
      AC_MSG_ERROR([--enable-osx-universal-binary requires --disable-dependency-tracking.
Please re-run configure with these options:
  --disable-dependency-tracking --enable-osx-universal-binary
  		  ])
    fi
    CXXFLAGS="$CXXFLAGS -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386"
    dnl LDFLAGS="$LDFLAGS -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386"
  fi

  CXXFLAGS="$CXXFLAGS"
  ;;
esac

AM_CFLAGS="$EXTRA_OPT_CFLAGS"
AM_CXXFLAGS="$EXTRA_OPT_CFLAGS"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)

AM_CONDITIONAL(HAVE_FLTK, test "x$have_fltk" = xyes)
AM_CONDITIONAL(HAVE_CG, test "x$have_cg" = xyes)
AM_CONDITIONAL(HAVE_CTL, test "x$have_ctl" = xyes)

AC_OUTPUT([
Makefile
config/Makefile
exrdisplay/Makefile
playexr/Makefile
doc/Makefile
])

if test "x$build_osxuniversal" = xyes; then
  AC_MSG_RESULT([
---------------------------------------------
Summary for OpenEXR features:

build exrdisplay FLTK application		$have_fltk
    with fragment shader support		$have_cg
ctl support                                     $have_ctl
build OS X universal binaries                   $build_osxuniversal
---------------------------------------------
  ])
else
  AC_MSG_RESULT([
---------------------------------------------
Summary for OpenEXR features:

build exrdisplay FLTK application		$have_fltk
    with fragment shader support		$have_cg
ctl support                                     $have_ctl
---------------------------------------------
  ])
fi
