// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2014 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SVD_DEFAULT
#error a macro SVD_DEFAULT(MatrixType) must be defined prior to including svd_common.h
#endif

#ifndef SVD_FOR_MIN_NORM
#error a macro SVD_FOR_MIN_NORM(MatrixType) must be defined prior to including svd_common.h
#endif

#ifndef SVD_STATIC_OPTIONS
#error a macro SVD_STATIC_OPTIONS(MatrixType, Options) must be defined prior to including svd_common.h
#endif

#include "svd_fill.h"
#include "solverbase.h"

// Check that the matrix m is properly reconstructed and that the U and V factors are unitary
// The SVD must have already been computed.
template <typename SvdType, typename MatrixType>
void svd_check_full(const MatrixType& m, const SvdType& svd) {
  Index rows = m.rows();
  Index cols = m.cols();

  enum { RowsAtCompileTime = MatrixType::RowsAtCompileTime, ColsAtCompileTime = MatrixType::ColsAtCompileTime };

  typedef typename MatrixType::Scalar Scalar;
  typedef typename MatrixType::RealScalar RealScalar;
  typedef Matrix<Scalar, RowsAtCompileTime, RowsAtCompileTime> MatrixUType;
  typedef Matrix<Scalar, ColsAtCompileTime, ColsAtCompileTime> MatrixVType;

  MatrixType sigma = MatrixType::Zero(rows, cols);
  sigma.diagonal() = svd.singularValues().template cast<Scalar>();
  MatrixUType u = svd.matrixU();
  MatrixVType v = svd.matrixV();
  RealScalar scaling = m.cwiseAbs().maxCoeff();
  if (scaling < (std::numeric_limits<RealScalar>::min)()) {
    VERIFY(sigma.cwiseAbs().maxCoeff() <= (std::numeric_limits<RealScalar>::min)());
  } else {
    VERIFY_IS_APPROX(m / scaling, u * (sigma / scaling) * v.adjoint());
  }
  VERIFY_IS_UNITARY(u);
  VERIFY_IS_UNITARY(v);
}

// Compare partial SVD defined by computationOptions to a full SVD referenceSvd
template <typename MatrixType, typename SvdType, int Options>
void svd_compare_to_full(const MatrixType& m, const SvdType& referenceSvd) {
  typedef typename MatrixType::RealScalar RealScalar;
  Index rows = m.rows();
  Index cols = m.cols();
  Index diagSize = (std::min)(rows, cols);
  RealScalar prec = test_precision<RealScalar>();

  SVD_STATIC_OPTIONS(MatrixType, Options) svd(m);

  VERIFY_IS_APPROX(svd.singularValues(), referenceSvd.singularValues());

  if (Options & (ComputeFullV | ComputeThinV)) {
    VERIFY((svd.matrixV().adjoint() * svd.matrixV()).isIdentity(prec));
    VERIFY_IS_APPROX(svd.matrixV().leftCols(diagSize) * svd.singularValues().asDiagonal() *
                         svd.matrixV().leftCols(diagSize).adjoint(),
                     referenceSvd.matrixV().leftCols(diagSize) * referenceSvd.singularValues().asDiagonal() *
                         referenceSvd.matrixV().leftCols(diagSize).adjoint());
  }

  if (Options & (ComputeFullU | ComputeThinU)) {
    VERIFY((svd.matrixU().adjoint() * svd.matrixU()).isIdentity(prec));
    VERIFY_IS_APPROX(svd.matrixU().leftCols(diagSize) * svd.singularValues().cwiseAbs2().asDiagonal() *
                         svd.matrixU().leftCols(diagSize).adjoint(),
                     referenceSvd.matrixU().leftCols(diagSize) *
                         referenceSvd.singularValues().cwiseAbs2().asDiagonal() *
                         referenceSvd.matrixU().leftCols(diagSize).adjoint());
  }

  // The following checks are not critical.
  // For instance, with Dived&Conquer SVD, if only the factor 'V' is computed then different matrix-matrix product
  // implementation will be used and the resulting 'V' factor might be significantly different when the SVD
  // decomposition is not unique, especially with single precision float.
  ++g_test_level;
  if (Options & ComputeFullU) VERIFY_IS_APPROX(svd.matrixU(), referenceSvd.matrixU());
  if (Options & ComputeThinU) VERIFY_IS_APPROX(svd.matrixU(), referenceSvd.matrixU().leftCols(diagSize));
  if (Options & ComputeFullV) VERIFY_IS_APPROX(svd.matrixV().cwiseAbs(), referenceSvd.matrixV().cwiseAbs());
  if (Options & ComputeThinV) VERIFY_IS_APPROX(svd.matrixV(), referenceSvd.matrixV().leftCols(diagSize));
  --g_test_level;
}

