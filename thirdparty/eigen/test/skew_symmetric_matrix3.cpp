// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"
#include <Eigen/LU>

namespace {
template <typename Scalar>
void constructors() {
  typedef Matrix<Scalar, 3, 1> Vector;
  const Vector v = Vector::Random();
  // l-value
  const SkewSymmetricMatrix3<Scalar> s1(v);
  const Vector& v1 = s1.vector();
  VERIFY_IS_APPROX(v1, v);
  VERIFY(s1.cols() == 3);
  VERIFY(s1.rows() == 3);

  // r-value
  const SkewSymmetricMatrix3<Scalar> s2(std::move(v));
  VERIFY_IS_APPROX(v1, s2.vector());
  VERIFY_IS_APPROX(s1.toDenseMatrix(), s2.toDenseMatrix());

  // from scalars
  SkewSymmetricMatrix3<Scalar> s4(v1(0), v1(1), v1(2));
  VERIFY_IS_APPROX(v1, s4.vector());

  // constructors with four vectors do not compile
  // Matrix<Scalar, 4, 1> vector4 = Matrix<Scalar, 4, 1>::Random();
  // SkewSymmetricMatrix3<Scalar> s5(vector4);
}

template <typename Scalar>
void assignments() {
  typedef Matrix<Scalar, 3, 1> Vector;
  typedef Matrix<Scalar, 3, 3> SquareMatrix;

  const Vector v = Vector::Random();

  // assign to square matrix
  SquareMatrix sq;
  sq = v.asSkewSymmetric();
  VERIFY(sq.isSkewSymmetric());

  // assign to skew symmetric matrix
  SkewSymmetricMatrix3<Scalar> sk;
  sk = v.asSkewSymmetric();
  VERIFY_IS_APPROX(v, sk.vector());
}

template <typename Scalar>
void plusMinus() {
  typedef Matrix<Scalar, 3, 1> Vector;
  typedef Matrix<Scalar, 3, 3> SquareMatrix;

  const Vector v1 = Vector::Random();
  const Vector v2 = Vector::Random();

  SquareMatrix sq1;
  sq1 = v1.asSkewSymmetric();
  SquareMatrix sq2;
  sq2 = v2.asSkewSymmetric();

  SkewSymmetricMatrix3<Scalar> sk1;
  sk1 = v1.asSkewSymmetric();
  SkewSymmetricMatrix3<Scalar> sk2;
  sk2 = v2.asSkewSymmetric();

  VERIFY_IS_APPROX((sk1 + sk2).toDenseMatrix(), sq1 + sq2);
  VERIFY_IS_APPROX((sk1 - sk2).toDenseMatrix(), sq1 - sq2);

  SquareMatrix sq3 = v1.asSkewSymmetric();
  VERIFY_IS_APPROX(sq3 = v1.asSkewSymmetric() + v2.asSkewSymmetric(), sq1 + sq2);
  VERIFY_IS_APPROX(sq3 = v1.asSkewSymmetric() - v2.asSkewSymmetric(), sq1 - sq2);
  VERIFY_IS_APPROX(sq3 = v1.asSkewSymmetric() - 2 * v2.asSkewSymmetric() + v1.asSkewSymmetric(), sq1 - 2 * sq2 + sq1);

  VERIFY_IS_APPROX((sk1 + sk1).vector(), 2 * v1);
  VERIFY((sk1 - sk1).vector().isZero());
  VERIFY((sk1 - sk1).toDenseMatrix().isZero());
}

template <typename Scalar>
void multiplyScale() {
  typedef Matrix<Scalar, 3, 1> Vector;
  typedef Matrix<Scalar, 3, 3> SquareMatrix;

  const Vector v1 = Vector::Random();
  SquareMatrix sq1;
  sq1 = v1.asSkewSymmetric();
  SkewSymmetricMatrix3<Scalar> sk1;
  sk1 = v1.asSkewSymmetric();

  const Scalar s1 = internal::random<Scalar>();
  VERIFY_IS_APPROX(SkewSymmetricMatrix3<Scalar>(sk1 * s1).vector(), sk1.vector() * s1);
  VERIFY_IS_APPROX(SkewSymmetricMatrix3<Scalar>(s1 * sk1).vector(), s1 * sk1.vector());
  VERIFY_IS_APPROX(sq1 * (sk1 * s1), (sq1 * sk1) * s1);

  const Vector v2 = Vector::Random();
  SquareMatrix sq2;
  sq2 = v2.asSkewSymmetric();
  SkewSymmetricMatrix3<Scalar> sk2;
  sk2 = v2.asSkewSymmetric();
  VERIFY_IS_APPROX(sk1 * sk2, sq1 * sq2);

  // null space
  VERIFY((sk1 * v1).isZero());
  VERIFY((sk2 * v2).isZero());
}

template <typename Matrix>
void skewSymmetricMultiplication(const Matrix& m) {
  typedef Eigen::Matrix<typename Matrix::Scalar, 3, 1> Vector;
  const Vector v = Vector::Random();
  const Matrix m1 = Matrix::Random(m.rows(), m.cols());
  const SkewSymmetricMatrix3<typename Matrix::Scalar> sk = v.asSkewSymmetric();
  VERIFY_IS_APPROX(m1.transpose() * (sk * m1), (m1.transpose() * sk) * m1);
  VERIFY((m1.transpose() * (sk * m1)).isSkewSymmetric());
}

template <typename Scalar>
void traceAndDet() {
  typedef Matrix<Scalar, 3, 1> Vector;
  const Vector v = Vector::Random();
  // this does not work, values larger than 1.e-08 can be seen
  // VERIFY_IS_APPROX(sq.determinant(), static_cast<Scalar>(0));
  VERIFY_IS_APPROX(v.asSkewSymmetric().determinant(), static_cast<Scalar>(0));
  VERIFY_IS_APPROX(v.asSkewSymmetric().toDenseMatrix().trace(), static_cast<Scalar>(0));
}

template <typename Scalar>
void transpose() {
  typedef Matrix<Scalar, 3, 1> Vector;
  const Vector v = Vector::Random();
  // By definition of a skew symmetric matrix: A^T = -A
  VERIFY_IS_APPROX(v.asSkewSymmetric().toDenseMatrix().transpose(), v.asSkewSymmetric().transpose().toDenseMatrix());
  VERIFY_IS_APPROX(v.asSkewSymmetric().transpose().vector(), (-v).asSkewSymmetric().vector());
}

template <typename Scalar>
void exponentialIdentity() {
  typedef Matrix<Scalar, 3, 1> Vector;
  const Vector v1 = Vector::Zero();
  VERIFY(v1.asSkewSymmetric().exponential().isIdentity());

  Vector v2 = Vector::Random();
  v2.normalize();
  VERIFY((2 * EIGEN_PI * v2).asSkewSymmetric().exponential().isIdentity());

  Vector v3;
  const auto precision = static_cast<Scalar>(1.1) * NumTraits<Scalar>::dummy_precision();
  v3 << 0, 0, precision;
  VERIFY(v3.asSkewSymmetric().exponential().isIdentity(precision));
}

template <typename Scalar>
void exponentialOrthogonality() {
  typedef Matrix<Scalar, 3, 1> Vector;
  typedef Matrix<Scalar, 3, 3> SquareMatrix;
  const Vector v = Vector::Random();
  SquareMatrix sq = v.asSkewSymmetric().exponential();
  VERIFY(sq.isUnitary());
}

template <typename Scalar>
void exponentialRotation() {
  typedef Matrix<Scalar, 3, 1> Vector;
  typedef Matrix<Scalar, 3, 3> SquareMatrix;

  // rotation axis is invariant
  const Vector v1 = Vector::Random();
  const SquareMatrix r1 = v1.asSkewSymmetric().exponential();
  VERIFY_IS_APPROX(r1 * v1, v1);

  // rotate around z-axis
  Vector v2;
  v2 << 0, 0, Scalar(EIGEN_PI);
  const SquareMatrix r2 = v2.asSkewSymmetric().exponential();
  VERIFY_IS_APPROX(r2 * (Vector() << 1, 0, 0).finished(), (Vector() << -1, 0, 0).finished());
  VERIFY_IS_APPROX(r2 * (Vector() << 0, 1, 0).finished(), (Vector() << 0, -1, 0).finished());
}

}  // namespace

