dnl
dnl SPDX-License-Identifier: BSD-3-Clause
dnl Copyright Contributors to the OpenEXR Project.
dnl

dnl Process this file with autoconf to produce a configure script.

AC_INIT(OpenEXR, 2.4.1)
AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(OPENEXR_VERSION_MAJOR, 2)
AC_SUBST(OPENEXR_VERSION_MINOR, 4)
AC_SUBST(OPENEXR_VERSION_PATCH, 1)

AC_SUBST(OPENEXR_VERSION, ${OPENEXR_VERSION_MAJOR}.${OPENEXR_VERSION_MINOR}.${OPENEXR_VERSION_PATCH})
AC_SUBST(OPENEXR_VERSION_API, ${OPENEXR_VERSION_MAJOR}_${OPENEXR_VERSION_MINOR})

AC_CANONICAL_HOST
AC_CONFIG_SRCDIR(IlmImfTest/main.cpp)
AC_CONFIG_HEADERS([config/OpenEXRConfig.h])
AC_CONFIG_HEADERS([config/OpenEXRConfigInternal.h])
AM_INIT_AUTOMAKE(1.6.3)  dnl Require automake 1.6.3 or better
AM_MAINTAINER_MODE


LIBTOOL_CURRENT=24
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

dnl Checks for zlib
PKG_CHECK_MODULES([ZLIB], [zlib], [], [AC_CHECK_LIB(z, compress, [:], [AC_MSG_ERROR([
*** OpenEXR requires a recent version of zlib, which you don't appear to have.
*** have.
***
*** This could be because the run-time linker is not finding zlib, or it
*** is finding the wrong version.  In this case, you'll need to set your
*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point
*** to the proper version.  Also, make sure you have run ldconfig if
*** that is required on your system.
])])])
AM_PATH_PKGCONFIG(
    [ILMBASE_CXXFLAGS],
    [ILMBASE_LDFLAGS],
    [ILMBASE_LIBS],
    [IlmBase],
    [OpenEXR],
    [],
    [-lImath -lHalf -lIlmThread -lIex -lpthread -lz],
    [ilmbase-prefix],
    [ILMBASE])

dnl Define the version string
AC_DEFINE_UNQUOTED([OPENEXR_VERSION_STRING], "${VERSION}", [OpenEXR version string])
AC_DEFINE_UNQUOTED([OPENEXR_PACKAGE_STRING], "${PACKAGE_STRING}", [OpenEXR version string])
AC_DEFINE_UNQUOTED([OPENEXR_VERSION_MAJOR], [${OPENEXR_VERSION_MAJOR}], [OpenEXR version string])
AC_DEFINE_UNQUOTED([OPENEXR_VERSION_MINOR], [${OPENEXR_VERSION_MINOR}], [OpenEXR version string])
AC_DEFINE_UNQUOTED([OPENEXR_VERSION_PATCH], [${OPENEXR_VERSION_PATCH}], [OpenEXR version string])

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

AS_IF([test "x${multithread}" != xno], [
	ACX_PTHREAD(
    [
      AC_DEFINE(OPENEXR_IMF_HAVE_PTHREAD)
      ILMBASE_LIBS="$PTHREAD_LIBS $ILMBASE_LIBS"
      ILMBASE_CXXFLAGS="$ILMBASE_CXXFLAGS $PTHREAD_CFLAGS"
      CC="$PTHREAD_CC"

      AM_POSIX_SEM()
    ],
    [AC_MSG_ERROR([POSIX thread support required])])

	AC_DEFINE([OPENEXR_IMF_HAVE_PTHREAD], [1], [Define if pthreads are available])

	CFLAGS="${CFLAGS} ${PTHREAD_CFLAGS}"
	CXXFLAGS="${CXXFLAGS} ${PTHREAD_CFLAGS}"
	LIBS="${LIBS} ${PTHREAD_CFLAGS}"
])

save_CXXFLAGS="${CXXFLAGS}"
save_LIBS="${LIBS}"
CXXFLAGS="${save_CXXFLAGS} ${ILMBASE_CFLAGS}"
LIBS="${save_LIBS} ${ILMBASE_LIBS}"
AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[
		#include <stdlib.h>
		#include <ImathFun.h>
	]], [[
		double d = IMATH_NAMESPACE::succd(.23); d+= .2;
	]])
])
CXXFLAGS="${save_CXXFLAGS}"
LIBS="${save_LIBS}"


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

