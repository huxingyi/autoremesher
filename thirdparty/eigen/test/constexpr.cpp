// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2022 Alex Richardson <alexrichardson@google.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TESTING_CONSTEXPR
#include "main.h"

template <typename Scalar, int Rows>
struct ConstexprTest {
  constexpr ConstexprTest(const Matrix<Scalar, Rows, Rows>& B) { A = B; }

  Matrix<Scalar, Rows, Rows> A;
};

EIGEN_DECLARE_TEST(constexpr) {
  // Clang accepts (some of) this code when using C++14/C++17, but GCC does not like
  // the fact that `T array[Size]` inside Eigen::internal::plain_array is not initialized
  // until after the constructor returns:
  // error: member ‘Eigen::internal::plain_array<int, 9, 0, 0>::array’ must be initialized by mem-initializer in
  // ‘constexpr’ constructor
#if __cpp_constexpr >= 201907L
  constexpr Matrix3i mat({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
  VERIFY_IS_EQUAL(mat.size(), 9);
  static_assert(mat(0, 0) == 1);
  static_assert(mat(0) == 1);
  static_assert(mat.coeff(0, 1) == 2);
  constexpr Array33i arr({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
  static_assert(arr(0, 0) == 1);
  static_assert(arr(0) == 1);
  VERIFY_IS_EQUAL(arr.size(), 9);
  static_assert(arr.coeff(0, 1) == 2);
  constexpr RowVector3i vec{{1, 2, 3}};
  static_assert(vec(0, 0) == 1);
  static_assert(vec[0] == 1);
  VERIFY_IS_EQUAL(vec.size(), 3);
  static_assert(vec.coeff(0, 1) == 2);

  // Check assignment. A wrapper struct is used to avoid copy ellision.
  constexpr ConstexprTest<double, 2> obj1(Matrix2d({{1, 2}, {3, 4}}));
  VERIFY_IS_EQUAL(obj1.A.size(), 4);
  static_assert(obj1.A(0, 0) == 1);
  static_assert(obj1.A(0) == 1);
  static_assert(obj1.A.coeff(0, 1) == 2);
  constexpr ConstexprTest<double, 3> obj2(Matrix3d({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}));
  VERIFY_IS_EQUAL(obj2.A.size(), 9);
  static_assert(obj2.A(0, 0) == 1);
  static_assert(obj2.A(0) == 1);
  static_assert(obj2.A.coeff(0, 1) == 2);

  // Also check dynamic size arrays/matrices with fixed-size storage (currently
  // only works if all elements are initialized, since otherwise the compiler
  // complains about uninitialized trailing elements.
  constexpr Matrix<int, Eigen::Dynamic, Eigen::Dynamic, 0, 3, 3> dyn_mat({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
  VERIFY_IS_EQUAL(dyn_mat.size(), 9);
  static_assert(dyn_mat(0, 0) == 1);
  static_assert(dyn_mat.coeff(0, 1) == 2);
  constexpr Array<int, Eigen::Dynamic, Eigen::Dynamic, 0, 3, 3> dyn_arr({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
  static_assert(dyn_arr(0, 0) == 1);
  static_assert(dyn_arr(0) == 1);
  VERIFY_IS_EQUAL(dyn_arr.size(), 9);
  static_assert(dyn_arr.coeff(0, 1) == 2);
#endif  // __cpp_constexpr >= 201907L
}

// Check that we can use the std::initializer_list constructor for constexpr variables.
#if __cpp_constexpr >= 201907L
// EIGEN_MAKE_UNALIGNED_ARRAY_ASSERT() will fail constexpr evaluation unless
// we have std::is_constant_evaluated().
constexpr Matrix<int, 2, 2> global_mat({{1, 2}, {3, 4}});

EIGEN_DECLARE_TEST(constexpr_global) {
  VERIFY_IS_EQUAL(global_mat.size(), 4);
  static_assert(global_mat(0, 0) == 1);
  static_assert(global_mat(0) == 1);
  static_assert(global_mat.coeff(0, 0) == 1);
}
#endif  // __cpp_constexpr >= 201907L
