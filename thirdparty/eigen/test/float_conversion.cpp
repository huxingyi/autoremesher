// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <sstream>

#include "main.h"

template <typename From, typename To>
void test_conversion() {
  typedef Array<From, Dynamic, 1> ArrayXFrom;
  typedef Array<To, Dynamic, 1> ArrayXTo;
  typedef Array<double, Dynamic, 1> ArrayXDouble;

  Index size = internal::random<Index>(1, EIGEN_TEST_MAX_SIZE);
  double from_min = static_cast<double>((std::numeric_limits<From>::min)());
  double from_range = static_cast<double>((std::numeric_limits<From>::max)()) - from_min;

  // ArrayXFrom::Random() only generates 32-bit values (#2749), so we generate
  // doubles and scale to fit the range.
  ArrayXDouble doubles = (ArrayXDouble::Random(size) + 1.0) * (from_range / 2.0) + from_min;
  ArrayXFrom from = doubles.template cast<From>();
  ArrayXTo to(size);
  for (Index i = 0; i < size; ++i) {
    to(i) = static_cast<To>(from(i));
  }
  VERIFY_IS_APPROX(from.template cast<To>(), to);
}

template <typename To>
void test_conversion_to() {
  CALL_SUBTEST((test_conversion<int64_t, To>()));
  CALL_SUBTEST((test_conversion<uint64_t, To>()));
  CALL_SUBTEST((test_conversion<int32_t, To>()));
  CALL_SUBTEST((test_conversion<uint32_t, To>()));
  CALL_SUBTEST((test_conversion<int16_t, To>()));
  CALL_SUBTEST((test_conversion<uint16_t, To>()));
  CALL_SUBTEST((test_conversion<int8_t, To>()));
  CALL_SUBTEST((test_conversion<uint8_t, To>()));
}

EIGEN_DECLARE_TEST(float_conversion) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST(test_conversion_to<float>());
    CALL_SUBTEST(test_conversion_to<double>());
  }
}