template <typename SvdType, typename MatrixType>
void svd_least_square(const MatrixType& m) {
  typedef typename MatrixType::Scalar Scalar;
  typedef typename MatrixType::RealScalar RealScalar;
  Index rows = m.rows();
  Index cols = m.cols();

  enum { RowsAtCompileTime = MatrixType::RowsAtCompileTime, ColsAtCompileTime = MatrixType::ColsAtCompileTime };

  typedef Matrix<Scalar, RowsAtCompileTime, Dynamic> RhsType;
  typedef Matrix<Scalar, ColsAtCompileTime, Dynamic> SolutionType;

  RhsType rhs = RhsType::Random(rows, internal::random<Index>(1, cols));
  SvdType svd(m);

  if (internal::is_same<RealScalar, double>::value)
    svd.setThreshold(RealScalar(1e-8));
  else if (internal::is_same<RealScalar, float>::value)
    svd.setThreshold(RealScalar(2e-4));

  SolutionType x = svd.solve(rhs);

  RealScalar residual = (m * x - rhs).norm();
  RealScalar rhs_norm = rhs.norm();
  if (!test_isMuchSmallerThan(residual, rhs.norm())) {
    // ^^^ If the residual is very small, then we have an exact solution, so we are already good.

    // evaluate normal equation which works also for least-squares solutions
    if (internal::is_same<RealScalar, double>::value || svd.rank() == m.diagonal().size()) {
      using std::sqrt;
      // This test is not stable with single precision.
      // This is probably because squaring m signicantly affects the precision.
      if (internal::is_same<RealScalar, float>::value) ++g_test_level;

      VERIFY_IS_APPROX(m.adjoint() * (m * x), m.adjoint() * rhs);

      if (internal::is_same<RealScalar, float>::value) --g_test_level;
    }

    // Check that there is no significantly better solution in the neighborhood of x
    for (Index k = 0; k < x.rows(); ++k) {
      using std::abs;

      SolutionType y(x);
      y.row(k) = (RealScalar(1) + 2 * NumTraits<RealScalar>::epsilon()) * x.row(k);
      RealScalar residual_y = (m * y - rhs).norm();
      VERIFY(test_isMuchSmallerThan(abs(residual_y - residual), rhs_norm) || residual < residual_y);
      if (internal::is_same<RealScalar, float>::value) ++g_test_level;
      VERIFY(test_isApprox(residual_y, residual) || residual < residual_y);
      if (internal::is_same<RealScalar, float>::value) --g_test_level;

      y.row(k) = (RealScalar(1) - 2 * NumTraits<RealScalar>::epsilon()) * x.row(k);
      residual_y = (m * y - rhs).norm();
      VERIFY(test_isMuchSmallerThan(abs(residual_y - residual), rhs_norm) || residual < residual_y);
      if (internal::is_same<RealScalar, float>::value) ++g_test_level;
      VERIFY(test_isApprox(residual_y, residual) || residual < residual_y);
      if (internal::is_same<RealScalar, float>::value) --g_test_level;
    }
  }
}

