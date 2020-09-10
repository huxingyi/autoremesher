#!/usr/bin/env bash

set -ex

BOOST_VERSION="$1"

# only install for Boost 1.61
apt-get install -y libbz2-dev

git clone https://github.com/boostorg/boost.git
cd boost

if [ "$BOOST_VERSION" == "latest" ]; then
    git checkout tags/boost-1.69.0 -b boost-1.69.0
else
    git checkout tags/boost-${BOOST_VERSION} -b boost-${BOOST_VERSION}
fi

git submodule update --init --

./bootstrap.sh --prefix=/usr/local
./b2 headers -j4
./b2 install link=shared variant=release \
    --with-atomic \
    --with-chrono \
    --with-date_time \
    --with-iostreams \
    --with-python \
    --with-regex \
    --with-system \
    --with-thread \
    -j4
