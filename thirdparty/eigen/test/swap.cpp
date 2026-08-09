// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename T>
struct other_matrix_type {
  typedef int type;
};

template <typename Scalar_, int Rows_, int Cols_, int Options_, int MaxRows_, int MaxCols_>
struct other_matrix_type<Matrix<Scalar_, Rows_, Cols_, Options_, MaxRows_, MaxCols_> > {
  typedef Matrix<Scalar_, Rows_, Cols_, Options_ ^ RowMajor, MaxRows_, MaxCols_> type;
};

template <typename MatrixType>
std::enable_if_t<(MatrixType::RowsAtCompileTime == 1 || MatrixType::RowsAtCompileTime == Dynamic), void> check_row_swap(
    MatrixType& m1) {
  if (m1.rows() != 1) {
    // test assertion on mismatching size -- matrix case
    VERIFY_RAISES_ASSERT(m1.swap(m1.row(0)));
    // test assertion on mismatching size -- xpr case
    VERIFY_RAISES_ASSERT(m1.row(0).swap(m1));
  }
}

template <typename MatrixType>
std::enable_if_t<!(MatrixType::RowsAtCompileTime == 1 || MatrixType::RowsAtCompileTime == Dynamic), void>
check_row_swap(MatrixType& /* unused */) {}

template <typename MatrixType>
void swap(const MatrixType& m) {
  typedef typename other_matrix_type<MatrixType>::type OtherMatrixType;
  typedef typename MatrixType::Scalar Scalar;

  eigen_assert((!internal::is_same<MatrixType, OtherMatrixType>::value));
  Index rows = m.rows();
  Index cols = m.cols();

  // construct 3 matrix guaranteed to be distinct
  MatrixType m1 = MatrixType::Random(rows, cols);
  MatrixType m2 = MatrixType::Random(rows, cols) + Scalar(100) * MatrixType::Identity(rows, cols);
  OtherMatrixType m3 = OtherMatrixType::Random(rows, cols) + Scalar(200) * OtherMatrixType::Identity(rows, cols);

  MatrixType m1_copy = m1;
  MatrixType m2_copy = m2;
  OtherMatrixType m3_copy = m3;

  // test swapping 2 matrices of same type
  Scalar *d1 = m1.data(), *d2 = m2.data();
  m1.swap(m2);
  VERIFY_IS_APPROX(m1, m2_copy);
  VERIFY_IS_APPROX(m2, m1_copy);
  if (MatrixType::SizeAtCompileTime == Dynamic) {
    VERIFY(m1.data() == d2);
    VERIFY(m2.data() == d1);
  }
  m1 = m1_copy;
  m2 = m2_copy;
  d1 = m1.data(), d2 = m2.data();
  swap(m1, m2);
  VERIFY_IS_APPROX(m1, m2_copy);
  VERIFY_IS_APPROX(m2, m1_copy);
  if (MatrixType::SizeAtCompileTime == Dynamic) {
    VERIFY(m1.data() == d2);
    VERIFY(m2.data() == d1);
  }
  m1 = m1_copy;
  m2 = m2_copy;

  // test swapping 2 matrices of different types
  m1.swap(m3);
  VERIFY_IS_APPROX(m1, m3_copy);
  VERIFY_IS_APPROX(m3, m1_copy);
  m1 = m1_copy;
  m3 = m3_copy;
  swap(m1, m3);
  VERIFY_IS_APPROX(m1, m3_copy);
  VERIFY_IS_APPROX(m3, m1_copy);
  m1 = m1_copy;
  m3 = m3_copy;

  // test swapping matrix with expression
  m1.swap(m2.block(0, 0, rows, cols));
  VERIFY_IS_APPROX(m1, m2_copy);
  VERIFY_IS_APPROX(m2, m1_copy);
  m1 = m1_copy;
  m2 = m2_copy;
  swap(m1, m2.block(0, 0, rows, cols));
  VERIFY_IS_APPROX(m1, m2_copy);
  VERIFY_IS_APPROX(m2, m1_copy);
  m1 = m1_copy;
  m2 = m2_copy;

  // test swapping two expressions of different types
  m1.transpose().swap(m3.transpose());
  VERIFY_IS_APPROX(m1, m3_copy);
  VERIFY_IS_APPROX(m3, m1_copy);
  m1 = m1_copy;
  m3 = m3_copy;
  swap(m1.transpose(), m3.transpose());
  VERIFY_IS_APPROX(m1, m3_copy);
  VERIFY_IS_APPROX(m3, m1_copy);
  m1 = m1_copy;
  m3 = m3_copy;

  check_row_swap(m1);
}

EIGEN_DECLARE_TEST(swap) {
  int s = internal::random<int>(1, EIGEN_TEST_MAX_SIZE);
  CALL_SUBTEST_1(swap(Matrix3f()));      // fixed size, no vectorization
  CALL_SUBTEST_2(swap(Matrix4d()));      // fixed size, possible vectorization
  CALL_SUBTEST_3(swap(MatrixXd(s, s)));  // dyn size, no vectorization
  CALL_SUBTEST_4(swap(MatrixXf(s, s)));  // dyn size, possible vectorization
  TEST_SET_BUT_UNUSED_VARIABLE(s)
}