dnl don't need this check anymore..
dnl dnl Checks for std::right etc. in iomanip
dnl AC_MSG_CHECKING(for complete iomanip support in C++ standard library)
dnl complete_iomanip="no"
dnl AC_LANG_SAVE
dnl AC_LANG_CPLUSPLUS
dnl AC_TRY_COMPILE([#include <iomanip>],[
dnl  	std::right;
dnl ],
dnl AC_DEFINE(OPENEXR_IMF_HAVE_COMPLETE_IOMANIP) complete_iomanip=yes)
dnl
dnl AC_MSG_RESULT($complete_iomanip)
dnl AC_LANG_RESTORE
AC_DEFINE([OPENEXR_IMF_HAVE_COMPLETE_IOMANIP], [1], [Define when std::right is available])

AC_MSG_CHECKING(for gcc optimization flags)
old_cflags=$CFLAGS
CFLAGS="$CFLAGS -pipe"
AC_TRY_COMPILE([#include <stdio.h>],
[ printf ("hello, world"); ],
[ EXTRA_OPT_CFLAGS="-pipe"],[ EXTRA_OPT_CFLAGS=""])
CFLAGS=$old_cflags
AC_MSG_RESULT([$EXTRA_OPT_CFLAGS])


dnl Check to see if the toolset supports AVX instructions in inline asm
AC_ARG_ENABLE([avx],
	AS_HELP_STRING([--enable-avx], [Enable avx optimization]))

AS_IF([test "x$enable_avx" = "xyes"], [
	dnl Enable AVX
	gcc_inline_asm_avx="yes"
	AC_DEFINE([OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX], [1], [Define if AVX is available])
], [
	dnl Disable AVX
	gcc_inline_asm_avx="no"
])

AM_CFLAGS="$EXTRA_OPT_CFLAGS"
AM_CXXFLAGS="$EXTRA_OPT_CFLAGS"
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)

dnl Check if sysconf(_SC_NPROCESSORS_ONLN) can be used for CPU count
AC_MSG_CHECKING([for sysconf(_SC_NPROCESSORS_ONLN)])
sysconf_nproc="no"
AC_COMPILE_IFELSE(
   [
        AC_LANG_PROGRAM(
           [#include <unistd.h>],
           [sysconf(_SC_NPROCESSORS_ONLN);]
        )
   ],
   [sysconf_nproc="yes"],
   [sysconf_nproc="no"]
)
AC_MSG_RESULT([$sysconf_nproc])
if test "x${sysconf_nproc}" = xyes ; then
    AC_DEFINE([OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN], [1], [Define if sysconf(_SC_NPROCESSORS_ONLN) can be used for CPU count])
fi

dnl Platform-specific stuff
case "$host" in
*linux*)
  AC_DEFINE([OPENEXR_IMF_HAVE_LINUX_PROCFS], [1], [Define if procfs is available])
  ;;
*darwin*) 
  AC_DEFINE([OPENEXR_IMF_HAVE_DARWIN], [1], [Define if on Darwin])

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
    dnl CXXFLAGS="$CXXFLAGS -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386"
    dnl LDFLAGS="$LDFLAGS -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386"
  fi

  ;;
esac


dnl build imfexamples example program?
build_imfexamples="no"
AC_ARG_ENABLE(imfexamples,
	      AC_HELP_STRING([--enable-imfexamples],
			     [build IlmImf example program [[default=no]]]),
	      [build_imfexamples="${enableval}"], [build_imfexamples=no])

AM_CONDITIONAL(BUILD_IMFEXAMPLES, test "x$build_imfexamples" = xyes)

dnl build imffuzztest?
build_imffuzztest="no"
AC_ARG_ENABLE(imffuzztest,
	  AC_HELP_STRING([--enable-imffuzztest],
		 [build IlmImf damaged input resilience test [[default=no]]]),
	  [build_imffuzztest="${enableval}"], [build_imffuzztest=no])

AM_CONDITIONAL(BUILD_IMFFUZZTEST, test "x$build_imffuzztest" = xyes)

dnl build imfhugetest?
build_imfhugetest="no"
AC_ARG_ENABLE(imfhugetest,
	  AC_HELP_STRING([--enable-imfhugetest],
		 [build IlmImf huge input resilience test [[default=no]]]),
	  [build_imfhugetest="${enableval}"], [build_imfhugetest=no])

AM_CONDITIONAL(BUILD_IMFHUGETEST, test "x$build_imfhugetest" = xyes)

