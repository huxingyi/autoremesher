#!/bin/bash

set -x

# Create and enter build directory.
rootdir=`pwd`
mkdir -p ${EIGEN_CI_BUILDDIR}
cd ${EIGEN_CI_BUILDDIR}

# Configure build.
cmake -G Ninja                                                   \
  -DCMAKE_CXX_COMPILER=${EIGEN_CI_CXX_COMPILER}                  \
  -DCMAKE_C_COMPILER=${EIGEN_CI_C_COMPILER}                      \
  -DCMAKE_CXX_COMPILER_TARGET=${EIGEN_CI_CXX_COMPILER_TARGET}    \
  ${EIGEN_CI_ADDITIONAL_ARGS} ${rootdir}

target=""
if [[ ${EIGEN_CI_BUILD_TARGET} ]]; then
  target="--target ${EIGEN_CI_BUILD_TARGET}"
fi

# Builds (particularly gcc) sometimes get killed, potentially when running
# out of resources.  In that case, keep trying to build the remaining
# targets (k0), then try to build again with a single thread (j1) to minimize
# resource use.
cmake --build . ${target} -- -k0 || cmake --build . ${target} -- -k0 -j1

# Return to root directory.
cd ${rootdir}

set +x
