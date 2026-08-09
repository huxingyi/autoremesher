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
#include "random_without_cast_overflow.h"

// suppress annoying unsigned integer warnings
template <typename Scalar, bool IsSigned = NumTraits<Scalar>::IsSigned>
struct negative_or_zero_impl {
  static Scalar run(const Scalar& a) { return -a; }
};
template <typename Scalar>
struct negative_or_zero_impl<Scalar, false> {
  static Scalar run(const Scalar&) { return 0; }
};
template <typename Scalar>
Scalar negative_or_zero(const Scalar& a) {
  return negative_or_zero_impl<Scalar>::run(a);
}

template <typename Scalar, std::enable_if_t<NumTraits<Scalar>::IsInteger, int> = 0>
std::vector<Scalar> special_values() {
  const Scalar zero = Scalar(0);
  const Scalar one = Scalar(1);
  const Scalar two = Scalar(2);
  const Scalar three = Scalar(3);
  const Scalar min = (std::numeric_limits<Scalar>::min)();
  const Scalar max = (std::numeric_limits<Scalar>::max)();
  return {zero, min, one, two, three, max};
}

template <typename Scalar, std::enable_if_t<!NumTraits<Scalar>::IsInteger, int> = 0>
std::vector<Scalar> special_values() {
  const Scalar zero = Scalar(0);
  const Scalar eps = Eigen::NumTraits<Scalar>::epsilon();
  const Scalar one_half = Scalar(0.5);
  const Scalar one = Scalar(1);
  const Scalar two = Scalar(2);
  const Scalar three = Scalar(3);
  const Scalar sqrt_half = Scalar(std::sqrt(0.5));
  const Scalar sqrt2 = Scalar(std::sqrt(2));
  const Scalar inf = Eigen::NumTraits<Scalar>::infinity();
  const Scalar nan = Eigen::NumTraits<Scalar>::quiet_NaN();
  // For 32-bit arm, working within or near the subnormal range can lead to incorrect results
  // due to FTZ.
  const Scalar denorm_min = EIGEN_ARCH_ARM ? zero : std::numeric_limits<Scalar>::denorm_min();
  const Scalar min =
      EIGEN_ARCH_ARM ? Scalar(1.1) * (std::numeric_limits<Scalar>::min)() : (std::numeric_limits<Scalar>::min)();
  const Scalar max = (std::numeric_limits<Scalar>::max)();
  const Scalar max_exp = (static_cast<Scalar>(int(Eigen::NumTraits<Scalar>::max_exponent())) * Scalar(EIGEN_LN2)) / eps;
  std::vector<Scalar> values = {zero,  denorm_min, min,   eps,     sqrt_half, one_half, one,
                                sqrt2, two,        three, max_exp, max,       inf,      nan};
  std::vector<Scalar> signed_values;
  for (Scalar value : values) {
    signed_values.push_back(value);
    signed_values.push_back(-value);
  }
  return signed_values;
}

template <typename Scalar>
void special_value_pairs(Array<Scalar, Dynamic, Dynamic>& x, Array<Scalar, Dynamic, Dynamic>& y) {
  std::vector<Scalar> vals = special_values<Scalar>();
  std::size_t num_cases = vals.size() * vals.size();
  // ensure both vectorized and non-vectorized paths taken
  const Index num_repeats = 2 * (Index)internal::packet_traits<Scalar>::size + 1;
  x.resize(num_repeats, num_cases);
  y.resize(num_repeats, num_cases);
  int count = 0;
  for (const Scalar x_case : vals) {
    for (const Scalar y_case : vals) {
      for (Index repeat = 0; repeat < num_repeats; ++repeat) {
        x(repeat, count) = x_case;
        y(repeat, count) = y_case;
      }
      ++count;
    }
  }
}

template <typename Scalar, typename Fn, typename RefFn>
void binary_op_test(std::string name, Fn fun, RefFn ref) {
  const Scalar tol = test_precision<Scalar>();
  Array<Scalar, Dynamic, Dynamic> lhs;
  Array<Scalar, Dynamic, Dynamic> rhs;
  special_value_pairs(lhs, rhs);

  Array<Scalar, Dynamic, Dynamic> actual = fun(lhs, rhs);
  bool all_pass = true;
  for (Index i = 0; i < lhs.rows(); ++i) {
    for (Index j = 0; j < lhs.cols(); ++j) {
      Scalar e = static_cast<Scalar>(ref(lhs(i, j), rhs(i, j)));
      Scalar a = actual(i, j);
#if EIGEN_ARCH_ARM
      // Work around NEON flush-to-zero mode.
      // If ref returns a subnormal value and Eigen returns 0, then skip the test.
      if (a == Scalar(0) && (e > -(std::numeric_limits<Scalar>::min)() && e < (std::numeric_limits<Scalar>::min)()) &&
          (e <= -std::numeric_limits<Scalar>::denorm_min() || e >= std::numeric_limits<Scalar>::denorm_min())) {
        continue;
      }
#endif
      bool success = (a == e) || ((numext::isfinite)(e) && internal::isApprox(a, e, tol)) ||
                     ((numext::isnan)(a) && (numext::isnan)(e));
      if ((a == a) && (e == e)) success &= (bool)numext::signbit(e) == (bool)numext::signbit(a);
      all_pass &= success;
      if (!success) {
        std::cout << name << "(" << lhs(i, j) << "," << rhs(i, j) << ") = " << a << " !=  " << e << std::endl;
      }
    }
  }
  VERIFY(all_pass);
}

#define BINARY_FUNCTOR_TEST_ARGS(fun)                                        \
  #fun, [](const auto& x_, const auto& y_) { return (Eigen::fun)(x_, y_); }, \
      [](const auto& x_, const auto& y_) { return (std::fun)(x_, y_); }

template <typename Scalar>
void binary_ops_test() {
  binary_op_test<Scalar>(BINARY_FUNCTOR_TEST_ARGS(pow));
#ifndef EIGEN_COMP_MSVC
  binary_op_test<Scalar>(BINARY_FUNCTOR_TEST_ARGS(atan2));
#else
  binary_op_test<Scalar>(
      "atan2", [](const auto& x, const auto& y) { return Eigen::atan2(x, y); },
      [](Scalar x, Scalar y) {
        auto t = Scalar(std::atan2(x, y));
        // Work around MSVC return value on underflow.
        // |atan(y/x)| is bounded above by |y/x|, so on underflow return y/x according to POSIX spec.
        // MSVC otherwise returns denorm_min.
        if (EIGEN_PREDICT_FALSE(std::abs(t) == std::numeric_limits<decltype(t)>::denorm_min())) {
          return x / y;
        }
        return t;
      });
#endif
}

template <typename Scalar, typename Fn, typename RefFn>
void unary_op_test(std::string name, Fn fun, RefFn ref) {
  const Scalar tol = test_precision<Scalar>();
  auto values = special_values<Scalar>();
  Map<Array<Scalar, Dynamic, 1>> valuesMap(values.data(), values.size());

  Array<Scalar, Dynamic, Dynamic> actual = fun(valuesMap);
  bool all_pass = true;
  for (Index i = 0; i < valuesMap.size(); ++i) {
    Scalar e = static_cast<Scalar>(ref(valuesMap(i)));
    Scalar a = actual(i);
#if EIGEN_ARCH_ARM
    // Work around NEON flush-to-zero mode.
    // If ref returns a subnormal value and Eigen returns 0, then skip the test.
    if (a == Scalar(0) && (e > -(std::numeric_limits<Scalar>::min)() && e < (std::numeric_limits<Scalar>::min)()) &&
        (e <= -std::numeric_limits<Scalar>::denorm_min() || e >= std::numeric_limits<Scalar>::denorm_min())) {
      continue;
    }
#endif
    bool success = (a == e) || ((numext::isfinite)(e) && internal::isApprox(a, e, tol)) ||
                   ((numext::isnan)(a) && (numext::isnan)(e));
    if ((a == a) && (e == e)) success &= (bool)numext::signbit(e) == (bool)numext::signbit(a);
    all_pass &= success;
    if (!success) {
      std::cout << name << "(" << valuesMap(i) << ") = " << a << " !=  " << e << std::endl;
    }
  }
  VERIFY(all_pass);
}

#define UNARY_FUNCTOR_TEST_ARGS(fun) \
  #fun, [](const auto& x_) { return (Eigen::fun)(x_); }, [](const auto& y_) { return (std::fun)(y_); }

template <typename Scalar>
void unary_ops_test() {
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(sqrt));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(cbrt));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(exp));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(exp2));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(log));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(sin));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(cos));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(tan));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(asin));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(acos));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(atan));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(sinh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(cosh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(tanh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(asinh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(acosh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(atanh));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(rint));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(floor));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(ceil));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(round));
  unary_op_test<Scalar>(UNARY_FUNCTOR_TEST_ARGS(trunc));
  /* FIXME: Enable when the behavior of rsqrt on denormals for half and double is fixed.
  unary_op_test<Scalar>("rsqrt",
                        [](const auto& x) { return Eigen::rsqrt(x); },
                        [](Scalar x) {
                          if (x >= 0 && x < (std::numeric_limits<Scalar>::min)()) {
                            // rsqrt return +inf for positive subnormals.
                            return NumTraits<Scalar>::infinity();
                          } else {
                            return  Scalar(std::sqrt(Scalar(1)/x));
                          }
                        });
  */
}

