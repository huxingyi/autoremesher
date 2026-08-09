// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2014 Benoit Steiner <benoit.steiner.goog@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"
#include "random_without_cast_overflow.h"

#include <Eigen/CXX11/Tensor>

using Eigen::array;
using Eigen::Tensor;

template <typename FromType, typename ToType>
static void test_type_cast() {
  Tensor<FromType, 2> ftensor(101, 201);
  // Generate random values for a valid cast.
  for (int i = 0; i < 101; ++i) {
    for (int j = 0; j < 201; ++j) {
      ftensor(i, j) = internal::random_without_cast_overflow<FromType, ToType>::value();
    }
  }

  Tensor<ToType, 2> ttensor(101, 201);
  ttensor = ftensor.template cast<ToType>();

  for (int i = 0; i < 101; ++i) {
    for (int j = 0; j < 201; ++j) {
      const ToType ref = internal::cast<FromType, ToType>(ftensor(i, j));
      VERIFY_IS_EQUAL(ttensor(i, j), ref);
    }
  }
}

template <typename Scalar, typename EnableIf = void>
struct test_cast_runner {
  static void run() {
    test_type_cast<Scalar, bool>();
    test_type_cast<Scalar, int8_t>();
    test_type_cast<Scalar, int16_t>();
    test_type_cast<Scalar, int32_t>();
    test_type_cast<Scalar, int64_t>();
    test_type_cast<Scalar, uint8_t>();
    test_type_cast<Scalar, uint16_t>();
    test_type_cast<Scalar, uint32_t>();
    test_type_cast<Scalar, uint64_t>();
    test_type_cast<Scalar, half>();
    test_type_cast<Scalar, bfloat16>();
    test_type_cast<Scalar, float>();
    test_type_cast<Scalar, double>();
    test_type_cast<Scalar, std::complex<float>>();
    test_type_cast<Scalar, std::complex<double>>();
  }
};

// Only certain types allow cast from std::complex<>.
template <typename Scalar>
struct test_cast_runner<Scalar, std::enable_if_t<NumTraits<Scalar>::IsComplex>> {
  static void run() {
    test_type_cast<Scalar, half>();
    test_type_cast<Scalar, bfloat16>();
    test_type_cast<Scalar, std::complex<float>>();
    test_type_cast<Scalar, std::complex<double>>();
  }
};

EIGEN_DECLARE_TEST(cxx11_tensor_casts) {
  CALL_SUBTEST(test_cast_runner<bool>::run());
  CALL_SUBTEST(test_cast_runner<int8_t>::run());
  CALL_SUBTEST(test_cast_runner<int16_t>::run());
  CALL_SUBTEST(test_cast_runner<int32_t>::run());
  CALL_SUBTEST(test_cast_runner<int64_t>::run());
  CALL_SUBTEST(test_cast_runner<uint8_t>::run());
  CALL_SUBTEST(test_cast_runner<uint16_t>::run());
  CALL_SUBTEST(test_cast_runner<uint32_t>::run());
  CALL_SUBTEST(test_cast_runner<uint64_t>::run());
  CALL_SUBTEST(test_cast_runner<half>::run());
  CALL_SUBTEST(test_cast_runner<bfloat16>::run());
  CALL_SUBTEST(test_cast_runner<float>::run());
  CALL_SUBTEST(test_cast_runner<double>::run());
  CALL_SUBTEST(test_cast_runner<std::complex<float>>::run());
  CALL_SUBTEST(test_cast_runner<std::complex<double>>::run());
}
