// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2024 Tobias Wood tobias@spinicist.org.uk
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

#include <Eigen/CXX11/Tensor>

using Eigen::array;
using Eigen::Tensor;

template <int DataLayout>
static void test_simple_roll() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();

  array<Index, 4> dim_roll;
  dim_roll[0] = 0;
  dim_roll[1] = 1;
  dim_roll[2] = 4;
  dim_roll[3] = 8;

  Tensor<float, 4, DataLayout> rolled_tensor;
  rolled_tensor = tensor.roll(dim_roll);

  VERIFY_IS_EQUAL(rolled_tensor.dimension(0), 2);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(1), 3);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(2), 5);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(3), 7);

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 5; ++k) {
        for (int l = 0; l < 7; ++l) {
          VERIFY_IS_EQUAL(tensor(i, (j + 1) % 3, (k + 4) % 5, (l + 8) % 7), rolled_tensor(i, j, k, l));
        }
      }
    }
  }

  dim_roll[0] = -3;
  dim_roll[1] = -2;
  dim_roll[2] = -1;
  dim_roll[3] = 0;

  rolled_tensor = tensor.roll(dim_roll);

  VERIFY_IS_EQUAL(rolled_tensor.dimension(0), 2);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(1), 3);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(2), 5);
  VERIFY_IS_EQUAL(rolled_tensor.dimension(3), 7);

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 5; ++k) {
        for (int l = 0; l < 7; ++l) {
          VERIFY_IS_EQUAL(tensor((i + 1) % 2, (j + 1) % 3, (k + 4) % 5, l), rolled_tensor(i, j, k, l));
        }
      }
    }
  }
}

template <int DataLayout>
static void test_expr_roll(bool LValue) {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();

  array<bool, 4> dim_roll;
  dim_roll[0] = 2;
  dim_roll[1] = 1;
  dim_roll[2] = 0;
  dim_roll[3] = 3;

  Tensor<float, 4, DataLayout> expected(tensor.dimensions());
  if (LValue) {
    expected.roll(dim_roll) = tensor;
  } else {
    expected = tensor.roll(dim_roll);
  }

  Tensor<float, 4, DataLayout> result(tensor.dimensions());

  array<ptrdiff_t, 4> src_slice_dim;
  src_slice_dim[0] = tensor.dimension(0);
  src_slice_dim[1] = tensor.dimension(1);
  src_slice_dim[2] = 1;
  src_slice_dim[3] = tensor.dimension(3);
  array<ptrdiff_t, 4> src_slice_start;
  src_slice_start[0] = 0;
  src_slice_start[1] = 0;
  src_slice_start[2] = 0;
  src_slice_start[3] = 0;
  array<ptrdiff_t, 4> dst_slice_dim = src_slice_dim;
  array<ptrdiff_t, 4> dst_slice_start = src_slice_start;

  for (int i = 0; i < tensor.dimension(2); ++i) {
    if (LValue) {
      result.slice(dst_slice_start, dst_slice_dim).roll(dim_roll) = tensor.slice(src_slice_start, src_slice_dim);
    } else {
      result.slice(dst_slice_start, dst_slice_dim) = tensor.slice(src_slice_start, src_slice_dim).roll(dim_roll);
    }
    src_slice_start[2] += 1;
    dst_slice_start[2] += 1;
  }

  VERIFY_IS_EQUAL(result.dimension(0), tensor.dimension(0));
  VERIFY_IS_EQUAL(result.dimension(1), tensor.dimension(1));
  VERIFY_IS_EQUAL(result.dimension(2), tensor.dimension(2));
  VERIFY_IS_EQUAL(result.dimension(3), tensor.dimension(3));

  for (int i = 0; i < expected.dimension(0); ++i) {
    for (int j = 0; j < expected.dimension(1); ++j) {
      for (int k = 0; k < expected.dimension(2); ++k) {
        for (int l = 0; l < expected.dimension(3); ++l) {
          VERIFY_IS_EQUAL(result(i, j, k, l), expected(i, j, k, l));
        }
      }
    }
  }

  dst_slice_start[2] = 0;
  result.setRandom();
  for (int i = 0; i < tensor.dimension(2); ++i) {
    if (LValue) {
      result.slice(dst_slice_start, dst_slice_dim).roll(dim_roll) = tensor.slice(dst_slice_start, dst_slice_dim);
    } else {
      result.slice(dst_slice_start, dst_slice_dim) = tensor.roll(dim_roll).slice(dst_slice_start, dst_slice_dim);
    }
    dst_slice_start[2] += 1;
  }

  for (int i = 0; i < expected.dimension(0); ++i) {
    for (int j = 0; j < expected.dimension(1); ++j) {
      for (int k = 0; k < expected.dimension(2); ++k) {
        for (int l = 0; l < expected.dimension(3); ++l) {
          VERIFY_IS_EQUAL(result(i, j, k, l), expected(i, j, k, l));
        }
      }
    }
  }
}

EIGEN_DECLARE_TEST(cxx11_tensor_roll) {
  CALL_SUBTEST(test_simple_roll<ColMajor>());
  CALL_SUBTEST(test_simple_roll<RowMajor>());
  CALL_SUBTEST(test_expr_roll<ColMajor>(true));
  CALL_SUBTEST(test_expr_roll<RowMajor>(true));
  CALL_SUBTEST(test_expr_roll<ColMajor>(false));
  CALL_SUBTEST(test_expr_roll<RowMajor>(false));
}
