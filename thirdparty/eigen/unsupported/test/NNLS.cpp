// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) Essex Edwards <essex.edwards@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_RUNTIME_NO_MALLOC

#include "main.h"
#include <unsupported/Eigen/NNLS>

/// Check that 'x' solves the NNLS optimization problem `min ||A*x-b|| s.t. 0 <= x`.
/// The \p tolerance parameter is the absolute tolerance on the gradient, A'*(A*x-b).
template <typename MatrixType, typename VectorB, typename VectorX, typename Scalar>
void verify_nnls_optimality(const MatrixType &A, const VectorB &b, const VectorX &x, const Scalar tolerance) {
  // The NNLS optimality conditions are:
  //
  // * 0 = A'*A*x - A'*b - lambda
  // * 0 <= x[i] \forall i
  // * 0 <= lambda[i] \forall i
  // * 0 = x[i]*lambda[i] \forall i
  //
  // we don't know lambda, but by assuming the first optimality condition is true,
  // we can derive it and then check the others conditions.
  const VectorX lambda = A.transpose() * (A * x - b);

  // NNLS solutions are EXACTLY not negative.
  VERIFY_LE(0, x.minCoeff());

  // Exact lambda would be non-negative, but computed lambda might leak a little
  VERIFY_LE(-tolerance, lambda.minCoeff());

  // x[i]*lambda[i] == 0 <~~> (x[i]==0) || (lambda[i] is small)
  VERIFY(((x.array() == Scalar(0)) || (lambda.array() <= tolerance)).all());
}

template <typename MatrixType, typename VectorB, typename VectorX>
void test_nnls_known_solution(const MatrixType &A, const VectorB &b, const VectorX &x_expected) {
  using Scalar = typename MatrixType::Scalar;

  using std::sqrt;
  const Scalar tolerance = sqrt(Eigen::GenericNumTraits<Scalar>::epsilon());
  Index max_iter = 5 * A.cols();  // A heuristic guess.
  NNLS<MatrixType> nnls(A, max_iter, tolerance);
  const VectorX x = nnls.solve(b);

  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY_IS_APPROX(x, x_expected);
  verify_nnls_optimality(A, b, x, tolerance);
}

template <typename MatrixType>
void test_nnls_random_problem(const MatrixType &) {
  //
  // SETUP
  //

  Index cols = MatrixType::ColsAtCompileTime;
  if (cols == Dynamic) cols = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  Index rows = MatrixType::RowsAtCompileTime;
  if (rows == Dynamic) rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
  VERIFY_LE(cols, rows);  // To have a unique LS solution: cols <= rows.

  // Make some sort of random test problem from a wide range of scales and condition numbers.
  using std::pow;
  using Scalar = typename MatrixType::Scalar;
  const Scalar sqrtConditionNumber = pow(Scalar(10), internal::random<Scalar>(Scalar(0), Scalar(2)));
  const Scalar scaleA = pow(Scalar(10), internal::random<Scalar>(Scalar(-3), Scalar(3)));
  const Scalar minSingularValue = scaleA / sqrtConditionNumber;
  const Scalar maxSingularValue = scaleA * sqrtConditionNumber;
  MatrixType A(rows, cols);
  generateRandomMatrixSvs(setupRangeSvs<Matrix<Scalar, Dynamic, 1>>(cols, minSingularValue, maxSingularValue), rows,
                          cols, A);

  // Make a random RHS also with a random scaling.
  using VectorB = decltype(A.col(0).eval());
  const Scalar scaleB = pow(Scalar(10), internal::random<Scalar>(Scalar(-3), Scalar(3)));
  const VectorB b = scaleB * VectorB::Random(A.rows());

  //
  // ACT
  //

  using Scalar = typename MatrixType::Scalar;
  using std::sqrt;
  const Scalar tolerance =
      sqrt(Eigen::GenericNumTraits<Scalar>::epsilon()) * b.cwiseAbs().maxCoeff() * A.cwiseAbs().maxCoeff();
  Index max_iter = 5 * A.cols();  // A heuristic guess.
  NNLS<MatrixType> nnls(A, max_iter, tolerance);
  const typename NNLS<MatrixType>::SolutionVectorType &x = nnls.solve(b);

  //
  // VERIFY
  //

  // In fact, NNLS can fail on some problems, but they are rare in practice.
  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  verify_nnls_optimality(A, b, x, tolerance);
}