template <typename Base, typename Exponent, bool ExpIsInteger = NumTraits<Exponent>::IsInteger>
struct ref_pow {
  static Base run(Base base, Exponent exponent) {
    EIGEN_USING_STD(pow);
    return static_cast<Base>(pow(base, static_cast<Base>(exponent)));
  }
};

template <typename Base, typename Exponent>
struct ref_pow<Base, Exponent, true> {
  static Base run(Base base, Exponent exponent) {
    EIGEN_USING_STD(pow);
    return static_cast<Base>(pow(base, exponent));
  }
};

template <typename Exponent, bool ExpIsInteger = NumTraits<Exponent>::IsInteger>
struct pow_helper {
  static bool is_integer_impl(const Exponent& exp) { return (numext::isfinite)(exp) && exp == numext::floor(exp); }
  static bool is_odd_impl(const Exponent& exp) {
    Exponent exp_div_2 = exp / Exponent(2);
    Exponent floor_exp_div_2 = numext::floor(exp_div_2);
    return exp_div_2 != floor_exp_div_2;
  }
};
template <typename Exponent>
struct pow_helper<Exponent, true> {
  static bool is_integer_impl(const Exponent&) { return true; }
  static bool is_odd_impl(const Exponent& exp) { return exp % 2 != 0; }
};
template <typename Exponent>
bool is_integer(const Exponent& exp) {
  return pow_helper<Exponent>::is_integer_impl(exp);
}
template <typename Exponent>
bool is_odd(const Exponent& exp) {
  return pow_helper<Exponent>::is_odd_impl(exp);
}

template <typename Base, typename Exponent>
void float_pow_test_impl() {
  const Base tol = test_precision<Base>();
  std::vector<Base> abs_base_vals = special_values<Base>();
  std::vector<Exponent> abs_exponent_vals = special_values<Exponent>();
  for (int i = 0; i < 100; i++) {
    abs_base_vals.push_back(internal::random<Base>(Base(0), Base(10)));
    abs_exponent_vals.push_back(internal::random<Exponent>(Exponent(0), Exponent(10)));
  }
  const Index num_repeats = internal::packet_traits<Base>::size + 1;
  ArrayX<Base> bases(num_repeats), eigenPow(num_repeats);
  bool all_pass = true;
  for (Base abs_base : abs_base_vals)
    for (Base base : {negative_or_zero(abs_base), abs_base}) {
      bases.setConstant(base);
      for (Exponent abs_exponent : abs_exponent_vals) {
        for (Exponent exponent : {negative_or_zero(abs_exponent), abs_exponent}) {
          eigenPow = bases.pow(exponent);
          for (Index j = 0; j < num_repeats; j++) {
            Base e = ref_pow<Base, Exponent>::run(bases(j), exponent);
            if (is_integer(exponent)) {
              // std::pow may return an incorrect result for a very large integral exponent
              // if base is negative and the exponent is odd, then the result must be negative
              // if std::pow returns otherwise, flip the sign
              bool exp_is_odd = is_odd(exponent);
              bool base_is_neg = !(numext::isnan)(base) && (bool)numext::signbit(base);
              bool result_is_neg = exp_is_odd && base_is_neg;
              bool ref_is_neg = !(numext::isnan)(e) && (bool)numext::signbit(e);
              bool flip_sign = result_is_neg != ref_is_neg;
              if (flip_sign) e = -e;
            }

            Base a = eigenPow(j);
#ifdef EIGEN_COMP_MSVC
            // Work around MSVC return value on underflow.
            // if std::pow returns 0 and Eigen returns a denormalized value, then skip the test
            int eigen_fpclass = std::fpclassify(a);
            if (e == Base(0) && eigen_fpclass == FP_SUBNORMAL) continue;
#endif

#ifdef EIGEN_VECTORIZE_NEON
            // Work around NEON flush-to-zero mode
            // if std::pow returns denormalized value and Eigen returns 0, then skip the test
            int ref_fpclass = std::fpclassify(e);
            if (a == Base(0) && ref_fpclass == FP_SUBNORMAL) continue;
#endif

            bool both_nan = (numext::isnan)(a) && (numext::isnan)(e);
            bool exact_or_approx = (a == e) || internal::isApprox(a, e, tol);
            bool same_sign = (bool)numext::signbit(e) == (bool)numext::signbit(a);
            bool success = both_nan || (exact_or_approx && same_sign);
            all_pass &= success;
            if (!success) {
              std::cout << "Base type: " << type_name(base) << ", Exponent type: " << type_name(exponent) << std::endl;
              std::cout << "pow(" << bases(j) << "," << exponent << ")   =   " << a << " !=  " << e << std::endl;
            }
          }
        }
      }
    }
  VERIFY(all_pass);
}

template <typename Scalar, typename ScalarExponent>
Scalar calc_overflow_threshold(const ScalarExponent exponent) {
  EIGEN_USING_STD(exp2);
  EIGEN_USING_STD(log2);
  EIGEN_STATIC_ASSERT((NumTraits<Scalar>::digits() < 2 * NumTraits<double>::digits()), BASE_TYPE_IS_TOO_BIG);

  if (exponent < 2)
    return NumTraits<Scalar>::highest();
  else {
    // base^e <= highest ==> base <= 2^(log2(highest)/e)
    // For floating-point types, consider the bound for integer values that can be reproduced exactly = 2 ^ digits
    double highest_bits = numext::mini(static_cast<double>(NumTraits<Scalar>::digits()),
                                       static_cast<double>(log2(NumTraits<Scalar>::highest())));
    return static_cast<Scalar>(numext::floor(exp2(highest_bits / static_cast<double>(exponent))));
  }
}

template <typename Base, typename Exponent>
void test_exponent(Exponent exponent) {
  EIGEN_STATIC_ASSERT(NumTraits<Base>::IsInteger, THIS TEST IS ONLY INTENDED FOR BASE INTEGER TYPES)
  const Base max_abs_bases = static_cast<Base>(10000);
  // avoid integer overflow in Base type
  Base threshold = calc_overflow_threshold<Base, Exponent>(numext::abs(exponent));
  // avoid numbers that can't be verified with std::pow
  double double_threshold = calc_overflow_threshold<double, Exponent>(numext::abs(exponent));
  // use the lesser of these two thresholds
  Base testing_threshold =
      static_cast<double>(threshold) < double_threshold ? threshold : static_cast<Base>(double_threshold);
  // test both vectorized and non-vectorized code paths
  const Index array_size = 2 * internal::packet_traits<Base>::size + 1;

  Base max_base = numext::mini(testing_threshold, max_abs_bases);
  Base min_base = negative_or_zero(max_base);

  ArrayX<Base> x(array_size), y(array_size);
  bool all_pass = true;
  for (Base base = min_base; base <= max_base; base++) {
    if (exponent < 0 && base == 0) continue;
    x.setConstant(base);
    y = x.pow(exponent);
    for (Base a : y) {
      Base e = ref_pow<Base, Exponent>::run(base, exponent);
      bool pass = (a == e);
      all_pass &= pass;
      if (!pass) {
        std::cout << "pow(" << base << "," << exponent << ")   =   " << a << " !=  " << e << std::endl;
      }
    }
  }
  VERIFY(all_pass);
}

template <typename Base, typename Exponent>
void int_pow_test_impl() {
  Exponent max_exponent = static_cast<Exponent>(NumTraits<Base>::digits());
  Exponent min_exponent = negative_or_zero(max_exponent);

  for (Exponent exponent = min_exponent; exponent < max_exponent; ++exponent) {
    test_exponent<Base, Exponent>(exponent);
  }
}

void float_pow_test() {
  float_pow_test_impl<float, float>();
  float_pow_test_impl<double, double>();
}

void mixed_pow_test() {
  // The following cases will test promoting a smaller exponent type
  // to a wider base type.
  float_pow_test_impl<double, int>();
  float_pow_test_impl<double, float>();
  float_pow_test_impl<float, half>();
  float_pow_test_impl<double, half>();
  float_pow_test_impl<float, bfloat16>();
  float_pow_test_impl<double, bfloat16>();

  // Although in the following cases the exponent cannot be represented exactly
  // in the base type, we do not perform a conversion, but implement
  // the operation using repeated squaring.
  float_pow_test_impl<float, int>();
  float_pow_test_impl<double, long long>();

  // The following cases will test promoting a wider exponent type
  // to a narrower base type. This should compile but would generate a
  // deprecation warning:
  // unary_pow_test<float, double>();
}

void int_pow_test() {
  int_pow_test_impl<int, int>();
  int_pow_test_impl<unsigned int, unsigned int>();
  int_pow_test_impl<long long, long long>();
  int_pow_test_impl<unsigned long long, unsigned long long>();

  // Although in the following cases the exponent cannot be represented exactly
  // in the base type, we do not perform a conversion, but implement the
  // operation using repeated squaring.
  int_pow_test_impl<long long, int>();
  int_pow_test_impl<int, unsigned int>();
  int_pow_test_impl<unsigned int, int>();
  int_pow_test_impl<long long, unsigned long long>();
  int_pow_test_impl<unsigned long long, long long>();
  int_pow_test_impl<long long, int>();
}

