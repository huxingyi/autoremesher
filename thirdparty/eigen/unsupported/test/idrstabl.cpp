// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2011 Gael Guennebaud <g.gael@free.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "../../test/sparse_solver.h"
#include <unsupported/Eigen/IterativeSolvers>

template <typename T>
void test_idrstabl_T() {
  IDRSTABL<SparseMatrix<T>, DiagonalPreconditioner<T> > idrstabl_colmajor_diag;
  IDRSTABL<SparseMatrix<T>, IncompleteLUT<T> > idrstabl_colmajor_ilut;

  idrstabl_colmajor_diag.setTolerance(NumTraits<T>::epsilon() * 4);
  idrstabl_colmajor_ilut.setTolerance(NumTraits<T>::epsilon() * 4);

  CALL_SUBTEST(check_sparse_square_solving(idrstabl_colmajor_diag));
  CALL_SUBTEST(check_sparse_square_solving(idrstabl_colmajor_ilut));
}

EIGEN_DECLARE_TEST(idrstabl) {
  CALL_SUBTEST_1((test_idrstabl_T<double>()));
  CALL_SUBTEST_2((test_idrstabl_T<std::complex<double> >()));
}
