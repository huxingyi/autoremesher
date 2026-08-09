#!/bin/bash

set -x

# Enter build directory.
rootdir=`pwd`
cd ${EIGEN_CI_BUILDDIR}

# Install xml processor.
apt-get update -y
apt-get install --no-install-recommends -y xsltproc

# Generate test results.
xsltproc ${rootdir}/ci/CTest2JUnit.xsl Testing/`head -n 1 < Testing/TAG`/Test.xml > "JUnitTestResults_$CI_JOB_ID.xml"

# Return to root directory.
cd ${rootdir}

set +x