namespace Eigen {
namespace internal {
template <typename Scalar>
struct test_signbit_op {
  Scalar constexpr operator()(const Scalar& a) const { return numext::signbit(a); }
  template <typename Packet>
  inline Packet packetOp(const Packet& a) const {
    return psignbit(a);
  }
};
template <typename Scalar>
struct functor_traits<test_signbit_op<Scalar>> {
  enum { Cost = 1, PacketAccess = true };  // todo: define HasSignbit flag
};
}  // namespace internal
}  // namespace Eigen

template <typename Scalar>
void signbit_test() {
  const size_t size = 100 * internal::packet_traits<Scalar>::size;
  ArrayX<Scalar> x(size), y(size);
  x.setRandom();
  std::vector<Scalar> special_vals = special_values<Scalar>();
  for (size_t i = 0; i < special_vals.size(); i++) {
    x(2 * i + 0) = special_vals[i];
    x(2 * i + 1) = negative_or_zero(special_vals[i]);
  }
  y = x.unaryExpr(internal::test_signbit_op<Scalar>());

  bool all_pass = true;
  for (size_t i = 0; i < size; i++) {
    const Scalar ref_val = numext::signbit(x(i));
    bool not_same = internal::predux_any(internal::bitwise_helper<Scalar>::bitwise_xor(ref_val, y(i)));
    if (not_same) std::cout << "signbit(" << x(i) << ") != " << y(i) << "\n";
    all_pass = all_pass && !not_same;
  }

  VERIFY(all_pass);
}
void signbit_tests() {
  signbit_test<float>();
  signbit_test<double>();
  signbit_test<Eigen::half>();
  signbit_test<Eigen::bfloat16>();
  signbit_test<int8_t>();
  signbit_test<int16_t>();
  signbit_test<int32_t>();
  signbit_test<int64_t>();
}

template <typename ArrayType>
void array_generic(const ArrayType& m) {
  typedef typename ArrayType::Scalar Scalar;
  typedef typename ArrayType::RealScalar RealScalar;
  typedef Array<Scalar, ArrayType::RowsAtCompileTime, 1> ColVectorType;
  typedef Array<Scalar, 1, ArrayType::ColsAtCompileTime> RowVectorType;

  Index rows = m.rows();
  Index cols = m.cols();

  ArrayType m1 = ArrayType::Random(rows, cols);
  if (NumTraits<RealScalar>::IsInteger && NumTraits<RealScalar>::IsSigned && !NumTraits<Scalar>::IsComplex) {
    // Here we cap the size of the values in m1 such that pow(3)/cube()
    // doesn't overflow and result in undefined behavior. Notice that because
    // pow(int, int) promotes its inputs and output to double (according to
    // the C++ standard), we have to make sure that the result fits in 53 bits
    // for int64,
    RealScalar max_val =
        numext::mini(RealScalar(std::cbrt(NumTraits<RealScalar>::highest())), RealScalar(std::cbrt(1LL << 53))) / 2;
    m1.array() = (m1.abs().array() <= max_val).select(m1, Scalar(max_val));
  }
  ArrayType m2 = ArrayType::Random(rows, cols), m3(rows, cols);
  ArrayType m4 = m1;  // copy constructor
  VERIFY_IS_APPROX(m1, m4);

  ColVectorType cv1 = ColVectorType::Random(rows);
  RowVectorType rv1 = RowVectorType::Random(cols);

  Scalar s1 = internal::random<Scalar>(), s2 = internal::random<Scalar>();

  // scalar addition
  VERIFY_IS_APPROX(m1 + s1, s1 + m1);
  VERIFY_IS_APPROX(m1 + s1, ArrayType::Constant(rows, cols, s1) + m1);
  VERIFY_IS_APPROX(s1 - m1, (-m1) + s1);
  VERIFY_IS_APPROX(m1 - s1, m1 - ArrayType::Constant(rows, cols, s1));
  VERIFY_IS_APPROX(s1 - m1, ArrayType::Constant(rows, cols, s1) - m1);
  VERIFY_IS_APPROX((m1 * Scalar(2)) - s2, (m1 + m1) - ArrayType::Constant(rows, cols, s2));
  m3 = m1;
  m3 += s2;
  VERIFY_IS_APPROX(m3, m1 + s2);
  m3 = m1;
  m3 -= s1;
  VERIFY_IS_APPROX(m3, m1 - s1);

  // scalar operators via Maps
  m3 = m1;
  m4 = m1;
  ArrayType::Map(m4.data(), m4.rows(), m4.cols()) -= ArrayType::Map(m2.data(), m2.rows(), m2.cols());
  VERIFY_IS_APPROX(m4, m3 - m2);

  m3 = m1;
  m4 = m1;
  ArrayType::Map(m4.data(), m4.rows(), m4.cols()) += ArrayType::Map(m2.data(), m2.rows(), m2.cols());
  VERIFY_IS_APPROX(m4, m3 + m2);

  m3 = m1;
  m4 = m1;
  ArrayType::Map(m4.data(), m4.rows(), m4.cols()) *= ArrayType::Map(m2.data(), m2.rows(), m2.cols());
  VERIFY_IS_APPROX(m4, m3 * m2);

  m3 = m1;
  m4 = m1;
  m2 = ArrayType::Random(rows, cols);
  m2 = (m2 == 0).select(1, m2);
  ArrayType::Map(m4.data(), m4.rows(), m4.cols()) /= ArrayType::Map(m2.data(), m2.rows(), m2.cols());
  VERIFY_IS_APPROX(m4, m3 / m2);

  // reductions
  VERIFY_IS_APPROX(m1.abs().colwise().sum().sum(), m1.abs().sum());
  VERIFY_IS_APPROX(m1.abs().rowwise().sum().sum(), m1.abs().sum());
  using numext::abs;
  VERIFY_IS_MUCH_SMALLER_THAN(abs(m1.colwise().sum().sum() - m1.sum()), m1.abs().sum());
  VERIFY_IS_MUCH_SMALLER_THAN(abs(m1.rowwise().sum().sum() - m1.sum()), m1.abs().sum());
  if (!internal::isMuchSmallerThan(abs(m1.sum() - (m1 + m2).sum()), m1.abs().sum(), test_precision<Scalar>()))
    VERIFY_IS_NOT_APPROX(((m1 + m2).rowwise().sum()).sum(), m1.sum());
  VERIFY_IS_APPROX(m1.colwise().sum(), m1.colwise().redux(internal::scalar_sum_op<Scalar, Scalar>()));

  // vector-wise ops
  m3 = m1;
  VERIFY_IS_APPROX(m3.colwise() += cv1, m1.colwise() + cv1);
  m3 = m1;
  VERIFY_IS_APPROX(m3.colwise() -= cv1, m1.colwise() - cv1);
  m3 = m1;
  VERIFY_IS_APPROX(m3.rowwise() += rv1, m1.rowwise() + rv1);
  m3 = m1;
  VERIFY_IS_APPROX(m3.rowwise() -= rv1, m1.rowwise() - rv1);

  // Conversion from scalar
  VERIFY_IS_APPROX((m3 = s1), ArrayType::Constant(rows, cols, s1));
  VERIFY_IS_APPROX((m3 = 1), ArrayType::Constant(rows, cols, 1));
  VERIFY_IS_APPROX((m3.topLeftCorner(rows, cols) = 1), ArrayType::Constant(rows, cols, 1));
  typedef Array<Scalar, ArrayType::RowsAtCompileTime == Dynamic ? 2 : ArrayType::RowsAtCompileTime,
                ArrayType::ColsAtCompileTime == Dynamic ? 2 : ArrayType::ColsAtCompileTime, ArrayType::Options>
      FixedArrayType;
  {
    FixedArrayType f1(s1);
    VERIFY_IS_APPROX(f1, FixedArrayType::Constant(s1));
    FixedArrayType f2(numext::real(s1));
    VERIFY_IS_APPROX(f2, FixedArrayType::Constant(numext::real(s1)));
    FixedArrayType f3((int)100 * numext::real(s1));
    VERIFY_IS_APPROX(f3, FixedArrayType::Constant((int)100 * numext::real(s1)));
    f1.setRandom();
    FixedArrayType f4(f1.data());
    VERIFY_IS_APPROX(f4, f1);
  }
  {
    FixedArrayType f1{s1};
    VERIFY_IS_APPROX(f1, FixedArrayType::Constant(s1));
    FixedArrayType f2{numext::real(s1)};
    VERIFY_IS_APPROX(f2, FixedArrayType::Constant(numext::real(s1)));
    FixedArrayType f3{(int)100 * numext::real(s1)};
    VERIFY_IS_APPROX(f3, FixedArrayType::Constant((int)100 * numext::real(s1)));
    f1.setRandom();
    FixedArrayType f4{f1.data()};
    VERIFY_IS_APPROX(f4, f1);
  }

  // pow
  VERIFY_IS_APPROX(m1.pow(2), m1.square());
  VERIFY_IS_APPROX(pow(m1, 2), m1.square());
  VERIFY_IS_APPROX(m1.pow(3), m1.cube());
  VERIFY_IS_APPROX(pow(m1, 3), m1.cube());
  VERIFY_IS_APPROX((-m1).pow(3), -m1.cube());
  VERIFY_IS_APPROX(pow(2 * m1, 3), 8 * m1.cube());
  ArrayType exponents = ArrayType::Constant(rows, cols, RealScalar(2));
  VERIFY_IS_APPROX(Eigen::pow(m1, exponents), m1.square());
  VERIFY_IS_APPROX(m1.pow(exponents), m1.square());
  VERIFY_IS_APPROX(Eigen::pow(2 * m1, exponents), 4 * m1.square());
  VERIFY_IS_APPROX((2 * m1).pow(exponents), 4 * m1.square());
  VERIFY_IS_APPROX(Eigen::pow(m1, 2 * exponents), m1.square().square());
  VERIFY_IS_APPROX(m1.pow(2 * exponents), m1.square().square());
  VERIFY_IS_APPROX(Eigen::pow(m1(0, 0), exponents), ArrayType::Constant(rows, cols, m1(0, 0) * m1(0, 0)));

  // Check possible conflicts with 1D ctor
  typedef Array<Scalar, Dynamic, 1> OneDArrayType;
  {
    OneDArrayType o1(rows);
    VERIFY(o1.size() == rows);
    OneDArrayType o2(static_cast<int>(rows));
    VERIFY(o2.size() == rows);
  }
  {
    OneDArrayType o1{rows};
    VERIFY(o1.size() == rows);
    OneDArrayType o4{int(rows)};
    VERIFY(o4.size() == rows);
  }
  // Check possible conflicts with 2D ctor
  typedef Array<Scalar, Dynamic, Dynamic> TwoDArrayType;
  typedef Array<Scalar, 2, 1> ArrayType2;
  {
    TwoDArrayType o1(rows, cols);
    VERIFY(o1.rows() == rows);
    VERIFY(o1.cols() == cols);
    TwoDArrayType o2(static_cast<int>(rows), static_cast<int>(cols));
    VERIFY(o2.rows() == rows);
    VERIFY(o2.cols() == cols);

    ArrayType2 o3(rows, cols);
    VERIFY(o3(0) == RealScalar(rows) && o3(1) == RealScalar(cols));
    ArrayType2 o4(static_cast<int>(rows), static_cast<int>(cols));
    VERIFY(o4(0) == RealScalar(rows) && o4(1) == RealScalar(cols));
  }
  {
    TwoDArrayType o1{rows, cols};
    VERIFY(o1.rows() == rows);
    VERIFY(o1.cols() == cols);
    TwoDArrayType o2{int(rows), int(cols)};
    VERIFY(o2.rows() == rows);
    VERIFY(o2.cols() == cols);

    ArrayType2 o3{rows, cols};
    VERIFY(o3(0) == RealScalar(rows) && o3(1) == RealScalar(cols));
    ArrayType2 o4{int(rows), int(cols)};
    VERIFY(o4(0) == RealScalar(rows) && o4(1) == RealScalar(cols));
  }
}