if test "x${build_imfhugetest}" != xno ; then
    AC_DEFINE([OPENEXR_IMF_HUGETEST], [1], [Define if IlmImf huge input resilience])
fi


dnl
dnl Handle namespacing configuration : internal library namespace
dnl                                    user-client namespace 
dnl
library_namespace_versioning="yes"
AC_ARG_ENABLE(namespaceversioning,
          AC_HELP_STRING([--enable-namespaceversioning],
                         [enable symbol versioning via versioned/custom namespace to prevent runtime conflicts <no/yes/custom> [[default=yes]]] ),
          [library_namespace_versioning="${enableval}"], 
          [library_namespace_versioning=yes])
              
dnl Suffix for the shared library via the '-release' option
LIB_SUFFIX=""
lib_suffix_valid="no"

lib_namespace="Imf"
if test "x${library_namespace_versioning}" = xyes ; then
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_INTERNAL_NAMESPACE], [Imf_${OPENEXR_VERSION_API}], [OpenEXR])
    AC_DEFINE([OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM], [1], [OpenEXR])

    lib_namespace="Imf_${OPENEXR_VERSION_API}"
    LIB_SUFFIX="${OPENEXR_VERSION_API}"
    lib_suffix_valid="yes"
elif test "x${library_namespace_versioning}" = xno ; then
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_INTERNAL_NAMESPACE], [Imf], [OpenEXR])

    lib_namespace="Imf"
else
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_INTERNAL_NAMESPACE], [${library_namespace_versioning}], [OpenEXR])
    AC_DEFINE([OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM], [1], [OpenEXR])

    lib_namespace="${library_namespace_versioning}"
    LIB_SUFFIX="${library_namespace_versioning}"
    lib_suffix_valid="yes"
fi
AC_SUBST(LIB_SUFFIX)

if test "x${lib_suffix_valid}" = xyes ; then
AC_SUBST(LIB_SUFFIX_DASH,"-${LIB_SUFFIX}")
AM_CONDITIONAL(LIB_SUFFIX_EXISTS,true)
else
AC_SUBST(LIB_SUFFIX_DASH,"")
AM_CONDITIONAL(LIB_SUFFIX_EXISTS,false)
fi


dnl
dnl User namespace 
dnl 
custom_usr_namespace="no"
usr_namespace="Imf"
AC_ARG_ENABLE(customusernamespace,
        AC_HELP_STRING([--enable-customusernamespace],
                       [user namespace; this is the namespace into which the library namespace will be exported to <no/custom> [[default=Imf]]] ),
        [custom_usr_namespace="${enableval}"], 
        [custom_usr_namespace=no])

if test "x${custom_usr_namespace}" = xyes ; then
    AC_MSG_WARN([Enabling 'custom user namespace' requires an additional argument, reverting to 'Imf'])
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_NAMESPACE], [Imf], [OpenEXR])
    usr_namespace="Imf"
elif test "x${custom_usr_namespace}" = xno ; then
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_NAMESPACE], [Imf], [OpenEXR])
    usr_namespace="Imf"
else
    AC_DEFINE_UNQUOTED([OPENEXR_IMF_NAMESPACE], [${custom_usr_namespace}], [OpenEXR])
    AC_DEFINE([OPENEXR_IMF_NAMESPACE_CUSTOM], [1], [OpenEXR])
    
    usr_namespace=${custom_usr_namespace}
fi



AC_OUTPUT([
Makefile
OpenEXR.pc
config/Makefile
IlmImf/Makefile
IlmImfTest/Makefile
IlmImfUtil/Makefile
IlmImfUtilTest/Makefile
IlmImfFuzzTest/Makefile
exrheader/Makefile
exr2aces/Makefile
exrmaketiled/Makefile
IlmImfExamples/Makefile
doc/Makefile
exrstdattr/Makefile
exrmakepreview/Makefile
exrenvmap/Makefile
exrmultiview/Makefile
exrmultipart/Makefile
])

AC_MSG_RESULT([
---------------------------------------------
Summary for OpenEXR features:

build IlmImf example program                    $build_imfexamples
build IlmImf damaged input resilience test      $build_imffuzztest
build IlmImf huge input test                    $build_imfhugetest
enable large stack optimizations                $large_stack
internal library namespace                      $lib_namespace
user-client namespace                           $usr_namespace])

if test "x$build_osxuniversal" = xyes; then
AC_MSG_RESULT([
build OS X universal binaries                   $build_osxuniversal])
fi

AC_MSG_RESULT([
---------------------------------------------
])
