// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2014 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// discard stack allocation as that too bypasses malloc
#define EIGEN_STACK_ALLOCATION_LIMIT 0
#define EIGEN_RUNTIME_NO_MALLOC
#include "main.h"
#include <Eigen/SVD>

#define SVD_DEFAULT(M) JacobiSVD<M>
#define SVD_FOR_MIN_NORM(M) JacobiSVD<M, ColPivHouseholderQRPreconditioner>
#define SVD_STATIC_OPTIONS(M, O) JacobiSVD<M, O>
#include "svd_common.h"

template <typename MatrixType>
void jacobisvd_method() {
  enum { Size = MatrixType::RowsAtCompileTime };
  typedef typename MatrixType::RealScalar RealScalar;
  typedef Matrix<RealScalar, Size, 1> RealVecType;
  MatrixType m = MatrixType::Identity();
  VERIFY_IS_APPROX(m.jacobiSvd().singularValues(), RealVecType::Ones());
  VERIFY_RAISES_ASSERT(m.jacobiSvd().matrixU());
  VERIFY_RAISES_ASSERT(m.jacobiSvd().matrixV());
  VERIFY_IS_APPROX(m.template jacobiSvd<ComputeFullU | ComputeFullV>().solve(m), m);
  VERIFY_IS_APPROX(m.template jacobiSvd<ComputeFullU | ComputeFullV>().transpose().solve(m), m);
  VERIFY_IS_APPROX(m.template jacobiSvd<ComputeFullU | ComputeFullV>().adjoint().solve(m), m);
}

template <typename MatrixType>
void jacobisvd_thin_options(const MatrixType& input = MatrixType()) {
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);

  svd_thin_option_checks<MatrixType, 0>(m);
  svd_thin_option_checks<MatrixType, ColPivHouseholderQRPreconditioner>(m);
  svd_thin_option_checks<MatrixType, HouseholderQRPreconditioner>(m);
}

template <typename MatrixType>
void jacobisvd_full_options(const MatrixType& input = MatrixType()) {
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);

  svd_option_checks_full_only<MatrixType, 0>(m);
  svd_option_checks_full_only<MatrixType, ColPivHouseholderQRPreconditioner>(m);
  svd_option_checks_full_only<MatrixType, HouseholderQRPreconditioner>(m);
  svd_option_checks_full_only<MatrixType, FullPivHouseholderQRPreconditioner>(
      m);  // FullPiv only used when computing full unitaries
}

template <typename MatrixType>
void jacobisvd_verify_assert(const MatrixType& input = MatrixType()) {
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);
  svd_verify_assert<MatrixType, 0>(m);
  svd_verify_assert<MatrixType, ColPivHouseholderQRPreconditioner>(m);
  svd_verify_assert<MatrixType, HouseholderQRPreconditioner>(m);
  svd_verify_assert_full_only<MatrixType, FullPivHouseholderQRPreconditioner>(m);

  svd_verify_constructor_options_assert<JacobiSVD<MatrixType>>(m);
  svd_verify_constructor_options_assert<JacobiSVD<MatrixType, ColPivHouseholderQRPreconditioner>>(m);
  svd_verify_constructor_options_assert<JacobiSVD<MatrixType, HouseholderQRPreconditioner>>(m);
  svd_verify_constructor_options_assert<JacobiSVD<MatrixType, FullPivHouseholderQRPreconditioner>>(m);
}

template <typename MatrixType>
void jacobisvd_verify_inputs(const MatrixType& input = MatrixType()) {
  // check defaults
  typedef JacobiSVD<MatrixType> DefaultSVD;
  MatrixType m(input.rows(), input.cols());
  svd_fill_random(m);
  DefaultSVD defaultSvd(m);
  VERIFY((int)DefaultSVD::QRPreconditioner == (int)ColPivHouseholderQRPreconditioner);
  VERIFY(!defaultSvd.computeU());
  VERIFY(!defaultSvd.computeV());

  // ColPivHouseholderQR is always default in presence of other options.
  VERIFY(((int)JacobiSVD<MatrixType, ComputeThinU>::QRPreconditioner == (int)ColPivHouseholderQRPreconditioner));
  VERIFY(((int)JacobiSVD<MatrixType, ComputeThinV>::QRPreconditioner == (int)ColPivHouseholderQRPreconditioner));
  VERIFY(((int)JacobiSVD<MatrixType, ComputeThinU | ComputeThinV>::QRPreconditioner ==
          (int)ColPivHouseholderQRPreconditioner));
  VERIFY(((int)JacobiSVD<MatrixType, ComputeFullU | ComputeFullV>::QRPreconditioner ==
          (int)ColPivHouseholderQRPreconditioner));
  VERIFY(((int)JacobiSVD<MatrixType, ComputeThinU | ComputeFullV>::QRPreconditioner ==
          (int)ColPivHouseholderQRPreconditioner));
  VERIFY(((int)JacobiSVD<MatrixType, ComputeFullU | ComputeThinV>::QRPreconditioner ==
          (int)ColPivHouseholderQRPreconditioner));
}

