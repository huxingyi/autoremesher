#!/bin/bash
scriptdir=$(dirname "$0")
$scriptdir/generate_todo_list.pl \
    --verbose \
    --output-dir todo_list \
    --exclude '/third_party/' \
    --exclude '/bin/medit/' \
    --exclude '/filtered_predicates/side\d\.h' \
    --exclude '/basic/license_msgs\.h' \
    --strip "$scriptdir/../" \
    $scriptdir/../src

