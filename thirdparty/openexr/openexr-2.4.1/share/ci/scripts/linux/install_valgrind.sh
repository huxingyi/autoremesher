#!/usr/bin/env bash

set -ex

VALGRIND_VERSION="3.15.0"

echo "Updating to Valgrind ${VALGRIND_VERSION}" 

wget -q https://sourceware.org/ftp/valgrind/valgrind-${VALGRIND_VERSION}.tar.bz2 
tar -xjf valgrind-${VALGRIND_VERSION}.tar.bz2

cd valgrind-${VALGRIND_VERSION}

# Build valgrind 
sed -i 's|/doc/valgrind||' docs/Makefile.in &&
./configure --prefix=/usr \
            --datadir=/usr/share/doc/valgrind-${VALGRIND_VERSION} &&
make

# Test the build - disabled
# NOTE: if enabled, must install prerequisites gedb-8.3 and six-1.12.0
# make regtest

# Install valgrind 
sudo make install

echo $(which valgrind)

cd ..