// check minimal norm solutions, the input matrix m is only used to recover problem size
template <typename MatrixType, int Options>
void svd_min_norm(const MatrixType& m) {
  typedef typename MatrixType::Scalar Scalar;
  Index cols = m.cols();

  enum { ColsAtCompileTime = MatrixType::ColsAtCompileTime };

  typedef Matrix<Scalar, ColsAtCompileTime, Dynamic> SolutionType;

  // generate a full-rank m x n problem with m<n
  enum {
    RankAtCompileTime2 = ColsAtCompileTime == Dynamic ? Dynamic : (ColsAtCompileTime) / 2 + 1,
    RowsAtCompileTime3 = ColsAtCompileTime == Dynamic ? Dynamic : ColsAtCompileTime + 1
  };
  typedef Matrix<Scalar, RankAtCompileTime2, ColsAtCompileTime> MatrixType2;
  typedef Matrix<Scalar, RankAtCompileTime2, 1> RhsType2;
  typedef Matrix<Scalar, ColsAtCompileTime, RankAtCompileTime2> MatrixType2T;
  Index rank = RankAtCompileTime2 == Dynamic ? internal::random<Index>(1, cols) : Index(RankAtCompileTime2);
  MatrixType2 m2(rank, cols);
  int guard = 0;
  do {
    m2.setRandom();
  } while (SVD_FOR_MIN_NORM(MatrixType2)(m2).setThreshold(test_precision<Scalar>()).rank() != rank && (++guard) < 10);
  VERIFY(guard < 10);

  RhsType2 rhs2 = RhsType2::Random(rank);
  // use QR to find a reference minimal norm solution
  HouseholderQR<MatrixType2T> qr(m2.adjoint());
  Matrix<Scalar, Dynamic, 1> tmp =
      qr.matrixQR().topLeftCorner(rank, rank).template triangularView<Upper>().adjoint().solve(rhs2);
  tmp.conservativeResize(cols);
  tmp.tail(cols - rank).setZero();
  SolutionType x21 = qr.householderQ() * tmp;
  // now check with SVD
  SVD_STATIC_OPTIONS(MatrixType2, Options) svd2(m2);
  SolutionType x22 = svd2.solve(rhs2);
  VERIFY_IS_APPROX(m2 * x21, rhs2);
  VERIFY_IS_APPROX(m2 * x22, rhs2);
  VERIFY_IS_APPROX(x21, x22);

  // Now check with a rank deficient matrix
  typedef Matrix<Scalar, RowsAtCompileTime3, ColsAtCompileTime> MatrixType3;
  typedef Matrix<Scalar, RowsAtCompileTime3, 1> RhsType3;
  Index rows3 = RowsAtCompileTime3 == Dynamic ? internal::random<Index>(rank + 1, 2 * cols) : Index(RowsAtCompileTime3);
  Matrix<Scalar, RowsAtCompileTime3, Dynamic> C = Matrix<Scalar, RowsAtCompileTime3, Dynamic>::Random(rows3, rank);
  MatrixType3 m3 = C * m2;
  RhsType3 rhs3 = C * rhs2;
  SVD_STATIC_OPTIONS(MatrixType3, Options) svd3(m3);
  SolutionType x3 = svd3.solve(rhs3);
  VERIFY_IS_APPROX(m3 * x3, rhs3);
  VERIFY_IS_APPROX(m3 * x21, rhs3);
  VERIFY_IS_APPROX(m2 * x3, rhs2);
  VERIFY_IS_APPROX(x21, x3);
}

template <typename MatrixType, typename SolverType>
void svd_test_solvers(const MatrixType& m, const SolverType& solver) {
  Index rows, cols, cols2;

  rows = m.rows();
  cols = m.cols();

  if (MatrixType::ColsAtCompileTime == Dynamic) {
    cols2 = internal::random<int>(2, EIGEN_TEST_MAX_SIZE);
  } else {
    cols2 = cols;
  }
  typedef Matrix<typename MatrixType::Scalar, MatrixType::ColsAtCompileTime, MatrixType::ColsAtCompileTime> CMatrixType;
  check_solverbase<CMatrixType, MatrixType>(m, solver, rows, cols, cols2);
}