template <typename ArrayType>
void comparisons(const ArrayType& m) {
  using numext::abs;
  typedef typename ArrayType::Scalar Scalar;
  typedef typename NumTraits<Scalar>::Real RealScalar;

  Index rows = m.rows();
  Index cols = m.cols();

  Index r = internal::random<Index>(0, rows - 1), c = internal::random<Index>(0, cols - 1);

  ArrayType m1 = ArrayType::Random(rows, cols), m2 = ArrayType::Random(rows, cols), m3(rows, cols), m4 = m1;

  m4 = (m4.abs() == Scalar(0)).select(1, m4);

  // use operator overloads with default return type

  VERIFY(((m1 + Scalar(1)) > m1).all());
  VERIFY(((m1 - Scalar(1)) < m1).all());
  if (rows * cols > 1) {
    m3 = m1;
    m3(r, c) += 1;
    VERIFY(!(m1 < m3).all());
    VERIFY(!(m1 > m3).all());
  }
  VERIFY(!(m1 > m2 && m1 < m2).any());
  VERIFY((m1 <= m2 || m1 >= m2).all());

  // comparisons array to scalar
  VERIFY((m1 != (m1(r, c) + 1)).any());
  VERIFY((m1 > (m1(r, c) - 1)).any());
  VERIFY((m1 < (m1(r, c) + 1)).any());
  VERIFY((m1 == m1(r, c)).any());

  // comparisons scalar to array
  VERIFY(((m1(r, c) + 1) != m1).any());
  VERIFY(((m1(r, c) - 1) < m1).any());
  VERIFY(((m1(r, c) + 1) > m1).any());
  VERIFY((m1(r, c) == m1).any());

  // currently, any() / all() are not vectorized, so use VERIFY_IS_CWISE_EQUAL to test vectorized path

  // use typed comparisons, regardless of operator overload behavior
  typename ArrayType::ConstantReturnType typed_true = ArrayType::Constant(rows, cols, Scalar(1));
  // (m1 + Scalar(1)) > m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseTypedGreater(m1), typed_true);
  // (m1 - Scalar(1)) < m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseTypedLess(m1), typed_true);
  // (m1 + Scalar(1)) == (m1 + Scalar(1))).all()
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseTypedEqual(m1 + Scalar(1)), typed_true);
  // (m1 - Scalar(1)) != m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseTypedNotEqual(m1), typed_true);
  // (m1 <= m2 || m1 >= m2).all()
  VERIFY_IS_CWISE_EQUAL(m1.cwiseTypedGreaterOrEqual(m2) || m1.cwiseTypedLessOrEqual(m2), typed_true);

  // use boolean comparisons, regardless of operator overload behavior
  ArrayXX<bool>::ConstantReturnType bool_true = ArrayXX<bool>::Constant(rows, cols, true);
  // (m1 + Scalar(1)) > m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseGreater(m1), bool_true);
  // (m1 - Scalar(1)) < m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseLess(m1), bool_true);
  // (m1 + Scalar(1)) == (m1 + Scalar(1))).all()
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseEqual(m1 + Scalar(1)), bool_true);
  // (m1 - Scalar(1)) != m1).all()
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseNotEqual(m1), bool_true);
  // (m1 <= m2 || m1 >= m2).all()
  VERIFY_IS_CWISE_EQUAL(m1.cwiseLessOrEqual(m2) || m1.cwiseGreaterOrEqual(m2), bool_true);

  // test typed comparisons with scalar argument
  VERIFY_IS_CWISE_EQUAL((m1 - m1).cwiseTypedEqual(Scalar(0)), typed_true);
  VERIFY_IS_CWISE_EQUAL((m1.abs() + Scalar(1)).cwiseTypedNotEqual(Scalar(0)), typed_true);
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseTypedGreater(m1.minCoeff()), typed_true);
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseTypedLess(m1.maxCoeff()), typed_true);
  VERIFY_IS_CWISE_EQUAL(m1.abs().cwiseTypedLessOrEqual(NumTraits<Scalar>::highest()), typed_true);
  VERIFY_IS_CWISE_EQUAL(m1.abs().cwiseTypedGreaterOrEqual(Scalar(0)), typed_true);

  // test boolean comparisons with scalar argument
  VERIFY_IS_CWISE_EQUAL((m1 - m1).cwiseEqual(Scalar(0)), bool_true);
  VERIFY_IS_CWISE_EQUAL((m1.abs() + Scalar(1)).cwiseNotEqual(Scalar(0)), bool_true);
  VERIFY_IS_CWISE_EQUAL((m1 + Scalar(1)).cwiseGreater(m1.minCoeff()), bool_true);
  VERIFY_IS_CWISE_EQUAL((m1 - Scalar(1)).cwiseLess(m1.maxCoeff()), bool_true);
  VERIFY_IS_CWISE_EQUAL(m1.abs().cwiseLessOrEqual(NumTraits<Scalar>::highest()), bool_true);
  VERIFY_IS_CWISE_EQUAL(m1.abs().cwiseGreaterOrEqual(Scalar(0)), bool_true);

  // test select
  VERIFY_IS_APPROX((m1 < m2).select(m1, m2), m1.cwiseMin(m2));
  VERIFY_IS_APPROX((m1 > m2).select(m1, m2), m1.cwiseMax(m2));
  Scalar mid = (m1.cwiseAbs().minCoeff() + m1.cwiseAbs().maxCoeff()) / Scalar(2);
  for (int j = 0; j < cols; ++j)
    for (int i = 0; i < rows; ++i) m3(i, j) = abs(m1(i, j)) < mid ? 0 : m1(i, j);
  VERIFY_IS_APPROX((m1.abs() < ArrayType::Constant(rows, cols, mid)).select(ArrayType::Zero(rows, cols), m1), m3);
  // shorter versions:
  VERIFY_IS_APPROX((m1.abs() < ArrayType::Constant(rows, cols, mid)).select(0, m1), m3);
  VERIFY_IS_APPROX((m1.abs() >= ArrayType::Constant(rows, cols, mid)).select(m1, 0), m3);
  // even shorter version:
  VERIFY_IS_APPROX((m1.abs() < mid).select(0, m1), m3);

  // count
  VERIFY(((m1.abs() + 1) > RealScalar(0.1)).count() == rows * cols);

  // and/or
  VERIFY((m1 < RealScalar(0) && m1 > RealScalar(0)).count() == 0);
  VERIFY((m1 < RealScalar(0) || m1 >= RealScalar(0)).count() == rows * cols);
  RealScalar a = m1.abs().mean();
  VERIFY((m1 < -a || m1 > a).count() == (m1.abs() > a).count());

  typedef Array<Index, Dynamic, 1> ArrayOfIndices;

  // TODO allows colwise/rowwise for array
  VERIFY_IS_APPROX(((m1.abs() + 1) > RealScalar(0.1)).colwise().count(),
                   ArrayOfIndices::Constant(cols, rows).transpose());
  VERIFY_IS_APPROX(((m1.abs() + 1) > RealScalar(0.1)).rowwise().count(), ArrayOfIndices::Constant(rows, cols));

  // simple data type that does not permit implicit conversions
  struct scalar_wrapper {
    Scalar m_data;
    scalar_wrapper() : m_data(0) {}
    explicit scalar_wrapper(Scalar data) : m_data(data) {}
    bool operator==(scalar_wrapper other) const { return m_data == other.m_data; }
  };

  // test bug2966: select did not support some scalar types that forbade implicit conversions from bool
  ArrayX<scalar_wrapper> m5(10);
  m5 = (m5 == scalar_wrapper(0)).select(m5, m5);
}

