# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

# This is a sample cmake test script that can be used to integrate into
# a larger CI setup if you are building your own versions of OpenEXR
# and also use a cdash (or cdash compliant) results server.
#
# There are also settings in the CMakeLists.txt you may wish to
# just set in there, or replicate here.

# Running ctest -S thisscript.cmake will build into the binary directory
# and run a few different tests based on what commands are specified
# (and the steps below). It is best to read the ctest docs to
# understand all these settings, and how to control it, this is merely
# provided as a sample

# An edited version (or multiple) are intended to be placed in the CI
# system, and putting O.S. / configuration specific control to this file
# instead of having to put it into the make CMakeLists.txt tree
# somehow.

# this contains the path to the source tree. This may come in as an
# environment variable from the CI system, but you are free to have
# any path in here
set(CTEST_SOURCE_DIRECTORY "$ENV{PATH_TO_OPENEXR_TREE}")
# Similarly, this is scratch space used to configure, build
# and run the various tests.
# For CI builds, it is recommended to make sure this is a
# unique tree for each build
set(CTEST_BINARY_DIRECTORY "/tmp/ctest")

# set an override for any compile flags to enable coverage
# NB: This can make some of the auxiliary binaries such as the
# dwa lookup table generator quite slow
#set(ENV{CXXFLAGS} "--coverage")

# If you have alternate build systems, you can control that here
#set(CTEST_CMAKE_GENERATOR "Ninja")
set(CTEST_USE_LAUNCHERS 1)

# The various paths to programs to run coverage and memory checks
set(CTEST_COVERAGE_COMMAND "gcov")
set(CTEST_MEMORYCHECK_COMMAND "valgrind")
#set(CTEST_MEMORYCHECK_TYPE "ThreadSanitizer")
# 

# any of the usual configurations (Debug, Release, etc).
# We do not attempt to create any alternate configurations
set(CTEST_CONFIGURATION_TYPE "RelWithDebInfo")

# can be Continuous, Nightly, or Experimental (see the cmake docs)
ctest_start("Continuous")

# applies the various ctest steps
ctest_configure()
ctest_build()
ctest_test()
ctest_coverage()
ctest_memcheck()

# This uploads the results to the server you configured
ctest_submit()
