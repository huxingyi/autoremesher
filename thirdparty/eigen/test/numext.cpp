// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2017 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename T, typename U>
bool check_if_equal_or_nans(const T& actual, const U& expected) {
  return (numext::equal_strict(actual, expected) || ((numext::isnan)(actual) && (numext::isnan)(expected)));
}

template <typename T, typename U>
bool check_if_equal_or_nans(const std::complex<T>& actual, const std::complex<U>& expected) {
  return check_if_equal_or_nans(numext::real(actual), numext::real(expected)) &&
         check_if_equal_or_nans(numext::imag(actual), numext::imag(expected));
}

template <typename T, typename U>
bool test_is_equal_or_nans(const T& actual, const U& expected) {
  if (check_if_equal_or_nans(actual, expected)) {
    return true;
  }

  // false:
  std::cerr << "\n    actual   = " << actual << "\n    expected = " << expected << "\n\n";
  return false;
}

#define VERIFY_IS_EQUAL_OR_NANS(a, b) VERIFY(test_is_equal_or_nans(a, b))

template <typename T>
void check_negate() {
  Index size = 1000;
  for (Index i = 0; i < size; i++) {
    T val = i == 0 ? T(0) : internal::random<T>(T(0), NumTraits<T>::highest());
    T neg_val = numext::negate(val);
    VERIFY_IS_EQUAL(T(val + neg_val), T(0));
    VERIFY_IS_EQUAL(numext::negate(neg_val), val);
  }
}

template <typename T>
void check_abs() {
  typedef typename NumTraits<T>::Real Real;
  Real zero(0);

  if (NumTraits<T>::IsSigned) VERIFY_IS_EQUAL(numext::abs(numext::negate(T(1))), T(1));
  VERIFY_IS_EQUAL(numext::abs(T(0)), T(0));
  VERIFY_IS_EQUAL(numext::abs(T(1)), T(1));

  for (int k = 0; k < 100; ++k) {
    T x = internal::random<T>();
    x = x / Real(2);
    if (NumTraits<T>::IsSigned) {
      VERIFY_IS_EQUAL(numext::abs(x), numext::abs(numext::negate(x)));
      VERIFY(numext::abs(numext::negate(x)) >= zero);
    }
    VERIFY(numext::abs(x) >= zero);
    VERIFY_IS_APPROX(numext::abs2(x), numext::abs2(numext::abs(x)));
  }
}

template <>
void check_abs<bool>() {
  for (bool x : {true, false}) {
    VERIFY_IS_EQUAL(numext::abs(x), x);
    VERIFY(numext::abs(x) >= false);
    VERIFY_IS_EQUAL(numext::abs2(x), numext::abs2(numext::abs(x)));
  }
}

template <typename T>
void check_arg() {
  typedef typename NumTraits<T>::Real Real;
  VERIFY_IS_EQUAL(numext::abs(T(0)), T(0));
  VERIFY_IS_EQUAL(numext::abs(T(1)), T(1));

  for (int k = 0; k < 100; ++k) {
    T x = internal::random<T>();
    Real y = numext::arg(x);
    VERIFY_IS_APPROX(y, std::arg(x));
  }
}

template <typename T>
struct check_sqrt_impl {
  static void run() {
    for (int i = 0; i < 1000; ++i) {
      const T x = numext::abs(internal::random<T>());
      const T sqrtx = numext::sqrt(x);
      VERIFY_IS_APPROX(sqrtx * sqrtx, x);
    }

    // Corner cases.
    const T zero = T(0);
    const T one = T(1);
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();
    VERIFY_IS_EQUAL(numext::sqrt(zero), zero);
    VERIFY_IS_EQUAL(numext::sqrt(inf), inf);
    VERIFY((numext::isnan)(numext::sqrt(nan)));
    VERIFY((numext::isnan)(numext::sqrt(-one)));
  }
};

