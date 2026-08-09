// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2025 The Eigen Authors
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename T>
void test_realview(const T&) {
  using Scalar = typename T::Scalar;
  using RealScalar = typename NumTraits<Scalar>::Real;

  constexpr Index minRows = T::RowsAtCompileTime == Dynamic ? 1 : T::RowsAtCompileTime;
  constexpr Index maxRows = T::MaxRowsAtCompileTime == Dynamic ? (EIGEN_TEST_MAX_SIZE / 2) : T::MaxRowsAtCompileTime;
  constexpr Index minCols = T::ColsAtCompileTime == Dynamic ? 1 : T::ColsAtCompileTime;
  constexpr Index maxCols = T::MaxColsAtCompileTime == Dynamic ? (EIGEN_TEST_MAX_SIZE / 2) : T::MaxColsAtCompileTime;

  constexpr Index rowFactor = (NumTraits<Scalar>::IsComplex && !T::IsRowMajor) ? 2 : 1;
  constexpr Index colFactor = (NumTraits<Scalar>::IsComplex && T::IsRowMajor) ? 2 : 1;
  constexpr Index sizeFactor = NumTraits<Scalar>::IsComplex ? 2 : 1;

  Index rows = internal::random<Index>(minRows, maxRows);
  Index cols = internal::random<Index>(minCols, maxCols);

  T A(rows, cols), B, C;

  VERIFY(A.realView().rows() == rowFactor * A.rows());
  VERIFY(A.realView().cols() == colFactor * A.cols());
  VERIFY(A.realView().size() == sizeFactor * A.size());

  RealScalar alpha = internal::random(RealScalar(1), RealScalar(2));
  A.setRandom();

  VERIFY_IS_APPROX(A.matrix().squaredNorm(), A.realView().matrix().squaredNorm());

  // test re-sizing realView during assignment
  B.realView() = A.realView();
  VERIFY_IS_APPROX(A, B);
  VERIFY_IS_APPROX(A.realView(), B.realView());

  // B = A * alpha
  for (Index r = 0; r < rows; r++) {
    for (Index c = 0; c < cols; c++) {
      B.coeffRef(r, c) = A.coeff(r, c) * Scalar(alpha);
    }
  }

  VERIFY_IS_APPROX(B.realView(), A.realView() * alpha);
  C = A;
  C.realView() *= alpha;
  VERIFY_IS_APPROX(B, C);

  alpha = internal::random(RealScalar(1), RealScalar(2));
  A.setRandom();

  // B = A / alpha
  for (Index r = 0; r < rows; r++) {
    for (Index c = 0; c < cols; c++) {
      B.coeffRef(r, c) = A.coeff(r, c) / Scalar(alpha);
    }
  }

  VERIFY_IS_APPROX(B.realView(), A.realView() / alpha);
  C = A;
  C.realView() /= alpha;
  VERIFY_IS_APPROX(B, C);
}

template <typename Scalar, int Rows, int Cols, int MaxRows = Rows, int MaxCols = Cols>
void test_realview_driver() {
  // if Rows == 1, don't test ColMajor as it is not a valid array
  using ColMajorMatrixType = Matrix<Scalar, Rows, Cols, Rows == 1 ? RowMajor : ColMajor, MaxRows, MaxCols>;
  using ColMajorArrayType = Array<Scalar, Rows, Cols, Rows == 1 ? RowMajor : ColMajor, MaxRows, MaxCols>;
  // if Cols == 1, don't test RowMajor as it is not a valid array
  using RowMajorMatrixType = Matrix<Scalar, Rows, Cols, Cols == 1 ? ColMajor : RowMajor, MaxRows, MaxCols>;
  using RowMajorArrayType = Array<Scalar, Rows, Cols, Cols == 1 ? ColMajor : RowMajor, MaxRows, MaxCols>;
  test_realview(ColMajorMatrixType());
  test_realview(ColMajorArrayType());
  test_realview(RowMajorMatrixType());
  test_realview(RowMajorArrayType());
}

template <int Rows, int Cols, int MaxRows = Rows, int MaxCols = Cols>
void test_realview_driver_complex() {
  test_realview_driver<float, Rows, Cols, MaxRows, MaxCols>();
  test_realview_driver<std::complex<float>, Rows, Cols, MaxRows, MaxCols>();
  test_realview_driver<double, Rows, Cols, MaxRows, MaxCols>();
  test_realview_driver<std::complex<double>, Rows, Cols, MaxRows, MaxCols>();
  test_realview_driver<long double, Rows, Cols, MaxRows, MaxCols>();
  test_realview_driver<std::complex<long double>, Rows, Cols, MaxRows, MaxCols>();
}

EIGEN_DECLARE_TEST(realview) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1((test_realview_driver_complex<Dynamic, Dynamic, Dynamic, Dynamic>()));
    CALL_SUBTEST_2((test_realview_driver_complex<Dynamic, Dynamic, 17, Dynamic>()));
    CALL_SUBTEST_3((test_realview_driver_complex<Dynamic, Dynamic, Dynamic, 19>()));
    CALL_SUBTEST_4((test_realview_driver_complex<Dynamic, Dynamic, 17, 19>()));
    CALL_SUBTEST_5((test_realview_driver_complex<17, Dynamic, 17, Dynamic>()));
    CALL_SUBTEST_6((test_realview_driver_complex<Dynamic, 19, Dynamic, 19>()));
    CALL_SUBTEST_7((test_realview_driver_complex<17, 19, 17, 19>()));
    CALL_SUBTEST_8((test_realview_driver_complex<Dynamic, 1>()));
    CALL_SUBTEST_9((test_realview_driver_complex<1, Dynamic>()));
    CALL_SUBTEST_10((test_realview_driver_complex<1, 1>()));
  }
}
