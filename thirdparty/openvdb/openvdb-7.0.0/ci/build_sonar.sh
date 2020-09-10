#!/usr/bin/env bash

set -ex

wget -q https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
unzip build-wrapper-linux-x86.zip

mkdir build
cd build
cmake \
    -DCMAKE_CXX_COMPILER=g++ \
    -DOPENVDB_ABI_VERSION_NUMBER=6 \
    -DUSE_BLOSC=ON \
    -DOPENVDB_CXX_STRICT=ON \
    -DOPENVDB_BUILD_UNITTESTS=ON \
    -DOPENVDB_BUILD_BINARIES=OFF \
    -DOPENVDB_CORE_STATIC=OFF \
    -DOPENVDB_BUILD_PYTHON_MODULE=OFF \
    -DOPENVDB_CODE_COVERAGE=ON \
    ..

../build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw_output make -j2