void test_nnls_handles_zero_rhs() {
  //
  // SETUP
  //
  const Index cols = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  const Index rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A = MatrixXd::Random(rows, cols);
  const VectorXd b = VectorXd::Zero(rows);

  //
  // ACT
  //
  NNLS<MatrixXd> nnls(A);
  const VectorXd x = nnls.solve(b);

  //
  // VERIFY
  //
  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY_LE(nnls.iterations(), 1);  // 0 or 1 would be be fine for an edge case like this.
  VERIFY_IS_EQUAL(x, VectorXd::Zero(cols));
}

void test_nnls_handles_Mx0_matrix() {
  //
  // SETUP
  //
  const Index rows = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A(rows, 0);
  const VectorXd b = VectorXd::Random(rows);

  //
  // ACT
  //
  NNLS<MatrixXd> nnls(A);
  const VectorXd x = nnls.solve(b);

  //
  // VERIFY
  //
  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY_LE(nnls.iterations(), 0);
  VERIFY_IS_EQUAL(x.size(), 0);
}

void test_nnls_handles_0x0_matrix() {
  //
  // SETUP
  //
  const MatrixXd A(0, 0);
  const VectorXd b(0);

  //
  // ACT
  //
  NNLS<MatrixXd> nnls(A);
  const VectorXd x = nnls.solve(b);

  //
  // VERIFY
  //
  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY_LE(nnls.iterations(), 0);
  VERIFY_IS_EQUAL(x.size(), 0);
}

void test_nnls_handles_dependent_columns() {
  //
  // SETUP
  //
  const Index rank = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE / 2);
  const Index cols = 2 * rank;
  const Index rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A = MatrixXd::Random(rows, rank) * MatrixXd::Random(rank, cols);
  const VectorXd b = VectorXd::Random(rows);

  //
  // ACT
  //
  const double tolerance = 1e-8;
  NNLS<MatrixXd> nnls(A);
  const VectorXd &x = nnls.solve(b);

  //
  // VERIFY
  //
  // What should happen when the input 'A' has dependent columns?
  // We might still succeed. Or we might not converge.
  // Either outcome is fine. If Success is indicated,
  // then 'x' must actually be a solution vector.

  if (nnls.info() == ComputationInfo::Success) {
    verify_nnls_optimality(A, b, x, tolerance);
  }
}

void test_nnls_handles_wide_matrix() {
  //
  // SETUP
  //
  const Index cols = internal::random<Index>(2, EIGEN_TEST_MAX_SIZE);
  const Index rows = internal::random<Index>(2, cols - 1);
  const MatrixXd A = MatrixXd::Random(rows, cols);
  const VectorXd b = VectorXd::Random(rows);

  //
  // ACT
  //
  const double tolerance = 1e-8;
  NNLS<MatrixXd> nnls(A);
  const VectorXd &x = nnls.solve(b);

  //
  // VERIFY
  //
  // What should happen when the input 'A' is wide?
  // The unconstrained least-squares problem has infinitely many solutions.
  // Subject the the non-negativity constraints,
  // the solution might actually be unique (e.g. it is [0,0,..,0]).
  // So, NNLS might succeed or it might fail.
  // Either outcome is fine. If Success is indicated,
  // then 'x' must actually be a solution vector.

  if (nnls.info() == ComputationInfo::Success) {
    verify_nnls_optimality(A, b, x, tolerance);
  }
}

// 4x2 problem, unconstrained solution positive
void test_nnls_known_1() {
  Matrix<double, 4, 2> A(4, 2);
  Matrix<double, 4, 1> b(4);
  Matrix<double, 2, 1> x(2);
  A << 1, 1, 2, 4, 3, 9, 4, 16;
  b << 0.6, 2.2, 4.8, 8.4;
  x << 0.1, 0.5;

  return test_nnls_known_solution(A, b, x);
}

// 4x3 problem, unconstrained solution positive
void test_nnls_known_2() {
  Matrix<double, 4, 3> A(4, 3);
  Matrix<double, 4, 1> b(4);
  Matrix<double, 3, 1> x(3);

  A << 1, 1, 1, 2, 4, 8, 3, 9, 27, 4, 16, 64;
  b << 0.73, 3.24, 8.31, 16.72;
  x << 0.1, 0.5, 0.13;

  test_nnls_known_solution(A, b, x);
}

