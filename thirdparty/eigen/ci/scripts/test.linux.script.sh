#!/bin/bash

set -x

# Enter build directory.
rootdir=`pwd`
cd ${EIGEN_CI_BUILDDIR}

target=""
if [[ ${EIGEN_CI_CTEST_REGEX} ]]; then
  target="-R ${EIGEN_CI_CTEST_REGEX}"
elif [[ ${EIGEN_CI_CTEST_LABEL} ]]; then
  target="-L ${EIGEN_CI_CTEST_LABEL}"
fi

set +x

ctest_cmd="ctest ${EIGEN_CI_CTEST_ARGS} --parallel ${NPROC} --output-on-failure --no-compress-output --build-noclean ${target}"

echo "Running initial tests..."
if ${ctest_cmd} -T test; then
  echo "Tests passed on the first attempt."
  exit_code=$?
else
  echo "Initial tests failed with exit code $?. Retrying up to ${EIGEN_CI_CTEST_REPEAT} times..."
  if ${ctest_cmd} --rerun-failed --repeat until-pass:${EIGEN_CI_CTEST_REPEAT}; then
    echo "Tests passed on retry."
    exit_code=42
  else
    exit_code=$?
  fi
fi

set -x

# Return to root directory.
cd ${rootdir}

set +x

exit $exit_code