// work around stupid msvc error when constructing at compile time an expression that involves
// a division by zero, even if the numeric type has floating point
template <typename Scalar>
EIGEN_DONT_INLINE Scalar zero() {
  return Scalar(0);
}

// workaround aggressive optimization in ICC
template <typename T>
EIGEN_DONT_INLINE T sub(T a, T b) {
  return a - b;
}

// This function verifies we don't iterate infinitely on nan/inf values,
// and that info() returns InvalidInput.
template <typename MatrixType>
void svd_inf_nan() {
  SVD_STATIC_OPTIONS(MatrixType, ComputeFullU | ComputeFullV) svd;
  typedef typename MatrixType::Scalar Scalar;
  Scalar some_inf = Scalar(1) / zero<Scalar>();
  VERIFY(sub(some_inf, some_inf) != sub(some_inf, some_inf));
  svd.compute(MatrixType::Constant(10, 10, some_inf));
  VERIFY(svd.info() == InvalidInput);

  Scalar nan = std::numeric_limits<Scalar>::quiet_NaN();
  VERIFY(nan != nan);
  svd.compute(MatrixType::Constant(10, 10, nan));
  VERIFY(svd.info() == InvalidInput);

  MatrixType m = MatrixType::Zero(10, 10);
  m(internal::random<int>(0, 9), internal::random<int>(0, 9)) = some_inf;
  svd.compute(m);
  VERIFY(svd.info() == InvalidInput);

  m = MatrixType::Zero(10, 10);
  m(internal::random<int>(0, 9), internal::random<int>(0, 9)) = nan;
  svd.compute(m);
  VERIFY(svd.info() == InvalidInput);

  // regression test for bug 791
  m.resize(3, 3);
  m << 0, 2 * NumTraits<Scalar>::epsilon(), 0.5, 0, -0.5, 0, nan, 0, 0;
  svd.compute(m);
  VERIFY(svd.info() == InvalidInput);

  Scalar min = (std::numeric_limits<Scalar>::min)();
  m.resize(4, 4);
  m << 1, 0, 0, 0, 0, 3, 1, min, 1, 0, 1, nan, 0, nan, nan, 0;
  svd.compute(m);
  VERIFY(svd.info() == InvalidInput);
}

// Regression test for bug 286: JacobiSVD loops indefinitely with some
// matrices containing denormal numbers.
template <typename>
void svd_underoverflow() {
#if defined __INTEL_COMPILER
// shut up warning #239: floating point underflow
#pragma warning push
#pragma warning disable 239
#endif
  Matrix2d M;
  M << -7.90884e-313, -4.94e-324, 0, 5.60844e-313;
  SVD_STATIC_OPTIONS(Matrix2d, ComputeFullU | ComputeFullV) svd;
  svd.compute(M);
  CALL_SUBTEST(svd_check_full(M, svd));

  // Check all 2x2 matrices made with the following coefficients:
  VectorXd value_set(9);
  value_set << 0, 1, -1, 5.60844e-313, -5.60844e-313, 4.94e-324, -4.94e-324, -4.94e-223, 4.94e-223;
  Array4i id(0, 0, 0, 0);
  int k = 0;
  do {
    M << value_set(id(0)), value_set(id(1)), value_set(id(2)), value_set(id(3));
    svd.compute(M);
    CALL_SUBTEST(svd_check_full(M, svd));

    id(k)++;
    if (id(k) >= value_set.size()) {
      while (k < 3 && id(k) >= value_set.size()) id(++k)++;
      id.head(k).setZero();
      k = 0;
    }

  } while ((id < int(value_set.size())).all());

#if defined __INTEL_COMPILER
#pragma warning pop
#endif

  // Check for overflow:
  Matrix3d M3;
  M3 << 4.4331978442502944e+307, -5.8585363752028680e+307, 6.4527017443412964e+307, 3.7841695601406358e+307,
      2.4331702789740617e+306, -3.5235707140272905e+307, -8.7190887618028355e+307, -7.3453213709232193e+307,
      -2.4367363684472105e+307;

  SVD_STATIC_OPTIONS(Matrix3d, ComputeFullU | ComputeFullV) svd3;
  svd3.compute(M3);  // just check we don't loop indefinitely
  CALL_SUBTEST(svd_check_full(M3, svd3));
}

