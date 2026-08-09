// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2012 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2023 Juraj Oršulić, University of Zagreb <juraj.orsulic@fer.hr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Silence warnings about using the deprecated non-canonical .eulerAngles(), which are still being tested.
#define EIGEN_NO_DEPRECATED_WARNING

#include "main.h"
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/SVD>

template <typename Scalar>
void verify_euler(const Matrix<Scalar, 3, 1>& ea, int i, int j, int k) {
  typedef Matrix<Scalar, 3, 3> Matrix3;
  typedef Matrix<Scalar, 3, 1> Vector3;
  typedef AngleAxis<Scalar> AngleAxisx;
  const Matrix3 m(AngleAxisx(ea[0], Vector3::Unit(i)) * AngleAxisx(ea[1], Vector3::Unit(j)) *
                  AngleAxisx(ea[2], Vector3::Unit(k)));
  const Scalar kPi = Scalar(EIGEN_PI);

  // Test non-canonical eulerAngles
  {
    Vector3 eabis = m.eulerAngles(i, j, k);
    Matrix3 mbis(AngleAxisx(eabis[0], Vector3::Unit(i)) * AngleAxisx(eabis[1], Vector3::Unit(j)) *
                 AngleAxisx(eabis[2], Vector3::Unit(k)));
    VERIFY_IS_APPROX(m, mbis);

    // approx_or_less_than does not work for 0
    VERIFY(0 < eabis[0] || test_isMuchSmallerThan(eabis[0], Scalar(1)));
    VERIFY_IS_APPROX_OR_LESS_THAN(eabis[0], kPi);
    VERIFY_IS_APPROX_OR_LESS_THAN(-kPi, eabis[1]);
    VERIFY_IS_APPROX_OR_LESS_THAN(eabis[1], kPi);
    VERIFY_IS_APPROX_OR_LESS_THAN(-kPi, eabis[2]);
    VERIFY_IS_APPROX_OR_LESS_THAN(eabis[2], kPi);
  }

  // Test canonicalEulerAngles
  {
    Vector3 eabis = m.canonicalEulerAngles(i, j, k);
    Matrix3 mbis(AngleAxisx(eabis[0], Vector3::Unit(i)) * AngleAxisx(eabis[1], Vector3::Unit(j)) *
                 AngleAxisx(eabis[2], Vector3::Unit(k)));
    VERIFY_IS_APPROX(m, mbis);

    VERIFY_IS_APPROX_OR_LESS_THAN(-kPi, eabis[0]);
    VERIFY_IS_APPROX_OR_LESS_THAN(eabis[0], kPi);
    if (i != k) {
      // Tait-Bryan sequence
      VERIFY_IS_APPROX_OR_LESS_THAN(-Scalar(kPi / 2), eabis[1]);
      VERIFY_IS_APPROX_OR_LESS_THAN(eabis[1], Scalar(kPi / 2));
    } else {
      // Proper Euler sequence
      // approx_or_less_than does not work for 0
      VERIFY(0 < eabis[1] || test_isMuchSmallerThan(eabis[1], Scalar(1)));
      VERIFY_IS_APPROX_OR_LESS_THAN(eabis[1], kPi);
    }
    VERIFY_IS_APPROX_OR_LESS_THAN(-kPi, eabis[2]);
    VERIFY_IS_APPROX_OR_LESS_THAN(eabis[2], kPi);
  }
}

template <typename Scalar>
void check_all_var(const Matrix<Scalar, 3, 1>& ea) {
  auto verify_permutation = [](const Matrix<Scalar, 3, 1>& eap) {
    verify_euler(eap, 0, 1, 2);
    verify_euler(eap, 0, 1, 0);
    verify_euler(eap, 0, 2, 1);
    verify_euler(eap, 0, 2, 0);

    verify_euler(eap, 1, 2, 0);
    verify_euler(eap, 1, 2, 1);
    verify_euler(eap, 1, 0, 2);
    verify_euler(eap, 1, 0, 1);

    verify_euler(eap, 2, 0, 1);
    verify_euler(eap, 2, 0, 2);
    verify_euler(eap, 2, 1, 0);
    verify_euler(eap, 2, 1, 2);
  };

  int i, j, k;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      for (k = 0; k < 3; k++) {
        Matrix<Scalar, 3, 1> eap(ea(i), ea(j), ea(k));
        verify_permutation(eap);
      }
}