// Simple 4x4 problem, unconstrained solution non-negative
void test_nnls_known_3() {
  Matrix<double, 4, 4> A(4, 4);
  Matrix<double, 4, 1> b(4);
  Matrix<double, 4, 1> x(4);

  A << 1, 1, 1, 1, 2, 4, 8, 16, 3, 9, 27, 81, 4, 16, 64, 256;
  b << 0.73, 3.24, 8.31, 16.72;
  x << 0.1, 0.5, 0.13, 0;

  test_nnls_known_solution(A, b, x);
}

// Simple 4x3 problem, unconstrained solution non-negative
void test_nnls_known_4() {
  Matrix<double, 4, 3> A(4, 3);
  Matrix<double, 4, 1> b(4);
  Matrix<double, 3, 1> x(3);

  A << 1, 1, 1, 2, 4, 8, 3, 9, 27, 4, 16, 64;
  b << 0.23, 1.24, 3.81, 8.72;
  x << 0.1, 0, 0.13;

  test_nnls_known_solution(A, b, x);
}

// Simple 4x3 problem, unconstrained solution indefinite
void test_nnls_known_5() {
  Matrix<double, 4, 3> A(4, 3);
  Matrix<double, 4, 1> b(4);
  Matrix<double, 3, 1> x(3);

  A << 1, 1, 1, 2, 4, 8, 3, 9, 27, 4, 16, 64;
  b << 0.13, 0.84, 2.91, 7.12;
  // Solution obtained by original nnls() implementation in Fortran
  x << 0.0, 0.0, 0.1106544;

  test_nnls_known_solution(A, b, x);
}

void test_nnls_small_reference_problems() {
  test_nnls_known_1();
  test_nnls_known_2();
  test_nnls_known_3();
  test_nnls_known_4();
  test_nnls_known_5();
}

void test_nnls_with_half_precision() {
  // The random matrix generation tools don't work with `half`,
  // so here's a simpler setup mostly just to check that NNLS compiles & runs with custom scalar types.

  using Mat = Matrix<half, 8, 2>;
  using VecB = Matrix<half, 8, 1>;
  using VecX = Matrix<half, 2, 1>;
  Mat A = Mat::Random();  // full-column rank with high probability.
  VecB b = VecB::Random();

  NNLS<Mat> nnls(A, 20, half(1e-2f));
  const VecX x = nnls.solve(b);

  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  verify_nnls_optimality(A, b, x, half(1e-1));
}

void test_nnls_special_case_solves_in_zero_iterations() {
  // The particular NNLS algorithm that is implemented starts with all variables
  // in the active set.
  // This test builds a system where all constraints are active at the solution,
  // so that initial guess is already correct.
  //
  // If the implementation changes to another algorithm that does not have this property,
  // then this test will need to change (e.g. starting from all constraints inactive,
  // or using ADMM, or an interior point solver).

  const Index n = 10;
  const Index m = 3 * n;
  const VectorXd b = VectorXd::Random(m);
  // With high probability, this is full column rank, which we need for uniqueness.
  MatrixXd A = MatrixXd::Random(m, n);
  // Make every column of `A` such that adding it to the active set only /increases/ the objective,
  // this ensuring the NNLS solution is all zeros.
  const VectorXd alignment = -(A.transpose() * b).cwiseSign();
  A = A * alignment.asDiagonal();

  NNLS<MatrixXd> nnls(A);
  nnls.solve(b);

  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY(nnls.iterations() == 0);
}

void test_nnls_special_case_solves_in_n_iterations() {
  // The particular NNLS algorithm that is implemented starts with all variables
  // in the active set and then adds one variable to the inactive set each iteration.
  // This test builds a system where all variables are inactive at the solution,
  // so it should take 'n' iterations to get there.
  //
  // If the implementation changes to another algorithm that does not have this property,
  // then this test will need to change (e.g. starting from all constraints inactive,
  // or using ADMM, or an interior point solver).

  const Index n = 10;
  const Index m = 3 * n;
  // With high probability, this is full column rank, which we need for uniqueness.
  const MatrixXd A = MatrixXd::Random(m, n);
  const VectorXd x = VectorXd::Random(n).cwiseAbs().array() + 1;  // all positive.
  const VectorXd b = A * x;

  NNLS<MatrixXd> nnls(A);
  nnls.solve(b);

  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
  VERIFY(nnls.iterations() == n);
}

