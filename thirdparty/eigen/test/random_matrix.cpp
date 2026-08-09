// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2021 Kolja Brix <kolja.brix@rwth-aachen.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"
#include <Eigen/SVD>

template <typename MatrixType>
void check_generateRandomUnitaryMatrix(const Index dim) {
  const MatrixType Q = generateRandomUnitaryMatrix<MatrixType>(dim);

  // validate dimensions
  VERIFY_IS_EQUAL(Q.rows(), dim);
  VERIFY_IS_EQUAL(Q.cols(), dim);

  VERIFY_IS_UNITARY(Q);
}

template <typename VectorType, typename RealScalarType>
void check_setupRandomSvs(const Index dim, const RealScalarType max) {
  const VectorType v = setupRandomSvs<VectorType, RealScalarType>(dim, max);

  // validate dimensions
  VERIFY_IS_EQUAL(v.size(), dim);

  // check entries
  for (Index i = 0; i < v.size(); ++i) VERIFY_GE(v(i), 0);
  for (Index i = 0; i < v.size() - 1; ++i) VERIFY_GE(v(i), v(i + 1));
}

template <typename VectorType, typename RealScalarType>
void check_setupRangeSvs(const Index dim, const RealScalarType min, const RealScalarType max) {
  const VectorType v = setupRangeSvs<VectorType, RealScalarType>(dim, min, max);

  // validate dimensions
  VERIFY_IS_EQUAL(v.size(), dim);

  // check entries
  if (dim == 1) {
    VERIFY_IS_APPROX(v(0), min);
  } else {
    VERIFY_IS_APPROX(v(0), max);
    VERIFY_IS_APPROX(v(dim - 1), min);
  }
  for (Index i = 0; i < v.size() - 1; ++i) VERIFY_GE(v(i), v(i + 1));
}

template <typename MatrixType, typename RealScalar, typename RealVectorType>
void check_generateRandomMatrixSvs(const Index rows, const Index cols, const Index diag_size, const RealScalar min_svs,
                                   const RealScalar max_svs) {
  RealVectorType svs = setupRangeSvs<RealVectorType, RealScalar>(diag_size, min_svs, max_svs);

  MatrixType M = MatrixType::Zero(rows, cols);
  generateRandomMatrixSvs(svs, rows, cols, M);

  // validate dimensions
  VERIFY_IS_EQUAL(M.rows(), rows);
  VERIFY_IS_EQUAL(M.cols(), cols);
  VERIFY_IS_EQUAL(svs.size(), diag_size);

  // validate singular values
  Eigen::JacobiSVD<MatrixType> SVD(M);
  VERIFY_IS_APPROX(svs, SVD.singularValues());
}

template <typename MatrixType>
void check_random_matrix(const MatrixType &m) {
  enum {
    Rows = MatrixType::RowsAtCompileTime,
    Cols = MatrixType::ColsAtCompileTime,
    DiagSize = internal::min_size_prefer_dynamic(Rows, Cols)
  };
  typedef typename MatrixType::Scalar Scalar;
  typedef typename NumTraits<Scalar>::Real RealScalar;
  typedef Matrix<RealScalar, DiagSize, 1> RealVectorType;

  const Index rows = m.rows(), cols = m.cols();
  const Index diag_size = (std::min)(rows, cols);
  const RealScalar min_svs = 1.0, max_svs = 1000.0;

  // check generation of unitary random matrices
  typedef Matrix<Scalar, Rows, Rows> MatrixAType;
  typedef Matrix<Scalar, Cols, Cols> MatrixBType;
  check_generateRandomUnitaryMatrix<MatrixAType>(rows);
  check_generateRandomUnitaryMatrix<MatrixBType>(cols);

  // test generators for singular values
  check_setupRandomSvs<RealVectorType, RealScalar>(diag_size, max_svs);
  check_setupRangeSvs<RealVectorType, RealScalar>(diag_size, min_svs, max_svs);

  // check generation of random matrices
  check_generateRandomMatrixSvs<MatrixType, RealScalar, RealVectorType>(rows, cols, diag_size, min_svs, max_svs);
}

EIGEN_DECLARE_TEST(random_matrix) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(check_random_matrix(Matrix<float, 1, 1>()));
    CALL_SUBTEST_2(check_random_matrix(Matrix<float, 4, 4>()));
    CALL_SUBTEST_3(check_random_matrix(Matrix<float, 2, 3>()));
    CALL_SUBTEST_4(check_random_matrix(Matrix<float, 7, 4>()));

    CALL_SUBTEST_5(check_random_matrix(Matrix<double, 1, 1>()));
    CALL_SUBTEST_6(check_random_matrix(Matrix<double, 6, 6>()));
    CALL_SUBTEST_7(check_random_matrix(Matrix<double, 5, 3>()));
    CALL_SUBTEST_8(check_random_matrix(Matrix<double, 4, 9>()));

    CALL_SUBTEST_9(check_random_matrix(Matrix<std::complex<float>, 12, 12>()));
    CALL_SUBTEST_10(check_random_matrix(Matrix<std::complex<float>, 7, 14>()));
    CALL_SUBTEST_11(check_random_matrix(Matrix<std::complex<double>, 15, 11>()));
    CALL_SUBTEST_12(check_random_matrix(Matrix<std::complex<double>, 6, 9>()));

    CALL_SUBTEST_13(check_random_matrix(
        MatrixXf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_14(check_random_matrix(
        MatrixXd(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_15(check_random_matrix(
        MatrixXcf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_16(check_random_matrix(
        MatrixXcd(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }
}
