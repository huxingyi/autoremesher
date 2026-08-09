// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2025 Johannes Zipfel <johzip1010@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "sparse_solver.h"
#include <Eigen/IterativeLinearSolvers>

template <typename T, typename I_>
void test_incompleteLUT_T() {
  IncompleteLUT<T, I_> ilut;
  ilut.setDroptol(NumTraits<T>::epsilon() * 4);
}

template <typename T>
void test_extract_LU() {
  typedef Eigen::SparseMatrix<T> SparseMatrix;

  SparseMatrix A(5, 5);
  std::vector<Eigen::Triplet<T>> triplets;
  triplets.push_back({0, 0, 4});
  triplets.push_back({0, 1, -1});
  triplets.push_back({0, 4, -1});
  triplets.push_back({1, 0, -1});
  triplets.push_back({1, 1, 4});
  triplets.push_back({1, 2, -1});
  triplets.push_back({2, 1, -1});
  triplets.push_back({2, 2, 4});
  triplets.push_back({2, 3, -1});
  triplets.push_back({3, 2, -1});
  triplets.push_back({3, 3, 4});
  triplets.push_back({3, 4, -1});
  triplets.push_back({4, 0, -1});
  triplets.push_back({4, 3, -1});
  triplets.push_back({4, 4, 4});

  A.setFromTriplets(triplets.begin(), triplets.end());

  IncompleteLUT<T> ilut;
  ilut.compute(A);

  Eigen::SparseMatrix<T> matL = ilut.matrixL();  // Extract L
  Eigen::SparseMatrix<T> matU = ilut.matrixU();  // Extract U

  Eigen::SparseMatrix<T> expectedMatL(5, 5);
  std::vector<Eigen::Triplet<T>> tripletsExL;
  tripletsExL.emplace_back(0, 0, 1);
  tripletsExL.emplace_back(1, 0, -0.25);
  tripletsExL.emplace_back(1, 1, 1);
  tripletsExL.emplace_back(2, 0, -0.25);
  tripletsExL.emplace_back(2, 1, -0.0666667);
  tripletsExL.emplace_back(2, 2, 1);
  tripletsExL.emplace_back(3, 2, -0.25);
  tripletsExL.emplace_back(3, 3, 1);
  tripletsExL.emplace_back(4, 1, -0.266667);
  tripletsExL.emplace_back(4, 3, -0.266667);
  tripletsExL.emplace_back(4, 4, 1);
  expectedMatL.setFromTriplets(tripletsExL.begin(), tripletsExL.end());

  Eigen::SparseMatrix<T> expectedMatU(5, 5);
  std::vector<Eigen::Triplet<T>> tripletsExU;
  tripletsExU.emplace_back(0, 0, 4);
  tripletsExU.emplace_back(0, 1, -1);
  tripletsExU.emplace_back(1, 1, 3.75);
  tripletsExU.emplace_back(1, 4, -1);
  tripletsExU.emplace_back(2, 2, 4);
  tripletsExU.emplace_back(2, 3, -1);
  tripletsExU.emplace_back(3, 3, 3.75);
  tripletsExU.emplace_back(3, 4, -1);
  tripletsExU.emplace_back(4, 4, 3.46667);
  expectedMatU.setFromTriplets(tripletsExU.begin(), tripletsExU.end());

  VERIFY_IS_APPROX(expectedMatL, matL);
  VERIFY_IS_APPROX(expectedMatU, matU);
}

EIGEN_DECLARE_TEST(incomplete_LUT) {
  CALL_SUBTEST_1((test_incompleteLUT_T<double, int>()));
  CALL_SUBTEST_1((test_incompleteLUT_T<float, int>()));
  CALL_SUBTEST_2((test_incompleteLUT_T<std::complex<double>, int>()));
  CALL_SUBTEST_3((test_incompleteLUT_T<double, long int>()));

  CALL_SUBTEST_4(test_extract_LU<double>());
  CALL_SUBTEST_4(test_extract_LU<float>());
}
