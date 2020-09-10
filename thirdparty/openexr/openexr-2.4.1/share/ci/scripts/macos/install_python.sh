#!/usr/bin/env bash

set -ex

PYTHON_VERSION="$1"
PYTHON_MAJOR="$(echo ${PYTHON_VERSION} | cut -f 1 -d .)"

MACOS_MAJOR="$(sw_vers -productVersion | cut -f 1 -d .)"
MACOS_MINOR="$(sw_vers -productVersion | cut -f 2 -d .)"

echo "Installing Python ${PYTHON_VERSION} Major ${PYTHON_MAJOR}"

# This workaround is needed for building Python on macOS >= 10.14:
#   https://developer.apple.com/documentation/xcode_release_notes/xcode_10_release_notes

if [[ "$MACOS_MAJOR" -gt 9 && "$MACOS_MINOR" -gt 13 ]]; then
    sudo installer \
        -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_${MACOS_MAJOR}.${MACOS_MINOR}.pkg \
        -target /
fi

unset CFLAGS

brew update
brew install pyenv openssl 

CFLAGS="-I/usr/local/Cellar/openssl/1.0.2s/include"
LDFLAGS="-L/usr/local/Cellar/openssl/1.0.2s/lib"

echo 'eval "$(pyenv init -)"' >> .bash_profile
source .bash_profile
env PYTHON_CONFIGURE_OPTS="--enable-framework" pyenv install -v ${PYTHON_VERSION}
pyenv global ${PYTHON_VERSION}

if [[ ${PYTHON_MAJOR}=2 ]]; then
    pip install numpy
else
    pip3 install numpy
fi
