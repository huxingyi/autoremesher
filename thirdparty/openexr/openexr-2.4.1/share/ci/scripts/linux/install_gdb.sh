#!/usr/bin/env bash

set -ex

GDB_VERSION="8.3"

echo "Installing gdb-${GDB_VERSION}" 

wget -q https://ftp.gnu.org/gnu/gdb/gdb-${GDB_VERSION}.tar.xz
tar -xvf gdb-${GDB_VERSION}.tar.xz

cd gdb-${GDB_VERSION}

# Build gdb
./configure --prefix=/usr \
            --with-system-readline \
            --with-python=/usr/bin/python3 &&
make
sudo make -C gdb install

cd ..
