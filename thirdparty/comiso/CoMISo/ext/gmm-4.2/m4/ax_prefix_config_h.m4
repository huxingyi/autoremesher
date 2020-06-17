dnl @synopsis AX_PREFIX_CONFIG_H [(OUTPUT-HEADER [,PREFIX [,ORIG-HEADER]])]
dnl
dnl This is a new variant from ac_prefix_config_ this one will use a
dnl lowercase-prefix if the config-define was starting with a
dnl lowercase-char, e.g. "#define const", "#define restrict", or
dnl "#define off_t", (and this one can live in another directory, e.g.
dnl testpkg/config.h therefore I decided to move the output-header to
dnl be the first arg)
dnl
dnl takes the usual config.h generated header file; looks for each of
dnl the generated "#define SOMEDEF" lines, and prefixes the defined name
dnl (ie. makes it "#define PREFIX_SOMEDEF". The result is written to
dnl the output config.header file. The PREFIX is converted to uppercase
dnl for the conversions.
dnl
dnl Defaults:
dnl
dnl   OUTPUT-HEADER = $PACKAGE-config.h
dnl   PREFIX = $PACKAGE
dnl   ORIG-HEADER, from AM_CONFIG_HEADER(config.h)
dnl
dnl Your configure.ac script should contain both macros in this order,
dnl and unlike the earlier variations of this prefix-macro it is okay to
dnl place the AX_PREFIX_CONFIG_H call before the AC_OUTPUT invokation.
dnl
dnl Example:
dnl
dnl   AC_INIT(config.h.in)        # config.h.in as created by "autoheader"
dnl   AM_INIT_AUTOMAKE(testpkg, 0.1.1)    # makes #undef VERSION and PACKAGE
dnl   AM_CONFIG_HEADER(config.h)          # prep config.h from config.h.in
dnl   AX_PREFIX_CONFIG_H(mylib/_config.h) # prep mylib/_config.h from it..
dnl   AC_MEMORY_H                         # makes "#undef NEED_MEMORY_H"
dnl   AC_C_CONST_H                        # makes "#undef const"
dnl   AC_OUTPUT(Makefile)                 # creates the "config.h" now
dnl                                       # and also mylib/_config.h
dnl
dnl if the argument to AX_PREFIX_CONFIG_H would have been omitted then the
dnl default outputfile would have been called simply "testpkg-config.h", but
dnl even under the name "mylib/_config.h" it contains prefix-defines like
dnl
dnl   #ifndef TESTPKG_VERSION
dnl   #define TESTPKG_VERSION "0.1.1"
dnl   #endif
dnl   #ifndef TESTPKG_NEED_MEMORY_H
dnl   #define TESTPKG_NEED_MEMORY_H 1
dnl   #endif
dnl   #ifndef _testpkg_const
dnl   #define _testpkg_const _const
dnl   #endif
dnl
dnl and this "mylib/_config.h" can be installed along with other
dnl header-files, which is most convenient when creating a shared
dnl library (that has some headers) where some functionality is
dnl dependent on the OS-features detected at compile-time. No
dnl need to invent some "mylib-confdefs.h.in" manually. :-)
dnl
dnl Note that some AC_DEFINEs that end up in the config.h file are
dnl actually self-referential - e.g. AC_C_INLINE, AC_C_CONST, and the
dnl AC_TYPE_OFF_T say that they "will define inline|const|off_t if the
dnl system does not do it by itself". You might want to clean up about
dnl these - consider an extra mylib/conf.h that reads something like:
dnl
dnl    #include <mylib/_config.h>
dnl    #ifndef _testpkg_const
dnl    #define _testpkg_const const
dnl    #endif
dnl
dnl and then start using _testpkg_const in the header files. That is
dnl also a good thing to differentiate whether some library-user has
dnl starting to take up with a different compiler, so perhaps it could
dnl read something like this:
dnl
dnl   #ifdef _MSC_VER
dnl   #include <mylib/_msvc.h>
dnl   #else
dnl   #include <mylib/_config.h>
dnl   #endif
dnl   #ifndef _testpkg_const
dnl   #define _testpkg_const const
dnl   #endif
dnl
dnl @version $Id: ax_prefix_config_h.m4 1867 2005-01-27 14:04:04Z pommier $
dnl @author  Guiodo Draheim <guidod@gmx.de>
dnl
AC_DEFUN([AX_PREFIX_CONFIG_H],[AC_REQUIRE([AC_CONFIG_HEADER])
AC_CONFIG_COMMANDS([ifelse($1,,$PACKAGE-config.h,$1)],[dnl
AS_VAR_PUSHDEF([_OUT],[ac_prefix_conf_OUT])dnl
AS_VAR_PUSHDEF([_DEF],[ac_prefix_conf_DEF])dnl
AS_VAR_PUSHDEF([_PKG],[ac_prefix_conf_PKG])dnl
AS_VAR_PUSHDEF([_LOW],[ac_prefix_conf_LOW])dnl
AS_VAR_PUSHDEF([_UPP],[ac_prefix_conf_UPP])dnl
AS_VAR_PUSHDEF([_INP],[ac_prefix_conf_INP])dnl
m4_pushdef([_script],[conftest.prefix])dnl
m4_pushdef([_symbol],[m4_cr_Letters[]m4_cr_digits[]_])dnl
_OUT=`echo ifelse($1, , $PACKAGE-config.h, $1)`
_DEF=`echo _$_OUT | sed -e "y:m4_cr_letters:m4_cr_LETTERS[]:" -e "s/@<:@^m4_cr_Letters@:>@/_/g"`
_PKG=`echo ifelse($2, , $PACKAGE, $2)`
_LOW=`echo _$_PKG | sed -e "y:m4_cr_LETTERS-:m4_cr_letters[]_:"`
_UPP=`echo $_PKG | sed -e "y:m4_cr_letters-:m4_cr_LETTERS[]_:"  -e "/^@<:@m4_cr_digits@:>@/s/^/_/"`
_INP=`echo "ifelse($3,,,$3)" | sed -e 's/ *//'`
if test ".$_INP" = "."; then
   for ac_file in : $CONFIG_HEADERS; do test "_$ac_file" = _: && continue
     case "$ac_file" in
        *.h) _INP=$ac_file ;;
        *)
     esac
     test ".$_INP" != "." && break
   done
fi
if test ".$_INP" = "."; then
   case "$_OUT" in
      */*) _INP=`basename "$_OUT"`
      ;;
      *-*) _INP=`echo "$_OUT" | sed -e "s/@<:@_symbol@:>@*-//"`
      ;;
      *) _INP=config.h
      ;;
   esac
fi
if test -z "$_PKG" ; then
   AC_MSG_ERROR([no prefix for _PREFIX_PKG_CONFIG_H])
else
  if test ! -f "$_INP" ; then if test -f "$srcdir/$_INP" ; then
     _INP="$srcdir/$_INP"
  fi fi
  AC_MSG_NOTICE(creating $_OUT - prefix $_UPP for $_INP defines)
  if test -f $_INP ; then
    echo "s/@%:@undef  *\\(@<:@m4_cr_LETTERS[]_@:>@\\)/@%:@undef $_UPP""_\\1/" > _script
    echo "s/@%:@undef  *\\(@<:@m4_cr_letters@:>@\\)/@%:@undef $_LOW""_\\1/" >> _script
    echo "s/@%:@def[]ine  *\\(@<:@m4_cr_LETTERS[]_@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef $_UPP""_\\1 \\" >> _script
    echo "@%:@def[]ine $_UPP""_\\1 \\2 \\" >> _script
    echo "@%:@endif/" >>_script
    echo "s/@%:@def[]ine  *\\(@<:@m4_cr_letters@:>@@<:@_symbol@:>@*\\)\\(.*\\)/@%:@ifndef $_LOW""_\\1 \\" >> _script
    echo "@%:@define $_LOW""_\\1 \\2 \\" >> _script
    echo "@%:@endif/" >> _script
    # now executing _script on _DEF input to create _OUT output file
    echo "@%:@ifndef $_DEF"      >$tmp/pconfig.h
    echo "@%:@def[]ine $_DEF 1" >>$tmp/pconfig.h
    echo ' ' >>$tmp/pconfig.h
    echo /'*' $_OUT. Generated automatically at end of configure. '*'/ >>$tmp/pconfig.h

    sed -f _script $_INP >>$tmp/pconfig.h
    echo ' ' >>$tmp/pconfig.h
    echo '/* once:' $_DEF '*/' >>$tmp/pconfig.h
    echo "@%:@endif" >>$tmp/pconfig.h
    if cmp -s $_OUT $tmp/pconfig.h 2>/dev/null; then
      AC_MSG_NOTICE([$_OUT is unchanged])
    else
      ac_dir=`AS_DIRNAME(["$_OUT"])`
      AS_MKDIR_P(["$ac_dir"])
      rm -f "$_OUT"
      mv $tmp/pconfig.h "$_OUT"
    fi
    cp _script _configs.sed
  else
    AC_MSG_ERROR([input file $_INP does not exist - skip generating $_OUT])
  fi
  rm -f conftest.*
fi
m4_popdef([_symbol])dnl
m4_popdef([_script])dnl
AS_VAR_POPDEF([_INP])dnl
AS_VAR_POPDEF([_UPP])dnl
AS_VAR_POPDEF([_LOW])dnl
AS_VAR_POPDEF([_PKG])dnl
AS_VAR_POPDEF([_DEF])dnl
AS_VAR_POPDEF([_OUT])dnl
],[PACKAGE="$PACKAGE"])])
