// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2011 Gael Guennebaud <g.gael@free.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "sparse_solver.h"
#include <Eigen/IterativeLinearSolvers>

template <typename T, typename I_>
void test_bicgstab_T() {
  BiCGSTAB<SparseMatrix<T, 0, I_>, DiagonalPreconditioner<T> > bicgstab_colmajor_diag;
  BiCGSTAB<SparseMatrix<T, 0, I_>, IdentityPreconditioner> bicgstab_colmajor_I;
  BiCGSTAB<SparseMatrix<T, 0, I_>, IncompleteLUT<T, I_> > bicgstab_colmajor_ilut;
  // BiCGSTAB<SparseMatrix<T>, SSORPreconditioner<T> >     bicgstab_colmajor_ssor;

  bicgstab_colmajor_diag.setTolerance(NumTraits<T>::epsilon() * 4);
  bicgstab_colmajor_ilut.setTolerance(NumTraits<T>::epsilon() * 4);

  CALL_SUBTEST(check_sparse_square_solving(bicgstab_colmajor_diag));
  //   CALL_SUBTEST( check_sparse_square_solving(bicgstab_colmajor_I)     );
  CALL_SUBTEST(check_sparse_square_solving(bicgstab_colmajor_ilut));
  // CALL_SUBTEST( check_sparse_square_solving(bicgstab_colmajor_ssor)     );
}

// https://gitlab.com/libeigen/eigen/-/issues/2856
void test_2856() {
  Eigen::MatrixXd D = Eigen::MatrixXd::Identity(14, 14);
  D(6, 13) = 1;
  D(13, 12) = 1;
  using CSRMatrix = Eigen::SparseMatrix<double, Eigen::RowMajor>;
  CSRMatrix A = D.sparseView();

  Eigen::VectorXd b = Eigen::VectorXd::Zero(14);
  b(12) = -1001;

  Eigen::BiCGSTAB<CSRMatrix> solver;
  solver.compute(A);
  Eigen::VectorXd x = solver.solve(b);
  Eigen::VectorXd expected = Eigen::VectorXd::Zero(14);
  expected(6) = -1001;
  expected(12) = -1001;
  expected(13) = 1001;
  VERIFY_IS_EQUAL(x, expected);

  Eigen::VectorXd residual = b - A * x;
  VERIFY(residual.isZero());
}

// https://gitlab.com/libeigen/eigen/-/issues/2899
void test_2899() {
  Eigen::MatrixXd A = Eigen::MatrixXd::Zero(4, 4);
  A(0, 0) = 1;
  A(1, 0) = -1.0 / 6;
  A(1, 1) = 2.0 / 3;
  A(1, 2) = -1.0 / 6;
  A(1, 3) = -1.0 / 3;
  A(2, 1) = -1.0 / 3;
  A(2, 2) = 1;
  A(2, 3) = -2.0 / 3;
  A(3, 1) = -1.0 / 3;
  A(3, 2) = -1.0 / 3;
  A(3, 3) = 2.0 / 3;
  Eigen::VectorXd b = Eigen::VectorXd::Zero(4);
  b(0) = 0;
  b(1) = 1;
  b(2) = 1;
  b(3) = 1;
  Eigen::BiCGSTAB<Eigen::MatrixXd> solver;
  solver.compute(A);
  Eigen::VectorXd x = solver.solve(b);
  Eigen::VectorXd expected(4);
  expected << 0, 15, 18, 18;
  VERIFY_IS_APPROX(x, expected);
  Eigen::VectorXd residual = b - A * x;
  VERIFY(residual.isZero());
}

EIGEN_DECLARE_TEST(bicgstab) {
  CALL_SUBTEST_1((test_bicgstab_T<double, int>()));
  CALL_SUBTEST_2((test_bicgstab_T<std::complex<double>, int>()));
  CALL_SUBTEST_3((test_bicgstab_T<double, long int>()));
  CALL_SUBTEST_4(test_2856());
  CALL_SUBTEST_5(test_2899());
}