template <typename T>
struct check_sqrt_impl<std::complex<T> > {
  static void run() {
    typedef typename std::complex<T> ComplexT;

    for (int i = 0; i < 1000; ++i) {
      const ComplexT x = internal::random<ComplexT>();
      const ComplexT sqrtx = numext::sqrt(x);
      VERIFY_IS_APPROX(sqrtx * sqrtx, x);
    }

    // Corner cases.
    const T zero = T(0);
    const T one = T(1);
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();

    // Set of corner cases from https://en.cppreference.com/w/cpp/numeric/complex/sqrt
    const int kNumCorners = 20;
    const ComplexT corners[kNumCorners][2] = {
        {ComplexT(zero, zero), ComplexT(zero, zero)},  {ComplexT(-zero, zero), ComplexT(zero, zero)},
        {ComplexT(zero, -zero), ComplexT(zero, zero)}, {ComplexT(-zero, -zero), ComplexT(zero, zero)},
        {ComplexT(one, inf), ComplexT(inf, inf)},      {ComplexT(nan, inf), ComplexT(inf, inf)},
        {ComplexT(one, -inf), ComplexT(inf, -inf)},    {ComplexT(nan, -inf), ComplexT(inf, -inf)},
        {ComplexT(-inf, one), ComplexT(zero, inf)},    {ComplexT(inf, one), ComplexT(inf, zero)},
        {ComplexT(-inf, -one), ComplexT(zero, -inf)},  {ComplexT(inf, -one), ComplexT(inf, -zero)},
        {ComplexT(-inf, nan), ComplexT(nan, inf)},     {ComplexT(inf, nan), ComplexT(inf, nan)},
        {ComplexT(zero, nan), ComplexT(nan, nan)},     {ComplexT(one, nan), ComplexT(nan, nan)},
        {ComplexT(nan, zero), ComplexT(nan, nan)},     {ComplexT(nan, one), ComplexT(nan, nan)},
        {ComplexT(nan, -one), ComplexT(nan, nan)},     {ComplexT(nan, nan), ComplexT(nan, nan)},
    };

    for (int i = 0; i < kNumCorners; ++i) {
      const ComplexT& x = corners[i][0];
      const ComplexT sqrtx = corners[i][1];
      VERIFY_IS_EQUAL_OR_NANS(numext::sqrt(x), sqrtx);
    }
  }
};

template <typename T>
void check_sqrt() {
  check_sqrt_impl<T>::run();
}

template <typename T>
struct check_rsqrt_impl {
  static void run() {
    const T zero = T(0);
    const T one = T(1);
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();

    for (int i = 0; i < 1000; ++i) {
      const T x = numext::abs(internal::random<T>());
      const T rsqrtx = numext::rsqrt(x);
      const T invx = one / x;
      VERIFY_IS_APPROX(rsqrtx * rsqrtx, invx);
    }

    // Corner cases.
    VERIFY_IS_EQUAL(numext::rsqrt(zero), inf);
    VERIFY_IS_EQUAL(numext::rsqrt(inf), zero);
    VERIFY((numext::isnan)(numext::rsqrt(nan)));
    VERIFY((numext::isnan)(numext::rsqrt(-one)));
  }
};

template <typename T>
struct check_rsqrt_impl<std::complex<T> > {
  static void run() {
    typedef typename std::complex<T> ComplexT;
    const T zero = T(0);
    const T one = T(1);
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();

    for (int i = 0; i < 1000; ++i) {
      const ComplexT x = internal::random<ComplexT>();
      const ComplexT invx = ComplexT(one, zero) / x;
      const ComplexT rsqrtx = numext::rsqrt(x);
      VERIFY_IS_APPROX(rsqrtx * rsqrtx, invx);
    }

// GCC and MSVC differ in their treatment of 1/(0 + 0i)
//   GCC/clang = (inf, nan)
//   MSVC = (nan, nan)
// and 1 / (x + inf i)
//   GCC/clang = (0, 0)
//   MSVC = (nan, nan)
#if (EIGEN_COMP_GNUC)
    {
      const int kNumCorners = 20;
      const ComplexT corners[kNumCorners][2] = {
          // Only consistent across GCC, clang
          {ComplexT(zero, zero), ComplexT(zero, zero)},
          {ComplexT(-zero, zero), ComplexT(zero, zero)},
          {ComplexT(zero, -zero), ComplexT(zero, zero)},
          {ComplexT(-zero, -zero), ComplexT(zero, zero)},
          {ComplexT(one, inf), ComplexT(inf, inf)},
          {ComplexT(nan, inf), ComplexT(inf, inf)},
          {ComplexT(one, -inf), ComplexT(inf, -inf)},
          {ComplexT(nan, -inf), ComplexT(inf, -inf)},
          // Consistent across GCC, clang, MSVC
          {ComplexT(-inf, one), ComplexT(zero, inf)},
          {ComplexT(inf, one), ComplexT(inf, zero)},
          {ComplexT(-inf, -one), ComplexT(zero, -inf)},
          {ComplexT(inf, -one), ComplexT(inf, -zero)},
          {ComplexT(-inf, nan), ComplexT(nan, inf)},
          {ComplexT(inf, nan), ComplexT(inf, nan)},
          {ComplexT(zero, nan), ComplexT(nan, nan)},
          {ComplexT(one, nan), ComplexT(nan, nan)},
          {ComplexT(nan, zero), ComplexT(nan, nan)},
          {ComplexT(nan, one), ComplexT(nan, nan)},
          {ComplexT(nan, -one), ComplexT(nan, nan)},
          {ComplexT(nan, nan), ComplexT(nan, nan)},
      };

      for (int i = 0; i < kNumCorners; ++i) {
        const ComplexT& x = corners[i][0];
        const ComplexT rsqrtx = ComplexT(one, zero) / corners[i][1];
        VERIFY_IS_EQUAL_OR_NANS(numext::rsqrt(x), rsqrtx);
      }
    }
#endif
  }
};

