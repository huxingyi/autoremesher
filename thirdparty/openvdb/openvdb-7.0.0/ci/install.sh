#!/usr/bin/env bash

set -ex

apt-get update

apt-get install -y zlib1g-dev
apt-get install -y wget
apt-get install -y unzip
apt-get install -y curl
apt-get install -y cmake
apt-get install -y git
apt-get install -y g++
apt-get install -y clang
apt-get install -y llvm
apt-get install -y pkg-config
apt-get install -y libglu1-mesa-dev
apt-get install -y libgl1-mesa-dev
apt-get install -y libcppunit-dev
apt-get install -y libjemalloc-dev
apt-get install -y liblog4cplus-dev
apt-get install -y libglfw3-dev
apt-get install -y python-dev
apt-get install -y python-numpy
apt-get install -y python-epydoc
apt-get install -y doxygen

# these libraries are required for vdb_view if USE_X11=ON
apt-get install -y libxinerama-dev
apt-get install -y libxrandr-dev
apt-get install -y libxcursor-dev
apt-get install -y libxi-dev
apt-get install -y libx11-dev
