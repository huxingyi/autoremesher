// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2014 Benoit Steiner <benoit.steiner.goog@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

#include <Eigen/CXX11/Tensor>

using Eigen::RowMajor;
using Eigen::Tensor;

using Scalar = float;

using TypedLTOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LT, true>;
using TypedLEOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LE, true>;
using TypedGTOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GT, true>;
using TypedGEOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GE, true>;
using TypedEQOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_EQ, true>;
using TypedNEOp = internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_NEQ, true>;

static void test_orderings() {
  Tensor<Scalar, 3> mat1(2, 3, 7);
  Tensor<Scalar, 3> mat2(2, 3, 7);

  mat1.setRandom();
  mat2.setRandom();

  Tensor<bool, 3> lt(2, 3, 7);
  Tensor<bool, 3> le(2, 3, 7);
  Tensor<bool, 3> gt(2, 3, 7);
  Tensor<bool, 3> ge(2, 3, 7);

  Tensor<Scalar, 3> typed_lt(2, 3, 7);
  Tensor<Scalar, 3> typed_le(2, 3, 7);
  Tensor<Scalar, 3> typed_gt(2, 3, 7);
  Tensor<Scalar, 3> typed_ge(2, 3, 7);

  lt = mat1 < mat2;
  le = mat1 <= mat2;
  gt = mat1 > mat2;
  ge = mat1 >= mat2;

  typed_lt = mat1.binaryExpr(mat2, TypedLTOp());
  typed_le = mat1.binaryExpr(mat2, TypedLEOp());
  typed_gt = mat1.binaryExpr(mat2, TypedGTOp());
  typed_ge = mat1.binaryExpr(mat2, TypedGEOp());

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        VERIFY_IS_EQUAL(lt(i, j, k), mat1(i, j, k) < mat2(i, j, k));
        VERIFY_IS_EQUAL(le(i, j, k), mat1(i, j, k) <= mat2(i, j, k));
        VERIFY_IS_EQUAL(gt(i, j, k), mat1(i, j, k) > mat2(i, j, k));
        VERIFY_IS_EQUAL(ge(i, j, k), mat1(i, j, k) >= mat2(i, j, k));

        VERIFY_IS_EQUAL(lt(i, j, k), (bool)typed_lt(i, j, k));
        VERIFY_IS_EQUAL(le(i, j, k), (bool)typed_le(i, j, k));
        VERIFY_IS_EQUAL(gt(i, j, k), (bool)typed_gt(i, j, k));
        VERIFY_IS_EQUAL(ge(i, j, k), (bool)typed_ge(i, j, k));
      }
    }
  }
}

static void test_equality() {
  Tensor<Scalar, 3> mat1(2, 3, 7);
  Tensor<Scalar, 3> mat2(2, 3, 7);

  mat1.setRandom();
  mat2.setRandom();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        if (internal::random<bool>()) {
          mat2(i, j, k) = mat1(i, j, k);
        }
      }
    }
  }

  Tensor<bool, 3> eq(2, 3, 7);
  Tensor<bool, 3> ne(2, 3, 7);

  Tensor<Scalar, 3> typed_eq(2, 3, 7);
  Tensor<Scalar, 3> typed_ne(2, 3, 7);

  eq = (mat1 == mat2);
  ne = (mat1 != mat2);

  typed_eq = mat1.binaryExpr(mat2, TypedEQOp());
  typed_ne = mat1.binaryExpr(mat2, TypedNEOp());

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        VERIFY_IS_EQUAL(eq(i, j, k), mat1(i, j, k) == mat2(i, j, k));
        VERIFY_IS_EQUAL(ne(i, j, k), mat1(i, j, k) != mat2(i, j, k));

        VERIFY_IS_EQUAL(eq(i, j, k), (bool)typed_eq(i, j, k));
        VERIFY_IS_EQUAL(ne(i, j, k), (bool)typed_ne(i, j, k));
      }
    }
  }
}

static void test_isnan() {
  Tensor<Scalar, 3> mat(2, 3, 7);

  mat.setRandom();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        if (internal::random<bool>()) {
          mat(i, j, k) = std::numeric_limits<Scalar>::quiet_NaN();
        }
      }
    }
  }
  Tensor<bool, 3> nan(2, 3, 7);
  nan = (mat.isnan)();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        VERIFY_IS_EQUAL(nan(i, j, k), (std::isnan)(mat(i, j, k)));
      }
    }
  }
}

static void test_isinf() {
  Tensor<Scalar, 3> mat(2, 3, 7);

  mat.setRandom();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        if (internal::random<bool>()) {
          mat(i, j, k) = std::numeric_limits<Scalar>::infinity();
        }
      }
    }
  }
  Tensor<bool, 3> inf(2, 3, 7);
  inf = (mat.isinf)();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        VERIFY_IS_EQUAL(inf(i, j, k), (std::isinf)(mat(i, j, k)));
      }
    }
  }
}

static void test_isfinite() {
  Tensor<Scalar, 3> mat(2, 3, 7);

  mat.setRandom();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        if (internal::random<bool>()) {
          mat(i, j, k) = std::numeric_limits<Scalar>::infinity();
        }
        if (internal::random<bool>()) {
          mat(i, j, k) = std::numeric_limits<Scalar>::quiet_NaN();
        }
      }
    }
  }
  Tensor<bool, 3> inf(2, 3, 7);
  inf = (mat.isfinite)();
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 7; ++k) {
        VERIFY_IS_EQUAL(inf(i, j, k), (std::isfinite)(mat(i, j, k)));
      }
    }
  }
}

EIGEN_DECLARE_TEST(cxx11_tensor_comparisons) {
  CALL_SUBTEST(test_orderings());
  CALL_SUBTEST(test_equality());
  CALL_SUBTEST(test_isnan());
  CALL_SUBTEST(test_isinf());
  CALL_SUBTEST(test_isfinite());
}