EIGEN_DECLARE_TEST(skew_symmetric_matrix3) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(constructors<float>());
    CALL_SUBTEST_1(constructors<double>());
    CALL_SUBTEST_1(assignments<float>());
    CALL_SUBTEST_1(assignments<double>());

    CALL_SUBTEST_2(plusMinus<float>());
    CALL_SUBTEST_2(plusMinus<double>());
    CALL_SUBTEST_2(multiplyScale<float>());
    CALL_SUBTEST_2(multiplyScale<double>());
    CALL_SUBTEST_2(skewSymmetricMultiplication(MatrixXf(3, internal::random<int>(3, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_2(skewSymmetricMultiplication(MatrixXd(3, internal::random<int>(3, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_2(traceAndDet<float>());
    CALL_SUBTEST_2(traceAndDet<double>());
    CALL_SUBTEST_2(transpose<float>());
    CALL_SUBTEST_2(transpose<double>());

    CALL_SUBTEST_3(exponentialIdentity<float>());
    CALL_SUBTEST_3(exponentialIdentity<double>());
    CALL_SUBTEST_3(exponentialOrthogonality<float>());
    CALL_SUBTEST_3(exponentialOrthogonality<double>());
    CALL_SUBTEST_3(exponentialRotation<float>());
    CALL_SUBTEST_3(exponentialRotation<double>());
  }
}
