#!/usr/bin/env bash

set -ex

OPENEXR_VERSION="$1"

git clone https://github.com/openexr/openexr.git
cd openexr

if [ "$OPENEXR_VERSION" != "latest" ]; then
    git checkout tags/v${OPENEXR_VERSION} -b v${OPENEXR_VERSION}
fi

# TODO: CMake support was only introduced with OpenEXR 2.3, expand this to use 2.2

mkdir build
cd build
cmake -DOPENEXR_BUILD_PYTHON_LIBS=OFF -DOPENEXR_BUILD_TESTS=OFF -DOPENEXR_BUILD_UTILS=OFF ../.
make -j4
make install