template <typename MatrixType>
void svd_triangular_matrix(const MatrixType& input = MatrixType()) {
  MatrixType matrix(input.rows(), input.cols());
  svd_fill_random(matrix);
  // Make sure that we only consider the 'Lower' part of the matrix.
  MatrixType matrix_self_adj = matrix.template selfadjointView<Lower>().toDenseMatrix();

  JacobiSVD<MatrixType, ComputeFullV> svd_triangular(matrix.template selfadjointView<Lower>());
  JacobiSVD<MatrixType, ComputeFullV> svd_full(matrix_self_adj);

  VERIFY_IS_APPROX(svd_triangular.singularValues(), svd_full.singularValues());
}

namespace Foo {
// older compiler require a default constructor for Bar
// cf: https://stackoverflow.com/questions/7411515/
class Bar {
 public:
  Bar() {}
};
bool operator<(const Bar&, const Bar&) { return true; }
}  // namespace Foo
// regression test for a very strange MSVC issue for which simply
// including SVDBase.h messes up with std::max and custom scalar type
void msvc_workaround() {
  const Foo::Bar a;
  const Foo::Bar b;
  const Foo::Bar c = std::max EIGEN_NOT_A_MACRO(a, b);
  EIGEN_UNUSED_VARIABLE(c)
}