template <typename ArrayType>
void array_real(const ArrayType& m) {
  using numext::abs;
  using std::sqrt;
  typedef typename ArrayType::Scalar Scalar;
  typedef typename NumTraits<Scalar>::Real RealScalar;

  Index rows = m.rows();
  Index cols = m.cols();

  ArrayType m1 = ArrayType::Random(rows, cols), m2 = ArrayType::Random(rows, cols), m3(rows, cols), m4 = m1;

  // avoid denormalized values so verification doesn't fail on platforms that don't support them
  // denormalized behavior is tested elsewhere (unary_op_test, binary_ops_test)
  const Scalar min = (std::numeric_limits<Scalar>::min)();
  m1 = (m1.abs() < min).select(Scalar(0), m1);
  m2 = (m2.abs() < min).select(Scalar(0), m2);
  m4 = (m4.abs() < min).select(Scalar(1), m4);

  Scalar s1 = internal::random<Scalar>();

  // these tests are mostly to check possible compilation issues with free-functions.
  VERIFY_IS_APPROX(m1.sin(), sin(m1));
  VERIFY_IS_APPROX(m1.cos(), cos(m1));
  VERIFY_IS_APPROX(m1.tan(), tan(m1));
  VERIFY_IS_APPROX(m1.asin(), asin(m1));
  VERIFY_IS_APPROX(m1.acos(), acos(m1));
  VERIFY_IS_APPROX(m1.atan(), atan(m1));
  VERIFY_IS_APPROX(m1.sinh(), sinh(m1));
  VERIFY_IS_APPROX(m1.cosh(), cosh(m1));
  VERIFY_IS_APPROX(m1.tanh(), tanh(m1));
  VERIFY_IS_APPROX(m1.atan2(m2), atan2(m1, m2));

  VERIFY_IS_APPROX(m1.tanh().atanh(), atanh(tanh(m1)));
  VERIFY_IS_APPROX(m1.sinh().asinh(), asinh(sinh(m1)));
  VERIFY_IS_APPROX(m1.cosh().acosh(), acosh(cosh(m1)));
  VERIFY_IS_APPROX(m1.tanh().atanh(), atanh(tanh(m1)));
  VERIFY_IS_APPROX(m1.logistic(), logistic(m1));

  VERIFY_IS_APPROX(m1.arg(), arg(m1));
  VERIFY_IS_APPROX(m1.round(), round(m1));
  VERIFY_IS_APPROX(m1.rint(), rint(m1));
  VERIFY_IS_APPROX(m1.floor(), floor(m1));
  VERIFY_IS_APPROX(m1.ceil(), ceil(m1));
  VERIFY_IS_APPROX(m1.trunc(), trunc(m1));
  VERIFY((m1.isNaN() == (Eigen::isnan)(m1)).all());
  VERIFY((m1.isInf() == (Eigen::isinf)(m1)).all());
  VERIFY((m1.isFinite() == (Eigen::isfinite)(m1)).all());
  VERIFY_IS_APPROX(m4.inverse(), inverse(m4));
  VERIFY_IS_APPROX(m1.abs(), abs(m1));
  VERIFY_IS_APPROX(m1.abs2(), abs2(m1));
  VERIFY_IS_APPROX(m1.square(), square(m1));
  VERIFY_IS_APPROX(m1.cube(), cube(m1));
  VERIFY_IS_APPROX(cos(m1 + RealScalar(3) * m2), cos((m1 + RealScalar(3) * m2).eval()));
  VERIFY_IS_APPROX(m1.sign(), sign(m1));
  VERIFY((m1.sqrt().sign().isNaN() == (Eigen::isnan)(sign(sqrt(m1)))).all());

  // avoid inf and NaNs so verification doesn't fail
  m3 = m4.abs();

  VERIFY_IS_APPROX(m3.sqrt(), sqrt(abs(m3)));
  VERIFY_IS_APPROX(m3.cbrt(), cbrt(m3));
  VERIFY_IS_APPROX(m3.rsqrt(), Scalar(1) / sqrt(abs(m3)));
  VERIFY_IS_APPROX(rsqrt(m3), Scalar(1) / sqrt(abs(m3)));
  VERIFY_IS_APPROX(m3.log(), log(m3));
  VERIFY_IS_APPROX(m3.log1p(), log1p(m3));
  VERIFY_IS_APPROX(m3.log10(), log10(m3));
  VERIFY_IS_APPROX(m3.log2(), log2(m3));

  VERIFY((!(m1 > m2) == (m1 <= m2)).all());

  VERIFY_IS_APPROX(sin(m1.asin()), m1);
  VERIFY_IS_APPROX(cos(m1.acos()), m1);
  VERIFY_IS_APPROX(tan(m1.atan()), m1);
  VERIFY_IS_APPROX(sinh(m1), Scalar(0.5) * (exp(m1) - exp(-m1)));
  VERIFY_IS_APPROX(cosh(m1), Scalar(0.5) * (exp(m1) + exp(-m1)));
  VERIFY_IS_APPROX(tanh(m1), (Scalar(0.5) * (exp(m1) - exp(-m1))) / (Scalar(0.5) * (exp(m1) + exp(-m1))));
  VERIFY_IS_APPROX(logistic(m1), (Scalar(1) / (Scalar(1) + exp(-m1))));
  VERIFY_IS_APPROX(arg(m1), ((m1 < Scalar(0)).template cast<Scalar>()) * Scalar(std::acos(Scalar(-1))));
  VERIFY((round(m1) <= ceil(m1) && round(m1) >= floor(m1)).all());
  VERIFY((rint(m1) <= ceil(m1) && rint(m1) >= floor(m1)).all());
  VERIFY(((ceil(m1) - round(m1)) <= Scalar(0.5) || (round(m1) - floor(m1)) <= Scalar(0.5)).all());
  VERIFY(((ceil(m1) - round(m1)) <= Scalar(1.0) && (round(m1) - floor(m1)) <= Scalar(1.0)).all());
  VERIFY(((ceil(m1) - rint(m1)) <= Scalar(0.5) || (rint(m1) - floor(m1)) <= Scalar(0.5)).all());
  VERIFY(((ceil(m1) - rint(m1)) <= Scalar(1.0) && (rint(m1) - floor(m1)) <= Scalar(1.0)).all());
  VERIFY((Eigen::isnan)((m1 * Scalar(0)) / Scalar(0)).all());
  VERIFY((Eigen::isinf)(m4 / Scalar(0)).all());
  VERIFY(((Eigen::isfinite)(m1) && (!(Eigen::isfinite)(m1 * Scalar(0) / Scalar(0))) &&
          (!(Eigen::isfinite)(m4 / Scalar(0))))
             .all());
  VERIFY_IS_APPROX(inverse(inverse(m4)), m4);
  VERIFY((abs(m1) == m1 || abs(m1) == -m1).all());
  VERIFY_IS_APPROX(m3, sqrt(abs2(m3)));
  VERIFY_IS_APPROX(m1.absolute_difference(m2), (m1 > m2).select(m1 - m2, m2 - m1));
  VERIFY_IS_APPROX(m1.sign(), -(-m1).sign());
  VERIFY_IS_APPROX(m1 * m1.sign(), m1.abs());
  VERIFY_IS_APPROX(m1.sign() * m1.abs(), m1);

  ArrayType tmp = m1.atan2(m2);
  for (Index i = 0; i < tmp.size(); ++i) {
    Scalar actual = tmp.array()(i);
    Scalar expected = Scalar(std::atan2(m1.array()(i), m2.array()(i)));
    VERIFY_IS_APPROX(actual, expected);
  }

  VERIFY_IS_APPROX(numext::abs2(numext::real(m1)) + numext::abs2(numext::imag(m1)), numext::abs2(m1));
  VERIFY_IS_APPROX(numext::abs2(Eigen::real(m1)) + numext::abs2(Eigen::imag(m1)), numext::abs2(m1));
  if (!NumTraits<Scalar>::IsComplex) VERIFY_IS_APPROX(numext::real(m1), m1);

  // shift argument of logarithm so that it is not zero
  Scalar smallNumber = NumTraits<Scalar>::dummy_precision();
  VERIFY_IS_APPROX((m3 + smallNumber).log(), log(abs(m3) + smallNumber));
  VERIFY_IS_APPROX((m3 + smallNumber + Scalar(1)).log(), log1p(abs(m3) + smallNumber));

  VERIFY_IS_APPROX(m1.exp() * m2.exp(), exp(m1 + m2));
  VERIFY_IS_APPROX(m1.exp(), exp(m1));
  VERIFY_IS_APPROX(m1.exp() / m2.exp(), (m1 - m2).exp());

  VERIFY_IS_APPROX(m1.expm1(), expm1(m1));
  VERIFY_IS_APPROX((m3 + smallNumber).exp() - Scalar(1), expm1(abs(m3) + smallNumber));

  VERIFY_IS_APPROX(m3.pow(RealScalar(0.5)), m3.sqrt());
  VERIFY_IS_APPROX(pow(m3, RealScalar(0.5)), m3.sqrt());
  VERIFY_IS_APPROX(m3.pow(RealScalar(1.0 / 3.0)), m3.cbrt());
  VERIFY_IS_APPROX(pow(m3, RealScalar(1.0 / 3.0)), m3.cbrt());

  VERIFY_IS_APPROX(m3.pow(RealScalar(-0.5)), m3.rsqrt());
  VERIFY_IS_APPROX(pow(m3, RealScalar(-0.5)), m3.rsqrt());

  // Avoid inf and NaN.
  m3 = (m1.square() < NumTraits<Scalar>::epsilon()).select(Scalar(1), m3);
  VERIFY_IS_APPROX(m3.pow(RealScalar(-2)), m3.square().inverse());

  // Test pow and atan2 on special IEEE values.
  unary_ops_test<Scalar>();
  binary_ops_test<Scalar>();

  VERIFY_IS_APPROX(log10(m3), log(m3) / numext::log(Scalar(10)));
  VERIFY_IS_APPROX(log2(m3), log(m3) / numext::log(Scalar(2)));

  // scalar by array division
  const RealScalar tiny = sqrt(std::numeric_limits<RealScalar>::epsilon());
  s1 += Scalar(tiny);
  m1 += ArrayType::Constant(rows, cols, Scalar(tiny));
  VERIFY_IS_CWISE_APPROX(s1 / m1, s1 * m1.inverse());

  // check inplace transpose
  m3 = m1;
  m3.transposeInPlace();
  VERIFY_IS_APPROX(m3, m1.transpose());
  m3.transposeInPlace();
  VERIFY_IS_APPROX(m3, m1);
}

