

# extract_psm: extracts a PSM (Pluggable Software Module)
# from geogram source tree.
#
# a PSM is a functionality of Geogram made available through a
# standalone .h/.cpp pair for users interested in that
# functionality who do not want to use the whole geogram package.

# remove_copyright()
#
# Arguments: none (stream function)
# Variables: none
#
# Removes the first comment from a source file
# (the one that contains the copyright)

remove_copyright() {
    awk '
    BEGIN {
      state = 0;
    } {
      if(state == 0) {
        if($0 ~ /\/\*.*/) {
          state = 1;
        }
      } else if(state == 1) {
        if($0 ~ / \*\/.*/) {
          state = 2;
        }
      } else {
        print;
      }
    }'
}

# extract_copyright()
#
# Arguments: $1: path to a sourcefile relative
#   to $PSM_DIR
# Variables: $PSM_DIR: the directory that
#   contains the .psm file
#
#   Extracts the copyright from $1 and sends it to the
# standard output.

extract_copyright() {
    cat $PSM_DIR/$1 | awk '
    BEGIN {
      state = 0;
    } {
      if(state == 0) {
        print;
        if($0 ~ /\/\*.*/) {
          state = 1;
        }
      } else if(state == 1) {
        print;
        if($0 ~ / \*\/.*/) {
          state = 2;
        }
      } 
    }'
}

# remove_includes()
#
# Arguments: none (stream function)
# Variables: none
#
# Removes all include statements referring to Geogram files

remove_includes() {
    egrep -v '#include "' | egrep -v '#include <geogram/'
}

# remove_doxygen()
#
# Arguments: none (stream function)
# Variables: none
#
# Removes all Doxygen comments

remove_doxygen() {
    sed -e 's|/\*\*.*\*/||g' | awk '
    BEGIN {
      state = 0;
    } {
      if(state == 0) {
        if($0 ~ /\/\*\*.*/) {
          state = 1;
        } else {
          print;
        }
      } else if(state == 1) {
        if($0 ~ / \*\/.*/) {
          state = 0;
        }
      }
    }'
}

# extract_file()
#
# Arguments: $1: path to a sourcefile relative
#   to $PSM_DIR
# Variables: $PSM_DIR: the directory that
#   contains the .psm file
#
#   Extracts a sourcefile, processes it to remove
# unnecessary information and sends it to the standard
# output.

extract_file() {
    echo
    echo "/******* extracted from" $1 "*******/"
    cat $PSM_DIR/$1 | remove_copyright | remove_includes | remove_doxygen
}

# extract_files()
#
# Arguments: $@: list of files to be extracted, all paths
#   relative to $PSM_DIR
# Variables: $PSM_DIR: the directory that
#   contains the .psm file
#
#   Extracts a list of sourcefiles, processes them to remove
# unnecessary information and sends them to the standard
# output.

extract_files() {
    extract_copyright $1
    cat <<EOF


/*
 *  This file is a PSM (pluggable software module)
 *   generated from the distribution of Geogram.
 *
 *  See Geogram documentation on:
 *   http://alice.loria.fr/software/geogram/doc/html/index.html
 *
 *  See documentation of the functions bundled in this PSM on:
 *   $PSM_DOC
 */


EOF
    for i in $@
    do
        extract_file $i
    done
}

# extract_header()
#
# Arguments: none
# Variables: $PSM_HEADER the absolute path of the generated header
#            $HEADERS the list of header files to be assembled, each
#             of them relative to $PSM_DIR
#            $PSM_DIR: the directory that contains the .psm file
#
# Assembles the header ($PSM_HEADER) from the source headers ($HEADERS)

extract_header() {
    echo "   generating" $PSM_HEADER ...
    cat <<EOF >$PSM_HEADER
#define GEOGRAM_PSM
#ifndef GEO_STATIC_LIBS
#define GEO_DYNAMIC_LIBS
#endif
EOF
    extract_files $HEADERS >> $PSM_HEADER
}

