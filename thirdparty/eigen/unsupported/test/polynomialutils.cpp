// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2010 Manuel Yguel <manuel.yguel@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"
#include <unsupported/Eigen/Polynomials>
#include <iostream>

using namespace std;

namespace Eigen {
namespace internal {
template <int Size>
struct increment_if_fixed_size {
  enum { ret = (Size == Dynamic) ? Dynamic : Size + 1 };
};
}  // namespace internal
}  // namespace Eigen

template <typename Scalar_, int Deg_>
void realRoots_to_monicPolynomial_test(int deg) {
  typedef internal::increment_if_fixed_size<Deg_> Dim;
  typedef Matrix<Scalar_, Dim::ret, 1> PolynomialType;
  typedef Matrix<Scalar_, Deg_, 1> EvalRootsType;

  PolynomialType pols(deg + 1);
  EvalRootsType roots = EvalRootsType::Random(deg);
  roots_to_monicPolynomial(roots, pols);

  EvalRootsType evr(deg);
  for (int i = 0; i < roots.size(); ++i) {
    evr[i] = std::abs(poly_eval(pols, roots[i]));
  }

  bool evalToZero = evr.isZero(test_precision<Scalar_>());
  if (!evalToZero) {
    cerr << evr.transpose() << endl;
  }
  VERIFY(evalToZero);
}

template <typename Scalar_>
void realRoots_to_monicPolynomial_scalar() {
  CALL_SUBTEST_2((realRoots_to_monicPolynomial_test<Scalar_, 2>(2)));
  CALL_SUBTEST_3((realRoots_to_monicPolynomial_test<Scalar_, 3>(3)));
  CALL_SUBTEST_4((realRoots_to_monicPolynomial_test<Scalar_, 4>(4)));
  CALL_SUBTEST_5((realRoots_to_monicPolynomial_test<Scalar_, 5>(5)));
  CALL_SUBTEST_6((realRoots_to_monicPolynomial_test<Scalar_, 6>(6)));
  CALL_SUBTEST_7((realRoots_to_monicPolynomial_test<Scalar_, 7>(7)));
  CALL_SUBTEST_8((realRoots_to_monicPolynomial_test<Scalar_, 17>(17)));

  CALL_SUBTEST_9((realRoots_to_monicPolynomial_test<Scalar_, Dynamic>(internal::random<int>(18, 26))));
}

template <typename Scalar_, int Deg_>
void CauchyBounds(int deg) {
  typedef internal::increment_if_fixed_size<Deg_> Dim;
  typedef Matrix<Scalar_, Dim::ret, 1> PolynomialType;
  typedef Matrix<Scalar_, Deg_, 1> EvalRootsType;

  PolynomialType pols(deg + 1);
  EvalRootsType roots = EvalRootsType::Random(deg);
  roots_to_monicPolynomial(roots, pols);
  Scalar_ M = cauchy_max_bound(pols);
  Scalar_ m = cauchy_min_bound(pols);
  Scalar_ Max = roots.array().abs().maxCoeff();
  Scalar_ min = roots.array().abs().minCoeff();
  bool eval = (M >= Max) && (m <= min);
  if (!eval) {
    cerr << "Roots: " << roots << endl;
    cerr << "Bounds: (" << m << ", " << M << ")" << endl;
    cerr << "Min,Max: (" << min << ", " << Max << ")" << endl;
  }
  VERIFY(eval);
}

template <typename Scalar_>
void CauchyBounds_scalar() {
  CALL_SUBTEST_2((CauchyBounds<Scalar_, 2>(2)));
  CALL_SUBTEST_3((CauchyBounds<Scalar_, 3>(3)));
  CALL_SUBTEST_4((CauchyBounds<Scalar_, 4>(4)));
  CALL_SUBTEST_5((CauchyBounds<Scalar_, 5>(5)));
  CALL_SUBTEST_6((CauchyBounds<Scalar_, 6>(6)));
  CALL_SUBTEST_7((CauchyBounds<Scalar_, 7>(7)));
  CALL_SUBTEST_8((CauchyBounds<Scalar_, 17>(17)));

  CALL_SUBTEST_9((CauchyBounds<Scalar_, Dynamic>(internal::random<int>(18, 26))));
}

EIGEN_DECLARE_TEST(polynomialutils) {
  for (int i = 0; i < g_repeat; i++) {
    realRoots_to_monicPolynomial_scalar<double>();
    realRoots_to_monicPolynomial_scalar<float>();
    CauchyBounds_scalar<double>();
    CauchyBounds_scalar<float>();
  }
}
