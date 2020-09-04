#!/bin/bash
scriptdir=$(dirname "$0")

# 1) Parse command line arguments

EXCLUDE_LIST="
*/medit/*
*/third_party/*
"

# 2) Setup cppcheck defines

PLATFORM_FLAGS="
-DCPPCHECK
-Ustdint_least_defined
-D__TEST_PSTDINT_FOR_CORRECTNESS
-D__linux__ -D__GNUC__ -D__cplusplus -D__x86_64 -D_OPENMP
-U__INTEL_COMPILER -U__ICC
-U__clang__
-UWIN32 -U_WIN32 -UWIN64 -U_WIN64 -U_MSC_VER
-U__APPLE__
-DGEO_OPENMP
-DGEO_OS_LINUX
-UGEO_OS_WINDOWS
-UGEO_DYNAMIC_LIBS
-Ugeogram_gfx_EXPORTS
-Ugeogram_EXPORTS
-Uvorpalib_EXPORTS
-DGEOGRAM_API=
-DVORPALIB_API=
"

COMMON_FLAGS="
-Umin
-Umax
-UNDEBUG
-UDOXYGEN_ONLY
-DVOR_NO_LICENSE
-DGEOGRAM_WITH_TETGEN
-DGEOGRAM_WITH_PDEL
"

MISC_FLAGS="
-UYY_USER_INIT
-UYYTYPE_INT8
-UYYTYPE_INT16
-UYYTYPE_UINT8
-UYYTYPE_UINT16
-Uyyoverflow
-Ushort
-DWIFCONTINUED
-UDBL_DIG
"

SHEWCHUCK_FLAGS="-USINGLE"

MEDIT_FLAGS="-Uubyte -Uushort"

GLEW_FLAGS="
-U__ANDROID__
-UGLEW_MX
-UGLEW_NO_GLU
"

FREEGLUT_FLAGS="
-U__SVR4
-U__sun
-U__FreeBSD__
-U__NetBSD__
-U_WIN32_WCE
-U_WIN32_CE
-DHAVE_CONFIG_H
-DHAVE_STDBOOL_H
-DHAVE_SYS_TIME_H
-DHAVE_SYS_TYPES_H
-DHAVE_UNISTD_H
-DHAVE_ERRNO_H
-DHAVE_SYS_IOCTL_H
-DHAVE_SYS_PARAM_H
-DHAVE_LIMITS_H
-DHAVE__DOPRNT
-DTIME_WITH_SYS_TIME
-DHAVE_X11_EXTENSIONS_XF86VMODE_H
-DHAVE_X11_EXTENSIONS_XINPUT2_H
-DHAVE_X11_EXTENSIONS_XRANDR_H
"

ANTTWEAKBAR_FLAGS="
-D_UNIX
-U_MACOSX
-U_WINDOWS
-UTW_EXPORTS
-UGLFW_CDECL
"

GLUT_VIEWER_FLAGS="
-UWITH_HDR
-DWITH_PNG
"

# 3) Run cppcheck

for p in $EXCLUDE_LIST; do
    exclude_options="$exclude_options -not -path $p"
done

cd $scriptdir/..
find src -type f -regex '.*\.\(c\|h\|cpp\|hpp\)$' $exclude_options |
cppcheck \
    --file-list=- \
    --enable=all \
    --inconclusive \
    --force \
    -I src/lib \
    $PLATFORM_FLAGS \
    $COMMON_FLAGS \
    $SHEWCHUCK_FLAGS \
    $FREEGLUT_FLAGS \
    $GLEW_FLAGS \
    $ANTTWEAKBAR_FLAGS \
    $GLUT_VIEWER_FLAGS \
    $MEDIT_FLAGS \
    $MISC_FLAGS \
    "$@"

