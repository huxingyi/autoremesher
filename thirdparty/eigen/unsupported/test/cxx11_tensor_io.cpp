// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2014 Benoit Steiner <benoit.steiner.goog@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "main.h"

#include <sstream>
#include <Eigen/CXX11/Tensor>

template <typename Scalar, int rank, int Layout>
struct test_tensor_ostream_impl {};

template <typename Scalar, int Layout>
struct test_tensor_ostream_impl<Scalar, 0, Layout> {
  static void run() {
    Eigen::Tensor<Scalar, 0> t;
    t.setValues(1);
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == "1");
  }
};

template <typename Scalar, int Layout>
struct test_tensor_ostream_impl<Scalar, 1, Layout> {
  static void run() {
    Eigen::Tensor<Scalar, 1> t = {3};
    t.setValues({1, 2, 3});
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == "1 2 3");
  }
};

template <typename Scalar, int Layout>
struct test_tensor_ostream_impl<Scalar, 2, Layout> {
  static void run() {
    Eigen::Tensor<Scalar, 2> t = {3, 2};
    t.setValues({{1, 2}, {3, 4}, {5, 6}});
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == "1 2\n3 4\n5 6");
  }
};

template <typename Scalar, int Layout>
struct test_tensor_ostream_impl<Scalar, 3, Layout> {
  static void run() {
    Eigen::Tensor<Scalar, 3> t = {4, 3, 2};
    t.setValues({{{1, 2}, {3, 4}, {5, 6}},
                 {{7, 8}, {9, 10}, {11, 12}},
                 {{13, 14}, {15, 16}, {17, 18}},
                 {{19, 20}, {21, 22}, {23, 24}}});
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == " 1  2\n 3  4\n 5  6\n\n 7  8\n 9 10\n11 12\n\n13 14\n15 16\n17 18\n\n19 20\n21 22\n23 24");
  }
};

template <int Layout>
struct test_tensor_ostream_impl<bool, 2, Layout> {
  static void run() {
    Eigen::Tensor<bool, 2> t = {3, 2};
    t.setValues({{false, true}, {true, false}, {false, false}});
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == "0 1\n1 0\n0 0");
  }
};

template <typename Scalar, int Layout>
struct test_tensor_ostream_impl<std::complex<Scalar>, 2, Layout> {
  static void run() {
    Eigen::Tensor<std::complex<Scalar>, 2> t = {3, 2};
    t.setValues({{std::complex<Scalar>(1, 2), std::complex<Scalar>(12, 3)},
                 {std::complex<Scalar>(-4, 2), std::complex<Scalar>(0, 5)},
                 {std::complex<Scalar>(-1, 4), std::complex<Scalar>(5, 27)}});
    std::ostringstream os;
    os << t.format(Eigen::TensorIOFormat::Plain());
    VERIFY(os.str() == " (1,2) (12,3)\n(-4,2)  (0,5)\n(-1,4) (5,27)");

    os.str("");
    os.clear();
    os << t.format(Eigen::TensorIOFormat::Numpy());
    VERIFY(os.str() == "[[ 1+2j 12+3j]\n [-4+2j  0+5j]\n [-1+4j 5+27j]]");

    os.str("");
    os.clear();
    os << t.format(Eigen::TensorIOFormat::Native());
    VERIFY(os.str() == "{{ {1, 2}, {12, 3}},\n {{-4, 2},  {0, 5}},\n {{-1, 4}, {5, 27}}}");
  }
};

template <typename Scalar, int rank, int Layout>
void test_tensor_ostream() {
  test_tensor_ostream_impl<Scalar, rank, Layout>::run();
}

void test_const_tensor_ostream() {
  Eigen::Tensor<float, 0> t;
  t.setValues(1);
  const Eigen::TensorMap<Eigen::Tensor<const float, 0, Eigen::RowMajor>, Eigen::Unaligned> t_const(
      t.data(), Eigen::DSizes<Eigen::DenseIndex, 0>{});
  std::ostringstream os;
  os << t_const.format(Eigen::TensorIOFormat::Plain());
  VERIFY(os.str() == "1");
}

EIGEN_DECLARE_TEST(cxx11_tensor_io) {
  CALL_SUBTEST((test_tensor_ostream<float, 0, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 1, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 2, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 3, Eigen::ColMajor>()));

  CALL_SUBTEST((test_tensor_ostream<double, 0, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 1, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 2, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 3, Eigen::ColMajor>()));

  CALL_SUBTEST((test_tensor_ostream<int, 0, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 1, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 2, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 3, Eigen::ColMajor>()));

  CALL_SUBTEST((test_tensor_ostream<float, 0, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 1, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 2, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<float, 3, Eigen::RowMajor>()));

  CALL_SUBTEST((test_tensor_ostream<double, 0, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 1, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 2, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<double, 3, Eigen::RowMajor>()));

  CALL_SUBTEST((test_tensor_ostream<int, 0, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 1, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 2, Eigen::RowMajor>()));
  CALL_SUBTEST((test_tensor_ostream<int, 3, Eigen::RowMajor>()));

  CALL_SUBTEST((test_tensor_ostream<bool, 2, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<bool, 2, Eigen::RowMajor>()));

  CALL_SUBTEST((test_tensor_ostream<std::complex<double>, 2, Eigen::ColMajor>()));
  CALL_SUBTEST((test_tensor_ostream<std::complex<float>, 2, Eigen::ColMajor>()));

  // Test printing TensorMap with const elements.
  CALL_SUBTEST((test_const_tensor_ostream()));
}