void test_nnls_returns_NoConvergence_when_maxIterations_is_too_low() {
  // Using the special case that takes `n` iterations,
  // from `test_nnls_special_case_solves_in_n_iterations`,
  // we can set max iterations too low and that should cause the solve to fail.

  const Index n = 10;
  const Index m = 3 * n;
  // With high probability, this is full column rank, which we need for uniqueness.
  const MatrixXd A = MatrixXd::Random(m, n);
  const VectorXd x = VectorXd::Random(n).cwiseAbs().array() + 1;  // all positive.
  const VectorXd b = A * x;

  NNLS<MatrixXd> nnls(A);
  const Index max_iters = n - 1;
  nnls.setMaxIterations(max_iters);
  nnls.solve(b);

  VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::NoConvergence);
  VERIFY(nnls.iterations() == max_iters);
}

void test_nnls_default_maxIterations_is_twice_column_count() {
  const Index cols = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  const Index rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A = MatrixXd::Random(rows, cols);

  NNLS<MatrixXd> nnls(A);

  VERIFY_IS_EQUAL(nnls.maxIterations(), 2 * cols);
}

void test_nnls_does_not_allocate_during_solve() {
  const Index cols = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  const Index rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A = MatrixXd::Random(rows, cols);
  const VectorXd b = VectorXd::Random(rows);

  NNLS<MatrixXd> nnls(A);

  internal::set_is_malloc_allowed(false);
  nnls.solve(b);
  internal::set_is_malloc_allowed(true);
}

void test_nnls_repeated_calls_to_compute_and_solve() {
  const Index cols2 = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  const Index rows2 = internal::random<Index>(cols2, EIGEN_TEST_MAX_SIZE);
  const MatrixXd A2 = MatrixXd::Random(rows2, cols2);
  const VectorXd b2 = VectorXd::Random(rows2);

  NNLS<MatrixXd> nnls;

  for (int i = 0; i < 4; ++i) {
    const Index cols = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
    const Index rows = internal::random<Index>(cols, EIGEN_TEST_MAX_SIZE);
    const MatrixXd A = MatrixXd::Random(rows, cols);

    nnls.compute(A);
    VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);

    for (int j = 0; j < 3; ++j) {
      const VectorXd b = VectorXd::Random(rows);
      const VectorXd x = nnls.solve(b);
      VERIFY_IS_EQUAL(nnls.info(), ComputationInfo::Success);
      verify_nnls_optimality(A, b, x, 1e-4);
    }
  }
}

EIGEN_DECLARE_TEST(NNLS) {
  // Small matrices with known solutions:
  CALL_SUBTEST_1(test_nnls_small_reference_problems());
  CALL_SUBTEST_1(test_nnls_handles_Mx0_matrix());
  CALL_SUBTEST_1(test_nnls_handles_0x0_matrix());

  for (int i = 0; i < g_repeat; i++) {
    // Essential NNLS properties, across different types.
    CALL_SUBTEST_2(test_nnls_random_problem(MatrixXf()));
    CALL_SUBTEST_3(test_nnls_random_problem(MatrixXd()));
    CALL_SUBTEST_4(test_nnls_random_problem(Matrix<double, 12, 5>()));
    CALL_SUBTEST_5(test_nnls_with_half_precision());

    // Robustness tests:
    CALL_SUBTEST_6(test_nnls_handles_zero_rhs());
    CALL_SUBTEST_6(test_nnls_handles_dependent_columns());
    CALL_SUBTEST_6(test_nnls_handles_wide_matrix());

    // Properties specific to the implementation,
    // not NNLS in general.
    CALL_SUBTEST_7(test_nnls_special_case_solves_in_zero_iterations());
    CALL_SUBTEST_7(test_nnls_special_case_solves_in_n_iterations());
    CALL_SUBTEST_7(test_nnls_returns_NoConvergence_when_maxIterations_is_too_low());
    CALL_SUBTEST_7(test_nnls_default_maxIterations_is_twice_column_count());
    CALL_SUBTEST_8(test_nnls_repeated_calls_to_compute_and_solve());

    // This test fails. It hits allocations in HouseholderSequence.h
    // test_nnls_does_not_allocate_during_solve();
  }
}
