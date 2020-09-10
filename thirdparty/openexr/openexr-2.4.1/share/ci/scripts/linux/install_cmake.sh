#!/usr/bin/env bash

set -ex

echo "Updating to newer cmake to enable python-3"

CMAKE_VERSION="$1"

curl --location "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh" -o /tmp/cmake.sh
cd /tmp && sh cmake.sh --skip-license --prefix=/usr/local --exclude-subdir
rm /tmp/cmake.sh

echo $(ls /usr/local/bin)
echo $(which cmake)
