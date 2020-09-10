dnl
dnl SPDX-License-Identifier: BSD-3-Clause
dnl Copyright Contributors to the OpenEXR Project.
dnl

dnl Process this file with autoconf to produce a configure script.
AC_INIT(IlmBase, 2.4.1)

AC_SUBST(ILMBASE_VERSION_MAJOR, 2)
AC_SUBST(ILMBASE_VERSION_MINOR, 4)
AC_SUBST(ILMBASE_VERSION_PATCH, 1)

AC_SUBST(ILMBASE_VERSION, ${ILMBASE_VERSION_MAJOR}.${ILMBASE_VERSION_MINOR}.${ILMBASE_VERSION_PATCH})
AC_SUBST(ILMBASE_VERSION_API, ${ILMBASE_VERSION_MAJOR}_${ILMBASE_VERSION_MINOR})

AC_CANONICAL_HOST
AC_CONFIG_SRCDIR(ImathTest/main.cpp)
AC_CONFIG_HEADER(config/IlmBaseConfig.h)
AC_CONFIG_HEADER(config/IlmBaseConfigInternal.h)
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

if test -z "${PKG_CONFIG_PATH}"; then
	PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
fi

export PKG_CONFIG_PATH

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
        AC_DEFINE(ILMBASE_FORCE_CXX03)
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

dnl Checks for libraries.
dnl --enable-threading
AC_ARG_ENABLE(threading,
              AC_HELP_STRING([--enable-threading],
                             [enable multi-threading [[default=yes]]]),
              [multithread="${enableval}"], [multithread=yes])

if test "${multithread}" != no ; then
    ACX_PTHREAD(
    [
	AC_DEFINE(HAVE_PTHREAD)
	LIBS="$PTHREAD_LIBS $LIBS"
	CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
	CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
	CC="$PTHREAD_CC"
	
	AM_POSIX_SEM()
    ],
    [AC_MSG_ERROR([POSIX thread support required])])
    AC_MSG_NOTICE([multithread true, LIBS = $LIBS, CC = $CC, CXXFLAGS = $CXXFLAGS])
fi

dnl
dnl Checks for header files.
dnl
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h unistd.h ucontext.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS(strerror)


dnl
dnl Checks for std::right etc. in iomanip
dnl
AC_MSG_CHECKING(for complete iomanip support in C++ standard library)
complete_iomanip="no"
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE([#include <iomanip>],[

	std::right;
],
AC_DEFINE(HAVE_COMPLETE_IOMANIP) complete_iomanip=yes)
AC_MSG_RESULT($complete_iomanip)


dnl
dnl Checks for dealing with fpe handling
dnl
AC_MSG_CHECKING(for fpe support handling)
control_register_support="no"
AC_TRY_COMPILE([
#include <stdint.h>
#include <ucontext.h>
],[
    ucontext_t ucon;
    uint32_t mxcsr = ucon.uc_mcontext.fpregs->mxcsr;
    uint16_t cw    = ucon.uc_mcontext.fpregs->cwd;

],
AC_DEFINE(ILMBASE_HAVE_CONTROL_REGISTER_SUPPORT) control_register_support=yes)
AC_MSG_RESULT($control_register_support)

AC_LANG_RESTORE

dnl --enable-large-stack
case "$host" in
*linux*)
  AC_ARG_ENABLE(large-stack,
		AC_HELP_STRING([--enable-large-stack],
			       [enable optimizations for systems that support
			       large stack sizes [[default=yes]]]),
		[large_stack="${enableval}"],
		[large_stack=yes])
  ;;
*)
  AC_ARG_ENABLE(large-stack,
		AC_HELP_STRING([--enable-large-stack],
			       [enable optimizations for systems that support
			       large stack sizes [[default=no]]]),
		[large_stack="${enableval}"],
		[large_stack=no])
  ;;
esac

if test "x${large_stack}" != xno ; then
    AC_DEFINE(ILMBASE_HAVE_LARGE_STACK)
fi

dnl Define the version string
AC_DEFINE_UNQUOTED(ILMBASE_VERSION_STRING, "${VERSION}")
AC_DEFINE_UNQUOTED(ILMBASE_PACKAGE_STRING, "${PACKAGE_STRING}")
AC_DEFINE_UNQUOTED(ILMBASE_VERSION_MAJOR, ${ILMBASE_VERSION_MAJOR})
AC_DEFINE_UNQUOTED(ILMBASE_VERSION_MINOR, ${ILMBASE_VERSION_MINOR})
AC_DEFINE_UNQUOTED(ILMBASE_VERSION_PATCH, ${ILMBASE_VERSION_PATCH})



