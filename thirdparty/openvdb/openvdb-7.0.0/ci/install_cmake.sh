#!/usr/bin/env bash

set -ex

CMAKE_VERSION="$1"

git clone https://github.com/Kitware/CMake.git
cd CMake

if [ "$CMAKE_VERSION" != "latest" ]; then
    git checkout tags/v${CMAKE_VERSION} -b v${CMAKE_VERSION}
fi

mkdir build
cd build
cmake ../.
make -j4
make install
