// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2015 Eugene Brevdo <ebrevdo@google.com>
//                    Benoit Steiner <benoit.steiner.goog@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

#include <Eigen/CXX11/Tensor>

using Eigen::array;
using Eigen::Pair;
using Eigen::Tensor;

template <int DataLayout>
static void test_simple_index_pairs() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();

  Tensor<Pair<DenseIndex, float>, 4, DataLayout> index_pairs(2, 3, 5, 7);
  index_pairs = tensor.index_pairs();

  for (DenseIndex n = 0; n < 2 * 3 * 5 * 7; ++n) {
    const Pair<DenseIndex, float>& v = index_pairs.coeff(n);
    VERIFY_IS_EQUAL(v.first, n);
    VERIFY_IS_EQUAL(v.second, tensor.coeff(n));
  }
}

template <int DataLayout>
static void test_index_pairs_dim() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();

  Tensor<Pair<DenseIndex, float>, 4, DataLayout> index_pairs(2, 3, 5, 7);

  index_pairs = tensor.index_pairs();

  for (Eigen::DenseIndex n = 0; n < tensor.size(); ++n) {
    const Pair<DenseIndex, float>& v = index_pairs(n);  //(i, j, k, l);
    VERIFY_IS_EQUAL(v.first, n);
    VERIFY_IS_EQUAL(v.second, tensor(n));
  }
}

template <int DataLayout>
static void test_argmax_pair_reducer() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();

  Tensor<Pair<DenseIndex, float>, 4, DataLayout> index_pairs(2, 3, 5, 7);
  index_pairs = tensor.index_pairs();

  Tensor<Pair<DenseIndex, float>, 0, DataLayout> reduced;
  DimensionList<DenseIndex, 4> dims;
  reduced = index_pairs.reduce(dims, internal::ArgMaxPairReducer<Pair<DenseIndex, float> >());

  Tensor<float, 0, DataLayout> maxi = tensor.maximum();

  VERIFY_IS_EQUAL(maxi(), reduced(0).second);

  array<DenseIndex, 3> reduce_dims;
  for (int d = 0; d < 3; ++d) reduce_dims[d] = d;
  Tensor<Pair<DenseIndex, float>, 1, DataLayout> reduced_by_dims(7);
  reduced_by_dims = index_pairs.reduce(reduce_dims, internal::ArgMaxPairReducer<Pair<DenseIndex, float> >());

  Tensor<float, 1, DataLayout> max_by_dims = tensor.maximum(reduce_dims);

  for (int l = 0; l < 7; ++l) {
    VERIFY_IS_EQUAL(max_by_dims(l), reduced_by_dims(l).second);
  }
}

template <int DataLayout>
static void test_argmin_pair_reducer() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();

  Tensor<Pair<DenseIndex, float>, 4, DataLayout> index_pairs(2, 3, 5, 7);
  index_pairs = tensor.index_pairs();

  Tensor<Pair<DenseIndex, float>, 0, DataLayout> reduced;
  DimensionList<DenseIndex, 4> dims;
  reduced = index_pairs.reduce(dims, internal::ArgMinPairReducer<Pair<DenseIndex, float> >());

  Tensor<float, 0, DataLayout> mini = tensor.minimum();

  VERIFY_IS_EQUAL(mini(), reduced(0).second);

  array<DenseIndex, 3> reduce_dims;
  for (int d = 0; d < 3; ++d) reduce_dims[d] = d;
  Tensor<Pair<DenseIndex, float>, 1, DataLayout> reduced_by_dims(7);
  reduced_by_dims = index_pairs.reduce(reduce_dims, internal::ArgMinPairReducer<Pair<DenseIndex, float> >());

  Tensor<float, 1, DataLayout> min_by_dims = tensor.minimum(reduce_dims);

  for (int l = 0; l < 7; ++l) {
    VERIFY_IS_EQUAL(min_by_dims(l), reduced_by_dims(l).second);
  }
}

template <int DataLayout>
static void test_simple_argmax() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();
  tensor(0, 0, 0, 0) = 10.0;

  Tensor<DenseIndex, 0, DataLayout> tensor_argmax;

  tensor_argmax = tensor.argmax();

  VERIFY_IS_EQUAL(tensor_argmax(0), 0);

  tensor(1, 2, 4, 6) = 20.0;

  tensor_argmax = tensor.argmax();

  VERIFY_IS_EQUAL(tensor_argmax(0), 2 * 3 * 5 * 7 - 1);
}

template <int DataLayout>
static void test_simple_argmin() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  tensor.setRandom();
  tensor = (tensor + tensor.constant(0.5)).log();
  tensor(0, 0, 0, 0) = -10.0;

  Tensor<DenseIndex, 0, DataLayout> tensor_argmin;

  tensor_argmin = tensor.argmin();

  VERIFY_IS_EQUAL(tensor_argmin(0), 0);

  tensor(1, 2, 4, 6) = -20.0;

  tensor_argmin = tensor.argmin();

  VERIFY_IS_EQUAL(tensor_argmin(0), 2 * 3 * 5 * 7 - 1);
}

