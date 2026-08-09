// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2011 Gael Guennebaud <g.gael@free.fr>
// Copyright (C) 2012 Kolja Brix <brix@igpm.rwth-aaachen.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "../../test/sparse_solver.h"
#include <Eigen/IterativeSolvers>

template <typename T>
void test_bicgstabl_T() {
  BiCGSTABL<SparseMatrix<T>, DiagonalPreconditioner<T> > bicgstabl_colmajor_diag;
  BiCGSTABL<SparseMatrix<T>, IncompleteLUT<T> > bicgstabl_colmajor_ilut;

  // This does not change the tolerance of the test, only the tolerance of the solver.
  bicgstabl_colmajor_diag.setTolerance(NumTraits<T>::epsilon() * 20);
  bicgstabl_colmajor_ilut.setTolerance(NumTraits<T>::epsilon() * 20);

  CALL_SUBTEST(check_sparse_square_solving(bicgstabl_colmajor_diag));
  CALL_SUBTEST(check_sparse_square_solving(bicgstabl_colmajor_ilut));
}

EIGEN_DECLARE_TEST(bicgstabl) {
  CALL_SUBTEST_1(test_bicgstabl_T<double>());
  CALL_SUBTEST_2(test_bicgstabl_T<std::complex<double> >());
}