template <typename ArrayType>
void array_complex(const ArrayType& m) {
  typedef typename ArrayType::Scalar Scalar;
  typedef typename NumTraits<Scalar>::Real RealScalar;

  Index rows = m.rows();
  Index cols = m.cols();

  ArrayType m1 = ArrayType::Random(rows, cols), m2(rows, cols), m4 = m1;

  m4.real() = (m4.real().abs() == RealScalar(0)).select(RealScalar(1), m4.real());
  m4.imag() = (m4.imag().abs() == RealScalar(0)).select(RealScalar(1), m4.imag());

  Array<RealScalar, -1, -1> m3(rows, cols);

  for (Index i = 0; i < m.rows(); ++i)
    for (Index j = 0; j < m.cols(); ++j) m2(i, j) = sqrt(m1(i, j));

  // these tests are mostly to check possible compilation issues with free-functions.
  VERIFY_IS_APPROX(m1.sin(), sin(m1));
  VERIFY_IS_APPROX(m1.cos(), cos(m1));
  VERIFY_IS_APPROX(m1.tan(), tan(m1));
  VERIFY_IS_APPROX(m1.sinh(), sinh(m1));
  VERIFY_IS_APPROX(m1.cosh(), cosh(m1));
  VERIFY_IS_APPROX(m1.tanh(), tanh(m1));
  VERIFY_IS_APPROX(m1.logistic(), logistic(m1));
  VERIFY_IS_APPROX(m1.arg(), arg(m1));
  VERIFY_IS_APPROX(m1.carg(), carg(m1));
  VERIFY_IS_APPROX(arg(m1), carg(m1));
  VERIFY((m1.isNaN() == (Eigen::isnan)(m1)).all());
  VERIFY((m1.isInf() == (Eigen::isinf)(m1)).all());
  VERIFY((m1.isFinite() == (Eigen::isfinite)(m1)).all());
  VERIFY_IS_APPROX(m4.inverse(), inverse(m4));
  VERIFY_IS_APPROX(m1.log(), log(m1));
  VERIFY_IS_APPROX(m1.log10(), log10(m1));
  VERIFY_IS_APPROX(m1.log2(), log2(m1));
  VERIFY_IS_APPROX(m1.abs(), abs(m1));
  VERIFY_IS_APPROX(m1.abs2(), abs2(m1));
  VERIFY_IS_APPROX(m1.sqrt(), sqrt(m1));
  VERIFY_IS_APPROX(m1.square(), square(m1));
  VERIFY_IS_APPROX(m1.cube(), cube(m1));
  VERIFY_IS_APPROX(cos(m1 + RealScalar(3) * m2), cos((m1 + RealScalar(3) * m2).eval()));
  VERIFY_IS_APPROX(m1.sign(), sign(m1));

  VERIFY_IS_APPROX(m1.exp() * m2.exp(), exp(m1 + m2));
  VERIFY_IS_APPROX(m1.exp(), exp(m1));
  VERIFY_IS_APPROX(m1.exp() / m2.exp(), (m1 - m2).exp());

  VERIFY_IS_APPROX(m1.expm1(), expm1(m1));
  VERIFY_IS_APPROX(expm1(m1), exp(m1) - 1.);
  // Check for larger magnitude complex numbers that expm1 matches exp - 1.
  VERIFY_IS_APPROX(expm1(10. * m1), exp(10. * m1) - 1.);

  VERIFY_IS_APPROX(sinh(m1), 0.5 * (exp(m1) - exp(-m1)));
  VERIFY_IS_APPROX(cosh(m1), 0.5 * (exp(m1) + exp(-m1)));
  VERIFY_IS_APPROX(tanh(m1), (0.5 * (exp(m1) - exp(-m1))) / (0.5 * (exp(m1) + exp(-m1))));
  VERIFY_IS_APPROX(logistic(m1), (1.0 / (1.0 + exp(-m1))));
  if (m1.size() > 0) {
    // Complex exponential overflow edge-case.
    Scalar old_m1_val = m1(0, 0);
    m1(0, 0) = std::complex<RealScalar>(1000.0, 1000.0);
    VERIFY_IS_APPROX(logistic(m1), (1.0 / (1.0 + exp(-m1))));
    m1(0, 0) = old_m1_val;  // Restore value for future tests.
  }

  for (Index i = 0; i < m.rows(); ++i)
    for (Index j = 0; j < m.cols(); ++j) m3(i, j) = std::atan2(m1(i, j).imag(), m1(i, j).real());
  VERIFY_IS_APPROX(arg(m1), m3);
  VERIFY_IS_APPROX(carg(m1), m3);

  std::complex<RealScalar> zero(0.0, 0.0);
  VERIFY((Eigen::isnan)(m1 * zero / zero).all());
#if EIGEN_COMP_MSVC
  // msvc complex division is not robust
  VERIFY((Eigen::isinf)(m4 / RealScalar(0)).all());
#else
#if EIGEN_COMP_CLANG
  // clang's complex division is notoriously broken too
  if ((numext::isinf)(m4(0, 0) / RealScalar(0))) {
#endif
    VERIFY((Eigen::isinf)(m4 / zero).all());
#if EIGEN_COMP_CLANG
  } else {
    VERIFY((Eigen::isinf)(m4.real() / zero.real()).all());
  }
#endif
#endif  // MSVC

  VERIFY(((Eigen::isfinite)(m1) && (!(Eigen::isfinite)(m1 * zero / zero)) && (!(Eigen::isfinite)(m1 / zero))).all());

  VERIFY_IS_APPROX(inverse(inverse(m4)), m4);
  VERIFY_IS_APPROX(conj(m1.conjugate()), m1);
  VERIFY_IS_APPROX(abs(m1), sqrt(square(m1.real()) + square(m1.imag())));
  VERIFY_IS_APPROX(abs(m1), sqrt(abs2(m1)));
  VERIFY_IS_APPROX(log10(m1), log(m1) / log(10));
  VERIFY_IS_APPROX(log2(m1), log(m1) / log(2));

  VERIFY_IS_APPROX(m1.sign(), -(-m1).sign());
  VERIFY_IS_APPROX(m1.sign() * m1.abs(), m1);

  // scalar by array division
  Scalar s1 = internal::random<Scalar>();
  const RealScalar tiny = std::sqrt(std::numeric_limits<RealScalar>::epsilon());
  s1 += Scalar(tiny);
  m1 += ArrayType::Constant(rows, cols, Scalar(tiny));
  VERIFY_IS_APPROX(s1 / m1, s1 * m1.inverse());

  // check inplace transpose
  m2 = m1;
  m2.transposeInPlace();
  VERIFY_IS_APPROX(m2, m1.transpose());
  m2.transposeInPlace();
  VERIFY_IS_APPROX(m2, m1);
  // Check vectorized inplace transpose.
  ArrayType m5 = ArrayType::Random(131, 131);
  ArrayType m6 = m5;
  m6.transposeInPlace();
  VERIFY_IS_APPROX(m6, m5.transpose());
}