AC_MSG_CHECKING(for gcc optimization flags)
old_cflags=$CFLAGS
CFLAGS="$CFLAGS -pipe"
AC_TRY_COMPILE([#include <stdio.h>],
[ printf ("hello, world"); ],
[ EXTRA_OPT_CFLAGS="-pipe"],[ EXTRA_OPT_CFLAGS=""])
CFLAGS=$old_cflags
AC_MSG_RESULT([$EXTRA_OPT_CFLAGS])



dnl
dnl Handle namespacing configuration : internal library namespace
dnl                                    public namespace 
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

dnl Internal library namespace
dnl 
lib_namespace=""
if test "x${library_namespace_versioning}" == xyes ; then
    AC_DEFINE_UNQUOTED(IMATH_INTERNAL_NAMESPACE,     Imath_${ILMBASE_VERSION_API})
    AC_DEFINE_UNQUOTED(IEX_INTERNAL_NAMESPACE,       Iex_${ILMBASE_VERSION_API})
    AC_DEFINE_UNQUOTED(ILMTHREAD_INTERNAL_NAMESPACE, IlmThread_${ILMBASE_VERSION_API})
    AC_DEFINE(ILMBASE_INTERNAL_NAMESPACE_CUSTOM)
    
    lib_namespace="${ILMBASE_VERSION_API}"
    LIB_SUFFIX="${ILMBASE_VERSION_API}"
    lib_suffix_valid="yes"
elif test "x${library_namespace_versioning}" == xno ; then
    AC_DEFINE_UNQUOTED(IMATH_INTERNAL_NAMESPACE,     Imath)
    AC_DEFINE_UNQUOTED(IEX_INTERNAL_NAMESPACE,       Iex)
    AC_DEFINE_UNQUOTED(ILMTHREAD_INTERNAL_NAMESPACE, IlmThread)
    lib_namespace=""
else
    AC_DEFINE_UNQUOTED(IMATH_INTERNAL_NAMESPACE,     Imath_${library_namespace_versioning})
    AC_DEFINE_UNQUOTED(IEX_INTERNAL_NAMESPACE,       Iex_${library_namespace_versioning})
    AC_DEFINE_UNQUOTED(ILMTHREAD_INTERNAL_NAMESPACE, IlmThread_${library_namespace_versioning})
    AC_DEFINE(ILMBASE_INTERNAL_NAMESPACE_CUSTOM)
    
    lib_namespace="${library_namespace_versioning}"
    LIB_SUFFIX="${library_namespace_versioning}"
    lib_suffix_valid="yes"
fi
AC_SUBST(LIB_SUFFIX)

if test "x${lib_suffix_valid}" == xyes ; then
AC_SUBST(LIB_SUFFIX_DASH,"-${LIB_SUFFIX}")
AM_CONDITIONAL(LIB_SUFFIX_EXISTS,true)
else
AC_SUBST(LIB_SUFFIX_DASH,"")
AM_CONDITIONAL(LIB_SUFFIX_EXISTS,false)
fi


dnl Public User namespace 
dnl 
custom_public_namespace="no"
public_namespace=""
AC_ARG_ENABLE(customusernamespace,
        AC_HELP_STRING([--enable-customusernamespace],
                       [user namespace; this is the namespace into which the library namespace will be exported to <no/custom> [[default=Imf]]] ),
        [custom_public_namespace="${enableval}"], 
        [custom_public_namespace=no])

if test "x${custom_public_namespace}" = xyes ; then
    AC_MSG_WARN([Enabling 'custom user namespace' requires an additional argument, reverting to default])
    AC_DEFINE(IMATH_NAMESPACE,     Imath)
    AC_DEFINE(IEX_NAMESPACE,       Iex)
    AC_DEFINE(ILMTHREAD_NAMESPACE, IlmThread)

    public_namespace=""
    custom_public_namespace=no
elif test "x${custom_public_namespace}" = xno ; then
    AC_DEFINE(IMATH_NAMESPACE,     Imath)
    AC_DEFINE(IEX_NAMESPACE,       Iex)
    AC_DEFINE(ILMTHREAD_NAMESPACE, IlmThread)
    
    public_namespace=""
else
    AC_DEFINE_UNQUOTED(IMATH_NAMESPACE,     Imath_${custom_public_namespace})
    AC_DEFINE_UNQUOTED(IEX_NAMESPACE,       Iex_${custom_public_namespace})
    AC_DEFINE_UNQUOTED(ILMTHREAD_NAMESPACE, IlmThread_${custom_public_namespace})
    AC_DEFINE(ILMBASE_NAMESPACE_CUSTOM)
    
    public_namespace=${custom_public_namespace}
fi



dnl Platform-specific stuff
case "$host" in
*darwin*) 
  dnl Compiler flags for all OS X builds

  dnl OSX architectures specification
  dnl NOTE: multiple architectures requires --disable-dependency-tracking
  dnl For more information: http://developer.apple.com/documentation/Porting/Conceptual/PortingUNIX/compiling/chapter_4_section_3.html
  AC_ARG_ENABLE(osx-arch,
	      AC_HELP_STRING([--enable-osx-arch],
			     [specify the architecture(s) for OS X builds. Ex: --enable-osx-arch="ppc i386"]),
	      [osx_arch="${enableval}"], [osx_arch=default])

  if test "${osx_arch}" != default ; then
     osx_arch_flags=$(printf ' -arch %s' ${osx_arch})
     CXXFLAGS="$CXXFLAGS $osx_arch_flags"

     dnl If multiple archs specified, make sure that --disable-dependency-tracking was also specified
     if test "$enable_dependency_tracking" != no ; then
        if echo "$osx_arch" | grep >/dev/null " " ; then
          AC_MSG_ERROR([building multiple OS X architectures requires --disable-dependency-tracking.
Please re-run configure with these options:
  --enable-osx-arch="${osx_arch}" --disable-dependency-tracking
  		  ])
        fi
     fi
  fi

  dnl OSX SDK specification
  AC_ARG_ENABLE(osx-sdk,
	      AC_HELP_STRING([--enable-osx-sdk],
			     [specify the sdk (sysroot) for use with OS X. Ex: --enable-osx-sdk=MacOSX10.5]),
	      [osx_sdk="${enableval}"], [osx_sdk=default])

  if test "${osx_sdk}" != default ; then
    CXXFLAGS="$CXXFLAGS -isysroot /Developer/SDKs/${osx_sdk}"
  fi

  dnl OS X universal binary support, requires --disable-dependency-tracking
  dnl NOTE: this argument is obsolete, and has been replaced by --enable-osx-sdk and --enable-osx-arch
  dnl This is equivalent to --enable-osx-sdk=MacOSX10.4u.sdk --enable-osx-arch="ppc i386"
  AC_ARG_ENABLE(osx-universal-binaries,
	      AC_HELP_STRING([--enable-osx-universal-binaries],
			     [build two-way universal (ppc, i386) binaries on OS X. OBSOLETE: Consider using --enable-osx-sdk and --enable-osx-arch instead. [[default=no]]]),
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
  ;;
esac

AM_CFLAGS="$EXTRA_OPT_CFLAGS"
AM_CXXFLAGS="$EXTRA_OPT_CFLAGS"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(HAVE_UCONTEXT_H)


AC_OUTPUT([
Makefile
IlmBase.pc
config/Makefile
Half/Makefile
HalfTest/Makefile
Iex/Makefile
IexMath/Makefile
IexTest/Makefile
IlmThread/Makefile
Imath/Makefile
ImathTest/Makefile
])


dnl
dnl Summary Configuration Messages
dnl
AC_MSG_RESULT([
---------------------------------------------
Summary for IlmBase features:
enable large stack optimizations             $large_stack
ISO C++ Standard                             $cxxstd])
  
if test "x${library_namespace_versioning}" != xno ; then
  AC_MSG_RESULT([
internal library namespaces                  Imath_$lib_namespace 
                                             Iex_$lib_namespace 
                                             IlmThread_$lib_namespace])
else
  AC_MSG_RESULT([
internal library namespaces                  Imath Iex IlmThread])
fi

if test "x${custom_public_namespace}" != xno ; then
  AC_MSG_RESULT([
public namespaces                            Imath_$public_namespace 
                                             Iex_$public_namespace 
                                             IlmThread_$public_namespace])
else
  AC_MSG_RESULT([
public namespaces                            Imath Iex IlmThread])
fi


if test "$osx_arch"; then
  AC_MSG_RESULT([
OS X architectures                           $osx_arch
OS X SDK                                     $osx_sdk
OS X two-way universal build (deprecated)    $build_osxuniversal])
fi

AC_MSG_RESULT([
---------------------------------------------
])