EIGEN_DECLARE_TEST(jacobisvd) {
  CALL_SUBTEST_1((jacobisvd_verify_inputs<Matrix4d>()));
  CALL_SUBTEST_2((jacobisvd_verify_inputs(Matrix<float, 5, Dynamic>(5, 6))));
  CALL_SUBTEST_3((jacobisvd_verify_inputs<Matrix<std::complex<double>, 7, 5>>()));

  CALL_SUBTEST_4((jacobisvd_verify_assert<Matrix3f>()));
  CALL_SUBTEST_5((jacobisvd_verify_assert<Matrix4d>()));
  CALL_SUBTEST_6((jacobisvd_verify_assert<Matrix<float, 10, 12>>()));
  CALL_SUBTEST_7((jacobisvd_verify_assert<Matrix<float, 12, 10>>()));
  CALL_SUBTEST_8((jacobisvd_verify_assert<MatrixXf>(MatrixXf(10, 12))));
  CALL_SUBTEST_9((jacobisvd_verify_assert<MatrixXcd>(MatrixXcd(7, 5))));

  CALL_SUBTEST_10(svd_all_trivial_2x2(jacobisvd_thin_options<Matrix2cd>));
  CALL_SUBTEST_11(svd_all_trivial_2x2(jacobisvd_thin_options<Matrix2d>));

  for (int i = 0; i < g_repeat; i++) {
    int r = internal::random<int>(1, 30), c = internal::random<int>(1, 30);

    TEST_SET_BUT_UNUSED_VARIABLE(r)
    TEST_SET_BUT_UNUSED_VARIABLE(c)

    CALL_SUBTEST_12((jacobisvd_thin_options<Matrix3f>()));
    CALL_SUBTEST_13((jacobisvd_full_options<Matrix3f>()));
    CALL_SUBTEST_14((jacobisvd_thin_options<Matrix4d>()));
    CALL_SUBTEST_15((jacobisvd_full_options<Matrix4d>()));
    CALL_SUBTEST_16((jacobisvd_thin_options<Matrix<float, 2, 3>>()));
    CALL_SUBTEST_17((jacobisvd_full_options<Matrix<float, 2, 3>>()));
    CALL_SUBTEST_18((jacobisvd_thin_options<Matrix<double, 4, 7>>()));
    CALL_SUBTEST_19((jacobisvd_full_options<Matrix<double, 4, 7>>()));
    CALL_SUBTEST_20((jacobisvd_thin_options<Matrix<double, 7, 4>>()));
    CALL_SUBTEST_21((jacobisvd_full_options<Matrix<double, 7, 4>>()));
    CALL_SUBTEST_22((jacobisvd_thin_options<Matrix<double, Dynamic, 5>>(Matrix<double, Dynamic, 5>(r, 5))));
    CALL_SUBTEST_23((jacobisvd_full_options<Matrix<double, Dynamic, 5>>(Matrix<double, Dynamic, 5>(r, 5))));
    CALL_SUBTEST_24((jacobisvd_thin_options<Matrix<double, 5, Dynamic>>(Matrix<double, 5, Dynamic>(5, c))));
    CALL_SUBTEST_25((jacobisvd_full_options<Matrix<double, 5, Dynamic>>(Matrix<double, 5, Dynamic>(5, c))));
    CALL_SUBTEST_26((jacobisvd_thin_options<MatrixXf>(MatrixXf(r, c))));
    CALL_SUBTEST_27((jacobisvd_full_options<MatrixXf>(MatrixXf(r, c))));
    CALL_SUBTEST_28((jacobisvd_thin_options<MatrixXcd>(MatrixXcd(r, c))));
    CALL_SUBTEST_29((jacobisvd_full_options<MatrixXcd>(MatrixXcd(r, c))));
    CALL_SUBTEST_30((jacobisvd_thin_options<MatrixXd>(MatrixXd(r, c))));
    CALL_SUBTEST_31((jacobisvd_full_options<MatrixXd>(MatrixXd(r, c))));
    CALL_SUBTEST_32((jacobisvd_thin_options<Matrix<double, 5, 7, RowMajor>>()));
    CALL_SUBTEST_33((jacobisvd_full_options<Matrix<double, 5, 7, RowMajor>>()));
    CALL_SUBTEST_34((jacobisvd_thin_options<Matrix<double, 7, 5, RowMajor>>()));
    CALL_SUBTEST_35((jacobisvd_full_options<Matrix<double, 7, 5, RowMajor>>()));

    MatrixXcd noQRTest = MatrixXcd(r, r);
    svd_fill_random(noQRTest);
    CALL_SUBTEST_36((svd_thin_option_checks<MatrixXcd, NoQRPreconditioner>(noQRTest)));
    CALL_SUBTEST_36((svd_option_checks_full_only<MatrixXcd, NoQRPreconditioner>(noQRTest)));

    CALL_SUBTEST_37((
        svd_check_max_size_matrix<Matrix<float, Dynamic, Dynamic, ColMajor, 13, 15>, ColPivHouseholderQRPreconditioner>(
            r, c)));
    CALL_SUBTEST_38(
        (svd_check_max_size_matrix<Matrix<float, Dynamic, Dynamic, ColMajor, 15, 13>, HouseholderQRPreconditioner>(r,
                                                                                                                   c)));
    CALL_SUBTEST_39((
        svd_check_max_size_matrix<Matrix<float, Dynamic, Dynamic, RowMajor, 13, 15>, ColPivHouseholderQRPreconditioner>(
            r, c)));
    CALL_SUBTEST_40(
        (svd_check_max_size_matrix<Matrix<float, Dynamic, Dynamic, RowMajor, 15, 13>, HouseholderQRPreconditioner>(r,
                                                                                                                   c)));

    // Test on inf/nan matrix
    CALL_SUBTEST_41((svd_inf_nan<MatrixXf>()));
    CALL_SUBTEST_42((svd_inf_nan<MatrixXd>()));

    CALL_SUBTEST_43((jacobisvd_verify_assert<Matrix<double, 6, 1>>()));
    CALL_SUBTEST_44((jacobisvd_verify_assert<Matrix<double, 1, 6>>()));
    CALL_SUBTEST_45((jacobisvd_verify_assert<Matrix<double, Dynamic, 1>>(Matrix<double, Dynamic, 1>(r))));
    CALL_SUBTEST_46((jacobisvd_verify_assert<Matrix<double, 1, Dynamic>>(Matrix<double, 1, Dynamic>(c))));
  }

  CALL_SUBTEST_47((jacobisvd_thin_options<MatrixXd>(
      MatrixXd(internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 2),
               internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 2)))));
  CALL_SUBTEST_48((jacobisvd_full_options<MatrixXd>(
      MatrixXd(internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 2),
               internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 2)))));
  CALL_SUBTEST_49((jacobisvd_thin_options<MatrixXcd>(
      MatrixXcd(internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 3),
                internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 3)))));
  CALL_SUBTEST_50((jacobisvd_full_options<MatrixXcd>(
      MatrixXcd(internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 3),
                internal::random<int>(EIGEN_TEST_MAX_SIZE / 4, EIGEN_TEST_MAX_SIZE / 3)))));

  // test matrixbase method
  CALL_SUBTEST_51((jacobisvd_method<Matrix2cd>()));
  CALL_SUBTEST_52((jacobisvd_method<Matrix3f>()));

  // Test problem size constructors
  CALL_SUBTEST_53(JacobiSVD<MatrixXf>(10, 10));

  // Check that preallocation avoids subsequent mallocs
  CALL_SUBTEST_54(svd_preallocate<void>());

  CALL_SUBTEST_55(svd_underoverflow<void>());

  // Check that the TriangularBase constructor works
  CALL_SUBTEST_56((svd_triangular_matrix<Matrix3d>()));
  CALL_SUBTEST_57((svd_triangular_matrix<Matrix4f>()));
  CALL_SUBTEST_58((svd_triangular_matrix<Matrix<double, 10, 10>>()));

  msvc_workaround();
}
