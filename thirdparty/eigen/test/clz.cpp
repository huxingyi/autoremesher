// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2023 The Eigen Authors
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename T>
int ref_clz(T val) {
  constexpr int kNumBits = sizeof(T) * CHAR_BIT;
  T kMsbMask = T(1) << (kNumBits - 1);
  int z = 0;
  for (; z < kNumBits && ((val & kMsbMask) == 0); ++z) {
    val <<= 1;
  }
  return z;
}

template <typename T>
int ref_ctz(T val) {
  constexpr int kNumBits = sizeof(T) * CHAR_BIT;
  T kLsbMask = T(1);
  int z = 0;
  for (; z < kNumBits && ((val & kLsbMask) == 0); ++z) {
    val >>= 1;
  }
  return z;
}

template <typename T>
void test_clz_ctz() {
  T step = sizeof(T) <= 2 ? 1 : (Eigen::NumTraits<T>::highest() / (T(1) << 16));
  T iters = Eigen::NumTraits<T>::highest() / step;
  for (T i = 0; i < iters; ++i) {
    T val = i * step;
    int expected_clz = ref_clz(val);
    int actual_clz = Eigen::internal::clz(val);
    VERIFY(expected_clz == actual_clz);

    int expected_ctz = ref_ctz(val);
    int actual_ctz = Eigen::internal::ctz(val);
    VERIFY(expected_ctz == actual_ctz);
  }
}

template <typename T>
void test_clz_ctz_random() {
  for (int i = 0; i < 1024 * 1024; ++i) {
    T val = Eigen::internal::random<T>();
    int expected_clz = ref_clz(val);
    int actual_clz = Eigen::internal::clz(val);
    VERIFY(expected_clz == actual_clz);

    int expected_ctz = ref_ctz(val);
    int actual_ctz = Eigen::internal::ctz(val);
    VERIFY(expected_ctz == actual_ctz);
  }
}

EIGEN_DECLARE_TEST(clz) {
  CALL_SUBTEST_1(test_clz_ctz<uint8_t>());
  CALL_SUBTEST_2(test_clz_ctz<uint16_t>());
  CALL_SUBTEST_3(test_clz_ctz<uint32_t>());
  CALL_SUBTEST_4(test_clz_ctz<uint64_t>());

  for (int i = 0; i < g_repeat; i++) {
    test_clz_ctz_random<uint32_t>();
    test_clz_ctz_random<uint64_t>();
  }
}