template <typename T>
void check_rsqrt() {
  check_rsqrt_impl<T>::run();
}

template <typename T>
struct check_signbit_impl {
  static void run() {
    T true_mask;
    std::memset(static_cast<void*>(&true_mask), 0xff, sizeof(T));
    T false_mask;
    std::memset(static_cast<void*>(&false_mask), 0x00, sizeof(T));

    std::vector<T> negative_values;
    std::vector<T> non_negative_values;

    if (NumTraits<T>::IsInteger) {
      negative_values = {static_cast<T>(-1), static_cast<T>(NumTraits<T>::lowest())};
      non_negative_values = {static_cast<T>(0), static_cast<T>(1), static_cast<T>(NumTraits<T>::highest())};
    } else {
      // does not have sign bit
      const T pos_zero = static_cast<T>(0.0);
      const T pos_one = static_cast<T>(1.0);
      const T pos_inf = std::numeric_limits<T>::infinity();
      const T pos_nan = std::numeric_limits<T>::quiet_NaN();
      // has sign bit
      const T neg_zero = numext::negate(pos_zero);
      const T neg_one = numext::negate(pos_one);
      const T neg_inf = numext::negate(pos_inf);
      const T neg_nan = numext::negate(pos_nan);

      negative_values = {neg_zero, neg_one, neg_inf, neg_nan};
      non_negative_values = {pos_zero, pos_one, pos_inf, pos_nan};
    }

    auto check_all = [](auto values, auto expected) {
      bool all_pass = true;
      for (T val : values) {
        const T numext_val = numext::signbit(val);
        bool not_same = internal::predux_any(internal::bitwise_helper<T>::bitwise_xor(expected, numext_val));
        all_pass = all_pass && !not_same;
        if (not_same) std::cout << "signbit(" << val << ") = " << numext_val << " != " << expected << std::endl;
      }
      return all_pass;
    };

    bool check_all_pass = check_all(non_negative_values, false_mask);
    check_all_pass = check_all_pass && check_all(negative_values, (NumTraits<T>::IsSigned ? true_mask : false_mask));
    VERIFY(check_all_pass);
  }
};
template <typename T>
void check_signbit() {
  check_signbit_impl<T>::run();
}

template <typename T>
void check_shift() {
  using SignedT = typename numext::get_integer_by_size<sizeof(T)>::signed_type;
  using UnsignedT = typename numext::get_integer_by_size<sizeof(T)>::unsigned_type;
  constexpr int kNumBits = CHAR_BIT * sizeof(T);
  for (int i = 0; i < 1000; ++i) {
    const T a = internal::random<T>();
    for (int s = 1; s < kNumBits; s++) {
      T a_bsll = numext::logical_shift_left(a, s);
      T a_bsll_ref = a << s;
      VERIFY_IS_EQUAL(a_bsll, a_bsll_ref);
      T a_bsrl = numext::logical_shift_right(a, s);
      T a_bsrl_ref = numext::bit_cast<T, UnsignedT>(numext::bit_cast<UnsignedT, T>(a) >> s);
      VERIFY_IS_EQUAL(a_bsrl, a_bsrl_ref);
      T a_bsra = numext::arithmetic_shift_right(a, s);
      T a_bsra_ref = numext::bit_cast<T, SignedT>(numext::bit_cast<SignedT, T>(a) >> s);
      VERIFY_IS_EQUAL(a_bsra, a_bsra_ref);
    }
  }
}

