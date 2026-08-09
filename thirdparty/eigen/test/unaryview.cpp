// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2021 Andrew Johnson <andrew.johnson@arjohnsonau.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <int OuterStride, int InnerStride, typename VectorType>
void unaryview_stride(const VectorType& m) {
  typedef typename VectorType::Scalar Scalar;
  Index rows = m.rows();
  Index cols = m.cols();
  VectorType vec = VectorType::Random(rows, cols);

  struct view_op {
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const Scalar& operator()(const Scalar& v) const { return v; }
  };

  CwiseUnaryView<view_op, VectorType, Stride<OuterStride, InnerStride>> vec_view(vec);
  VERIFY(vec_view.outerStride() == (OuterStride == 0 ? 0 : OuterStride));
  VERIFY(vec_view.innerStride() == (InnerStride == 0 ? 1 : InnerStride));
}

void test_mutable_unaryview() {
  struct Vec3 {
    double x;
    double y;
    double z;
  };

  Eigen::Vector<Vec3, 3> m;
  auto x_view = m.unaryViewExpr([](Vec3& v) -> double& { return v.x; });
  auto y_view = m.unaryViewExpr([](Vec3& v) -> double& { return v.y; });
  auto z_view = m.unaryViewExpr([](Vec3& v) -> double& { return v.z; });

  x_view.setConstant(1);
  y_view.setConstant(2);
  z_view.setConstant(3);

  for (int i = 0; i < m.size(); ++i) {
    VERIFY_IS_EQUAL(m(i).x, 1);
    VERIFY_IS_EQUAL(m(i).y, 2);
    VERIFY_IS_EQUAL(m(i).z, 3);
  }
}

void test_unaryview_solve() {
  // Random upper-triangular system.
  Eigen::MatrixXd A = Eigen::MatrixXd::Random(5, 5);
  A.triangularView<Eigen::Lower>().setZero();
  A.diagonal().setRandom();
  Eigen::VectorXd b = Eigen::VectorXd::Random(5);

  struct trivial_view_op {
    double& operator()(double& x) const { return x; }
    const double& operator()(const double& x) const { return x; }
  };

  // Non-const view:
  {
    auto b_view = b.unaryViewExpr(trivial_view_op());
    b_view(0) = 1;  // Allows modification.
    Eigen::VectorXd x = A.triangularView<Eigen::Upper>().solve(b_view);
    VERIFY_IS_APPROX(A * x, b);
  }

  // Const view:
  {
    const auto b_view = b.unaryViewExpr(trivial_view_op());
    Eigen::VectorXd x = A.triangularView<Eigen::Upper>().solve(b_view);
    VERIFY_IS_APPROX(A * x, b);
  }

  // Non-const view of const matrix:
  {
    const Eigen::VectorXd const_b = b;
    auto b_view = const_b.unaryViewExpr(trivial_view_op());
    Eigen::VectorXd x = A.triangularView<Eigen::Upper>().solve(b_view);
    VERIFY_IS_APPROX(A * x, b);
  }

  // Const view of const matrix:
  {
    const Eigen::VectorXd const_b = b;
    const auto b_view = const_b.unaryViewExpr(trivial_view_op());
    Eigen::VectorXd x = A.triangularView<Eigen::Upper>().solve(b_view);
    VERIFY_IS_APPROX(A * x, b);
  }

  // Eigen::MatrixXd out =
  //       mat_in.real()
  //             .triangularView<Eigen::Upper>()
  //             .solve(mat_in.unaryViewExpr([&](const auto& x){ return std::real(x); }));
}

EIGEN_DECLARE_TEST(unaryviewstride) {
  CALL_SUBTEST_1((unaryview_stride<1, 2>(MatrixXf())));
  CALL_SUBTEST_1((unaryview_stride<0, 0>(MatrixXf())));
  CALL_SUBTEST_2((unaryview_stride<1, 2>(VectorXf())));
  CALL_SUBTEST_2((unaryview_stride<0, 0>(VectorXf())));
  CALL_SUBTEST_3((unaryview_stride<1, 2>(RowVectorXf())));
  CALL_SUBTEST_3((unaryview_stride<0, 0>(RowVectorXf())));
  CALL_SUBTEST_4(test_mutable_unaryview());
  CALL_SUBTEST_4(test_unaryview_solve());
}
