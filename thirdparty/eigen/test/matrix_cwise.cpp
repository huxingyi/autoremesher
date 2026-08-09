// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <vector>
#include "main.h"

template <typename MatrixType, typename NewScalar>
struct matrix_of {
  using type = MatrixType;
};

template <typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols, typename NewScalar>
struct matrix_of<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>, NewScalar> {
  using type = Eigen::Matrix<NewScalar, Rows, Cols, Options, MaxRows, MaxCols>;
};

// Unary function reference.
template <typename MatrixType, typename Func,
          typename OutMatrixType = typename matrix_of<
              MatrixType, typename Eigen::internal::result_of<Func(typename MatrixType::Scalar)>::type>::type>
OutMatrixType cwise_ref(const MatrixType& m, Func f = Func()) {
  OutMatrixType out(m.rows(), m.cols());
  for (Eigen::Index r = 0; r < m.rows(); ++r) {
    for (Eigen::Index c = 0; c < m.cols(); ++c) {
      out(r, c) = f(m(r, c));
    }
  }
  return out;
}

// Binary function reference.
template <typename MatrixType, typename Func,
          typename OutMatrixType = typename matrix_of<
              MatrixType, typename Eigen::internal::result_of<Func(typename MatrixType::Scalar,
                                                                   typename MatrixType::Scalar)>::type>::type>
OutMatrixType cwise_ref(const MatrixType& m1, const MatrixType& m2, Func f = Func()) {
  OutMatrixType out(m1.rows(), m1.cols());
  for (Eigen::Index r = 0; r < m1.rows(); ++r) {
    for (Eigen::Index c = 0; c < m1.cols(); ++c) {
      out(r, c) = f(m1(r, c), m2(r, c));
    }
  }
  return out;
}

