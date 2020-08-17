#!/bin/bash
scriptdir=`dirname "$0"`
set -x
valgrind --verbose \
    --leak-check=full \
    --show-reachable=yes \
    --error-limit=no \
    --log-file=valgrind.log \
    --suppressions="$scriptdir/valgrind.supp" \
    "$@"

