#!/usr/bin/env bash

set -ex

BOOST_VERSION="$1"
BOOST_MAJOR_MINOR=$(echo "${BOOST_VERSION}" | cut -d. -f-2)
BOOST_MAJOR=$(echo "${BOOST_VERSION}" | cut -d. -f-1)
BOOST_MINOR=$(echo "${BOOST_MAJOR_MINOR}" | cut -d. -f2-)
BOOST_PATCH=$(echo "${BOOST_VERSION}" | cut -d. -f3-)
BOOST_VERSION_U="${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}"

mkdir _boost
cd _boost

wget -q https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION_U}.tar.gz
tar -xzf boost_${BOOST_VERSION_U}.tar.gz

cd boost_${BOOST_VERSION_U}
sh bootstrap.sh
./b2 install -j4 variant=release toolset=gcc \
    --with-system \
    --with-regex \
    --with-filesystem \
    --with-thread \
    --with-python \
    --prefix=/usr/local

cd ../..
rm -rf _boost
