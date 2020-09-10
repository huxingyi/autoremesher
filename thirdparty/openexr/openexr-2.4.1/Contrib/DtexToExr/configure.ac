dnl
dnl SPDX-License-Identifier: BSD-3-Clause
dnl Copyright Contributors to the OpenEXR Project.
dnl

dnl Process this file with autoconf to produce a configure script.
AC_INIT(DtexToExr, 1.0.0)
AC_SUBST(DTEXTOEXR_VERSION, 1.0.0)
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR(DtexToExr.cpp)
AM_INIT_AUTOMAKE(1.6.3)  dnl Require automake 1.6.3 or better
AM_MAINTAINER_MODE


LIBTOOL_CURRENT=6
LIBTOOL_REVISION=0
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


dnl
dnl get ccflags and libs from openexr and prmansdk packages
dnl

dnl
dnl get ccflags and libs from other packages in use and check 
dnl whether test programs compile.
dnl OpenEXR is required. IlmBase is required.
dnl

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


PRMANSDK_CXXFLAGS=""
PRMANSDK_LDFLAGS=""
PRMANSDK_LIBS=""
PRMANSDK_LIBNAME="prman"

AC_ARG_WITH(
   [prmansdk-include-dir],
   [AS_HELP_STRING([--with-prmansdk-include-dir],
   [prman sdk include directory])],
   [PRMANSDK_CXXFLAGS="-I$withval"])

AC_ARG_WITH(
   [prmansdk-lib-dir],
   [AS_HELP_STRING([--with-prmansdk-lib-dir],
   [prman sdk library directory])],
   [PRMANSDK_LDFLAGS="-L$withval -Wl,-rpath $withval"])

AC_ARG_WITH(
   [prmansdk-libname],
   [AS_HELP_STRING([--with-prmansdk-libname],
   [prman sdk library name (default:prman)])],
   [PRMANSDK_LIBNAME="$withval"])

PRMANSDK_LIBS="-l$PRMANSDK_LIBNAME"

export PRMANSDK_CXXFLAGS
export PRMANSDK_LDFLAGS
export PRMANSDK_LIBS
export PRMANSDK_LIBNAME

AC_SUBST([PRMANSDK_CXXFLAGS])
AC_SUBST([PRMANSDK_LDFLAGS])
AC_SUBST([PRMANSDK_LIBS])
AC_SUBST([PRMANSDK_LIBNAME])

CXXFLAGS="$CXXFLAGS $PRMANSDK_CXXFLAGS $OPENEXR_CXXFLAGS"
LDFLAGS="$LDFLAGS $PRMANSDK_LDFLAGS $OPENEXR_LDFLAGS"

dnl
dnl OpenEXR test program
dnl

AM_COMPILELINKRUN(
   [OpenEXR],
   [openexrtest],
   [$OPENEXR_CXXFLAGS],
   [$OPENEXR_LDFLAGS],
   [$OPENEXR_LIBS],[[
#include <stdlib.h>
#include <ImfTestFile.h>
#include <OpenEXRConfig.h>
]],
   [[OPENEXR_IMF_NAMESPACE::isOpenExrFile("notExist");]],
   AC_MSG_RESULT([Compiled and ran OpenEXR test program.]), 
   AC_MSG_ERROR([Could not compile OpenEXR test program.]))

dnl
dnl Dtex test program
dnl

AM_COMPILELINKRUN(
   [PrmanSDK],
   [prmansdktest],
   [$PRMANSDK_CXXFLAGS],
   [$PRMANSDK_LDFLAGS],
   [$PRMANSDK_LIBS],
[[
#include <stdio.h>
#include <stdlib.h>
#include <dtex.h>
]],
[[ 
DtexCache* dtexCache = DtexCreateCache( 10000, NULL );
if ( dtexCache ) { DtexDestroyCache( dtexCache ); }
]],
   AC_MSG_RESULT([Compiled and ran PrmanSDK test program.]), 
   AC_MSG_ERROR([Could not compile PrmanSDK test program.]))


dnl
dnl if some autoconf expert knows how to get the actual prefix used, please 
dnl tell us.  Until then...
dnl 
if test "x$prefix" != "xNONE"; then
   ACTUAL_PREFIX=$prefix
else
   ACTUAL_PREFIX=/usr/local
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

AC_OUTPUT([
Makefile
])

if test "x$build_osxuniversal" = xyes; then
  AC_MSG_RESULT([
---------------------------------------------
Summary for DtexToExr features:

build OS X universal binaries                   $build_osxuniversal
---------------------------------------------
  ])
else
  AC_MSG_RESULT([
---------------------------------------------
Summary for DtexToExr features:

---------------------------------------------
  ])
fi
