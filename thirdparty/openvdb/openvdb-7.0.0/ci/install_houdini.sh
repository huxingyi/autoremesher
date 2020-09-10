#!/usr/bin/env bash

set -e

HOUDINI_MAJOR="$1"
HOUPASS="$2"

pip install --user mechanize

export PYTHONPATH=${PYTHONPATH}:/usr/lib/python2.7/dist-packages
# download and unpack latest houdini headers and libraries from daily-builds
python ci/download_houdini.py $HOUDINI_MAJOR $HOUPASS

tar -xzf hou.tar.gz
ln -s houdini* hou
cd hou
tar -xzf houdini.tar.gz

cd -