template <typename MatrixType>
void svd_all_trivial_2x2(void (*cb)(const MatrixType&)) {
  MatrixType M;
  VectorXd value_set(3);
  value_set << 0, 1, -1;
  Array4i id(0, 0, 0, 0);
  int k = 0;
  do {
    M << value_set(id(0)), value_set(id(1)), value_set(id(2)), value_set(id(3));

    cb(M);

    id(k)++;
    if (id(k) >= value_set.size()) {
      while (k < 3 && id(k) >= value_set.size()) id(++k)++;
      id.head(k).setZero();
      k = 0;
    }

  } while ((id < int(value_set.size())).all());
}

template <typename>
void svd_preallocate() {
  Vector3f v(3.f, 2.f, 1.f);
  MatrixXf m = v.asDiagonal();

  internal::set_is_malloc_allowed(false);
  VERIFY_RAISES_ASSERT(VectorXf tmp(10);)
  SVD_DEFAULT(MatrixXf) svd;
  internal::set_is_malloc_allowed(true);
  svd.compute(m);
  VERIFY_IS_APPROX(svd.singularValues(), v);
  VERIFY_RAISES_ASSERT(svd.matrixU());
  VERIFY_RAISES_ASSERT(svd.matrixV());

  SVD_STATIC_OPTIONS(MatrixXf, ComputeFullU | ComputeFullV) svd2(3, 3);
  internal::set_is_malloc_allowed(false);
  svd2.compute(m);
  internal::set_is_malloc_allowed(true);
  VERIFY_IS_APPROX(svd2.singularValues(), v);
  VERIFY_IS_APPROX(svd2.matrixU(), Matrix3f::Identity());
  VERIFY_IS_APPROX(svd2.matrixV(), Matrix3f::Identity());
  internal::set_is_malloc_allowed(false);
  svd2.compute(m);
  internal::set_is_malloc_allowed(true);
}

template <typename MatrixType, int QRPreconditioner = 0>
void svd_verify_assert_full_only(const MatrixType& input = MatrixType()) {
  enum { RowsAtCompileTime = MatrixType::RowsAtCompileTime };

  typedef Matrix<typename MatrixType::Scalar, RowsAtCompileTime, 1> RhsType;
  RhsType rhs = RhsType::Zero(input.rows());
  EIGEN_UNUSED_VARIABLE(rhs);  // Only used if asserts are enabled.
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);

  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner) svd0;
  VERIFY_RAISES_ASSERT((svd0.matrixU()));
  VERIFY_RAISES_ASSERT((svd0.singularValues()));
  VERIFY_RAISES_ASSERT((svd0.matrixV()));
  VERIFY_RAISES_ASSERT((svd0.solve(rhs)));
  VERIFY_RAISES_ASSERT((svd0.transpose().solve(rhs)));
  VERIFY_RAISES_ASSERT((svd0.adjoint().solve(rhs)));

  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner) svd1(m);
  VERIFY_RAISES_ASSERT((svd1.matrixU()));
  VERIFY_RAISES_ASSERT((svd1.matrixV()));
  VERIFY_RAISES_ASSERT((svd1.solve(rhs)));

  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullU) svdFullU(m);
  VERIFY_RAISES_ASSERT((svdFullU.matrixV()));
  VERIFY_RAISES_ASSERT((svdFullU.solve(rhs)));
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullV) svdFullV(m);
  VERIFY_RAISES_ASSERT((svdFullV.matrixU()));
  VERIFY_RAISES_ASSERT((svdFullV.solve(rhs)));
}