template <typename ArrayType>
void min_max(const ArrayType& m) {
  typedef typename ArrayType::Scalar Scalar;

  Index rows = m.rows();
  Index cols = m.cols();

  ArrayType m1 = ArrayType::Random(rows, cols);

  // min/max with array
  Scalar maxM1 = m1.maxCoeff();
  Scalar minM1 = m1.minCoeff();

  VERIFY_IS_APPROX(ArrayType::Constant(rows, cols, minM1), (m1.min)(ArrayType::Constant(rows, cols, minM1)));
  VERIFY_IS_APPROX(m1, (m1.min)(ArrayType::Constant(rows, cols, maxM1)));

  VERIFY_IS_APPROX(ArrayType::Constant(rows, cols, maxM1), (m1.max)(ArrayType::Constant(rows, cols, maxM1)));
  VERIFY_IS_APPROX(m1, (m1.max)(ArrayType::Constant(rows, cols, minM1)));

  // min/max with scalar input
  VERIFY_IS_APPROX(ArrayType::Constant(rows, cols, minM1), (m1.min)(minM1));
  VERIFY_IS_APPROX(m1, (m1.min)(maxM1));

  VERIFY_IS_APPROX(ArrayType::Constant(rows, cols, maxM1), (m1.max)(maxM1));
  VERIFY_IS_APPROX(m1, (m1.max)(minM1));

  // min/max with various NaN propagation options.
  if (m1.size() > 1 && !NumTraits<Scalar>::IsInteger) {
    m1(0, 0) = NumTraits<Scalar>::quiet_NaN();
    maxM1 = m1.template maxCoeff<PropagateNaN>();
    minM1 = m1.template minCoeff<PropagateNaN>();
    VERIFY((numext::isnan)(maxM1));
    VERIFY((numext::isnan)(minM1));

    maxM1 = m1.template maxCoeff<PropagateNumbers>();
    minM1 = m1.template minCoeff<PropagateNumbers>();
    VERIFY(!(numext::isnan)(maxM1));
    VERIFY(!(numext::isnan)(minM1));
  }
}

template <typename Scalar>
struct shift_imm_traits {
  enum { Cost = 1, PacketAccess = internal::packet_traits<Scalar>::HasShift };
};

template <int N, typename Scalar>
struct logical_left_shift_op {
  Scalar operator()(const Scalar& v) const { return numext::logical_shift_left(v, N); }
  template <typename Packet>
  Packet packetOp(const Packet& v) const {
    return internal::plogical_shift_left<N>(v);
  }
};
template <int N, typename Scalar>
struct logical_right_shift_op {
  Scalar operator()(const Scalar& v) const { return numext::logical_shift_right(v, N); }
  template <typename Packet>
  Packet packetOp(const Packet& v) const {
    return internal::plogical_shift_right<N>(v);
  }
};
template <int N, typename Scalar>
struct arithmetic_right_shift_op {
  Scalar operator()(const Scalar& v) const { return numext::arithmetic_shift_right(v, N); }
  template <typename Packet>
  Packet packetOp(const Packet& v) const {
    return internal::parithmetic_shift_right<N>(v);
  }
};

namespace Eigen {
namespace internal {
template <int N, typename Scalar>
struct functor_traits<logical_left_shift_op<N, Scalar>> : shift_imm_traits<Scalar> {};
template <int N, typename Scalar>
struct functor_traits<logical_right_shift_op<N, Scalar>> : shift_imm_traits<Scalar> {};
template <int N, typename Scalar>
struct functor_traits<arithmetic_right_shift_op<N, Scalar>> : shift_imm_traits<Scalar> {};
}  // namespace internal
}  // namespace Eigen

template <typename ArrayType>
struct shift_test_impl {
  typedef typename ArrayType::Scalar Scalar;
  static constexpr size_t Size = sizeof(Scalar);
  static constexpr size_t MaxShift = (CHAR_BIT * Size) - 1;

  template <size_t N = 1>
  static inline std::enable_if_t<(N > MaxShift), void> run(const ArrayType&) {}
  template <size_t N = 1>
  static inline std::enable_if_t<(N <= MaxShift), void> run(const ArrayType& m) {
    const Index rows = m.rows();
    const Index cols = m.cols();

    ArrayType m1 = ArrayType::Random(rows, cols), m2(rows, cols), m3(rows, cols);

    m2 = m1.unaryExpr([](const Scalar& v) { return numext::logical_shift_left(v, N); });
    m3 = m1.unaryExpr(logical_left_shift_op<N, Scalar>());
    VERIFY_IS_CWISE_EQUAL(m2, m3);

    m2 = m1.unaryExpr([](const Scalar& v) { return numext::logical_shift_right(v, N); });
    m3 = m1.unaryExpr(logical_right_shift_op<N, Scalar>());
    VERIFY_IS_CWISE_EQUAL(m2, m3);

    m2 = m1.unaryExpr([](const Scalar& v) { return numext::arithmetic_shift_right(v, N); });
    m3 = m1.unaryExpr(arithmetic_right_shift_op<N, Scalar>());
    VERIFY_IS_CWISE_EQUAL(m2, m3);

    run<N + 1>(m);
  }
};
template <typename ArrayType>
void shift_test(const ArrayType& m) {
  shift_test_impl<ArrayType>::run(m);
}

template <typename ArrayType>
struct typed_logicals_test_impl {
  using Scalar = typename ArrayType::Scalar;

  static bool scalar_to_bool(const Scalar& x) { return x != Scalar(0); }
  static Scalar bool_to_scalar(bool x) { return x ? Scalar(1) : Scalar(0); }

  static Scalar eval_bool_and(const Scalar& x, const Scalar& y) {
    return bool_to_scalar(scalar_to_bool(x) && scalar_to_bool(y));
  }
  static Scalar eval_bool_or(const Scalar& x, const Scalar& y) {
    return bool_to_scalar(scalar_to_bool(x) || scalar_to_bool(y));
  }
  static Scalar eval_bool_xor(const Scalar& x, const Scalar& y) {
    return bool_to_scalar(scalar_to_bool(x) != scalar_to_bool(y));
  }
  static Scalar eval_bool_not(const Scalar& x) { return bool_to_scalar(!scalar_to_bool(x)); }

  static void run(const ArrayType& m) {
    Index rows = m.rows();
    Index cols = m.cols();

    ArrayType m1(rows, cols), m2(rows, cols), m3(rows, cols), m4(rows, cols);

    m1.setRandom();
    m2.setRandom();
    m1 *= ArrayX<bool>::Random(rows, cols).cast<Scalar>();
    m2 *= ArrayX<bool>::Random(rows, cols).cast<Scalar>();

    // test boolean and
    m3 = m1 && m2;
    m4 = m1.binaryExpr(m2, [](const Scalar& x, const Scalar& y) { return eval_bool_and(x, y); });
    VERIFY_IS_CWISE_EQUAL(m3, m4);
    for (const Scalar& val : m3) VERIFY(val == Scalar(0) || val == Scalar(1));

    // test boolean or
    m3 = m1 || m2;
    m4 = m1.binaryExpr(m2, [](const Scalar& x, const Scalar& y) { return eval_bool_or(x, y); });
    VERIFY_IS_CWISE_EQUAL(m3, m4);
    for (const Scalar& val : m3) VERIFY(val == Scalar(0) || val == Scalar(1));

    // test boolean xor
    m3 = m1.binaryExpr(m2, internal::scalar_boolean_xor_op<Scalar>());
    m4 = m1.binaryExpr(m2, [](const Scalar& x, const Scalar& y) { return eval_bool_xor(x, y); });
    VERIFY_IS_CWISE_EQUAL(m3, m4);
    for (const Scalar& val : m3) VERIFY(val == Scalar(0) || val == Scalar(1));

    // test boolean not
    m3 = !m1;
    m4 = m1.unaryExpr([](const Scalar& x) { return eval_bool_not(x); });
    VERIFY_IS_CWISE_EQUAL(m3, m4);
    for (const Scalar& val : m3) VERIFY(val == Scalar(0) || val == Scalar(1));

    // test something more complicated
    m3 = m1 && m2;
    m4 = !(!m1 || !m2);
    VERIFY_IS_CWISE_EQUAL(m3, m4);

    m3 = m1.binaryExpr(m2, internal::scalar_boolean_xor_op<Scalar>());
    m4 = (!m1).binaryExpr((!m2), internal::scalar_boolean_xor_op<Scalar>());
    VERIFY_IS_CWISE_EQUAL(m3, m4);

    const size_t bytes = size_t(rows) * size_t(cols) * sizeof(Scalar);

    std::vector<uint8_t> m1_buffer(bytes), m2_buffer(bytes), m3_buffer(bytes), m4_buffer(bytes);

    std::memcpy(m1_buffer.data(), m1.data(), bytes);
    std::memcpy(m2_buffer.data(), m2.data(), bytes);

    // test bitwise and
    m3 = m1 & m2;
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], uint8_t(m1_buffer[i] & m2_buffer[i]));

    // test bitwise or
    m3 = m1 | m2;
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], uint8_t(m1_buffer[i] | m2_buffer[i]));

    // test bitwise xor
    m3 = m1 ^ m2;
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], uint8_t(m1_buffer[i] ^ m2_buffer[i]));

    // test bitwise not
    m3 = ~m1;
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], uint8_t(~m1_buffer[i]));

    // test something more complicated
    m3 = m1 & m2;
    m4 = ~(~m1 | ~m2);
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    std::memcpy(m4_buffer.data(), m4.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], m4_buffer[i]);

    m3 = m1 ^ m2;
    m4 = (~m1) ^ (~m2);
    std::memcpy(m3_buffer.data(), m3.data(), bytes);
    std::memcpy(m4_buffer.data(), m4.data(), bytes);
    for (size_t i = 0; i < bytes; i++) VERIFY_IS_EQUAL(m3_buffer[i], m4_buffer[i]);
  }
};
template <typename ArrayType>
void typed_logicals_test(const ArrayType& m) {
  typed_logicals_test_impl<ArrayType>::run(m);
}

