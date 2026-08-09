## Eigen CI infrastructure

Eigen's CI infrastructure uses three stages:
  1. A `checkformat` stage to verify MRs satisfy proper formatting style, as
     defined by `clang-format`.
  2. A `build` stage to build the unit-tests.
  3. A `test` stage to run the unit-tests.

For merge requests, only a small subset of tests are built/run, and only on a
small subset of platforms.  This is to reduce our overall testing infrastructure
resource usage.  In addition, we have nightly jobs that build and run the full
suite of tests on most officially supported platforms.
