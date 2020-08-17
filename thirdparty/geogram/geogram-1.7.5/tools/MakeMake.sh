# MakeMake.sh: generates a Makefile for compiling with MinGW
#
# This script should be launched from the main geogram directory
# It creates a build/plain_make subdirectory with a makefile
# It is meant for users who do not want to use CMake (for instance,
# developpers using Rtools for writing extension packages for R).
#
# It generates a Makefile for compilation with MinGW (g++ for Windows),
# Makefile can be easily edited and adapted to other compilers.

if [ ! -d tools ]; then
   echo MakeMake.sh should only be called from main geogram directory
   exit -1
fi

BUILDIR=build/simple_make
mkdir -p $BUILDIR

cat <<EOF > $BUILDIR/README.txt
This directory contains a Makefile pre-configured for building geogram 
libraries. It is for users who do not wish to use CMake (for instance 
developpers of R extension packages using Rtools). The Makefile is 
configured for MingW, but can be easily adapted to other compilers.

Other users may use CMake/configure.sh instead (just launch ./configure.sh
from the main geogram directory)
EOF

cat <<EOF

******
******
***                                    
*** Creating plain Makefile-based tree in $BUILDIR
***
******
******

EOF



exec > $BUILDIR/Makefile

DATE=$(date)
SRCDIR=src/lib

CMAKELIST=CMakeLists.txt
VERSION_MAJOR=$(cat $CMAKELIST | grep 'set(VORPALINE_VERSION_MAJOR' | sed -e 's|[^0-9]||g')
VERSION_MINOR=$(cat $CMAKELIST | grep 'set(VORPALINE_VERSION_MINOR' | sed -e 's|[^0-9]||g')
VERSION_PATCH=$(cat $CMAKELIST | grep 'set(VORPALINE_VERSION_PATCH' | sed -e 's|[^0-9]||g')
VERSION=$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH

mkdir -p $BUILDIR/geogram
cat > $BUILDIR/geogram/version.h << EOF
#ifndef GEOGRAM_BASIC_VERSION
#define GEOGRAM_BASIC_VERSION

#define VORPALINE_VERSION_MAJOR "$VERSION_MAJOR"
#define VORPALINE_VERSION_MINOR "$VERSION_MINOR"
#define VORPALINE_VERSION_PATCH "$VERSION_PATCH"
#define VORPALINE_VERSION "$VERSION"
#define VORPALINE_BUILD_NUMBER ""
#define VORPALINE_BUILD_DATE "$DATE"
#define VORPALINE_SVN_REVISION "????"

#endif
EOF

cat << EOF
# This Makefile was automatically generated using
# geogram's MakeMake utility.

SRCDIR=../..
CC=x86_64-w64-mingw32-gcc-win32
CXX=x86_64-w64-mingw32-g++-win32
AR=x86_64-w64-mingw32-ar
RANLIB=x86_64-w64-mingw32-ranlib

# Uncomment for DLL build (untested), see also end of this file.
# EXPORTS=-Dgeogram_EXPORTS -Dexploragram_EXPORTS
EXPORTS=
OPTIMIZE=-O2 -g -fopenmp

# Remove the options that you do not use
# PDEL:   parallel 3D Delaunay -DGEOGRAM_WITH_PDEL (not implemented yet for MingW)
# LUA:    Lua language interpreter
# HLBFGS: non-linear optimizer
# TETGEN: tetrahedral mesh generator (Hang Si). Note: see license !
# TRIANGLE: triangle mesh genetator (Jonathan Shewchuk). Note: see license !

OPTIONS=-DGEOGRAM_WITH_LUA \
	-DGEOGRAM_WITH_HLBFGS \
        -DGEOGRAM_WITH_TETGEN \
	-DGEOGRAM_WITH_TRIANGLE

COPT=-I\$(SRCDIR)/src/lib -I. \$(EXPORTS) \$(OPTIONS) \$(OPTIMIZE)
CXXOPT=-I\$(SRCDIR)/src/lib -I. -Wno-deprecated -std=c++11 \$(EXPORTS) \$(OPTIONS) \$(OPTIMIZE)
LDXX=\$(CXX)
LDXXOPT=-static \$(OPTIMIZE)

all: vorpalite_static.exe
EOF

CSRC=$(find $SRCDIR/geogram $SRCDIR/exploragram  -name "*.c" -print)
CPPSRC=$(find $SRCDIR/geogram $SRCDIR/exploragram -name "*.cpp" -print)

SRC2OBJ="sed -e 's|$SRCDIR/||g' -e 's|/|_|g' -e 's|\.cpp|\.o|g' -e 's|\.c|\.o|g'"

OBJ=$(echo $CSRC $CPPSRC | eval $SRC2OBJ)

echo
echo OBJ=$OBJ
echo

for i in $CSRC 
do
   obj=$(echo $i | eval $SRC2OBJ)
   echo $obj: \$\(SRCDIR\)/$i
cat << EOF
	\$(CC) -c \$(COPT) \$(SRCDIR)/$i -o $obj

EOF
done

for i in $CPPSRC 
do
   obj=$(echo $i | eval $SRC2OBJ)
   echo $obj: \$\(SRCDIR\)/$i
cat << EOF
	\$(CXX) -c \$(CXXOPT) \$(SRCDIR)/$i -o $obj

EOF
done

#============================================================================================

cat <<EOF

libexplogeogram_static.a: \$(OBJ)
	\$(AR) cq libexplogeogram_static.a \$(OBJ)
	\$(RANLIB) libexplogeogram_static.a

vorpalite.o: \$(SRCDIR)/src/bin/vorpalite/main.cpp
	\$(CXX) -c \$(CXXOPT)  \$(SRCDIR)/src/bin/vorpalite/main.cpp -o vorpalite.o

vorpalite_static.exe: vorpalite.o libexplogeogram_static.a
	\$(LDXX) \$(LDXXOPT) -o vorpalite_static.exe vorpalite.o libexplogeogram_static.a

# DLL build (untested)
#
#libexplogeogram_dll.a: \$(OBJ)
#	\$(CXX) -shared -o libexplogeogram_dll.dll \$(OBJ) -Wl,--out-implib,libexplogeogram_dll.a
#
#vorpalite_dynamic.exe: vorpalite.o libexplogeogram_dll.a
#	\$(CXX) \$(CXXOPT) -o vorpalite_dynamic.exe vorpalite.o libexplogeogram_dll.a

EOF
