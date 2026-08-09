# Changelog

## [5.0.1] - 2025-11-11

A few bug-fixes from the master branch, including
- Dirty git state [#2995]
- Failing geo_homogeneous tests [#2977]
- Alignment issues [#2982, #2984]
- Missing C++20 `<version>` header [#2986]
- BLAS/LAPACK build on windows [#2980]

See the full lists of [addressed bugs](https://gitlab.com/libeigen/eigen/-/issues?state=all&label_name%5B%5D=release%3A%3A5.0.1) and [merge requests](https://gitlab.com/libeigen/eigen/-/merge_requests?state=all&label_name%5B%5D=release%3A%3A5.0.1) for more details.

## [5.0.0] - 2025-09-30

Eigen 5.0 provides many new features, performance enhancements, and bugfixes throughout Eigen’s core template expression infrastructure and linear algebra facilities.  The full set of changes and related issues are too large to list here, but can be accessed via the release milestone %"5.0".

This is the last major release to support the C++14 language standard. The master branch and subsequent releases will require support for C++17.

### Versioning

This release marks a transition to [Semantic Versioning](https://semver.org/). Previously, Eigen used a WORLD.MAJOR.MINOR scheme. From now on, version numbers will follow the MAJOR.MINOR.PATCH format, indicating breaking changes, new features, and bug fixes, respectively. The WORLD version will remain 3 for this and subsequent releases for posterity. See the table below:
```
╔═════════╦═════╦═════╗
║ Release ║ 3.4 ║ 5.0 ║
╠═════════╬═════╬═════╣
║ WORLD   ║  3  ║  3  ║
║ MAJOR   ║  4  ║  5  ║
║ MINOR   ║  0  ║  0  ║
║ PATCH   ║  -  ║  0  ║
╚═════════╩═════╩═════╝
```

### Breaking changes

* Eigen 5.X.X requires C++14.  When building with GNU-compatible compilers, set `-std=c++14` or later.  As part of this change, some macros such as `EIGEN_HAS_CXX11` have also been removed.
* The CMake build system has been modernized and older properties have been removed - projects relying on CMake may need to update their configurations [!485].
* All LGPL-licensed code has been removed (i.e. Constrained Conjugate Gradient) [!1197].  These were "unsupported" anyways, and weren't widely used.
* Due to name conflicts with other projects, `Eigen::all` and `Eigen::last` have been moved to `Eigen::placeholders::all` and `Eigen::placeholders::last` [!649].
* Any direct inclusion of an internal header (i.e. under a `../src/..` path) will result in a compilation error [!631].
* Runtime SVD options for computing thin/full U/V have been deprecated: use compile-time options instead [!826].
* Scalar (i.e. non-vectorized) comparisons now return masks with values of `Scalar(1)` rather than having all bits set to avoid undefined behavior [!1862].
* BLAS return types have been changed for Eigen BLAS to `void` instead of `int` for compatibility with other BLAS implementations [!1497].
* `Eigen::aligned_allocator` no longer inherits from `std::allocator` due to a change in the standard and the use of `allocate_at_least` [!1795].
* Euler angles are now returned in a more canonical form, potentially resulting in a change of behavior [!1301, !1314].
* Eigen's random number generation has changed, resulting in a change of behavior.  Please do not rely on specific random numbers from Eigen - these were never guaranteed to be consistent across Eigen versions, nor are they generally consistent across platforms [!1437].

## [3.4.0] - 2021-08-18

**Notice:** 3.4.x will be the last major release series of Eigen that will support c++03.

### Breaking changes

* Using float or double for indexing matrices, vectors and arrays will now fail to compile
* **Behavioral change:** `Transform::computeRotationScaling()` and `Transform::computeScalingRotation()` are now more continuous across degeneracies (see !349).

### New features

* Add c++11 **`initializer_list` constructors** to Matrix and Array [\[doc\]](http://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html#title3)
* Add STL-compatible **iterators** for dense expressions [\[doc\]](http://eigen.tuxfamily.org/dox-devel/group__TutorialSTL.html).
* New versatile API for sub-matrices, **slices**, and **indexed views** [\[doc\]](http://eigen.tuxfamily.org/dox-devel/group__TutorialSlicingIndexing.html).
* Add C++11 **template aliases** for Matrix, Vector, and Array of common sizes, including generic `Vector<Type,Size>` and `RowVector<Type,Size>` aliases [\[doc\]](http://eigen.tuxfamily.org/dox-devel/group__matrixtypedefs.html).
* New support for `bfloat16`.

### New backends

* **Arm SVE:** fixed-length [Scalable Vector Extensions](https://developer.arm.com/Architectures/Scalable%20Vector%20Extensions) vectors for `uint32_t` and `float` are available.
* **MIPS MSA:**: [MIPS SIMD Architecture (MSA)](https://www.mips.com/products/architectures/ase/simd/) 
* **AMD ROCm/HIP:** generic GPU backend that unifies support for [NVIDIA/CUDA](https://developer.nvidia.com/cuda-toolkit) and [AMD/HIP](https://rocmdocs.amd.com/en/latest/).
* **Power 10 MMA:** initial support for [Power 10 matrix multiplication assist instructions](https://arxiv.org/pdf/2104.03142.pdf) for float32 and float64, real and complex.

### Improvements

* Eigen now uses the c++11 **alignas** keyword for static alignment. Users targeting C++17 only and recent compilers (e.g., GCC>=7, clang>=5, MSVC>=19.12) will thus be able to completely forget about all [issues](http://eigen.tuxfamily.org/dox-devel/group__TopicUnalignedArrayAssert.html) related to static alignment, including `EIGEN_MAKE_ALIGNED_OPERATOR_NEW`.
* Various performance improvements for products and Eigen's GEBP and GEMV kernels have been implemented:
  * By using half and quater-packets the performance of matrix multiplications of small to medium sized matrices has been improved
  * Eigen's GEMM now falls back to GEMV if it detects that a matrix is a run-time vector
  * The performance of matrix products using Arm Neon has been drastically improved (up to 20%)
  * Performance of many special cases of matrix products has been improved
* Large speed up from blocked algorithm for `transposeInPlace`.
* Speed up misc. operations by propagating compile-time sizes (col/row-wise reverse, PartialPivLU, and others)
* Faster specialized SIMD kernels for small fixed-size inverse, LU decomposition, and determinant.
* Improved or added vectorization of partial or slice reductions along the outer-dimension, for instance: `colmajor_mat.rowwise().mean()`.

### Elementwise math functions

* Many functions are now implemented and vectorized in generic (backend-agnostic) form.
* Many improvements to correctness, accuracy, and compatibility with c++ standard library.
  * Much improved implementation of `ldexp`.
  * Misc. fixes for corner cases, NaN/Inf inputs and singular points of many functions.
  * New implementation of the Payne-Hanek for argument reduction algorithm for `sin` and `cos` with huge arguments.
  * New faithfully rounded algorithm for `pow(x,y)`.
* Speedups from (new or improved) vectorized versions of `pow`, `log`, `sin`, `cos`, `arg`, `pow`, `log2`, complex `sqrt`, `erf`, `expm1`, `logp1`, `logistic`, `rint`, `gamma` and `bessel` functions, and more.
* Improved special function support (Bessel and gamma functions, `ndtri`, `erfc`, inverse hyperbolic functions and more)
* New elementwise functions for `absolute_difference`, `rint`.

### Dense matrix decompositions and solvers

* All dense linear solvers (i.e., Cholesky, *LU, *QR, CompleteOrthogonalDecomposition, *SVD) now inherit SolverBase and thus support `.transpose()`, `.adjoint()` and `.solve()` APIs.
* SVD implementations now have an `info()` method for checking convergence.
* Most decompositions now fail quickly when invalid inputs are detected.
* Optimized the product of a `HouseholderSequence` with the identity, as well as the evaluation of a `HouseholderSequence` to a dense matrix using faster blocked product.
* Fixed aliasing issues with in-place small matrix inversions.
* Fixed several edge-cases with empty or zero inputs.

### Sparse matrix support, decompositions and solvers

* Enabled assignment and addition with diagonal matrix expressions.
* Support added for SuiteSparse KLU routines via the `KLUSupport` module.  SuiteSparse must be installed to use this module.
* `SparseCholesky` now works with row-major matrices.
* Various bug fixes and performance improvements.

### Type support

* Improved support for `half`
  * Native support added for ARM `__fp16`, CUDA/HIP `__half`, and `F16C` conversion intrinsics.
  * Better vectorization support added across all backends.
* Improved bool support
  * Partial vectorization support added for boolean operations.
  * Significantly improved performance (x25) for logical operations with `Matrix` or `Tensor` of `bool`.
* Improved support for custom types
  * More custom types work out-of-the-box (see #2201).

### Backend-specific improvements

* **Arm NEON**
  * Now provides vectorization for `uint64_t`, `int64_t`, `uint32_t`, `int16_t`, `uint16_t`, `int16_t`, `int8_t`, and `uint8_t`
  * Emulates `bfloat16` support when using `Eigen::bfloat16`
  * Supports emulated and native `float16` when using `Eigen::half`
* **SSE/AVX/AVX512**
  * General performance improvements and bugfixes.
  * Enabled AVX512 instructions by default if available.
  * New `std::complex`, `half`, and `bfloat16` vectorization support added.
  * Many missing packet functions added.
* **Altivec/Power**
  * General performance improvement and bugfixes.
  * Enhanced vectorization of real and complex scalars.
  * Changes to the `gebp_kernel` specific to Altivec, using VSX implementation of the MMA instructions that gain speed improvements up to 4x for matrix-matrix products.
  * Dynamic dispatch for GCC greater than 10 enabling selection of MMA or VSX instructions based on `__builtin_cpu_supports`.
* **GPU (CUDA and HIP)**
  * Several optimized math functions added, better support for `std::complex`.
  * Added option to disable CUDA entirely by defining `EIGEN_NO_CUDA`.
  * Many more functions can now be used in device code (e.g. comparisons, small matrix inversion).
* **ZVector**
  * Vectorized `float` and `std::complex<float>` support added.
  * Added z14 support.
* **SYCL**
  * Redesigned SYCL implementation for use with the [https://eigen.tuxfamily.org/dox/unsupported/eigen_tensors.html Tensor] module, which can be enabled by defining `EIGEN_USE_SYCL`.
  * New generic memory model introduced used by `TensorDeviceSycl`.
  * Better integration with OpenCL devices.
  * Added many math function specializations.

### Miscellaneous API Changes

* New `setConstant(...)` methods for preserving one dimension of a matrix by passing in `NoChange`.
* Added `setUnit(Index i)` for vectors that sets the ''i'' th coefficient to one and all others to zero.
* Added `transpose()`, `adjoint()`, `conjugate()` methods to `SelfAdjointView`.
* Added `shiftLeft<N>()` and `shiftRight<N>()` coefficient-wise arithmetic shift functions to Arrays.
* Enabled adding and subtracting of diagonal expressions.
* Allow user-defined default cache sizes via defining `EIGEN_DEFAULT_L1_CACHE_SIZE`, ..., `EIGEN_DEFAULT_L3_CACHE_SIZE`.
* Added `EIGEN_ALIGNOF(X)` macro for determining alignment of a provided variable.
* Allow plugins for `VectorwiseOp` by defining a file `EIGEN_VECTORWISEOP_PLUGIN` (e.g. `-DEIGEN_VECTORWISEOP_PLUGIN=my_vectorwise_op_plugins.h`).
* Allow disabling of IO operations by defining `EIGEN_NO_IO`.

### Improvement to NaN propagation

* Improvements to NaN correctness for elementwise functions.
* New `NaNPropagation` template argument to control whether NaNs are propagated or suppressed in elementwise `min/max` and corresponding reductions on `Array`, `Matrix`, and `Tensor`.

### New low-latency non-blocking ThreadPool module
* Originally a part of the Tensor module, `Eigen::ThreadPool` is now separate and more portable, and forms the basis for multi-threading in TensorFlow, for example.

### Changes to Tensor module

* Support for c++03 was officially dropped in Tensor module, since most of the code was written in c++11 anyway. This will prevent building the code for CUDA with older version of `nvcc`.
* Performance optimizations of Tensor contraction
  * Speed up "outer-product-like" operations by parallelizing over the contraction dimension, using thread_local buffers and recursive work splitting.
  * Improved threading heuristics.
  * Support for fusing element-wise operations into contraction during evaluation.
* Performance optimizations of other Tensor operator
  * Speedups from improved vectorization, block evaluation, and multi-threading for most operators.
  * Significant speedup to broadcasting.
  * Reduction of index computation overhead, e.g. using fast divisors in TensorGenerator, squeezing dimensions in TensorPadding.
* Complete rewrite of the block (tiling) evaluation framework for tensor expressions lead to significant speedups and reduced number of memory allocations.
* Added new API for asynchronous evaluation of tensor expressions.
* Misc. minor behavior changes & fixes:
  * Fix const correctness for TensorMap.
  * Modify tensor argmin/argmax to always return first occurrence.
  * More numerically stable tree reduction.
  * Improve randomness of the tensor random generator.
  * Update the padding computation for `PADDING_SAME` to be consistent with TensorFlow.
  * Support static dimensions (aka IndexList) in resizing/reshape/broadcast.
  * Improved accuracy of Tensor FFT.

### Changes to sparse iterative solvers
* Added new IDRS iterative linear solver.

### Other relevant changes

* Eigen now provides an option to test with an external BLAS library

See the [announcement](https://www.eigen.tuxfamily.org/index.php?title=3.4) for more details.

## [3.3.9] - 2020-12-04

Changes since 3.3.8:

* Commit 4e5385c90: Introduce rendering Doxygen math formulas with MathJax and the option `EIGEN_DOC_USE_MATHJAX` to control this.
* #1746: Removed implementation of standard copy-constructor and standard copy-assign-operator from PermutationMatrix and Transpositions to allow malloc-less `std::move`.
* #2036: Make sure the find_standard_math_library_test_program compiles and doesn't optimize away functions we try to test for.
* #2046: Rename test/array.cpp to test/array_cwise.cpp to fix an issue with the C++ standard library header "array"
* #2040: Fix an issue in test/ctorleak that occured when disabling exceptions.
* #2011: Remove error counting in OpenMP parallel section in Eigen's GEMM parallelizing logic.
* #2012: Define coeff-wise binary array operators for base class to fix an issue when using Eigen with C++20
* Commit bfdd4a990: Fix an issue with Intel® MKL PARDISO support.

## [3.3.8] - 2020-10-05

Changes since 3.3.7:

* General bug fixes
  * #1995: Fix a failure in the GEBP kernel when using small L1 cache sizes, OpenMP and FMA.
  * #1990: Make CMake accept installation paths relative to `CMAKE_INSTALL_PREFIX`.
  * #1974: Fix issue when reserving an empty sparse matrix
  * #1823: Fix incorrect use of `std::abs`
  * #1788: Fix rule-of-three violations inside the stable modules. This fixes deprecated-copy warnings when compiling with GCC>=9 Also protect some additional Base-constructors from getting called by user code code (#1587)
  * #1796: Make matrix squareroot usable for Map and Ref types
  * #1281: Fix AutoDiffScalar's make_coherent for nested expression of constant ADs.
  * #1761: Fall back `is_integral` to `std::is_integral` in c++11 and fix `internal::is_integral<size_t/ptrdiff_t>` with MSVC 2013 and older.
  * #1741: Fix `self_adjoint*matrix`, `triangular*matrix`, and `triangular^1*matrix` with a destination having a non-trivial inner-stride
  * #1741: Fix SelfAdjointView::rankUpdate and product to triangular part for destination with non-trivial inner stride
  * #1741: Fix `C.noalias() = A*C;` with `C.innerStride()!=1`
  * #1695: Fix a numerical robustness issue in BDCSVD
  * #1692: Enable enum as sizes of Matrix and Array
  * #1689: Fix used-but-marked-unused warning
  * #1679: Avoid possible division by 0 in complex-schur
  * #1676: Fix C++17 template deduction in DenseBase
  * #1669: Fix PartialPivLU/inverse with zero-sized matrices.
  * #1557: Fix RealSchur and EigenSolver for matrices with only zeros on the diagonal.
* Performance related fixes
  * #1562: Optimize evaluation of small products of the form s*A*B by rewriting them as: s*(A.lazyProduct(B)) to save a costly temporary. Measured speedup from 2x to 5x...
  * Commit 165db26dc and 8ee2e10af: Fix performance issue with SimplicialLDLT for complexes coefficients
* Misc commits
  * Commit 5f1082d0b: Fix `QuaternionBase::cast` for quaternion map and wrapper.
  * Commit a153dbae9: Fix case issue with Lapack unit tests.
  * Commit 3d7e2a1f3: Fix possible conflict with an externally defined "real" type when using gcc-5.
  * Commit 1760432f6: Provide `numext::[u]int{32,64}_t`.
  * Commit 3d18879fc: Initialize isometric transforms like affine transforms.
  * Commit 160c0a340: Change typedefs from private to protected to fix MSVC compilation.
  * Commit 3cf273591: Fix compilation of FFTW unit test.
  * Commit 6abc9e537: Fix compilation of BLAS backend and frontend.
  * Commit 47e2f8a42: Fix real/imag namespace conflict.
  * Commit 71d0402e3: Avoid throwing in destructors.
  * Commit 0dd9643ad: Fix precision issue in `SelfAdjointEigenSolver.h`
  * Commit 6ed74ac97: Make `digits10()` return an integer.
  * Commit 841d844f9: Use pade for matrix exponential also for complex values.
  * Commit 4387298e8: Cast Index to RealScalar in SVDBase to fix an issue when RealScalar is not implicitly convertible to Index.
  * Commit fe8cd812b: Provide `EIGEN_HAS_C99_MATH` when using MSVC.
  * Commit 7c4208450: Various fixes in polynomial solver and its unit tests.
  * Commit e777674a8 and 4415d4e2d: Extend polynomial solver unit tests to complexes.
  * Commit 222ce4b49: Automatically switch between EigenSolver and ComplexEigenSolver, and fix a few Real versus Scalar issues.
  * Commit 7b93328ba: Enable construction of `Ref<VectorType>` from a runtime vector.
  * Commit c28ba89fe: Fix a problem of old gcc versions having problems with recursive #pragma GCC diagnostic push/pop.
  * Commit 210d510a9: Fix compilation with expression template scalar type.
  * Commit efd72cddc: Backport AVX512 implementation to 3.3.
  * Commit 5e484fa11: Fix StlDeque compilation issue with GCC 10.
  * Commit a796be81a: Avoid false-positive test results in non-linear optimization tests
  * Commit 9f202c6f1: Fix undefined behaviour caused by uncaught exceptions in OMP section of parallel GEBP kernel.
  * Commit 4707c3aa8: Fix a bug with half-precision floats on GPUs.
* Fixed warnings
  * Commit 14db78c53: Fix some maybe-uninitialized warnings in AmbiVector.h and test bdcsvd.
  * Commit f1b1f13d3: silent cmake warnings in Lapack CMakeLists.txt
  * Commit 8fb28db12: Rename variable which shadows class name in Polynomials module.
  * Commit f1c12d8ff: Workaround gcc's `alloc-size-larger-than=` warning in DenseStorage.h
  * Commit 6870a39fe: Hide some unused variable warnings in g++8.1 in Tensor contraction mapper.
  * Commit bb9981e24: Fix gcc 8.1 warning: "maybe use uninitialized" in std tests
  * Commit eea99eb4e: Fix always true warning with gcc 4.7in test numext.
  * Commit 65a6d4151: Fix nonnull-compare warning in test geo_alignedbox.
  * Commit 74a0c08d7: Disable ignoring attributes warning in vectorization logic test.
  * Commit 6c4d57dc9: Fix a gcc7 warning about bool * bool in abs2 default implementation.
  * Commit 89a86ed42: Fix a warning in SparseSelfAdjointView about a branch statement always evaluation to false.

## [3.3.8-rc1] - 2020-09-14

Changes since 3.3.7:

* General bug fixes
  * #1974: Fix issue when reserving an empty sparse matrix
  * #1823: Fix incorrect use of `std::abs`
  * #1788: Fix rule-of-three violations inside the stable modules. This fixes deprecated-copy warnings when compiling with GCC>=9 Also protect some additional Base-constructors from getting called by user code code (#1587)
  * #1796: Make matrix squareroot usable for Map and Ref types
  * #1281: Fix AutoDiffScalar's `make_coherent` for nested expression of constant ADs.
  * #1761: Fall back `is_integral` to `std::is_integral` in c++11 and fix `internal::is_integral<size_t/ptrdiff_t>` with MSVC 2013 and older.
  * #1741: Fix `self_adjoint*matrix`, `triangular*matrix`, and `triangular^1*matrix` with a destination having a non-trivial inner-stride
  * #1741: Fix SelfAdjointView::rankUpdate and product to triangular part for destination with non-trivial inner stride
  * #1741: Fix `C.noalias() = A*C;` with `C.innerStride()!=1`
  * #1695: Fix a numerical robustness issue in BDCSVD
  * #1692: Enable enum as sizes of Matrix and Array
  * #1689: Fix used-but-marked-unused warning
  * #1679: Avoid possible division by 0 in complex-schur
  * #1676: Fix C++17 template deduction in DenseBase
  * #1669: Fix PartialPivLU/inverse with zero-sized matrices.
  * #1557: Fix RealSchur and EigenSolver for matrices with only zeros on the diagonal.
* Performance related fixes
  * #1562: Optimize evaluation of small products of the form s*A*B by rewriting them as: s*(A.lazyProduct(B)) to save a costly temporary. Measured speedup from 2x to 5x...
  * Commit 165db26dc and 8ee2e10af: Fix performance issue with SimplicialLDLT for complexes coefficients
* Misc commits
  * Commit 5f1082d0b: Fix `QuaternionBase::cast` for quaternion map and wrapper.
  * Commit a153dbae9: Fix case issue with Lapack unit tests.
  * Commit 3d7e2a1f3: Fix possible conflict with an externally defined "real" type when using gcc-5.
  * Commit 1760432f6: Provide `numext::[u]int{32,64}_t`.
  * Commit 3d18879fc: Initialize isometric transforms like affine transforms.
  * Commit 160c0a340: Change typedefs from private to protected to fix MSVC compilation.
  * Commit 3cf273591: Fix compilation of FFTW unit test.
  * Commit 6abc9e537: Fix compilation of BLAS backend and frontend.
  * Commit 47e2f8a42: Fix real/imag namespace conflict.
  * Commit 71d0402e3: Avoid throwing in destructors.
  * Commit 0dd9643ad: Fix precision issue in SelfAdjointEigenSolver.h
  * Commit 6ed74ac97: Make digits10() return an integer.
  * Commit 841d844f9: Use pade for matrix exponential also for complex values.
  * Commit 4387298e8: Cast Index to RealScalar in SVDBase to fix an issue when RealScalar is not implicitly convertible to Index.
  * Commit fe8cd812b: Provide `EIGEN_HAS_C99_MATH` when using MSVC.
  * Commit 7c4208450: Various fixes in polynomial solver and its unit tests.
  * Commit e777674a8 and 4415d4e2d: Extend polynomial solver unit tests to complexes.
  * Commit 222ce4b49: Automatically switch between EigenSolver and ComplexEigenSolver, and fix a few Real versus Scalar issues.
  * Commit 5110d803e: Change license from LGPL to MPL2 with agreement from David Harmon. (grafted from 2df4f0024666a9085fe47f14e2290bd61676dbbd )
  * Commit 7b93328ba: Enable construction of `Ref<VectorType>` from a runtime vector.
  * Commit c28ba89fe: Fix a problem of old gcc versions having problems with recursive #pragma GCC diagnostic push/pop.
  * Commit 210d510a9: Fix compilation with expression template scalar type.
* Fixed warnings
  * Commit 14db78c53: Fix some maybe-uninitialized warnings in AmbiVector.h and test bdcsvd.
  * Commit f1b1f13d3: silent cmake warnings in Lapack CMakeLists.txt
  * Commit 8fb28db12: Rename variable which shadows class name in Polynomials module.
  * Commit f1c12d8ff: Workaround gcc's `alloc-size-larger-than=` warning in DenseStorage.h
  * Commit 6870a39fe: Hide some unused variable warnings in g++8.1 in Tensor contraction mapper.
  * Commit bb9981e24: Fix gcc 8.1 warning: "maybe use uninitialized" in std tests
  * Commit eea99eb4e: Fix always true warning with gcc 4.7in test `numext`.
  * Commit 65a6d4151: Fix nonnull-compare warning in test `geo_alignedbox`.
  * Commit 74a0c08d7: Disable ignoring attributes warning in vectorization logic test.
  * Commit 6c4d57dc9: Fix a gcc7 warning about bool * bool in abs2 default implementation.
  * Commit efd72cddc: Backport AVX512 implementation to 3.3.
  * Commit 5e484fa11: Fix StlDeque compilation issue with GCC 10.
  * Commit 89a86ed42: Fix a warning in SparseSelfAdjointView about a branch statement always evaluation to false.
  * Commit dd6de618: Fix a bug with half-precision floats on GPUs.

## [3.3.7] - 2018-12-11
¸¸¸¸
Changes since 3.3.6:

* #1643: Fix compilation with GCC>=6 and compiler optimization turned off.

## [3.3.6] - 2018-12-10

Changes since 3.3.5:

* #1617: Fix triangular solve crashing for empty matrix.
* #785: Make dense Cholesky decomposition work for empty matrices.
* #1634: Remove double copy in move-ctor of non movable Matrix/Array.
* Changeset a2d6c106a450: Workaround weird MSVC bug.
* #1637 Workaround performance regression in matrix products with gcc>=6 and clang>=6.0.
* Changeset 9ccbaaf3dd4c: Fix some implicit 0 to Scalar conversions.
* #1605: Workaround ABI issue with vector types (aka `__m128`) versus scalar types (aka float).
* Changeset 148e579cc004: Fix for gcc<4.6 regarding usage of #pragma GCC diagnostic push/pop.
* Changeset bc000deaae45: Fix conjugate-gradient for right-hand-sides with a very small magnitude.
* Changeset 5be00b0e2964: Fix product of empty arrays (returned 0 instead of 1).
* #1590: Fix collision with some system headers defining the macro FP32.
* #1584: Fix possible undefined behavior in random generation.
* Changeset e4127b0f7d3b: Fix fallback to BLAS for rankUpdate.
* Fixes for NVCC 9.
* Fix matrix-market IO.
* Various fixes in the doc.
* Various minor warning fixes/workarounds.

## [3.3.5] - 2018-07-23

Changes since 3.3.4:

* General bug fixes:
  * Fix GeneralizedEigenSolver when requesting for eigenvalues only (ab3fa2e12308)
  * #1560 fix product with a 1x1 diagonal matrix (483beabab9bf)
  * #1543: fix linear indexing in generic block evaluation
  * Fix compilation of product with inverse transpositions (e.g., `mat * Transpositions().inverse()`) (170914dbbcc3)
  * #1509: fix `computeInverseWithCheck` for complexes (a2a2c3c86507)
  * #1521: avoid signalling `NaN` in hypot and make it std::complex<> friendly (b18e2d422b09).
  * #1517: fix triangular product with unit diagonal and nested scaling factor: `(s*A).triangularView<UpperUnit>()*B` (c24844195d90)
  * Fix compilation of stableNorm for some odd expressions as input (33b972d8b384)
  * #1485: fix linking issue of non template functions (d18877f18d8e)
  * Fix overflow issues in BDCSVD (7a875acfb05f)
  * #1468: add missing `std::` to `memcpy` (32a6db0f8cd5)
  * #1453: fix Map with non-default inner-stride but no outer-stride (1ca9072b51d8)
  * Fix mixing types in sparse matrix products (4ead16cdd6c8)
  * #1544: Generate correct Q matrix in complex case (39125654ce9e)
  * #1461: fix compilation of `Map<const Quaternion>::x()` (9a266e5118cf)

* Backends:
  * Fix MKL backend for symmetric eigenvalues on row-major matrices (eab7afe25273)
  * #1527: fix support for MKL's VML (86a939451c75)
  * Fix incorrect ldvt in LAPACKE call from JacobiSVD (bfc66e8b9a3b)
  * Fix support for MKL's BLAS when using `MKL_DIRECT_CALL` (9df7f3d8e9cd, 3108fbf76708, 292dea7922e7)
  * Use MKL's lapacke.h header when using MKL (070b5958e0ae)

* Diagnostics:
  * #1516: add assertion for out-of-range diagonal index in `MatrixBase::diagonal(i)` (273738ba6f6e)
  * Add static assertion for fixed sizes `Ref<>` (1724dae8b834)
  * Add static assertion on selfadjoint-view's UpLo parameter. (74daf12e525e, 190b46dd1f05)
  * #1479: fix failure detection in LDLT (c20043c8fd64)

* Compiler support:
  * #1555: compilation fix with XLC
  * Workaround MSVC 2013 ambiguous calls (c92536d92647)
  * Adds missing `EIGEN_STRONG_INLINE` to help MSVC properly inlining small vector calculations (01fb6217335b)
  * Several minor warning fixes: f90d136c8445, 542fb03968c2, "used uninitialized" (7634a44bfe11), Wint-in-bool-context (3d1795da28c2, d1c2d6683c55)
  * #1428: make NEON vectorization compilable by MSVC. (* 1e2d2693b911, 927d023ceaab)
  * Fix compilation and SSE support with PGI compiler (bb87f618bfc3 450c5e5d2771)
  * #1555: compilation fix with XLC (20ca86888e70)
  * #1520: workaround some `-Wfloat-equal` warnings by calling `std::equal_to` (1c4fdad7bd6f)
  * Make the TensorStorage class compile with clang 3.9 (a7144f8d6a94)
  * Misc: some old compiler fixes (b60cbbef3791)
  * Fix MSVC warning C4290: C++ exception specification ignored except to indicate a function is not `__declspec(nothrow)` (3df78d5afc1e)

* Architecture support:
  * Several AVX512 fixes for `log`, `sqrt`, `rsqrt`, non `AVX512ER` CPUs, `apply_rotation_in_the_plane` 5c59564bfb92 1939c971a3db c2f9e6cb37e5, 609e425166f6.
  * AltiVec fixes: 1641a6cdd5a4
  * NEON fixes: const-cast (877a2b64c9ba), compilation of Jacobi rotations (bc837b797559,#1436).
  * Changeset 971b32440c74: Define `pcast<>` for SSE types even when AVX is enabled. (otherwise float are silently reinterpreted as int instead of being converted)
  * #1494: makes `pmin`/`pmax` behave on Altivec/VSX as on x86 regarding NaNs (892c0a79ce93)

* Documentation:
  * Update manual pages regarding BDCSVD (#1538)
  * Add aliasing in common pitfaffs (656712d48f6b)
  * Update `aligned_allocator` (6fc0f2be70a4)
  * #1456: add perf recommendation for LLT and storage format (55fbf4fedd04,  9fd138e2b333)
  * #1455: Cholesky module depends on Jacobi for rank-updates (b87875abf8dc)
  * #1458: fix documentation of LLT and LDLT `info()` method (ac2c97edff07)
  * Warn about constness in `LLT::solveInPlace` (51e1aa153957)
  * Fix lazyness of `operator*` with CUDA (fa77d713359d)
  * #336: improve doc for `PlainObjectBase::Map` (18868228adae)

* Other general improvements:
  * Enable linear indexing in generic block evaluation (15752027ec2f, 80af7d6a47c1, #1543).
  * Fix packet and alignment propagation logic of `Block<Xpr>` expressions. In particular, `(A+B).col(j)` now preserve vectorisation. (9c9e90f6db7e)
  * Several fixes regarding custom scalar type support: hypot (385d8b5e42c2), boost-multiprec (5f71579a2d3f), literal casts (e6577f3c3049, fbb0c510c52f),
  * LLT: avoid making a copy when decomposing in place (9d03711df8bc), const the arg to `solveInPlace()` to allow passing `.transpose()`, `.block()`, etc. (0137ed4f19b6).
  * Add possibility to overwrite `EIGEN_STRONG_INLINE` (6d6e5fcd4356)
  * #1528: use `numeric_limits::min()` instead of `1/highest()` that might underflow (9ff315024335)
  * #1532: disable `stl::*_negate` in C++17 (they are deprecated) (3fb42ff7b278)
  * Add C++11 `max_digits10` for half (70ac6c923001)
  * Make sparse QR result sizes consistent with dense QR (2136cfa17e28)

* Unsupported/unit-tests/cmake/unvisible internals/etc.
  * #1484: restore deleted line for 128 bits long doubles, and improve dispatching logic. (c8e663fe87ec)
  * #1462: remove all occurences of the deprecated `__CUDACC_VER__` macro by introducing `EIGEN_CUDACC_VER` (e7c065ec717b)
  * Changeset fea50d40ea79: Fix oversharding bug in parallelFor.
  * Changeset 866d222d6065: commit 45e9c9996da790b55ed9c4b0dfeae49492ac5c46 (HEAD -> memory_fix)
  * Changeset 48048172e5aa: Fix int versus Index
  * Changeset 906a98fe39c3: fix linking issue
  * Changeset 352489edbe36: Fix short vs long
  * Changeset 81e94eea024c: Fix cmake scripts with no fortran compiler
  * Changeset 8bd392ca0e3f: add cmake-option to enable/disable creation of tests
  * Changeset 02c0cef97fb5: Use col method for column-major matrix
  * Changeset a8d2459f8e1f: #1449: fix `redux_3` unit test
  * Changeset e90a14609a56: Fix uninitialized output argument.
  * Changeset 5d40715db6a7: Handle min/max/inf/etc issue in `cuda_fp16.h` directly in `test/main.h`
  * Changeset 2f9de522457b: Add tests for sparseQR results (value and size) covering bugs 1522 and 1544
  * Changeset 4662c610c13c: `SelfAdjointView<...,Mode>` causes a static assert since commit d820ab9edc0b
  * Changeset 96134409fc91: weird compilation issue in `mapped_matrix.cpp`

## [3.3.4] - 2017-06-15

Changes since 3.3.3:

* General:
  * Improve speed of Jacobi rotation when mixing complex and real types.
  * #1405: enable StrictlyLower/StrictlyUpper triangularView as the destination of matrix*matrix products.
  * UmfPack support: enable changes in the control settings and add report functions.
  * #1423: fix LSCG's Jacobi preconditioner for row-major matrices.
  * #1424: fix compilation issue with abs and unsigned integers as scalar type.
  * #1410: fix lvalue propagation of Array/Matrix-Wrapper with a const nested expression.
  * #1403: fix several implicit scalar type conversion making SVD decompositions compatible with ceres::Jet.
  * Fix some real-to-scalar-to-real useless conversions in `ColPivHouseholderQR`.
* Regressions:
  * Fix `dense * sparse_selfadjoint_view` product.
  * #1417: make LinSpace compatible with std::complex.
  * #1400: fix `stableNorm` alignment issue with `EIGEN_DONT_ALIGN_STATICALLY`.
  * #1411: fix alignment issue in `Quaternion`.
  * Fix compilation of operations between nested Arrays.
  * #1435: fix aliasing issue in expressions like: `A = C - B*A`.
* Others:
  * Fix compilation with gcc 4.3 and ARM NEON.
  * Fix prefetches on ARM64 and ARM32.
  * Fix out-of-bounds check in COLAMD.
  * Few minor fixes regarding nvcc/CUDA support, including #1396.
  * Improve cmake scripts for Pastix and BLAS detection.
  * #1401: fix compilation of `cond ? x : -x` with `x` an `AutoDiffScalar`
  * Fix compilation of matrix log with Map as input.
  * Add specializations of `std::numeric_limits` for `Eigen::half` and and `AutoDiffScalar`
  * Fix compilation of streaming nested Array, i.e., `std::cout << Array<Array<...>>`

## [3.3.3] - 2017-02-21

Changes since 3.3.2:

* General:
  * Improve multi-threading heuristic for matrix products with a small number of columns.
  * #1395: fix compilation of JacobiSVD for vectors type.
  * Fix pruning in `(sparse*sparse).pruned()` when the result is nearly dense.
  * #1382: move using `std::size_t`/`ptrdiff_t` to Eigen's namespace.
  * Fix compilation and inlining when using clang-cl with visual studio.
  * #1392: fix `#include <Eigen/Sparse>` with mpl2-only.
* Regressions:
  * #1379: fix compilation in `sparse*diagonal*dense` with OpenMP.
  * #1373: add missing assertion on size mismatch with compound assignment operators (e.g., mat += mat.col(j))
  * #1375: fix cmake installation with cmake 2.8.
  * #1383: fix LinSpaced with integers for `LinPspaced(n,0,n-1)` with `n==0` or the `high<low` case.
  * #1381: fix `sparse.diagonal()` used as a rvalue.
  * #1384: fix evaluation of "sparse/scalar" that used the wrong evaluation path.
  * #478: fix regression in the eigen decomposition of zero matrices.
  * Fix a compilation issue with MSVC regarding the usage of `CUDACC_VER`
  * #1393: enable Matrix/Array explicit constructor from types with conversion operators.
  * #1394: fix compilation of `SelfAdjointEigenSolver<Matrix>(sparse*sparse)`.
* Others:
  * Fix ARM NEON wrapper for 16 byte systems.
  * #1391: include IO.h before DenseBase to enable its usage in DenseBase plugins.
  * #1389: fix std containers support with MSVC and AVX.
  * #1380: fix matrix exponential with `Map<>`.
  * #1369: fix type mismatch warning with OpenMP.
  * Fix usage of `size_t` instead of Index in sefl-adjoint `matrix * vector`
  * #1378: fix doc (`DiagonalIndex` vs `Diagonal`).

## [3.3.2] - 2017-01-18
Changes since 3.3.1:

* General:
  * Add `transpose`, `adjoint`, `conjugate` methods to `SelfAdjointView` (useful to write generic code)
  * Make sure that `HyperPlane::transform` maintains a unit normal vector in the Affine case.
  * Several documentation improvements, including: several doxygen workarounds, #1336, #1370, StorageIndex, selfadjointView, sparseView(), sparse triangular solve, AsciiQuickReference.txt, ...
* Regressions:
  * #1358: fix compilation of `sparse += sparse.selfadjointView()`.
  * #1359: fix compilation of `sparse /=scalar`, `sparse *=scalar`, and `col_major_sparse.row() *= scalar`.
  * #1361:  fix compilation of mat=perm.inverse()
  * Some fixes in sparse coeff-wise binary operations: add missing `.outer()` member to iterators, and properly report storage order.
  * Fix aliasing issue in code as `A.triangularView() = B*A.sefladjointView()*B.adjoint()`
* Performance:
  * Improve code generation for `mat*vec` on some compilers.
  * Optimize horizontal adds in SSE3 and AVX.
  * Speed up row-major TRSM (triangular solve with a matrix as right-hand-side) by reverting `vec/y` to `vec*(1/y)`. The rationale is:
    * div is extremely costly
    * this is consistent with the column-major case
    * this is consistent with all other BLAS implementations
  * Remove one temporary in `SparseLU::solve()`
* Others:
  * Fix BLAS backend for symmetric rank K updates.
  * #1360: fix `-0` vs `+0` issue with Altivec
  * #1363: fix mingw's ABI issue
  * #1367: fix compilation with gcc 4.1.
  * Fix ABI issue with AVX and old gcc versions.
  * Fix some warnings with ICC, Power8, etc.
  * Fix compilation with MSVC 2017

## [3.3.1] - 2016-12-06

Changes since 3.3.0:

* #426: add operators `&&` and `||` to dense and sparse matrices (only dense arrays were supported)
* #1319: add support for CMake's imported targets.
* #1343: fix compilation regression in `array = matrix_product` and `mat+=selfadjoint_view`
* Fix regression in assignment of sparse block to sparse block.
* Fix a memory leak in `Ref<SparseMatrix>` and `Ref<SparseVector>`.
* #1351: fix compilation of random with old compilers.
* Fix a performance regression in (mat*mat)*vec for which mat*mat was evaluated multiple times.
* Fix a regression in `SparseMatrix::ReverseInnerIterator`
* Fix performance issue of products for dynamic size matrices with fixed max size.
* implement `float`/`std::complex<float>` for ZVector
* Some fixes for expression-template scalar-types
* #1356: fix undefined behavior with nullptr.
* Workaround some compilation errors with MSVC and MSVC/clr
* #1348: document `EIGEN_MAX_ALIGN_BYTES` and `EIGEN_MAX_STATIC_ALIGN_BYTES`, and reflect in the doc that `EIGEN_DONT_ALIGN*` are deprecated.
* Bugs #1346,#1347: make Eigen's installation relocatable.
* Fix some harmless compilation warnings.

## [3.3] - 2016-11-10

For a comprehensive list of change since the 3.2 series, see this [page](https://www.eigen.tuxfamily.org/index.php?title=3.3).


Main changes since 3.3-rc2:
* Fix regression in printing sparse expressions.
* Fix sparse solvers when using a SparseVector as the result and/or right-hand-side.

## [3.3-rc2] - 2016-11-04

For a comprehensive list of change since the 3.2 series, see this [page](https://www.eigen.tuxfamily.org/index.php?title=3.3).

Main changes since 3.3-rc1:
* Core module
  * Add supports for AVX512 SIMD instruction set.
  * Bugs #698 and  #1004: Improve numerical robustness of LinSpaced methods for both real and integer scalar types ([details](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1DenseBase.html#aaef589c1dbd7fad93f97bd3fa1b1e768)).
  * Fix a regression in `X = (X*X.transpose())/scalar` with `X` rectangular (`X` was resized before the evaluation).
  * #1311: Fix alignment logic in some cases of `(scalar*small).lazyProduct(small)`
  * #1317:  fix a performance regression from 3.2 with clang and some nested block expressions.
  * #1308: fix compilation of some small products involving nullary-expressions.
  * #1333: Fix a regression with `mat.array().sum()`
  * #1328: Fix a compilation issue with old compilers introduced in 3.3-rc1.
  * #1325: Fix compilation on NEON with clang
  * Properly handle negative inputs in vectorized sqrt.
  * Improve cost-model to determine the ideal number of threads in matrix-matrix products.
* Geometry module
  * #1304: Fix `Projective * scaling` and `Projective *= scaling`.
  * #1310: Workaround a compilation regression from 3.2 regarding triangular * homogeneous
  * #1312: Quaternion to AxisAngle conversion now ensures the angle will be in the range `[0,pi]`. This also increases accuracy when `q_w` is negative.
* Tensor module
  * Add support for OpenCL.
  * Improved random number generation.
* Other
  * #1330: SuiteSparse, explicitly handle the fact that Cholmod does not support single precision float numbers.
  * SuiteSparse, fix SPQR for rectangular matrices
  * Fix compilation of `qr.inverse()` for column and full pivoting variants

## [3.2.10] - 2016-10-04

Changes since 3.2.9:

Main fixes and improvements:
* #1272: Core module, improve comma-initializer in handling empty matrices.
* #1276: Core module, remove all references to `std::binder*` when C++11 is enabled (those are deprecated).
* #1304: Geometry module, fix `Projective * scaling` and `Projective *= scaling`.
* #1300: Sparse module, compilation fix for some block expression and SPQR support.
* Sparse module, fix support for row (resp. column) of a column-major (resp. row-major) sparse matrix.
* LU module, fix 4x4 matrix inversion for non-linear destinations.
* Core module, a few fixes regarding custom complex types.
* #1275: backported improved random generator from 3.3
* Workaround MSVC 2013 compilation issue in Reverse
* Fix UmfPackLU constructor for expressions.
* #1273: fix shortcoming in eigen_assert macro
* #1249:  disable the use of `__builtin_prefetch` for compilers other than GCC, clang, and ICC.
* #1265: fix doc of QR decompositions

## [3.3-rc1] - 2016-09-22

For a comprehensive list of change since the 3.2 series, see this [page](https://www.eigen.tuxfamily.org/index.php?title=3.3).

Main changes since 3.3-beta2:

* New features and improvements:
  * #645: implement eigenvector computation in GeneralizedEigenSolver
  * #1271: add a `SparseMatrix::coeffs()` method returning a linear view of the non-zeros (for compressed mode only).
  * #1286: Improve support for custom nullary functors: now the functor only has to expose one relevant operator among `f()`, `f(i)`, `f(i,j)`.
  * #1272: improve comma-initializer in handling empty matrices.
  * #1268: detect failure in LDLT and report them through info()
  * Add support for scalar factor in sparse-selfadjoint `*` dense products, and enable `+=`/`-=` assignment for such products.
  * Remove temporaries in product expressions matching `d?=a-b*c` by rewriting them as `d?=a; d?=b*c;`
  * Vectorization improvements for some small product cases.

* Doc:
  * #1265: fix outdated doc in QR facto
  * #828: improve documentation of sparse block methods, and sparse unary methods.
  * Improve documentation regarding nullary functors, and add an example demonstrating the use of nullary expression to perform fancy matrix manipulations.
  * Doc: explain how to use Accelerate as a LAPACK backend.

* Bug fixes and internal changes:
  * Numerous fixes regarding support for custom complex types.
  * #1273: fix shortcoming in `eigen_assert` macro
  * #1278: code formatting
  * #1270: by-pass hand written `pmadd` with recent clang versions.
  * #1282: fix implicit double to float conversion warning
  * #1167: simplify installation of header files using cmake's `install(DIRECTORY ...)` command
  * #1283: fix products involving an uncommon `vector.block(..)` expressions.
  * #1285: fix a minor regression in LU factorization.
  * JacobiSVD now consider any denormal number as zero.
  * Numerous fixes regarding support for CUDA/NVCC (including bugs #1266)
  * Fix an alignment issue in gemv, symv, and trmv for statically allocated temporaries.
  * Fix 4x4 matrix inversion for non-linear destinations.
  * Numerous improvements and fixes in half precision scalar type.
  * Fix vectorization logic for coeff-based product for some corner cases
  * Bugs #1260, #1261, #1264: several fixes in AutoDiffScalar.

## [3.3-beta2] - 2016-08-26

For a comprehensive list of change since the 3.2 series, see this [page](https://www.eigen.tuxfamily.org/index.php?title=3.3).

Main changes since 3.3-beta1:

* Dense features:
  * #707: Add support for [inplace](http://eigen.tuxfamily.org/dox-devel/group__InplaceDecomposition.html) dense decompositions.
  * #977: normalize(d) left the input unchanged if its norm is 0 or too close to 0.
  * #977: add stableNormalize[d] methods: they are analogues to normalize[d] but with carefull handling of under/over-flow.
  * #279: Implement generic scalar*expr and expr*scalar operators. This is especially useful for custom scalar types, e.g., to enable `float*expr<multi_prec>` without conversion.
  * New unsupported/Eigen/SpecialFunctions module providing the following coefficient-wise math functions: erf, erfc, lgamma, digamma, polygamma, igamma, igammac, zeta, betainc.
  * Add fast reciprocal condition estimators in dense LU and Cholesky factorizations.
  * #1230: add support for `SelfadjointView::triangularView()` and `diagonal()`
  * #823: add `Quaternion::UnitRandom()` method.
  * Add exclusive or operator for bool arrays.
  * Relax dependency on MKL for `EIGEN_USE_BLAS` and `EIGEN_USE_LAPACKE`: any BLAS and LAPACK libraries can now be used as backend (see  [doc](http://eigen.tuxfamily.org/dox-devel/TopicUsingBlasLapack.html)).
  * Add static assertion to `x()`, `y()`, `z()`, `w()` accessors
  * #51: avoid dynamic memory allocation in fixed-size rank-updates, matrix products evaluated within a triangular part, and selfadjoint times matrix products.
  * #696: enable zero-sized block at compile-time by relaxing the respective assertion
  * #779: in `Map`, allows non aligned buffers for buffers smaller than the requested alignment.
  * Add a complete orthogonal decomposition class: [CompleteOrthogonalDecomposition](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1CompleteOrthogonalDecomposition.html)
  * Improve robustness of JacoviSVD with complexes (underflow, noise amplification in complex to real conversion, compare off-diagonal entries to the current biggest diagonal entry instead of the global biggest, null inputs).
  * Change Eigen's ColPivHouseholderQR to use a numerically stable norm downdate formula (changeset acce4dd0500f)
  * #1214: consider denormals as zero in D&C SVD. This also workaround infinite binary search when compiling with ICC's unsafe optimizations.
  * Add log1p for arrays.
  * #1193: now `lpNorm<Infinity>` supports empty inputs.
  * #1151: remove useless critical section in matrix product
  * Add missing non-const reverse method in `VectorwiseOp` (e.g., this enables `A.rowwise().reverse() = ...`)
  * Update RealQZ to reduce 2x2 diagonal block of T corresponding to non reduced diagonal block of S to positive diagonal form.

* Sparse features:
  * #632: add support for "dense +/- sparse" operations. The current implementation is based on SparseView to make the dense subexpression compatible with the sparse one.
  * #1095: add Cholmod*::logDeterminant/determinant functions.
  * Add `SparseVector::conservativeResize()` method
  * #946: generalize `Cholmod::solve` to handle any rhs expressions.
  * #1150: make IncompleteCholesky more robust by iteratively increase the shift until the factorization succeed (with at most 10 attempts)
  * #557: make InnerIterator of sparse storage types more versatile by adding default-ctor, copy-ctor/assignment.
  * #694: document that `SparseQR::matrixR` is not sorted.
  * Block expressions now expose all the information defining the block.
  * Fix GMRES returned error.
  * #1119: add support for SuperLU 5

* Performance improvements:
  *  #256: enable vectorization with unaligned loads/stores. This concerns all architectures and all sizes. This new behavior can be disabled by defining `EIGEN_UNALIGNED_VECTORIZE=0`
  * Add support for s390x(zEC13) ZVECTOR instruction set.
  * Optimize mixing of real with complex matrices by avoiding a conversion from real to complex when the real types do not match exactly. (see 76faf4a9657e)
  * Speedup square roots in performance critical methods such as norm, normalize(d).
  * #1154: use dynamic scheduling for spmv products.
  * #667,  #1181: improve perf with MSVC and ICC through `FORCE_INLINE`
  * Improve heuristics for switching between coeff-based and general matrix product implementation at compile-time.
  * Add vectorization of tanh for float (SSE/AVX)
  * Improve cost estimates of numerous functors.
  * Numerous improvements regarding half-packet vectorization: coeff-based products (e.g.,  `Matrix4f*Vector4f` is now vectorized again when using AVX), reductions, linear vs inner traversals.
  * Fix performance regression: with AVX, unaligned stores were emitted instead of aligned ones for fixed size assignment.
  * #1201: optimize `affine*vector` products.
  * #1191: prevent Clang/ARM from rewriting VMLA into VMUL+VADD.
  * Small speed-up in `Quaternion::slerp`.
  * #1201:  improve code generation of affine*vec with MSVC

* Doc:
  * Add [documentation and exemple](http://eigen.tuxfamily.org/dox-devel/group__MatrixfreeSolverExample.html) for matrix-free solving.
  * A new documentation [page](http://eigen.tuxfamily.org/dox-devel/group__CoeffwiseMathFunctions.html) summarizing coefficient-wise math functions.
  * #1144: clarify the doc about aliasing in case of resizing and matrix product.
  * A new documentation [page](http://eigen.tuxfamily.org/dox-devel/group__DenseDecompositionBenchmark.html) summarizing the true performance of Eigen's dense decomposition algorithms.

* Misc improvements:
  * Allow one generic scalar argument for all binary operators/functions.
  * Add a `EIGEN_MAX_CPP_VER` option to limit the C++ version to be used, as well as [fine grained options](http://eigen.tuxfamily.org/dox-devel/TopicPreprocessorDirectives.html#title1) to control individual language features.
  * A new [ScalarBinaryOpTraits](http://eigen.tuxfamily.org/dox-devel/structEigen_1_1ScalarBinaryOpTraits.html) class allowing to control how different scalar types are mixed.
  * `NumTraits` now exposes a `digits10` function making `internal::significant_decimals_impl` deprecated.
  * Countless improvements and fixes in Tensors module.
  * #1156: fix several function declarations whose arguments were passed by value instead of being passed by reference
  * #1164: fix `std::list` and `std::deque` specializations such that our aligned allocator is automatically activated only when the user did not specified an allocator (or specified the default std::allocator).
  * #795: mention allocate_shared as a candidate for aligned_allocator.
  * #1170: skip calls to memcpy/memmove for empty inputs.
  * #1203: by-pass large stack-allocation in stableNorm if `EIGEN_STACK_ALLOCATION_LIMIT` is too small
  * Improve constness of blas level-2/3 interface.
  * Implement stricter argument checking for SYRK and SY2K
  * Countless improvements in the documentations.
  * Internal: Remove `posix_memalign`, `_mm_malloc`, and `_aligned_malloc` special paths.
  * Internal: Remove custom unaligned loads for SSE
  * Internal: introduce `[U]IntPtr` types to be used for casting pointers to integers.
  * Internal: `NumTraits` now exposes `infinity()`
  * Internal: `EvalBeforeNestingBit` is now deprecated.
  * #1213: workaround gcc linking issue with anonymous enums.
  * #1242: fix comma initializer with empty matrices.
  * #725: make move ctor/assignment noexcept
  * Add minimal support for `Array<string>`
  * Improve support for custom scalar types bases on expression template (e.g., `boost::multiprecision::number<>` type). All dense decompositions are successfully tested.

* Most visible fixes:
  * #1144: fix regression in `x=y+A*x`  (aliasing issue)
  * #1140: fix usage of `_mm256_set_m128` and `_mm256_setr_m128` in AVX support
  * #1141: fix some missing initializations in CholmodSupport
  * #1143: workaround gcc bug #10200
  * #1145, #1147,  #1148,  #1149: numerous fixes in PastixSupport
  * #1153: don't rely on `__GXX_EXPERIMENTAL_CXX0X__` to detect C++11 support.
  * #1152: fix data race in static initialization of blas routines.
  * fix some buffer overflow in product block size computation.
  * #96,  #1006: fix by value argument in result_of
  * #178: clean several `const_cast`.
  * Fix compilation in `ceil()` function.
  * #698: fix linspaced for integer types.
  * #1161: fix division by zero for huge scalar types in cache block size computation.
  * #774: fix a numerical issue in Umeyama algorithm that produced unwanted reflections.
  * #901: fix triangular-view with unit diagonal of sparse rectangular matrices.
  * #1166: fix shortcoming in gemv when the destination is not a vector at compile-time.
  * #1172: make `SparseMatrix::valuePtr` and `innerIndexPtr` properly return null for empty matrices
  * #537:  fix a compilation issue in Quaternion with Apples's compiler
  * #1186: fix usage of `vreinterpretq_u64_f64` (NEON)
  * #1190: fix usage of `__ARM_FEATURE_FMA` on Clang/ARM
  * #1189:  fix pow/atan2 compilation for `AutoDiffScalar`
  * Fix detection of same input-output when applied permutations, or on solve operations.
  * Workaround a division by zero in triangular solve when outerstride==0
  * Fix compilation of s`parse.cast<>().transpose()`.
  * Fix double-conversion warnings throughout the code.
  *  #1207: fix logical-op warnings
  *  #1222,  #1223: fix compilation in `AutoDiffScalar`.
  *  #1229: fix usage of `Derived::Options` in MatrixFunctions.
  *  #1224: fix regression in `(dense*dense).sparseView()`.
  *  #1231: fix compilation regression regarding complex_array/=real_array.
  *  #1221: disable gcc 6 warning: ignoring attributes on template argument.
  * Workaround clang/llvm bug 27908
  *  #1236: fix possible integer overflow in sparse matrix product.
  *  #1238: fix `SparseMatrix::sum()` overload for un-compressed mode
  * #1240: remove any assumption on NEON vector types
  * Improves support for MKL's PARDISO solver.
  * Fix support for Visual 2010.
  * Fix support for gcc 4.1.
  * Fix support for ICC 2016
  * Various Altivec/VSX fixes: exp, support for clang 3.9,
  *  #1258: fix compilation of `Map<SparseMatrix>::coeffRef`
  *  #1249: fix compilation with compilers that do not support `__builtin_prefetch` .
  *  #1250: fix `pow()` for `AutoDiffScalar` with custom nested scalar type.

## [3.2.9] - 2016-08-18

Changes since 3.2.8:

* Main fixes and improvements:
  * Improve numerical robustness of JacobiSVD (backported from 3.3)
  * #1017: prevents underflows in `makeHouseholder`
  * Fix numerical accuracy issue in the extraction of complex eigenvalue pairs in real generalized eigenvalue problems.
  * Fix support for `vector.homogeneous().asDiagonal()`
  * #1238: fix `SparseMatrix::sum()` overload for un-compressed mode
  * #1213: workaround gcc linking issue with anonymous enums.
  * #1236: fix possible integer overflow in sparse-sparse product
  * Improve detection of identical matrices when applying a permutation (e.g., `mat = perm * mat`)
  * Fix usage of nesting type in blas_traits. In practice, this fixes compilation of expressions such as `A*(A*A)^T`
  * CMake: fixes support of Ninja generator
  * Add a StorageIndex typedef to sparse matrices and expressions to ease porting code to 3.3 (see http://eigen.tuxfamily.org/index.php?title=3.3#Index_typedef)
  * #1200: make `aligned_allocator` c++11 compatible (backported from 3.3)
  * #1182: improve generality of `abs2` (backported from 3.3)
  * #537: fix compilation of Quaternion with Apples's compiler
  * #1176: allow products between compatible scalar types
  * #1172: make `valuePtr` and `innerIndexPtr` properly return null for empty sparse matrices.
  * #1170: skip calls to `memcpy`/`memmove` for empty inputs.

* Others:
  * #1242: fix comma initializer with empty matrices.
  * Improves support for MKL's PARDISO solver.
  * Fix a compilation issue with Pastix solver.
  * Add some missing explicit scalar conversions
  * Fix a compilation issue with matrix exponential (unsupported MatrixFunctions module).
  * #734: fix a storage order issue in unsupported Spline module
  * #1222: fix a compilation issue in AutoDiffScalar
  * #1221: shutdown some GCC6's warnings.
  * #1175: fix index type conversion warnings in sparse to dense conversion.

## [3.2.8] - 2016-02-16
Changes since 3.2.7:

* Main fixes and improvements:
  * Make `FullPivLU::solve` use `rank()` instead of `nonzeroPivots()`.
  * Add `EIGEN_MAPBASE_PLUGIN`
  * #1166: fix issue in matrix-vector products when the destination is not a vector at compile-time.
  * #1100: Improve cmake/pkg-config support.
  * #1113: fix name conflict with C99's "I".
  * Add missing delete operator overloads in `EIGEN_MAKE_ALIGNED_OPERATOR_NEW`
  * Fix `(A*B).maxCoeff(i)` and similar.
  * Workaround an ICE with VC2015 Update1 x64.
  * #1156: fix several function declarations whose arguments were passed by value instead of being passed by reference
  * #1164: fix `std::list` and `std::deque` specializations such that our aligned allocator is automatically activatived only when the user did not specified an allocator (or specified the default `std::allocator`).

* Others:
  * Fix BLAS backend (aka MKL) for empty matrix products.
  * #1134: fix JacobiSVD pre-allocation.
  * #1111: fix infinite recursion in `sparse_column_major.row(i).nonZeros()` (it now produces a compilation error)
  * #1106: workaround a compilation issue in Sparse module for msvc-icc combo
  * #1153: remove the usage of `__GXX_EXPERIMENTAL_CXX0X__` to detect C++11 support
  * #1143: work-around gcc bug in COLAMD
  * Improve support for matrix products with empty factors.
  * Fix and clarify documentation of Transform wrt `operator*(MatrixBase)`
  * Add a matrix-free conjugate gradient example.
  * Fix cost computation in CwiseUnaryView (internal)
  * Remove custom unaligned loads for SSE.
  * Some warning fixes.
  * Several other documentation clarifications.

## [3.3-beta1] - 2015-12-16

For a comprehensive list of change since the 3.2 series, see this [page](https://www.eigen.tuxfamily.org/index.php?title=3.3).

Main changes since 3.3-alpha1:

* Dense features:
  * Add `LU::transpose().solve()` and `LU::adjoint().solve()` API.
  * Add `Array::rsqrt()` method as a more efficient shorcut for `sqrt().inverse()`.
  * Add `Array::sign()` method for real and complexes.
  * Add `lgamma`, `erf`, and `erfc` functions for arrays.
  * Add support for row/col-wise `lpNorm()`.
  * Add missing `Rotation2D::operator=(Matrix2x2)`.
  * Add support for `permutation * homogenous`.
  * Improve numerical accuracy in LLT and triangular solve by using true scalar divisions (instead of x * (1/y)).
  * Add `EIGEN_MAPBASE_PLUGIN` and `EIGEN_QUATERNION_PLUGIN`.
  *  #1074: forbid the creation of PlainObjectBase objects.

* Sparse features:
  * Add IncompleteCholesky preconditioner.
  * Improve support for [matrix-free iterative solvers](http://eigen.tuxfamily.org/dox/group__MatrixfreeSolverExample.html)
  * Extend `setFromTriplets` API to allow passing a functor object controlling how to collapse duplicated entries.
  * #918: add access to UmfPack return code and parameters.
  * Add support for `dense.cwiseProduct(sparse)`, thus enabling `(dense*sparse).diagonal()` expressions.
  * Add support to directly evaluate the product of two sparse matrices within a dense matrix.
  * #1064: add support for `Ref<SparseVector>`.
  * Add supports for real mul/div `sparse<complex>` operations.
  * #1086: replace deprecated `UF_long` by `SuiteSparse_long`.
  * Make `Ref<SparseMatrix>` more versatile.

* Performance improvements:
  * #1115: enable static alignment and thus small size vectorization on ARM.
  * Add temporary-free evaluation of `D.nolias() *= C + A*B`.
  * Add vectorization of round, ceil and floor for SSE4.1/AVX.
  * Optimize assignment into a `Block<SparseMatrix>` by using Ref and avoiding useless updates in non-compressed mode. This make row-by-row filling of a row-major sparse matrix very efficient.
  * Improve internal cost model leading to faster code in some cases (see changeset 77ff3386b7d2).
  * #1090: improve redux evaluation logic.
  * Enable unaligned vectorization of small fixed size matrix products.

* Misc improvements:
  * Improve support for `isfinite`/`isnan`/`isinf` in fast-math mode.
  * Make the IterativeLinearSolvers module compatible with MPL2-only mode by defaulting to COLAMDOrdering and NaturalOrdering for ILUT and ILLT respectively.
  * Avoid any OpenMP calls if multi-threading is explicitly disabled at runtime.
  * Make abs2 compatible with custom complex types.
  * #1109: use noexcept instead of throw for C++11 compilers.
  * #1100: Improve cmake/pkg-config support.
  * Countless improvements and fixes in Tensors module.

* Most visible fixes:
  * #1105: fix default preallocation when moving from compressed to uncompressed mode in SparseMatrix.
  * Fix UmfPackLU constructor for expressions.
  * Fix degenerate cases in syrk and trsm BLAS API.
  * Fix matrix to quaternion (and angleaxis) conversion for matrix expression.
  * Fix compilation of sparse-triangular to dense assignment.
  * Fix several minor performance issues in the nesting of matrix products.
  * #1092: fix iterative solver ctors for expressions as input.
  * #1099: fix missing include for CUDA.
  * #1102: fix multiple definition linking issue.
  * #1088: fix setIdenity for non-compressed sparse-matrix.
  * Fix `SparseMatrix::insert`/`coeffRef` for non-empty compressed matrix.
  * #1113: fix name conflict with C99's "I".
  * #1075: fix `AlignedBox::sample` for runtime dimension.
  * #1103: fix NEON vectorization of `complex<double>` multiplication.
  * #1134: fix JacobiSVD pre-allocation.
  * Fix ICE with VC2015 Update1.
  * Improve cmake install scripts.

## [3.2.7] - 2015-11-05

Changes since 3.2.6:

* Main fixes and improvements:
  * Add support for `dense.cwiseProduct(sparse)`.
  * Fix a regression regarding `(dense*sparse).diagonal()`.
  * Make the `IterativeLinearSolvers` module compatible with MPL2-only mode by defaulting to `COLAMDOrdering` and `NaturalOrdering` for ILUT and ILLT respectively.
  * #266: backport support for c++11 move semantic
  * `operator/=(Scalar)` now performs a true division (instead of `mat*(1/s)`)
  * Improve numerical accuracy in LLT and triangular solve by using true scalar divisions (instead of `mat * (1/s)`)
  * #1092: fix iterative solver constructors for expressions as input
  * #1088: fix `setIdenity` for non-compressed sparse-matrix
  * #1086: add support for recent SuiteSparse versions

* Others:
  * Add overloads for real-scalar times `SparseMatrix<complex>` operations. This avoids real to complex conversions, and also fixes a compilation issue with MSVC.
  * Use explicit Scalar types for AngleAxis initialization
  * Fix several shortcomings in cost computation (avoid multiple re-evaluation in some very rare cases).
  * #1090: fix a shortcoming in redux logic for which slice-vectorization plus unrolling might happen.
  * Fix compilation issue with MSVC by backporting `DenseStorage::operator=` from devel branch.
  * #1063: fix nesting of unsupported/AutoDiffScalar to prevent dead references when computing second-order derivatives
  * #1100: remove explicit `CMAKE_INSTALL_PREFIX` prefix to conform to cmake install's `DESTINATION` parameter.
  * unsupported/ArpackSupport is now properly installed by make install.
  * #1080: warning fixes

## [3.2.6] - 2015-10-01

Changes since 3.2.5:

* fix some compilation issues with MSVC 2013, including bugs #1000 and #1057
* SparseLU: fixes to support `EIGEN_DEFAULT_TO_ROW_MAJOR` (#1053), and for empty (#1026) and some structurally rank deficient matrices (#792)
* #1075: fix `AlignedBox::sample()` for Dynamic dimension
* fix regression in AMD ordering when a column has only one off-diagonal non-zero (used in sparse Cholesky)
* fix Jacobi preconditioner with zero diagonal entries
* fix Quaternion identity initialization for non-implicitly convertible types
* #1059: fix `predux_max<Packet4i>` for NEON
* #1039: fix some issues when redefining `EIGEN_DEFAULT_DENSE_INDEX_TYPE`
* #1062: fix SelfAdjointEigenSolver for RowMajor matrices
* MKL: fix support for the 11.2 version, and fix a naming conflict (#1067)
  * #1033: explicit type conversion from 0 to RealScalar

## [3.3-alpha1] - 2015-09-04

See the [announcement](https://www.eigen.tuxfamily.org/index.php?title=3.3).

## [3.2.5] - 2015-06-16

Changes since 3.2.4:

* Changes with main impact:
  * Improve robustness of SimplicialLDLT to semidefinite problems by correctly handling structural zeros in AMD reordering
  * Re-enable supernodes in SparseLU (fix a performance regression in SparseLU)
  * Use zero guess in `ConjugateGradients::solve`
  * Add `PermutationMatrix::determinant` method
  * Fix `SparseLU::signDeterminant()` method, and add a SparseLU::determinant() method
  * Allows Lower|Upper as a template argument of CG and MINRES: in this case the full matrix will be considered
  * #872: remove usage of std::bind* functions (deprecated in c++11)

* Numerical robustness improvements:
  * #1014: improve numerical robustness of the 3x3 direct eigenvalue solver
  * #1013: fix 2x2 direct eigenvalue solver for identical eigenvalues
  * #824: improve accuracy of `Quaternion::angularDistance`
  * #941: fix an accuracy issue in ColPivHouseholderQR by continuing the decomposition on a small pivot
  * #933: improve numerical robustness in RealSchur
  * Fix default threshold value in SPQR

* Other changes:
  * Fix usage of `EIGEN_NO_AUTOMATIC_RESIZING`
  * Improved support for custom scalar types in SparseLU
  * Improve cygwin compatibility
  * #650: fix an issue with sparse-dense product and rowmajor matrices
  * #704: fix MKL support (HouseholderQR)
  * #705: fix handling of Lapack potrf return code (LLT)
  * #714: fix matrix product with OpenMP support
  * #949: add static assertions for incompatible scalar types in many of the dense decompositions
  * #957,  #1000: workaround MSVC/ICC compilation issues when using sparse blocks
  * #969: fix ambiguous calls to Ref
  * #972, #986: add support for coefficient-based product with 0 depth
  * #980: fix taking a row (resp. column) of a column-major (resp. row-major) sparse matrix
  * #983: fix an alignement issue in Quaternion
  * #985: fix RealQZ when either matrix had zero rows or columns
  * #987: fix alignement guess in diagonal product
  * #993: fix a pitfall with matrix.inverse()
  * #996, #1016: fix scalar conversions
  * #1003: fix handling of pointers non aligned on scalar boundary in slice-vectorization
  * #1010: fix member initialization in IncompleteLUT
  * #1012: enable alloca on Mac OS or if alloca is defined as macro
  * Doc and build system: #733, #914, #952,  #961, #999

## [3.2.4] - 2015-01-21

Changes since 3.2.3:

* Fix compilation regression in Rotation2D
* #920: fix compilation issue with MSVC 2015.
* #921: fix utilization of bitwise operation on enums in `first_aligned`.
* Fix compilation with NEON on some platforms.

## [3.2.3] - 2014-12-16

Changes since 3.2.2:

* Core:
  * Enable `Mx0 * 0xN` matrix products.
  * #859: fix returned values for vectorized versions of `exp(NaN)`, `log(NaN)`, `sqrt(NaN)` and `sqrt(-1)`.
  * #879: tri1 = mat * tri2 was compiling and running incorrectly if tri2 was not numerically triangular. Workaround the issue by evaluating mat*tri2 into a temporary.
  * #854: fix numerical issue in SelfAdjointEigenSolver::computeDirect for 3x3 matrices.
  * #884: make sure there no call to malloc for zero-sized matrices or for a Ref<> without temporaries.
  * #890: fix aliasing detection when applying a permutation.
  * #898: MSVC optimization by adding inline hint to const_cast_ptr.
  * #853: remove enable_if<> in Ref<> ctor.

* Dense solvers:
  * #894: fix the sign returned by LDLT for multiple calls to `compute()`.
  * Fix JacobiSVD wrt underflow and overflow.
  * #791: fix infinite loop in JacobiSVD in the presence of NaN.

* Sparse:
  * Fix out-of-bounds memory write when the product of two sparse matrices is completely dense and performed using pruning.
  * UmfPack support: fix redundant evaluation/copies when calling `compute()`, add support for generic expressions as input, and fix extraction of the L and U factors (#911).
  * Improve `SparseMatrix::block` for const matrices (the generic path was used).
  * Fix memory pre-allocation when permuting inner vectors of a sparse matrix.
  * Fix `SparseQR::rank` for a completely empty matrix.
  * Fix `SparseQR` for row-major inputs.
  * Fix `SparseLU::absDeterminant` and add respective unit test.
  * BiCGSTAB: make sure that good initial guesses are not destroyed by a bad preconditioner.

* Geometry:
  * Fix `Hyperplane::Through(a,b,c)` when points are aligned or identical.
  * Fix linking issues in OpenGLSupport.

* OS, build system and doc:
  * Various compilation fixes including: #821,  #822, #857, #871, #873.
  * Fix many compilation warnings produced by recent compilers including: #909.
  * #861: enable posix_memalign with PGI.
  * Fix BiCGSTAB doc example.

## [3.2.2] - 2014-08-04

Changes since 3.2.1:

* Core:
  * Relax Ref such that `Ref<MatrixXf>` accepts a `RowVectorXf` which can be seen as a degenerate `MatrixXf(1,N)`
  * Fix performance regression for the vectorization of sub columns/rows of matrices.
  * `EIGEN_STACK_ALLOCATION_LIMIT`: Raise its default value to 128KB, make use of it to assert on  maximal fixed size object, and allows it to be 0 to mean "no limit".
  * #839: Fix 1x1 triangular matrix-vector product.
  * #755: `CommaInitializer` produced wrong assertions in absence of Return-Value-Optimization.

* Dense solvers:
  * Add a `rank()` method with threshold control to JacobiSVD, and make solve uses it to return the minimal norm solution for rank-deficient problems.
  * Various numerical fixes in JacobiSVD, including:#843, and the move from Lapack to Matlab strategy for the default threshold.
  * Various numerical fixes in LDLT, including the case of semi-definite complex matrices.
  * Fix `ColPivHouseholderQR::rank()`.
  * #222: Make temporary matrix column-major independently of `EIGEN_DEFAULT_TO_ROW_MAJOR` in BlockHouseholder.

* Sparse:
  * #838: Fix `dense * sparse` and `sparse * dense` outer products and detect outer products from either the lhs or rhs.
  * Make the ordering method of SimplicialL[D]LT configurable.
  * Fix regression in the restart mechanism of BiCGSTAB.
  * #836: extend SparseQR to support more columns than rows.
  * #808: Use double instead of float for the increasing size ratio in `CompressedStorage::resize`, fix implicit conversions from int/longint to float/double, and fix `set_from_triplets` temporary matrix type.
  * #647: Use `smart_copy` instead of bitwise memcpy in CompressedStorage.
  * GMRES: Initialize essential Householder vector with correct dimension.

* Geometry:
  * #807: Missing scalar type cast in `umeyama()`
  * #806: Missing scalar type cast in `Quaternion::setFromTwoVectors()`
  * #759: Removed hard-coded double-math from `Quaternion::angularDistance`.

* OS, build system and doc:
  * Fix compilation with Windows CE.
  * Fix some ICEs with VC11.
  * Check IMKL version for compatibility with Eigen
  * #754: Only inserted (`!defined(_WIN32_WCE)`) analog to alloc and free implementation.
  * #803: Avoid `char*` to `int*` conversion.
  * #819: Include path of details.h file.
  * #738: Use the "current" version of cmake project directories to ease the inclusion of Eigen within other projects.
  * #815: Fix doc of FullPivLU wrt permutation matrices.
  * #632: doc: Note that `dm2 = sm1 + dm1` is not possible
  * Extend AsciiQuickReference (real, imag, conjugate, rot90)

## [3.2.1] - 2014-02-26

Changes since 3.2.0:

* Eigen2 support is now deprecated and will be removed in version 3.3.
* Core:
  * Bug fix for Ref object containing a temporary matrix.
  * #654: Allow construction of row vector from 1D array.
  * #679: Support `cwiseMin()` and `cwiseMax()` on maps.
  * Support `conservativeResize()` on vectors.
  * Improve performance of vectorwise and replicate expressions.
  * #642: Add vectorization of sqrt for doubles, and make sqrt really safe if `EIGEN_FAST_MATH` is disabled.
  * #616: Try harder to align columns when printing matrices and arrays.
  * #579: Add optional run-time parameter to fixed-size block methods.
  * Implement `.all()` and `.any()` for zero-sized objects
  * #708: Add placement new and delete for arrays.
  * #503: Better C++11 support.
* Dense linear algebra:
  * #689: Speed up some matrix-vector products by using aligned loads if possible.
  * Make solve in `FullPivHouseholderQR` return least-square solution if there is no exact solution.
  * #678: Fix `fullPivHouseholderQR` for rectangular matrices.
  * Fix a 0/0 issue in JacobiSVD.
  * #736: Wrong result in `LDLT::isPositiveDefinite()` for semi-definite matrices.
  * #740: Fix overflow issue in `stableNorm()`.
  * Make pivoting HouseholderQR compatible with custom scalar types.
* Geometry:
  * Fix compilation of Transform * UniformScaling
* Sparse matrices:
  * Fix elimination tree and SparseQR for fat rectangular matrices.
  * #635: add `isCompressed` to `MappedSparseMatrix` for compatibility.
  * #664: Support iterators without `operator<` in `setFromTriplets()`.
  * Fixes in SparseLU: infinite loop, aliasing issue when solving, overflow in memory allocation, use exceptions only if enabled (#672).
  * Fixes in SparseQR: reduce explicit zero, assigning result to map, assert catching non-conforming sizes, memory leak.
  * #681: Uninitialized value in CholmodSupport which may lead to incorrect results.
  * Fix some issues when using a non-standard index type (#665 and more)
  * Update constrained CG (unsupported module) to Eigen3.
* OS and build system:
  * MacOS put OpenGL header files somewhere else from where we expected it.
  * Do not assume that `alloca()` is 16-byte aligned on Windows.
  * Compilation fixes when using ICC with Visual Studio.
  * Fix Fortran compiler detection in CMake files.
* Fix some of our tests (bugs #744 and #748 and more).
* Fix a few compiler warnings (bug #317 and more).
* Documentation fixes (bugs #609, #638 and #739 and more).

## [3.1.4] - 2013-08-02

Changes since 3.1.3:

* #620: Fix robustness and performance issues in JacobiSVD::solve.
* #613: Fix accuracy of SSE sqrt for very small numbers.
* #608: Fix sign computation in LDLT.
* Fix write access to CwiseUnaryView expressions.
* Fix compilation of `transposeInPlace()` for Array expressions.
* Fix non const `data()` member in Array and Matrix wrappers.
* Fix a few warnings and compilation issues with recent compiler versions.
* Documentation fixes.

## [3.0.7] - 2013-08-02

Changes since 3.0.6:

* Fix traits of `Map<Quaternion>`.
* Fix a few warnings (#507) and documentation (#531).

## [3.2.0] - 2013-07-24

Major new features and optimizations since 3.1:

* Dense world
  * New [`Ref<>`](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1Ref.html) class allowing to write non templated function taking various kind of Eigen dense objects without copies.
  * New [RealQZ](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1RealQZ.html) factorization and [GeneralizedEigenSolver](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1GeneralizedEigenSolver.html).
  * Add vector-wise normalized and normalize functions, and hasNaN/allFinite members.
  * Add mixed static/dynamic-size `.block<.,.>()` functions.
  * Optimize outer products for non rank-1 update operations.
  * Optimize diagonal products (enable vectorization in more cases).
  * Improve robustness and performance in `JacobiSVD::solve()`.
* Sparse world
  * New [SparseLU](http://eigen.tuxfamily.org/dox-devel/group__SparseLU__Module.html) module: built-in sparse LU with supernodes and numerical row pivoting (port of SuperLU making the SuperLUSupport module obsolete).
  * New [SparseQR](http://eigen.tuxfamily.org/dox-devel/group__SparseQR__Module.html) module: rank-revealing sparse QR factorization with numerical column pivoting.
  * New [COLAMD](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1COLAMDOrdering.html) ordering and unified [ordering API](http://eigen.tuxfamily.org/dox-devel/group__OrderingMethods__Module.html).
  * Add support for generic blocks of sparse matrices (read-only).
  * Add conservative resize feature on sparse matrices.
  * Add uniform support for solving sparse systems with sparse right hand sides.
  * Add support for sparse matrix time sparse self-adjoint view products.
  * Improve BiCGSTAB robustness with restart.
* Support to external libraries
  * New [MetisSupport](http://eigen.tuxfamily.org/dox-devel/group__MetisSupport__Module.html) module: wrapper to the famous graph partitioning library.
  * New [SPQRSupport](http://eigen.tuxfamily.org/dox-devel/group__SPQRSupport__Module.html) module: wrapper to suitesparse's supernodal QR solver.

Eigen 3.2 represents about 600 commits since Eigen 3.1.

## [3.2-rc2] - 2013-07-19

Changes since 3.2-rc1:

* Rename `DenseBase::isFinite()` to `allFinite()` to avoid a future naming collision.
* Fix an ICE with ICC 11.1.

## [3.2-rc1] - 2013-07-17

Main changes since 3.2-beta1:
* New features:
  * #562: Add vector-wise normalized and normalize functions.
  * #564: Add `hasNaN` and `isFinite` members.
  * #579: Add support for mixed static/dynamic-size `.block()`.
  * #588: Add support for determinant in SparseLU.
  * Add support in SparseLU to solve with L and U factors independently.
  * Allow multiplication-like binary operators to be applied on type combinations supported by `scalar_product_traits`.
  * #596: Add conversion from `SparseQR::matrixQ()` to a `SparseMatrix`.
  * #553: Add support for sparse matrix time sparse self-adjoint view products.

* Accuracy and performance:
  * Improve BiCGSTAB robustness: fix a divide by zero and allow to restart with a new initial residual reference.
  * #71: Enable vectorization of diagonal products in more cases.
  * #620: Fix robustness and performance issues in JacobiSVD::solve.
  * #609: Improve accuracy and consistency of the eulerAngles functions.
  * #613: Fix accuracy of SSE sqrt for very small numbers.
  * Enable SSE with ICC even when it mimics a gcc version lower than 4.2.
  * Add SSE4 min/max for integers.
  * #590 & #591: Minor improvements in NEON vectorization.

* Bug fixes:
  * Fix `HouseholderSequence::conjugate()` and `::adjoint()`.
  * Fix SparseLU for dense matrices and matrices in non compressed mode.
  * Fix `SparseMatrix::conservativeResize()` when one dimension is null.
  * Fix `transposeInpPlace` for arrays.
  * Fix `handmade_aligned_realloc`.
  * #554: Fix detection of the presence of `posix_memalign` with mingw.
  * #556: Workaround mingw bug with `-O3` or `-fipa-cp-clone` options.
  * #608: Fix sign computation in LDLT.
  * #567: Fix iterative solvers to immediately return when the initial guess is the true solution and for trivial solution.
  * #607: Fix support for implicit transposition from dense to sparse vectors.
  * #611: Fix support for products of the form `diagonal_matrix * sparse_matrix * diagonal_matrix`.

* Others:
  * #583: Add compile-time assertion to check DenseIndex is signed.
  * #63: Add lapack unit tests. They are automatically downloaded and configured if `EIGEN_ENABLE_LAPACK_TESTS` is ON.
  * #563: Assignment to `Block<SparseMatrix>` is now allowed on non-compressed matrices.
  * #626: Add assertion on input ranges for coeff* and insert members for sparse objects.
  * #314: Move special math functions from internal to numext namespace.
  * Fix many warnings and compilation issues with recent compiler versions.
  * Many other fixes including #230, #482, #542, #561, #564, #565, #566, #578, #581, #595, #597, #598, #599, #605, #606, #615.

## [3.1.3] - 2013-04-16
Changes since 3.1.2:

* #526 - Fix linear vectorized transversal in linspace.
* #551 - Fix compilation issue when using `EIGEN_DEFAULT_DENSE_INDEX_TYPE`.
* #533 - Fix some missing const qualifiers in Transpose
* Fix a compilation with CGAL::Gmpq by adding explicit internal:: namespace when calling `abs()`.
* Fix computation of outer-stride when calling `.real()` or `.imag()`.
* Fix `handmade_aligned_realloc` (affected `conservativeResize()`).
* Fix sparse vector assignment from a sparse matrix.
* Fix `log(0)` with SSE.
* Fix bug in aligned_free with windows CE.
* Fix traits of `Map<Quaternion>`.
* Fix a few warnings (#507, #535, #581).
* Enable SSE with ICC even when it mimics a gcc version lower than 4.2
* Workaround [gcc-4.7 bug #53900](http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53900) (too aggressive optimization in our alignment check)

## [3.2-beta1] - 2013-03-07

Main changes since 3.1:

* Dense modules
  * A new [`Ref<>`](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1Ref.html) class allowing to write non templated function taking various kind of Eigen dense objects without copies.
  * New [RealQZ](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1RealQZ.html) factorization and [GeneralizedEigenSolver](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1GeneralizedEigenSolver.html)
  * Optimized outer products for non rank-1 update operations.

* Sparse modules
  * New [SparseLU](http://eigen.tuxfamily.org/dox-devel/group__SparseLU__Module.html) module: built-in sparse LU with supernodes and numerical row pivoting (port of SuperLU making the SuperLUSupport module obsolete).
  * New [SparseQR](http://eigen.tuxfamily.org/dox-devel/group__SparseQR__Module.html) module: rank-revealing sparse QR factorization with numerical column pivoting.
  * OrderingMethods: extended with [COLAMD](http://eigen.tuxfamily.org/dox-devel/classEigen_1_1COLAMDOrdering.html) ordering and a unified [ordering](http://eigen.tuxfamily.org/dox-devel/group__OrderingMethods__Module.html) API.
  * Support for generic blocks of sparse matrices.
  * Add conservative resize feature on sparse matrices.
  * Add uniform support for solving sparse systems with sparse right hand sides.

* Support to external libraries
  * New [MetisSupport](http://eigen.tuxfamily.org/dox-devel/group__MetisSupport__Module.html) module: wrapper to the famous graph partitioning library.
  * New [SPQRSupport](http://eigen.tuxfamily.org/dox-devel/group__SPQRSupport__Module.html) module: wrapper to suitesparse's supernodal QR solver.

* Misc
  * Improved presentation and clarity of Doxygen generated documentation (modules are now organized into chapters, treeview panel and search engine for quick navagitation).
  * New compilation token `EIGEN_INITIALIZE_MATRICES_BY_NAN` to help debugging.
  * All bug fixes of the 3.1 branch, plus a couple of other fixes (including 211, 479, 496, 508, 552)

## [3.1.2] - 2012-11-05

Changes since 3.1.1:

* #524 - Pardiso's parameter array does not have to be aligned!
* #521 - Disable `__cpuidex` on architectures different that x86 or x86-64 with MSVC.
* #519 - `AlignedBox::dim()` was wrong for dynamic dimensions.
* #515 - Fix missing explicit scalar conversion.
* #511 - Fix pretty printers on windows.
* #509 - Fix warnings with gcc 4.7
* #501 - Remove aggressive mat/scalar optimization (was replaced by `mat*(1/scalar)` for non integer types).
* #479 - Use EISPACK's strategy re max number of iters in Schur decomposition.
* Add support for scalar multiple of diagonal matrices.
* Forward `resize()` function from Array/Matrix wrappers to the nested expression such that `mat.array().resize(a,b)` is now allowed.
* Windows CE: fix the lack of the `aligned_malloc` function on this platform.
* Fix comma initializer when inserting empty matrices.
* Fix `dense=sparse*diagonal` products.
* Fix compilation with `m.array().min(scalar)` and `m.array().max(scalar)`.
* Fix out-of-range memory access in GEMV (the memory was not used for the computation, only to assemble unaligned packets from aligned packet loads).
* Fix various regressions with MKL support.
* Fix aliasing issue in sparse matrix assignment.
* Remove stupid assert in blue norm.
* Workaround a weird compilation error with MSVC.

## [3.1.1] - 2012-07-22

Changes since 3.1.0:
* [relicense to MPL2](https://www.eigen.tuxfamily.org/index.php?title=Main_Page#License)
* add a `EIGEN_MPL2_ONLY` build option to generate compiler errors when including non-MPL2 modules
* remove dynamic allocation for triangular matrix-matrix products of fixed size objects
* Fix possible underflow issues in SelfAdjointEigenSolver
* Fix issues with fixed-size Diagonal (sub/super diagonal size computation was wrong)
* #487 - Geometry module: `isometry * scaling` compilation error
* #486 - MKL support: fixed multiple-references linker errors with various decompositions
* #480 - work around compilation error on Android NDK due to isfinite being defined as a macro
* #485 - IterativeLinearSolvers: conflict between a typedef and template type parameter
* #479 - Eigenvalues/Schur: Adjust max iterations count to matrix size
* Fixed Geometry module compilation under MSVC
* Fixed Sparse module compilation under MSVC 2005

## [3.0.6] - 2012-07-09
Changes since 3.0.5:
* #447 - fix infinite recursion in `ProductBase::coeff()`
* #478 - fix RealSchur on a zero matrix
* #477 - fix warnings with gcc 4.7
* #475 - `.exp()` now returns `+inf` when overflow occurs (SSE)
* #466 - fix a possible race condition in OpenMP environment (for non OpenMP thread model it is recommended to upgrade to 3.1)
* #362 - fix missing specialization for affine-compact `*` projective
* #451 - fix a clang warning
* Fix compilation of `somedensematrix.llt().matrixL().transpose()`
* Fix miss-use of the cost-model in Replicate
* Fix use of int versus Index types for `Block::m_outerStride`
* Fix ambiguous calls to some std functions
* Fix geometry tutorial on scalings
* Fix RVCT 3.1 compiler errors
* Fix implicit scalar conversion in Transform
* Fix typo in NumericalDiff (unsupported module)
* Fix LevenbergMarquart for non double scalar type (unsupported module)

## [3.1.0] - 2012-06-24

Major changes between Eigen 3.0 and Eigen 3.1:
* New features
  * **New set of officially supported Sparse Modules**
  ** This includes sparse matrix storage, assembly, and many built-in (Cholesky, CG, BiCGSTAB, ILU), and third-party (PaStiX, Cholmod, UmfPack, SuperLU, Pardiso) solvers
  ** See this [page](http://eigen.tuxfamily.org/dox-devel/TutorialSparse.html) for an overview of the features
  * **Optional support for Intel MKL**
  ** This includes the BLAS, LAPACK, VML, and Pardiso components
  ** See this [page](http://eigen.tuxfamily.org/dox-devel/TopicUsingIntelMKL.html) for the details
  * Core
  ** New vector-wise operators: `*`, `/`, `*=`, `/=`
  ** New coefficient-wise operators: `&&`, `||`,  `min(Scalar)`, `max(Scalar)`, `pow`, `operator/(Scalar,ArrayBase)`
  * Decompositions
  ** Add incremental rank-updates in LLTand LDLT
  ** New `SelfAdjointEigenSolver::computeDirect()` function for fast eigen-decomposition through closed-form formulas (only for 2x2 and 3x3 real matrices)
* Optimizations
  * Memory optimizations in JacobiSVD and triangular solves.
  * Optimization of reductions via partial unrolling (e.g., dot, sum, norm, etc.)
  * Improved performance of small matrix-matrix products and some Transform<> operations

Eigen 3.1 represents about 600 commits since Eigen 3.0.

## [3.1.0-rc2] - 2012-06-21

Changes since 3.1.0-rc1:
* Fix a couple of compilation warnings
* Improved documentation, in particular regarding the Geometry and Sparse tutorials, and sparse solver modules
* Fix double preconditioner allocation in `JacobiSVD`
* #466: `RealSchur` failed on a zero matrix
* Update Adolc and MPReal support modules

## [3.1.0-rc1] - 2012-06-14

Main changes since 3.1.0-beta1:
* #466: fix a possible race condition issue. from now, multithreaded applications that call Eigen from multiple thread must initialize Eigen by calling `initParallel()`.
* For consistency, `SimplicialLLT` and `SimplicialLDLT` now factorizes `P A P^-1` (instead of `P^-1 A P`).
* #475: now the vectorized `exp` operator returns +inf when overflow occurs
* Fix the use of MKL with MSVC by disabling MKL's pow functions.
* Avoid dynamic allocation for fixed size triangular solving
* Fix a compilation issue with ICC 11.1
* Fix ambiguous calls in the math functors
* Fix BTL interface.

## [3.1.0-beta1] - 2012-06-07

Main changes since 3.1.0-alpha2:
* **API changes**
  * `SimplicialLLt` and `SimplicialLDLt` are now renamed `SimplicialLLT` and `SimplicialLDLT` for consistency with the other modules.
  * The Pardiso support module is now spelled "PardisoSupport"
* Dense modules:
  * Add `operator/(Scalar,ArrayBase)` and coefficient-wise pow operator.
  * Fix automatic evaluation of expressions nested by Replicate (performance improvement)
  * #447 - fix infinite recursion in `ProductBase::coeff()`
  * #455 - add support for c++11 in `aligned_allocator`
  * `LinSpace`: add a missing variant, and fix the size=1 case
* Sparse modules:
  * Add an **IncompleteLU** preconditioner with dual thresholding.
  * Add an interface to the parallel **Pastix** solver
  * Improve applicability of permutations (add `SparseMatrixBase::twistedBy`, handle non symmetric permutations)
  * `CholmodDecomposition` now has explicit variants: `CholmodSupernodalLLT`, `CholmodSimplicialLLT`, `CholmodSimplicialLDLT`
  * Add analysePattern/factorize methods to iterative solvers
  * Preserve explicit zero in a sparse assignment
  * Speedup `sparse * dense` products
  * Fix a couple of issues with Pardiso support
* Geometry module:
  * Improve performance of some `Transform<>` operations by better preserving the alignment status.
  * #415 - wrong return type in `Rotation2D::operator*=`
  * #439 - add `Quaternion::FromTwoVectors()` static constructor
  * #362 - missing specialization for affine-compact `*` projective
* Others:
  * add support for RVCT 3.1 compiler
  * New tutorial page on Map
  * and many other bug fixes such as: #417, #419, #450

## [3.0.5] - 2012-02-10

Changes since 3.0.4:
* #417 - fix nesting of `Map` expressions
* #415 - fix return value of `Rotation2D::operator*=`
* #410 - fix a possible out of range access in `EigenSolver`
* #406 - fix infinite loop/deadlock when using OpenMP and Eigen
* Changeset 4462 - fix broken asserts revealed by Clang
* Changeset 4457 - fix description of `rankUpdate()` in quick reference guide
* Changeset 4455 - fix out-of-range int constant in 4x4 inverse
* #398 - fix in slerp: the returned quaternion was not always normalized
* Changeset 4432 - fix asserts in eigenvalue decompositions
* Changeset 4416 - fix MSVC integer overflow warning

## [3.1.0-alpha2] - 2012-02-06

Main changes since 3.0.1-alpha1:
* New optional support for Intel MKL and other BLAS including: ([details](http://eigen.tuxfamily.org/dox-devel/TopicUsingIntelMKL.html))
  * BLAS (arbitrary BLAS)
  * Intel LAPACKE
  * Intel VML (coefficient-wise math operations)
  * Intel PARDISO (sparse direct solver)
* Dense modules:
  * improved performance of small matrix-matrix products
  * Feature #319 - add a rankUpdate function to LDLt and LLT for updates/downdates
  * Feature #400 - new coefficient wise min/max functions taking one scalar argument
* Sparse modules:
  * new fast sparse matrix assembly interface from a random list of triplets (see `SparseMatrix::setFromTriplets()`)
  * new shifting feature in SimplicialCholesky (see `SimplicialCholeskyBase::setShift()`)
  * add checks for positive definiteness in SimplicialCholesky
  * improved heuristic to predict the nnz of a `sparse*sparse` product
  * add support for uncompressed SparseMatrix in CholmodSupport
* Geometry module:
  * Feature #297 - add `ParametrizedLine::intersectionPoint()` and `intersectionParam()` functions
* Others:
  * fix many warnings and compilation issues with ICC 12 and -strict-ansi
  * fix some ICE with MSVC10
  * add the possibility to disable calls to cpuid (`-DEIGEN_NO_CPUID`) and other asm directives
  * and many other bug fixes such as: #406, #410, #398, #396, #394, #354, #352, #301,


## [3.1.0-alpha1] - 2011-12-06

Main changes since 3.0:
* Officially supported set of sparse modules. See this [page](http://eigen.tuxfamily.org/dox-devel/TutorialSparse.html) for an overview of the features. Main changes:
  * new `SparseCore` module equivalent to the old `Sparse` module, the `Sparse` module is now a super module including all sparse-related modules
    * the `SparseMatrix` class is now more versatile and supports an uncompressed mode for fast element insertion
    * the `SparseMatrix` class now offer a unique and simplified API to insert elements
    * `DynamicSparseMatrix` has been deprecated (moved into `unsupported/SparseExtra`)
    * new conservative `sparse * sparse` matrix product which is also used by default
  * new `SparseCholesky` module featuring the SimplicialLLT and SimplicialLDLT built-in solvers
  * new `IterativeLinearSolvers` module featuring a conjugate gradient and stabilized bi-conjugate gradient iterative solvers with a basic Jacobi preconditioner
* New `SelfAdjointEigenSolver::computeDirect()` function for fast eigen-decomposition through closed-form formulas (only for 2x2 and 3x3 real matrices)
* New `LLT::rankUpdate()` function supporting both updates and down-dates
* Optimization of reduction via partial unrolling (e.g., dot, sum, norm, etc.)
* New coefficient-wise operators: `&&` and `||`
* Feature #157 - New vector-wise operations for arrays: `*`, `/`, `*=`, and `/=`.
* Feature #206 - Pre-allocation of intermediate buffers in JacobiSVD
* Feature #370 - New typedefs for AlignedBox
* All the fixes and improvements of the 3.0 branch up to the 3.0.4 release (see below)


## [3.0.4] - 2011-12-06
Changes since 3.0.3:

* #363 - check for integer overflow in size computations
* #369 - Quaternion alignment is broken (and more alignment fixes)
* #354 - Converge better in SelfAdjointEigenSolver, and allow better handling of non-convergent cases
* #347 - Fix compilation on ARM NEON with LLVM 3.0 and iOS SDK 5.0
* #372 - Put unsupported modules documentation at the right place
* #383 - Fix C++11 compilation problem due to some constructs mis-interpreted as c++11 user-defined literals
* #373 - Compilation error with clang 2.9 when exceptions are disabled
* Fix compilation issue with `QuaternionBase::cast`


## [2.0.17] - 2011-12-06

Changes since 2.0.16:

* Fix a compilation bug in `aligned_allocator`: the allocate method should take a void pointer
* Fix a typo in ParametrizedLine documentation


## [3.0.3] - 2011-10-06

Changes since 3.0.2:

* Fix compilation errors when Eigen2 support is enabled.
* Fix bug in evaluating expressions of the form `matrix1 * matrix2 * scalar1 * scalar2`.
* Fix solve using LDLT for singular matrices if solution exists.
* Fix infinite loop when computing SVD of some matrices with very small numbers.
* Allow user to specify pkgconfig destination.
* Several improvements to the documentation.


## [3.0.2] - 2011-08-26

Changes since 3.0.1:

* `Windows.h`: protect min/max calls from macros having the same name (no need to `#undef` min/max anymore).
* MinGW: fix compilation issues and pretty gdb printer.
* Standard compliance: fix aligned_allocator and remove uses of long long.
* MPReal: updates for the new version.
* Other fixes:
  * fix aligned_stack_memory_handler for null pointers.
  * fix std::vector support with gcc 4.6.
  * fix linking issue with OpenGL support.
  * fix SelfAdjointEigenSolver for 1x1 matrices.
  * fix a couple of warnings with new compilers.
  * fix a few documentation issues.


## [3.0.1] - 2011-05-30

Changes since 3.0.0:

* Fix many bugs regarding ARM and NEON (Now all tests succeed on ARM/NEON).
* Fix compilation on gcc 4.6
* Improved support for custom scalar types:
  * Fix memory leak issue for scalar types throwing exceptions.
  * Fix implicit scalar type conversion.
  * Math functions can be defined in the scalar type's namespace.
* Fix bug in trapezoidal matrix time matrix product.
* Fix asin.
* Fix compilation with MSVC 2005 (SSE was wrongly enabled).
* Fix bug in `EigenSolver`: normalize the eigen vectors.
* Fix Qt support in Transform.
* Improved documentation.

## [2.0.16] - 2011-05-28

Changes since 2.0.15:

* Fix bug in 3x3 tridiagonlisation (and consequently in 3x3 selfadjoint eigen decomposition).
* Fix compilation for new gcc 4.6.
* Fix performance regression since 2.0.12: in some matrix-vector product, complex matrix expressions were not pre-evaluated.
* Fix documentation of Least-Square.
* New feature: support for `part<SelfAdjoint>`.
* Fix bug in SparseLU::setOrderingMethod.

## [3.0.0] - 2011-03-19

Released at the [meeting](https://www.eigen.tuxfamily.org/index.php?title=Paris_2011_Meeting).

See the [Eigen 3.0 release notes](https://www.eigen.tuxfamily.org/index.php?title=3.0).

Only change since 3.0-rc1:
* Fixed compilation of the unsupported 'openglsupport' test.

## [3.0-rc1] - 2011-03-14

Main changes since 3.0-beta4:

* Core: added new `EIGEN_RUNTIME_NO_MALLOC` option and new `set_is_malloc_allowed()` option to finely control where dynamic memory allocation is allowed. Useful for unit-testing of functions that must not cause dynamic memory allocations.
* Core: SSE performance fixes (follow-up from #203).
* Core: Fixed crashes when using `EIGEN_DONT_ALIGN` or `EIGEN_DONT_ALIGN_STATICALLY` (#213 and friends).
* Core: `EIGEN_DONT_ALIGN` and `EIGEN_DONT_ALIGN_STATICALLY` are now covered by unit tests.
* Geometry: Fixed transform * matrix products (#207).
* Geometry: compilation fix for mixing CompactAffine with Homogeneous objects
* Geometry: compilation fix for 1D transform
* SVD: fix non-computing constructors (correctly forward `computationOptions`) (#206)
* Sparse: fix resizing when the destination sparse matrix is row major (#37)
* more Eigen2Support improvements
* more unit test fixes/improvements
* more documentation improvements
* more compiler warnings fixes
* fixed GDB pretty-printer for dynamic-size matrices (#210)

## [3.0-beta4] - 2011-02-28

Main changes since 3.0-beta3:

* Non-vectorization bug fixes:
  * fix #89: work around an extremely evil compiler bug on old GCC (<= 4.3) with the standard `assert()` macro
  * fix Umfpack back-end in the complex case
* Vectorization bug fixes:
  * fix a segfault in "slice vectorization" when the destination might not be aligned on a scalar (`complex<double>`)
  * fix #195: fast SSE unaligned loads fail on GCC/i386 and on Clang
  * fix #186: worked around a GCC 4.3 i386 backend issue with SSE
  * fix #203: SSE: a workaround used in pset1() resulted in poor assembly
  * worked around a GCC 4.2.4 internal compiler error with vectorization of complex numbers
  * lots of AltiVec compilation fixes
  * NEON compilation fixes
* API additions and error messages improvements
  * Transform: prevent bad user code from compiling
  * fix #190: directly pass Transform Options to Matrix, allowing to use RowMajor. Fix issues in Transform with non-default Options.
  * factorize implementation of standard real unary math functions, and add acos, asin
* Build/tests system
  * Lots of unit test improvements
  * fix installation of unsupported modules
  * fixed many compiler warnings, especially on the Intel compiler and on LLVM/Clang
  * CTest/CMake improvements
  * added option to build in 32bit mode
* BLAS/LAPACK implementation improvements
  * The Blas library and tests are now automatically built as part of the tests.
  * expanded LAPACK interface (including syev)
  * now Sparse solver backends use our own BLAS/LAPACK implementation
  * fix #189 (cblat1 test failure)
* Documentation
  * improved conservativeResize methods documentation
  * documented sorting of eigenvalues
  * misc documentation improvements
  * improve documentation of plugins

## [3.0-beta3] - 2011-02-12

The biggest news is that the API is now **100% stable**.

Main changes since 3.0-beta2:

* The "too many to list them all" category:
  * lots of bug fixes
  * lots of performance fixes
  * lots of compiler support fixes
  * lots of warning fixes
  * lots of unit tests improvements and fixes
  * lots of documentation improvements
  * lots of build system fixes
* API changes:
  * replaced `ei_` prefix by `internal::` namespace. For example, `ei_cos(x)` becomes `internal::cos(x)`.
  * renamed `PlanarRotation` -> `JacobiRotation`
  * renamed `DenseStorageBase` -> `PlainObjectBase`
  * HouseholderSequence API cleanup
  * refactored internal metaprogramming helpers to follow closely the standard library
  * made UpperBidiagonalization internal
  * made BandMatrix/TridiagonalMatrix internal
  * Core: also see below, "const correctness".
  * Sparse: `EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET` must be defined to use Eigen/Sparse
  * Core: `random<interger type>()` now spans over range of width `RAND_MAX`
* New API:
  * Core: added Map static methods taking strides
  * SVD: added `jacobiSvd()` method
  * Sparse: many misc improvements and new features. Improved support for Cholmod, Amd, SuperLU and other back-ends.
  * Core: allow mixed real-complex dot products
  * Geometry: allow mixed real-complex cross products
  * Geometry: allow to pass Options parameters to Transform, Quaternion and other templates, to control memory alignment
  * QR: add threshold API to FullPivHouseholderQR
  * Core: added tan function
* Const correctness:
  * Eigen now properly enforces const-correctness everywhere, for example with Map objects. This will break compilation of code that relied on former behavior.
  * A new kind of test suite was added to check that, 'failtest'.
* BLAS/LAPACK:
  * Complete BLAS library built on top of Eigen. Imported BLAS test suite, which allowed to fix many issues.
  * Partial LAPACK implementation. Passing part of the LAPACK test suite, which also allowed to fix some issues.
* Eigen 2 Support:
  * tons of improvements in `EIGEN2_SUPPORT`
  * new incremental migration path: see http://eigen.tuxfamily.org/dox-devel/Eigen2SupportModes.html
  * imported a copy of the Eigen 2 test suite, made sure that Eigen 3 passes it. That also allowed to fix several issues.


## [3.0-beta2] - 2010-10-15

Main changes since 3.0-beta1:

* Add support for the vectorization of `std::complex<>` with SSE, AltiVec and NEON.
* Add support for mixed `real * complex` matrix products with vectorization.
* Finalize the JacobiSVD class with: compile time options, thin/full decompositions, and least-square solving.
* Several improvement of the Transform class. In particular, there is no default mode anymore.
* New methods: `middleRows()`, `middleCols()`, `TriangularMatrix::conjugate()`
* New unsupported modules: OpenGL, MPFR C++
* Many improvements in the support of empty objects.
* Many improvements of the vectorization logic.
* Add the possibility to extend QuaternionBase.
* Vectorize Quaternion multiplication with double.
* Significant improvements of the documentation.
* Improved compile time errors.
* Enforce static allocation of temporary buffers in gemm (when possible).
* Fix aligned_delete for null pointers and non trivial dtors.
* Fix eigen decomposition of 3x3 float matrices.
* Fix 4x4 matrix inversions (vectorization).
* Many fixes in QR: solving with `m>n`, use of rank, etc.
* Fixes for MSVC for windows mobile and CLang.
* Remove the Taucs backend (obsolete).
* Remove the old SVD class (was causing too much troubles, a new decompozition based on bidiagonalisation/householder should come back soon, `JacobiSVD` can be used meanwhile).

## [2.0.15] - 2010-07-16

Changes since 2.0.14:

* Fix bug: certain cases of matrix-vector product (depending on storage order) were blocked by an assertion failure.
* Fix LU and QR solve when rank==0, fix LLT when the matrix is purely 0.
* Fix a couple of bugs with QR solving especially with rows>cols.
* Fix bug with custom scalar types that have non-trivial destructor.
* Fix for ICC in SSE code.
* Fix some C++ issues found by Clang (patch by Nick Lewycky).

## [3.0-beta1] - 2010-07-05

See the [announcement](https://www.eigen.tuxfamily.org/index.php?title=3.0).

## [2.0.14] - 2010-06-22

Changes since 2.0.13:

* Fix #141: crash in SSE (alignment problem) when using dynamic-size matrices with a max-size fixed at compile time that is not a multiple of 16 bytes. For example, `Matrix<double,Dynamic,Dynamic,AutoAlign,5,5>`.
* Fix #142: LU of fixed-size matrices was causing dynamic memory allocation (patch by Stuart Glaser).
* Fix #127: remove useless static keywords (also fixes warnings with clang++).

## [2.0.13] - 2010-06-10

Changes since 2.0.12:

* Fix #132: crash in certain matrix-vector products. Unit test added.
* Fix #125: colwise `norm()` and `squaredNorm()` on complex types do not return real types
* Fully support the QCC/QNX compiler (thanks to Piotr Trojanek). The support in 2.0.12 was incomplete. The whole test suite is now successful.
* As part of the QCC support work, a lot of standards compliance work: put `std::` in front of a lot of things such as `size_t`, check whether the math library needs to be linked to explicitly.
* Fix precision issues in LDLT. The `isPositiveDefinite()` method is now always returning true, but it was conceptually broken anyway, since a non-pivoting LDLT decomposition can't know that.
* Compilation fix in `ldlt()` on expressions.
* Actually install the Eigen/Eigen and Eigen/Dense public headers!
* Fix readcost for complex types.
* Fix compilation of the BTL benchmarks.
* Some dox updates.

## [2.0.12] - 2010-02-12

Changes since 2.0.11:

* `EIGEN_DEFAULT_TO_ROW_MAJOR` is fully supported and tested.
* Several important fixes for row-major matrices.
* Fix support of several algorithms for mixed fixed-dynamic size matrices where the fixed dimension is greater than the dynamic dimension. For example: `Matrix<float,3,Dynamic>(3,2)`
* fix `EIGEN_DONT_ALIGN`: now it _really_ disables vectorization (was giving a `#error` unless you also used `EIGEN_DONT_VECTORIZE`).
* Fix #92: Support QNX's QCC compiler (patch by Piotr Trojanek)
* Fix #90, missing type cast in LU, allow to use LU with MPFR (patch by 'Wolf').
* Fix ICC compiler support: work around a bug present at least in ICC 11.1.
* Compilation fixes for `computeInverse()` on expressions.
* Fix a gap in a unit-test (thanks to Jitse Niesen)
* Backport improvements to benchmarking code.
* Documentation fixes

## [2.0.11] - 2010-01-10

Changes since 2.0.10:

* Complete rewrite of the 4x4 matrix inversion: we now use the usual cofactors approach, so no numerical stability problems anymore (bug #70)
* Still 4x4 matrix inverse: SSE path for the float case, borrowing code by Intel, giving very high performance.
* Fix crash happening on 32-bit x86 Linux with SSE, when double's were created at non-8-byte-aligned locations (bug #79).
* Fix bug in Part making it crash in certain products (bug #80).
* Precision improvements in Quaternion SLERP (bug #71).
* Fix sparse triangular solver for lower/row-major matrices (bug #74).
* Fix MSVC 2010 compatibility.
* Some documentation improvements.

## [2.0.10] - 2009-11-25

Changes since 2.0.9:

* Rewrite 4x4 matrix inverse to improve precision, and add a new unit test to guarantee that precision. It's less fast, but it's still faster than the cofactors method.
* Fix bug #62: crash in SSE code with MSVC 2008 (Thanks to Hauke Heibel).
* Fix bug #65: `MatrixBase::nonZeros()` was recursing infinitely
* Fix PowerPC platform detection on Mac OSX.
* Prevent the construction of bogus MatrixBase objects and generate good compilation errors for that. Done by making the default constructor protected, and adding some private constructors.
* Add option to initialize all matrices by zero: just #define `EIGEN_INITIALIZE_MATRICES_BY_ZERO`
* Improve Map documentation
* Install the pkg-config file to share/pkgconfig, instead of lib/pkgconfig (thanks to Thomas Capricelli)
* fix warnings
* fix compilation with MSVC 2010
* adjust to repository name change

## [2.0.9] - 2009-10-24

Changes since 2.0.8:

* Really fix installation and the pkg-config file.
* Install the `NewStdVector` header that was introduced in 2.0.6.

## [2.0.8] - 2009-10-23

Changes since 2.0.7:

* fix installation error introduced in 2.0.7: it was choking on the pkg-config file eigen2.pc not being found. The fix had been proposed long ago by Ingmar Vanhassel for the development branch, and when recently the pkg-config support was back-ported to the 2.0 branch, nobody thought of backporting this fix too, and apparently nobody tested "make install" !
* SVD: add default constructor. Users were relying on the compiler to generate one, and apparenty 2.0.7 triggered a little MSVC 2008 subtlety in this respect. Also added an assert.

## [2.0.7] - 2009-10-22

Changes since 2.0.6:

* fix bug #61: crash when using Qt `QVector` on Windows 32-bit. By Hauke Heibel.
* fix bug #10: the `reallocateSparse` function was half coded
* fix bug in `SparseMatrix::resize()` not correctly initializing by zero
* fix another bug in `SparseMatrix::resize()` when `outerSize==0`. By Hauke Heibel.
* fully support GCC 3.3. It was working in 2.0.2, there was a compilation error in 2.0.6, now for the first time in 2.0.7 it's 100% tested (the test suite passes without any errors, warnings, or failed tests).
* SVD: add missing assert (help catch mistakes)
* fixed warnings in unit-tests (Hauke Heibel)
* finish syncing `Memory.h` with the devel branch. This is cleaner and fixes a warning. The choice of system aligned malloc function may be affected by this change.
* add pkg-config support by Rhys Ulerich.
* documentation fix and doc-generation-script updates by Thomas Capricelli

## [2.0.6] - 2009-09-23

Changes since 2.0.5:

* fix bug: visitors didn't work on row-vectors.
* fix bug #50: compilation errors with `swap()`.
* fix bug #42: Add `Transform::Identity()` as mentioned in the tutorial.
* allow to disable all alignment code by defining `EIGEN_DONT_ALIGN` (backport from devel branch).
* backport the devel branch's `StdVector` header as `NewStdVector`. You may also #define `EIGEN_USE_NEW_STDVECTOR` to make `StdVector` use it automatically. However, by default it isn't used by `StdVector`, to preserve compatibility.
* Vectorized quaternion product (for float) by Rohit Garg (backport from devel branch).
* allow to override `EIGEN_RESTRICT` and add `EIGEN_DONT_USE_RESTRICT_KEYWORD`
* fix a warning in `ei_aligned_malloc`; fixed by backporting the body from the devel branch; may result in a different choice of system aligned malloc function.
* update the documentation.

## [2.0.5] - 2009-08-22

Changes since 2.0.4:

* fix bug: in rare situations involving mixed storage orders, a matrix product could be evaluated as its own transpose
* fix bug: `LU::solve()` crashed when called on the LU decomposition of a zero matrix
* fix bug: `EIGEN_STACK_ALLOCATION_LIMIT` was too high, resulting in stack overflow for a user. Now it is set as in the devel branch.
* fix compilation bug: our `StdVector` header didn't work with GCC 4.1. (Bug #41)
* fix compilation bug: missing return statement in `Rotation2D::operator*=` (Bug #36)
* in StdVector, a more useful `#error` message about the #including order
* add `EIGEN_TRANSFORM_PLUGIN` allowing to customize the Transform class
* fix a warning with MSVC
* fix a bug in our cmake code when building unit-tests (thanks to Marcus Hanwell)
* work around a bug in cmake that made it fail to build unit-tests when fortran wasn't installed
* in our cmake code, remove the part about retrieving the mercurial info and appending it to the version number in the dox
* dox: remove the old example list
* fix the option to build a binary library, although it's not very useful and will be removed
* add basic .hgignore file and script to build the docs (thanks to Thomas Capricelli)

## [2.0.4] - 2009-08-01

Changes since 2.0.3:
* Several fixes in the overloaded new and delete operators. Thanks to Hauke Heibel.
* compilation fix: add the missing `ei_atan2` function. Thanks to Manuel Yguel.
* Use `ei_atan2` instead of using `std::atan2` directly.
* several compilation fixes in the Qt interoperability code: methods `toQTransform()` and `toQMatrix()`. Thanks to Anthony Truchet.
* compilation fix and simplification in Matrix assignment
* compilation fixes in `a *= b` and  `a = a*b` when a has to be resized.
* remove a "stupid" version of `ei_pow`. for integers for gcc >= 4.3
* bug fix in `Quaternion::setFromTwoVectors()`
* several ctest improvements: use our own dashboard, use a separate project for the 2.0 branch.
* documentation: improvement on the pages on unaligned arrays (the online copies have been updated immediately).

## [2.0.3] - 2009-06-21

Changes since 2.0.2:
* precision and reliability fixes in various algorithms, especially LLT, QR, Tridiagonalization, and also a precision improvement in LU.
* fix LLT and LDLT solve() on uninitialized result (was causing an assertion).
* add Eigen/Eigen and Eigen/Dense headers for convenience
* document a newly found cause for the "unaligned array" assertion
* backport documentation improvements on transpose() and adjoint()
* updates in the Sparse module (was needed to support KDE 4.3)

## [2.0.2] - 2009-05-22

Changes since 2.0.1:
* Fix `linearRegression()` compilation, actually it is reimplemented using the better fitHyperplane() which does total least-squares.
* Add missing `setZero()` etc... variants taking size parameters and resizing. These were mentioned in the tutorial but weren't implemented.
* Fix `posix_memalign` platform check. This fixes portability issues. Thanks to Ross Smith.
* Fix detection of SSE2 on the Windows 64-bit platform.
* Fix compatibility with the old GCC 3.3: it is now fully supported again.
* Fix warnings with recent GCC (4.4.0 and 4.3.3).

## [2.0.1] - 2009-04-14

Changes since 2.0.0:
* disable alignment altogether on exotic platforms on which we don't vectorize anyway. This allows e.g. to use Eigen on ARM platforms.
* new StdVector header with a new workaround for the problems with std::vector.
* workarounds for MSVC internal compiler errors
* MSVC 9 compilation fix (patch by Hauke Heibel)
* fixes for various bugs in Maps/Blocks that could give wrong results
* fix bug in 4x4 matrix inverse that could give wrong results
* compilation fix in SliceVectorization
* fix wrong static assertion (patch by Markus Moll)
* add missing operators in `aligned_allocator` (thanks to Hauke Heibel)

## [2.0.0] - 2009-02-02

First public release.