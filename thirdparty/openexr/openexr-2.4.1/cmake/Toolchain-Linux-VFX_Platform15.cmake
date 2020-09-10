# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

# This is only an example of a theoretical toolchain file
# that might be used for compiling to target a VFX
# reference platform (https://vfxplatform.com) that is
# consistent with the CY2015 spec.
#
# A toolchain file is loaded early in the cmake configure
# process to enable compiler checks to use the appropriate
# compilers.
#
# Read the docs to understand more:
# https://cmake.org/cmake/help/v3.12/manual/cmake-toolchains.7.html
#
# Then to run, do something like
# mkdir vfx_2015
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-Linux-VFX_Platform15.cmake -DCMAKE_INSTALL_PREFIX=/path/to/vfx_2015 ..
# (plus any other settings you'd like)
# make
# make test
# make install

# by not setting this, it will assume it's a mostly standard linux environment
#set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER gcc-4.8.2)
set(CMAKE_CXX_COMPILER g++-4.8.2)

set(CMAKE_FIND_ROOT_PATH /my/vfx_2015/libraries)

set(CMAKE_CXX_STANDARD 11)

# read the docs to understand whether this is what you want
# if you use system libraries for some things, it may not be!!!
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
