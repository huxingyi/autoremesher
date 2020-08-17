#!/bin/bash
scriptdir=`dirname "$0"`
cd $scriptdir/.. || exit 1
tools/indent.pl \
    --verbose \
    --replace \
    --no-backup \
    --exclude '/third_party/' \
    --exclude '/bin/medit/' \
    --exclude '/filtered_predicates/side\d\.h' \
    --exclude '/basic/license_msgs\.h' \
    src