EIGEN_DECLARE_TEST(numext) {
  for (int k = 0; k < g_repeat; ++k) {
    CALL_SUBTEST(check_negate<signed char>());
    CALL_SUBTEST(check_negate<unsigned char>());
    CALL_SUBTEST(check_negate<short>());
    CALL_SUBTEST(check_negate<unsigned short>());
    CALL_SUBTEST(check_negate<int>());
    CALL_SUBTEST(check_negate<unsigned int>());
    CALL_SUBTEST(check_negate<long>());
    CALL_SUBTEST(check_negate<unsigned long>());
    CALL_SUBTEST(check_negate<half>());
    CALL_SUBTEST(check_negate<bfloat16>());
    CALL_SUBTEST(check_negate<float>());
    CALL_SUBTEST(check_negate<double>());
    CALL_SUBTEST(check_negate<long double>());
    CALL_SUBTEST(check_negate<std::complex<float> >());
    CALL_SUBTEST(check_negate<std::complex<double> >());

    CALL_SUBTEST(check_abs<bool>());
    CALL_SUBTEST(check_abs<signed char>());
    CALL_SUBTEST(check_abs<unsigned char>());
    CALL_SUBTEST(check_abs<short>());
    CALL_SUBTEST(check_abs<unsigned short>());
    CALL_SUBTEST(check_abs<int>());
    CALL_SUBTEST(check_abs<unsigned int>());
    CALL_SUBTEST(check_abs<long>());
    CALL_SUBTEST(check_abs<unsigned long>());
    CALL_SUBTEST(check_abs<half>());
    CALL_SUBTEST(check_abs<bfloat16>());
    CALL_SUBTEST(check_abs<float>());
    CALL_SUBTEST(check_abs<double>());
    CALL_SUBTEST(check_abs<long double>());
    CALL_SUBTEST(check_abs<std::complex<float> >());
    CALL_SUBTEST(check_abs<std::complex<double> >());

    CALL_SUBTEST(check_arg<std::complex<float> >());
    CALL_SUBTEST(check_arg<std::complex<double> >());

    CALL_SUBTEST(check_sqrt<float>());
    CALL_SUBTEST(check_sqrt<double>());
    CALL_SUBTEST(check_sqrt<std::complex<float> >());
    CALL_SUBTEST(check_sqrt<std::complex<double> >());

    CALL_SUBTEST(check_rsqrt<float>());
    CALL_SUBTEST(check_rsqrt<double>());
    CALL_SUBTEST(check_rsqrt<std::complex<float> >());
    CALL_SUBTEST(check_rsqrt<std::complex<double> >());

    CALL_SUBTEST(check_signbit<half>());
    CALL_SUBTEST(check_signbit<bfloat16>());
    CALL_SUBTEST(check_signbit<float>());
    CALL_SUBTEST(check_signbit<double>());

    CALL_SUBTEST(check_signbit<uint8_t>());
    CALL_SUBTEST(check_signbit<uint16_t>());
    CALL_SUBTEST(check_signbit<uint32_t>());
    CALL_SUBTEST(check_signbit<uint64_t>());

    CALL_SUBTEST(check_signbit<int8_t>());
    CALL_SUBTEST(check_signbit<int16_t>());
    CALL_SUBTEST(check_signbit<int32_t>());
    CALL_SUBTEST(check_signbit<int64_t>());

    CALL_SUBTEST(check_shift<int8_t>());
    CALL_SUBTEST(check_shift<int16_t>());
    CALL_SUBTEST(check_shift<int32_t>());
    CALL_SUBTEST(check_shift<int64_t>());

    CALL_SUBTEST(check_shift<uint8_t>());
    CALL_SUBTEST(check_shift<uint16_t>());
    CALL_SUBTEST(check_shift<uint32_t>());
    CALL_SUBTEST(check_shift<uint64_t>());
  }
}