template <typename SrcType, typename DstType, int RowsAtCompileTime, int ColsAtCompileTime>
struct cast_test_impl {
  using SrcArray = Array<SrcType, RowsAtCompileTime, ColsAtCompileTime>;
  using DstArray = Array<DstType, RowsAtCompileTime, ColsAtCompileTime>;
  struct RandomOp {
    inline SrcType operator()(const SrcType&) const {
      return internal::random_without_cast_overflow<SrcType, DstType>::value();
    }
  };

  static constexpr int SrcPacketSize = internal::packet_traits<SrcType>::size;
  static constexpr int DstPacketSize = internal::packet_traits<DstType>::size;
  static constexpr int MaxPacketSize = internal::plain_enum_max(SrcPacketSize, DstPacketSize);

  static void run() {
    const Index testRows = RowsAtCompileTime == Dynamic ? ((10 * MaxPacketSize) + 1) : RowsAtCompileTime;
    const Index testCols = ColsAtCompileTime == Dynamic ? ((10 * MaxPacketSize) + 1) : ColsAtCompileTime;
    const Index testSize = testRows * testCols;
    const Index minTestSize = 100;
    const Index repeats = numext::div_ceil(minTestSize, testSize);

    SrcArray src(testRows, testCols);
    DstArray dst(testRows, testCols);

    for (Index repeat = 0; repeat < repeats; repeat++) {
      src = src.unaryExpr(RandomOp());
      dst = src.template cast<DstType>();

      for (Index j = 0; j < testCols; j++)
        for (Index i = 0; i < testRows; i++) {
          SrcType srcVal = src(i, j);
          DstType refVal = internal::cast_impl<SrcType, DstType>::run(srcVal);
          DstType dstVal = dst(i, j);
          bool isApprox = verifyIsApprox(dstVal, refVal);
          if (!isApprox)
            std::cout << type_name(srcVal) << ": [" << +srcVal << "] to " << type_name(dstVal) << ": [" << +dstVal
                      << "] != [" << +refVal << "]\n";
          VERIFY(isApprox);
        }
    }
  }
};

template <int RowsAtCompileTime, int ColsAtCompileTime, typename... ScalarTypes>
struct cast_tests_impl {
  using ScalarTuple = std::tuple<ScalarTypes...>;
  static constexpr size_t ScalarTupleSize = std::tuple_size<ScalarTuple>::value;

  template <size_t i = 0, size_t j = i + 1, bool Done = (i >= ScalarTupleSize - 1) || (j >= ScalarTupleSize)>
  static std::enable_if_t<Done> run() {}

  template <size_t i = 0, size_t j = i + 1, bool Done = (i >= ScalarTupleSize - 1) || (j >= ScalarTupleSize)>
  static std::enable_if_t<!Done> run() {
    using Type1 = typename std::tuple_element<i, ScalarTuple>::type;
    using Type2 = typename std::tuple_element<j, ScalarTuple>::type;
    cast_test_impl<Type1, Type2, RowsAtCompileTime, ColsAtCompileTime>::run();
    cast_test_impl<Type2, Type1, RowsAtCompileTime, ColsAtCompileTime>::run();
    static constexpr size_t next_i = (j == ScalarTupleSize - 1) ? (i + 1) : (i + 0);
    static constexpr size_t next_j = (j == ScalarTupleSize - 1) ? (i + 2) : (j + 1);
    run<next_i, next_j>();
  }
};

// for now, remove all references to 'long double' until test passes on all platforms
template <int RowsAtCompileTime, int ColsAtCompileTime>
void cast_test() {
  cast_tests_impl<RowsAtCompileTime, ColsAtCompileTime, bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t,
                  uint32_t, uint64_t, float, double, /*long double, */ half, bfloat16>::run();
}

EIGEN_DECLARE_TEST(array_cwise) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(array_generic(Array<float, 1, 1>()));
    CALL_SUBTEST_2(array_generic(Array22f()));
    CALL_SUBTEST_3(array_generic(Array44d()));
    CALL_SUBTEST_4(array_generic(
        ArrayXXcf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_5(array_generic(
        ArrayXXf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_8(array_generic(
        ArrayXXi(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_7(array_generic(Array<Index, Dynamic, Dynamic>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE),
                                                                internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_8(shift_test(
        ArrayXXi(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_9(shift_test(Array<Index, Dynamic, Dynamic>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE),
                                                             internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_10(array_generic(Array<uint32_t, Dynamic, Dynamic>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE),
                                                                    internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_11(array_generic(Array<uint64_t, Dynamic, Dynamic>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE),
                                                                    internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(comparisons(Array<float, 1, 1>()));
    CALL_SUBTEST_2(comparisons(Array22f()));
    CALL_SUBTEST_3(comparisons(Array44d()));
    CALL_SUBTEST_7(comparisons(
        ArrayXXf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_8(comparisons(
        ArrayXXi(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_6(min_max(Array<float, 1, 1>()));
    CALL_SUBTEST_7(min_max(Array22f()));
    CALL_SUBTEST_8(min_max(Array44d()));
    CALL_SUBTEST_9(min_max(
        ArrayXXf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_10(min_max(
        ArrayXXi(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_11(array_real(Array<float, 1, 1>()));
    CALL_SUBTEST_12(array_real(Array22f()));
    CALL_SUBTEST_13(array_real(Array44d()));
    CALL_SUBTEST_14(array_real(
        ArrayXXf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_15(array_real(Array<Eigen::half, 32, 32>()));
    CALL_SUBTEST_16(array_real(Array<Eigen::bfloat16, 32, 32>()));
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_17(array_complex(
        ArrayXXcf(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_18(array_complex(
        ArrayXXcd(internal::random<int>(1, EIGEN_TEST_MAX_SIZE), internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }

  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_19(float_pow_test());
    CALL_SUBTEST_20(int_pow_test());
    CALL_SUBTEST_21(mixed_pow_test());
    CALL_SUBTEST_22(signbit_tests());
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_23(typed_logicals_test(ArrayX<int>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_24(typed_logicals_test(ArrayX<float>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_25(typed_logicals_test(ArrayX<double>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_26(typed_logicals_test(ArrayX<std::complex<float>>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
    CALL_SUBTEST_27(typed_logicals_test(ArrayX<std::complex<double>>(internal::random<int>(1, EIGEN_TEST_MAX_SIZE))));
  }

  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_28((cast_test<1, 1>()));
    CALL_SUBTEST_29((cast_test<3, 1>()));
    CALL_SUBTEST_30((cast_test<5, 1>()));
    CALL_SUBTEST_31((cast_test<9, 1>()));
    CALL_SUBTEST_32((cast_test<17, 1>()));
    CALL_SUBTEST_33((cast_test<Dynamic, 1>()));
  }

  VERIFY((internal::is_same<internal::global_math_functions_filtering_base<int>::type, int>::value));
  VERIFY((internal::is_same<internal::global_math_functions_filtering_base<float>::type, float>::value));
  VERIFY((internal::is_same<internal::global_math_functions_filtering_base<Array2i>::type, ArrayBase<Array2i>>::value));
  typedef CwiseUnaryOp<internal::scalar_abs_op<double>, ArrayXd> Xpr;
  VERIFY((internal::is_same<internal::global_math_functions_filtering_base<Xpr>::type, ArrayBase<Xpr>>::value));
}