template <int DataLayout>
static void test_argmax_dim() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  std::vector<int> dims{2, 3, 5, 7};

  for (int dim = 0; dim < 4; ++dim) {
    tensor.setRandom();
    tensor = (tensor + tensor.constant(0.5)).log();

    Tensor<DenseIndex, 3, DataLayout> tensor_argmax;
    array<DenseIndex, 4> ix;
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          for (int l = 0; l < 7; ++l) {
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;
            ix[3] = l;
            if (ix[dim] != 0) continue;
            // suppose dim == 1, then for all i, k, l, set tensor(i, 0, k, l) = 10.0
            tensor(ix) = 10.0;
          }
        }
      }
    }

    tensor_argmax = tensor.argmax(dim);

    VERIFY_IS_EQUAL(tensor_argmax.size(), ptrdiff_t(2 * 3 * 5 * 7 / tensor.dimension(dim)));
    for (ptrdiff_t n = 0; n < tensor_argmax.size(); ++n) {
      // Expect max to be in the first index of the reduced dimension
      VERIFY_IS_EQUAL(tensor_argmax.data()[n], 0);
    }

    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          for (int l = 0; l < 7; ++l) {
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;
            ix[3] = l;
            if (ix[dim] != tensor.dimension(dim) - 1) continue;
            // suppose dim == 1, then for all i, k, l, set tensor(i, 2, k, l) = 20.0
            tensor(ix) = 20.0;
          }
        }
      }
    }

    tensor_argmax = tensor.argmax(dim);

    VERIFY_IS_EQUAL(tensor_argmax.size(), ptrdiff_t(2 * 3 * 5 * 7 / tensor.dimension(dim)));
    for (ptrdiff_t n = 0; n < tensor_argmax.size(); ++n) {
      // Expect max to be in the last index of the reduced dimension
      VERIFY_IS_EQUAL(tensor_argmax.data()[n], tensor.dimension(dim) - 1);
    }
  }
}

template <int DataLayout>
static void test_argmin_dim() {
  Tensor<float, 4, DataLayout> tensor(2, 3, 5, 7);
  std::vector<int> dims{2, 3, 5, 7};

  for (int dim = 0; dim < 4; ++dim) {
    tensor.setRandom();
    tensor = (tensor + tensor.constant(0.5)).log();

    Tensor<DenseIndex, 3, DataLayout> tensor_argmin;
    array<DenseIndex, 4> ix;
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          for (int l = 0; l < 7; ++l) {
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;
            ix[3] = l;
            if (ix[dim] != 0) continue;
            // suppose dim == 1, then for all i, k, l, set tensor(i, 0, k, l) = -10.0
            tensor(ix) = -10.0;
          }
        }
      }
    }

    tensor_argmin = tensor.argmin(dim);

    VERIFY_IS_EQUAL(tensor_argmin.size(), ptrdiff_t(2 * 3 * 5 * 7 / tensor.dimension(dim)));
    for (ptrdiff_t n = 0; n < tensor_argmin.size(); ++n) {
      // Expect min to be in the first index of the reduced dimension
      VERIFY_IS_EQUAL(tensor_argmin.data()[n], 0);
    }

    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          for (int l = 0; l < 7; ++l) {
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;
            ix[3] = l;
            if (ix[dim] != tensor.dimension(dim) - 1) continue;
            // suppose dim == 1, then for all i, k, l, set tensor(i, 2, k, l) = -20.0
            tensor(ix) = -20.0;
          }
        }
      }
    }

    tensor_argmin = tensor.argmin(dim);

    VERIFY_IS_EQUAL(tensor_argmin.size(), ptrdiff_t(2 * 3 * 5 * 7 / tensor.dimension(dim)));
    for (ptrdiff_t n = 0; n < tensor_argmin.size(); ++n) {
      // Expect min to be in the last index of the reduced dimension
      VERIFY_IS_EQUAL(tensor_argmin.data()[n], tensor.dimension(dim) - 1);
    }
  }
}

EIGEN_DECLARE_TEST(cxx11_tensor_argmax) {
  CALL_SUBTEST(test_simple_index_pairs<RowMajor>());
  CALL_SUBTEST(test_simple_index_pairs<ColMajor>());
  CALL_SUBTEST(test_index_pairs_dim<RowMajor>());
  CALL_SUBTEST(test_index_pairs_dim<ColMajor>());
  CALL_SUBTEST(test_argmax_pair_reducer<RowMajor>());
  CALL_SUBTEST(test_argmax_pair_reducer<ColMajor>());
  CALL_SUBTEST(test_argmin_pair_reducer<RowMajor>());
  CALL_SUBTEST(test_argmin_pair_reducer<ColMajor>());
  CALL_SUBTEST(test_simple_argmax<RowMajor>());
  CALL_SUBTEST(test_simple_argmax<ColMajor>());
  CALL_SUBTEST(test_simple_argmin<RowMajor>());
  CALL_SUBTEST(test_simple_argmin<ColMajor>());
  CALL_SUBTEST(test_argmax_dim<RowMajor>());
  CALL_SUBTEST(test_argmax_dim<ColMajor>());
  CALL_SUBTEST(test_argmin_dim<RowMajor>());
  CALL_SUBTEST(test_argmin_dim<ColMajor>());
}