template <typename Scalar>
void eulerangles() {
  typedef Matrix<Scalar, 3, 3> Matrix3;
  typedef Matrix<Scalar, 3, 1> Vector3;
  typedef Array<Scalar, 3, 1> Array3;
  typedef Quaternion<Scalar> Quaternionx;
  typedef AngleAxis<Scalar> AngleAxisx;

  const Scalar kPi = Scalar(EIGEN_PI);
  const Scalar smallVal = static_cast<Scalar>(0.001);

  Scalar a = internal::random<Scalar>(-kPi, kPi);
  Quaternionx q1;
  q1 = AngleAxisx(a, Vector3::Random().normalized());
  Matrix3 m;
  m = q1;

  Vector3 ea = m.eulerAngles(0, 1, 2);
  check_all_var(ea);
  ea = m.eulerAngles(0, 1, 0);
  check_all_var(ea);

  // Check with purely random Quaternion:
  q1.coeffs() = Quaternionx::Coefficients::Random().normalized();
  m = q1;
  ea = m.eulerAngles(0, 1, 2);
  check_all_var(ea);
  ea = m.eulerAngles(0, 1, 0);
  check_all_var(ea);

  // Check with random angles in range [-pi:pi]x[-pi:pi]x[-pi:pi].
  ea = Array3::Random() * kPi;
  check_all_var(ea);

  auto test_with_some_zeros = [=](const Vector3& eaz) {
    check_all_var(eaz);
    Vector3 ea_glz = eaz;
    ea_glz[0] = Scalar(0);
    check_all_var(ea_glz);
    ea_glz[0] = internal::random<Scalar>(-smallVal, smallVal);
    check_all_var(ea_glz);
    ea_glz[2] = Scalar(0);
    check_all_var(ea_glz);
    ea_glz[2] = internal::random<Scalar>(-smallVal, smallVal);
    check_all_var(ea_glz);
  };
  // Check gimbal lock configurations and a bit noisy gimbal locks
  Vector3 ea_gl = ea;
  ea_gl[1] = kPi / 2;
  test_with_some_zeros(ea_gl);
  ea_gl[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_gl[1] = -kPi / 2;
  test_with_some_zeros(ea_gl);
  ea_gl[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_gl[1] = kPi / 2;
  ea_gl[2] = ea_gl[0];
  test_with_some_zeros(ea_gl);
  ea_gl[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_gl[1] = -kPi / 2;
  test_with_some_zeros(ea_gl);
  ea_gl[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);

  // Similar to above, but with pi instead of pi/2
  Vector3 ea_pi = ea;
  ea_pi[1] = kPi;
  test_with_some_zeros(ea_gl);
  ea_pi[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_pi[1] = -kPi;
  test_with_some_zeros(ea_gl);
  ea_pi[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_pi[1] = kPi;
  ea_pi[2] = ea_pi[0];
  test_with_some_zeros(ea_gl);
  ea_pi[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);
  ea_pi[1] = -kPi;
  test_with_some_zeros(ea_gl);
  ea_pi[1] += internal::random<Scalar>(-smallVal, smallVal);
  test_with_some_zeros(ea_gl);

  ea[2] = ea[0] = internal::random<Scalar>(0, kPi);
  check_all_var(ea);

  ea[0] = ea[1] = internal::random<Scalar>(0, kPi);
  check_all_var(ea);

  ea[1] = 0;
  check_all_var(ea);

  ea.head(2).setZero();
  check_all_var(ea);

  ea.setZero();
  check_all_var(ea);
}

EIGEN_DECLARE_TEST(geo_eulerangles) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(eulerangles<float>());
    CALL_SUBTEST_2(eulerangles<double>());
  }
}