# extract_source()
#
# Arguments: none
# Variables: $PSM_SOURCE the absolute path of the generated header
#            $SOURCES the list of header files to be assembled, each
#             of them relative to $PSM_DIR
#            $PSM_DIR: the directory that contains the .psm file
#
# Assembles the source ($PSM_SOURCE) from the sources ($SOURCES)

extract_source() {
    echo "   generating" $PSM_SOURCE ...
    echo '#include "'$PSM_HEADER'"' > $PSM_SOURCE
    echo >> $PSM_SOURCE
    extract_files $SOURCES >> $PSM_SOURCE
}

# extract_example()
#
# Arguments: none
# Variables: $EXAMPLE: path to the example source, relative to $PSM_DIR
#            $PSM_DIR: the directory that contains the .psm file
#
# If $EXAMPLE is a non-empty string, extracts it.

extract_example() {
    if [ -z $EXAMPLE ]
    then
        echo "   No example file"
        return;
    fi
    PSM_EXAMPLE_BIN=$PSM_NAME"_example"
    PSM_EXAMPLE=$PSM_NAME"_example.c"
    COMPILER=gcc
    if [[ $EXAMPLE =~ ^.*\.cpp$ ]]
    then
        PSM_EXAMPLE=$PSM_NAME"_example.cpp"
	COMPILER=g++
    fi
    echo "   generating $PSM_EXAMPLE ..."
    cat <<EOF > $PSM_EXAMPLE
/*
 * To compile under Linux: 
 *   $COMPILER -O3 -fopenmp -frounding-math -ffp-contract=off --std=c++11 $PSM_EXAMPLE $PSM_SOURCE -o $PSM_EXAMPLE_BIN -ldl -lm
 */
EOF
    echo >> $PSM_EXAMPLE     
    extract_copyright $EXAMPLE >> $PSM_EXAMPLE    
    echo '#include "'$PSM_HEADER'"' >> $PSM_EXAMPLE
    cat $PSM_DIR/$EXAMPLE | remove_copyright | remove_includes >> $PSM_EXAMPLE
}


# extract_copydir()
#
# Arguments: none
# Variables: $COPYDIR: path to the subdirectory to copy, relative to $PSM_DIR
#            $PSM_DIR: the directory that contains the .psm file
#
# If $COPYDIR is a non-empty string, copy the subdirectory into the PSM distrib.

extract_copydir() {
   if [ -z $COPYDIR ]
   then
       return;
   fi
   cp -r $PSM_DIR/$COPYDIR .
}

extract_readme() {
    cat <<EOF
This is $PSM_NAME Pluggable Software Module, extracted
from GEOGRAM source tree. It contains a standalone .cpp/.h
pair that can be used in any program and that does not have
any dependency. 

It may also contain an example program that can be compiled by using:
  g++ --std=c++11 ${PSM_NAME}_psm.cpp ${PSM_NAME}_example.cpp -o ${PSM_NAME}_example
(or gcc if it is plain C, as in OpenNL)

Some examples may require additional compilation flags (see comments at the beginning
of the example source, e.g. Delaunay_example.cpp).
EOF
}

# Get directory and PSM name from PSM file name
PSM_DIR=`dirname $1`
PSM_NAME=`basename $1 .psm`

# Read the PSM file, and get the value of
#  $SOURCES, $HEADERS, $DOC and $EXAMPLE
. $1
echo Extracting $PSM_NAME Pluggable Software Module...

PSM_HEADER=$PSM_NAME"_psm.h"
PSM_SOURCE=$PSM_NAME"_psm.c"

# If sources contain C++ files, then generated source is
# a C++ file
if [[ $SOURCES =~ ^.*\.cpp.*$ ]]
then
    PSM_SOURCE=$PSM_NAME"_psm.cpp"
fi

PSM_DOC=$DOC

extract_header
extract_source
extract_example
extract_copydir
extract_readme > README.txt