template <typename MatrixType>
void test_cwise_real(const MatrixType& m) {
  using Scalar = typename MatrixType::Scalar;
  Index rows = m.rows();
  Index cols = m.cols();
  MatrixType m1 = MatrixType::Random(rows, cols);
  MatrixType m2, m3, m4;

  // Supported unary ops.
  VERIFY_IS_CWISE_APPROX(m1.cwiseAbs(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::abs(x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseSign(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::sign(x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseCbrt(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::cbrt(x); }));
  // For integers, avoid division by zero.
  m2 = m1;
  if (Eigen::NumTraits<Scalar>::IsInteger) {
    m2 = m1.unaryExpr([](const Scalar& x) { return Eigen::numext::equal_strict(x, Scalar(0)) ? Scalar(1) : x; });
  }
  VERIFY_IS_CWISE_APPROX(m2.cwiseInverse(), cwise_ref(m2, [](const Scalar& x) { return Scalar(Scalar(1) / x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseArg(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::arg(x); }));
  // Only take sqrt of positive values.
  m2 = m1.cwiseAbs();
  VERIFY_IS_CWISE_APPROX(m2.cwiseSqrt(), cwise_ref(m2, [](const Scalar& x) { return Eigen::numext::sqrt(x); }));
  // Only find Square/Abs2 of +/- sqrt values so we don't overflow.
  m2 = m2.cwiseSqrt().array() * m1.cwiseSign().array();
  VERIFY_IS_CWISE_APPROX(m2.cwiseAbs2(), cwise_ref(m2, [](const Scalar& x) { return Eigen::numext::abs2(x); }));
  VERIFY_IS_CWISE_APPROX(m2.cwiseSquare(), cwise_ref(m2, [](const Scalar& x) { return Scalar(x * x); }));
  VERIFY_IS_CWISE_APPROX(m2.cwisePow(Scalar(2)),
                         cwise_ref(m2, [](const Scalar& x) { return Eigen::numext::pow(x, Scalar(2)); }));

  // Supported binary ops.
  m1.setRandom(rows, cols);
  m2.setRandom(rows, cols);
  VERIFY_IS_CWISE_EQUAL(m1.cwiseMin(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateFast>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateNaN>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateNumbers>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.cwiseMax(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateFast>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateNaN>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateNumbers>(m2),
                        cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  // Scalar comparison.
  Scalar mean = Eigen::NumTraits<Scalar>::highest() / Scalar(2) + Eigen::NumTraits<Scalar>::lowest() / Scalar(2);
  m4.setConstant(rows, cols, mean);
  VERIFY_IS_CWISE_EQUAL(m1.cwiseMin(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateFast>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateNaN>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMin<PropagateNumbers>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::mini(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.cwiseMax(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateFast>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateNaN>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  VERIFY_IS_CWISE_EQUAL(m1.template cwiseMax<PropagateNumbers>(mean),
                        cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Eigen::numext::maxi(x, y); }));
  // For products, avoid integer overflow by limiting the input < sqrt(max).
  m3 = m1;
  m4 = m2;
  if (Eigen::NumTraits<Scalar>::IsInteger) {
    const Scalar kMax = Eigen::numext::sqrt(Eigen::NumTraits<Scalar>::highest());
    m3 = m1 - ((m1 / kMax) * kMax);
    m4 = m2 - ((m2 / kMax) * kMax);
  }
  VERIFY_IS_CWISE_APPROX(m3.cwiseProduct(m4),
                         cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return Scalar(x * y); }));
  // For quotients involving integers, avoid division by zero.
  m4 = m2;
  if (Eigen::NumTraits<Scalar>::IsInteger) {
    m4 = m2.unaryExpr([](const Scalar& x) { return Eigen::numext::equal_strict(x, Scalar(0)) ? Scalar(1) : x; });
  }
  VERIFY_IS_CWISE_APPROX(m1.cwiseQuotient(m4),
                         cwise_ref(m1, m4, [](const Scalar& x, const Scalar& y) { return Scalar(x / y); }));
  // For equality comparisons, limit range to increase number of equalities.
  if (Eigen::NumTraits<Scalar>::IsInteger) {
    const Scalar kMax = Scalar(10);
    m3 = m1 - ((m1 / kMax) * kMax);
    m4 = m2 - ((m2 / kMax) * kMax);
    mean = Eigen::NumTraits<Scalar>::IsSigned ? Scalar(0) : kMax / Scalar(2);
  } else {
    const Scalar kShift = Scalar(10);
    m3 = (m1 * kShift).array().floor() / kShift;
    m4 = (m2 * kShift).array().floor() / kShift;
    mean = Scalar(0);
  }
  VERIFY_IS_CWISE_EQUAL(m3.cwiseEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseNotEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseLess(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x < y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseGreater(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x > y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseLessOrEqual(m4),
                        cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x <= y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseGreaterOrEqual(m4),
                        cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x >= y; }));
  // Typed-Equality.
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedNotEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedLess(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x < y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedGreater(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x > y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedLessOrEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x <= y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedGreaterOrEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x >= y ? Scalar(1) : Scalar(0);
                        }));
  // Scalar.
  m4.setConstant(rows, cols, mean);
  VERIFY_IS_CWISE_EQUAL(m3.cwiseEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseNotEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseLess(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x < y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseGreater(mean),
                        cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x > y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseLessOrEqual(mean),
                        cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x <= y; }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseGreaterOrEqual(mean),
                        cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) { return x >= y; }));
  // Typed.
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedNotEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedLess(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x < y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedGreater(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x > y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedLessOrEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x <= y ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedGreaterOrEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return x >= y ? Scalar(1) : Scalar(0);
                        }));
}

template <typename MatrixType>
void test_cwise_complex(const MatrixType& m) {
  using Scalar = typename MatrixType::Scalar;
  using RealScalar = typename NumTraits<Scalar>::Real;
  Index rows = m.rows();
  Index cols = m.cols();
  MatrixType m1 = MatrixType::Random(rows, cols);
  MatrixType m2, m3, m4;

  // Supported unary ops.
  VERIFY_IS_CWISE_APPROX(m1.cwiseAbs(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::abs(x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseSqrt(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::sqrt(x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseInverse(), cwise_ref(m1, [](const Scalar& x) { return Scalar(Scalar(1) / x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseArg(), cwise_ref(m1, [](const Scalar& x) { return Eigen::numext::arg(x); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseCArg(), cwise_ref(m1, [](const Scalar& x) { return Scalar(Eigen::numext::arg(x)); }));
  // Only find Square/Abs2 of +/- sqrt values so we don't overflow.
  m2 = m1.cwiseSqrt().array() * m1.cwiseSign().array();
  VERIFY_IS_CWISE_APPROX(m2.cwiseAbs2(), cwise_ref(m2, [](const Scalar& x) { return Eigen::numext::abs2(x); }));
  VERIFY_IS_CWISE_APPROX(m2.cwiseSquare(), cwise_ref(m2, [](const Scalar& x) { return Scalar(x * x); }));
  VERIFY_IS_CWISE_APPROX(m2.cwisePow(Scalar(2)),
                         cwise_ref(m2, [](const Scalar& x) { return Eigen::numext::pow(x, Scalar(2)); }));

  // Supported binary ops.
  m1.setRandom(rows, cols);
  m2.setRandom(rows, cols);
  VERIFY_IS_CWISE_APPROX(m1.cwiseProduct(m2),
                         cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Scalar(x * y); }));
  VERIFY_IS_CWISE_APPROX(m1.cwiseQuotient(m2),
                         cwise_ref(m1, m2, [](const Scalar& x, const Scalar& y) { return Scalar(x / y); }));
  // For equality comparisons, limit range to increase number of equalities.
  {
    const RealScalar kShift = RealScalar(10);
    m3 = m1;
    m4 = m2;
    m3.real() = (m1.real() * kShift).array().floor() / kShift;
    m3.imag() = (m1.imag() * kShift).array().floor() / kShift;
    m4.real() = (m2.real() * kShift).array().floor() / kShift;
    m4.imag() = (m2.imag() * kShift).array().floor() / kShift;
  }
  VERIFY_IS_CWISE_EQUAL(m3.cwiseEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseNotEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y);
                        }));
  // Typed-Equality.
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedNotEqual(m4), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  // Scalar.
  Scalar mean = Scalar(0);
  m4.setConstant(rows, cols, mean);
  VERIFY_IS_CWISE_EQUAL(m3.cwiseEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseNotEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y);
                        }));
  // Typed.
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
  VERIFY_IS_CWISE_EQUAL(m3.cwiseTypedNotEqual(mean), cwise_ref(m3, m4, [](const Scalar& x, const Scalar& y) {
                          return !Eigen::numext::equal_strict(x, y) ? Scalar(1) : Scalar(0);
                        }));
}

EIGEN_DECLARE_TEST(matrix_cwise) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(test_cwise_real(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_1(test_cwise_real(Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_1(test_cwise_real(Eigen::Matrix<Eigen::half, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_1(test_cwise_real(Eigen::Matrix<Eigen::bfloat16, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_2(test_cwise_complex(Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_2(test_cwise_complex(Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_3(test_cwise_real(Eigen::Matrix<int8_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_3(test_cwise_real(Eigen::Matrix<int16_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_3(test_cwise_real(Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_3(test_cwise_real(Eigen::Matrix<int64_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_4(test_cwise_real(Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_4(test_cwise_real(Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_4(test_cwise_real(Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
    CALL_SUBTEST_4(test_cwise_real(Eigen::Matrix<uint64_t, Eigen::Dynamic, Eigen::Dynamic>(20, 20)));
  }
}
