#!/usr/bin/env bash

set -ex

SIX_VERSION="1.12.0"

echo "Installing six-${SIX_VERSION}" 

wget -q  https://files.pythonhosted.org/packages/source/s/six/six-${SIX_VERSION}.tar.gz
tar -xvf six-${SIX_VERSION}.tar.gz

cd six-${SIX_VERSION}

# Install six
python3 setup.py build
sudo python3 setup.py install --optimize=1

cd ..