template <typename MatrixType, int QRPreconditioner = 0>
void svd_verify_assert(const MatrixType& input = MatrixType()) {
  enum { RowsAtCompileTime = MatrixType::RowsAtCompileTime };
  typedef Matrix<typename MatrixType::Scalar, RowsAtCompileTime, 1> RhsType;
  RhsType rhs = RhsType::Zero(input.rows());
  EIGEN_UNUSED_VARIABLE(rhs);  // Only used if asserts are enabled.
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);

  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeThinU) svdThinU(m);
  VERIFY_RAISES_ASSERT((svdThinU.matrixV()));
  VERIFY_RAISES_ASSERT((svdThinU.solve(rhs)));
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeThinV) svdThinV(m);
  VERIFY_RAISES_ASSERT((svdThinV.matrixU()));
  VERIFY_RAISES_ASSERT((svdThinV.solve(rhs)));

  svd_verify_assert_full_only<MatrixType, QRPreconditioner>(m);
}

template <typename MatrixType, int Options>
void svd_compute_checks(const MatrixType& m) {
  typedef SVD_STATIC_OPTIONS(MatrixType, Options) SVDType;

  enum {
    RowsAtCompileTime = MatrixType::RowsAtCompileTime,
    ColsAtCompileTime = MatrixType::ColsAtCompileTime,
    DiagAtCompileTime = internal::min_size_prefer_dynamic(RowsAtCompileTime, ColsAtCompileTime),
    MatrixURowsAtCompileTime = SVDType::MatrixUType::RowsAtCompileTime,
    MatrixUColsAtCompileTime = SVDType::MatrixUType::ColsAtCompileTime,
    MatrixVRowsAtCompileTime = SVDType::MatrixVType::RowsAtCompileTime,
    MatrixVColsAtCompileTime = SVDType::MatrixVType::ColsAtCompileTime
  };

  SVDType staticSvd(m);

  VERIFY(MatrixURowsAtCompileTime == RowsAtCompileTime);
  VERIFY(MatrixVRowsAtCompileTime == ColsAtCompileTime);
  if (Options & ComputeThinU) VERIFY(MatrixUColsAtCompileTime == DiagAtCompileTime);
  if (Options & ComputeFullU) VERIFY(MatrixUColsAtCompileTime == RowsAtCompileTime);
  if (Options & ComputeThinV) VERIFY(MatrixVColsAtCompileTime == DiagAtCompileTime);
  if (Options & ComputeFullV) VERIFY(MatrixVColsAtCompileTime == ColsAtCompileTime);

  if (Options & (ComputeThinU | ComputeFullU))
    VERIFY(staticSvd.computeU());
  else
    VERIFY(!staticSvd.computeU());
  if (Options & (ComputeThinV | ComputeFullV))
    VERIFY(staticSvd.computeV());
  else
    VERIFY(!staticSvd.computeV());

  if (staticSvd.computeU()) VERIFY(staticSvd.matrixU().isUnitary());
  if (staticSvd.computeV()) VERIFY(staticSvd.matrixV().isUnitary());

  if (staticSvd.computeU() && staticSvd.computeV()) {
    svd_test_solvers(m, staticSvd);
    svd_least_square<SVDType, MatrixType>(m);
    // svd_min_norm generates non-square matrices so it can't be used with NoQRPreconditioner
    if ((Options & internal::QRPreconditionerBits) != NoQRPreconditioner) svd_min_norm<MatrixType, Options>(m);
  }
}

