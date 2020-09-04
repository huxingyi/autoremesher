#!/bin/bash

# Generates a webpage around a .js executable compiled with Emscripten
#
# Usage: gen_emscripten_html.sh executable_name.js
#
#    or: gen_emscripten_html.sh executable_name.js  file1 file2 ... filen
#
#   The second form generates a "pseufo file system" that will be visible 
# from the executable using standard POSIX calls.


SCRIPTDIR=`dirname "$0"`
EXENAME=`basename $1 .js`
shift
FILES=$@

if [ -z $EXENAME ]
then
    echo "Usage: gen_emscripten_html.sh exename.js (optional list of files to pack)"
    exit -1
fi

if [ -z "$FILES" ]
then
   echo generating $EXENAME.html
   cat $SCRIPTDIR/template_emscripten.html | sed -e 's/%EXENAME%/'$EXENAME'/g' > $EXENAME.html
else
   echo packaging $FILES "---->" $EXENAME'_data.data' ',' $EXENAME'_data.js'
   python $EMSCRIPTEN/tools/file_packager.py $EXENAME'_data.data' --preload $FILES > $EXENAME'_data.js'
   echo generating $EXENAME.html
   cat $SCRIPTDIR/template_emscripten.html | sed -e 's/%EXENAME%/'$EXENAME'/g' \
       -e 's|<!-- DATAFILE -->|<script async type="text/javascript" src="'$EXENAME'_data.js"></script>|g' \
       > $EXENAME.html
fi

echo "copying FileSaver.js (required)"
cp $SCRIPTDIR/FileSaver.js .