template <typename MatrixType, int QRPreconditioner = 0>
void svd_thin_option_checks(const MatrixType& input) {
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);

  svd_compute_checks<MatrixType, QRPreconditioner>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeThinU>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeThinV>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeThinU | ComputeThinV>(m);

  svd_compute_checks<MatrixType, QRPreconditioner | ComputeThinU | ComputeFullV>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeFullU | ComputeThinV>(m);

  typedef SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullU | ComputeFullV) FullSvdType;
  FullSvdType fullSvd(m);
  svd_check_full(m, fullSvd);
  svd_compare_to_full<MatrixType, FullSvdType, QRPreconditioner | ComputeFullU | ComputeFullV>(m, fullSvd);
}

template <typename MatrixType, int QRPreconditioner = 0>
void svd_option_checks_full_only(const MatrixType& input) {
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeFullU>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeFullV>(m);
  svd_compute_checks<MatrixType, QRPreconditioner | ComputeFullU | ComputeFullV>(m);

  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullU | ComputeFullV) fullSvd(m);
  svd_check_full(m, fullSvd);
}

template <typename MatrixType, int QRPreconditioner = 0>
void svd_check_max_size_matrix(int initialRows, int initialCols) {
  enum {
    MaxRowsAtCompileTime = MatrixType::MaxRowsAtCompileTime,
    MaxColsAtCompileTime = MatrixType::MaxColsAtCompileTime
  };

  int rows = MaxRowsAtCompileTime == Dynamic ? initialRows : (std::min)(initialRows, (int)MaxRowsAtCompileTime);
  int cols = MaxColsAtCompileTime == Dynamic ? initialCols : (std::min)(initialCols, (int)MaxColsAtCompileTime);

  MatrixType m(rows, cols);
  svd_fill_random(m);
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeThinU | ComputeThinV) thinSvd(m);
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeThinU | ComputeFullV) mixedSvd1(m);
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullU | ComputeThinV) mixedSvd2(m);
  SVD_STATIC_OPTIONS(MatrixType, QRPreconditioner | ComputeFullU | ComputeFullV) fullSvd(m);

  MatrixType n(MaxRowsAtCompileTime, MaxColsAtCompileTime);
  svd_fill_random(n);
  thinSvd.compute(n);
  mixedSvd1.compute(n);
  mixedSvd2.compute(n);
  fullSvd.compute(n);

  MatrixX<typename MatrixType::Scalar> dynamicMatrix(MaxRowsAtCompileTime + 1, MaxColsAtCompileTime + 1);

  VERIFY_RAISES_ASSERT(thinSvd.compute(dynamicMatrix));
  VERIFY_RAISES_ASSERT(mixedSvd1.compute(dynamicMatrix));
  VERIFY_RAISES_ASSERT(mixedSvd2.compute(dynamicMatrix));
  VERIFY_RAISES_ASSERT(fullSvd.compute(dynamicMatrix));
}

template <typename SvdType, typename MatrixType>
void svd_verify_constructor_options_assert(const MatrixType& m) {
  typedef typename MatrixType::Scalar Scalar;
  Index rows = m.rows();

  enum { RowsAtCompileTime = MatrixType::RowsAtCompileTime, ColsAtCompileTime = MatrixType::ColsAtCompileTime };

  typedef Matrix<Scalar, RowsAtCompileTime, 1> RhsType;
  RhsType rhs(rows);
  svd_fill_random(rhs);
  SvdType svd;
  VERIFY_RAISES_ASSERT(svd.matrixU())
  VERIFY_RAISES_ASSERT(svd.singularValues())
  VERIFY_RAISES_ASSERT(svd.matrixV())
  VERIFY_RAISES_ASSERT(svd.solve(rhs))
  VERIFY_RAISES_ASSERT(svd.transpose().solve(rhs))
  VERIFY_RAISES_ASSERT(svd.adjoint().solve(rhs))
}

#undef SVD_DEFAULT
#undef SVD_FOR_MIN_NORM
#undef SVD_STATIC_OPTIONS
