// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2023
// Alejandro Acosta    Codeplay Software Ltd.
// Contact: <eigen@codeplay.com>
// Copyright (C) 2015-2016 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TEST_NO_LONGDOUBLE
#define EIGEN_DEFAULT_DENSE_INDEX_TYPE int

#define EIGEN_USE_SYCL
#include "main.h"

#include <Eigen/Dense>

template <bool verifyNan = false, bool singleTask = false, typename Operation, typename Input, typename Output>
void run_and_verify(Operation& ope, size_t num_elements, const Input& in, Output& out) {
  Output out_gpu, out_cpu;
  out_gpu = out_cpu = out;
  auto queue = sycl::queue{sycl::default_selector_v};

  auto in_size_bytes = sizeof(typename Input::Scalar) * in.size();
  auto out_size_bytes = sizeof(typename Output::Scalar) * out.size();
  auto in_d = sycl::malloc_device<typename Input::Scalar>(in.size(), queue);
  auto out_d = sycl::malloc_device<typename Output::Scalar>(out.size(), queue);

  queue.memcpy(in_d, in.data(), in_size_bytes).wait();
  queue.memcpy(out_d, out.data(), out_size_bytes).wait();

  if constexpr (singleTask) {
    queue.single_task([=]() { ope(in_d, out_d); }).wait();
  } else {
    queue
        .parallel_for(sycl::range{num_elements},
                      [=](sycl::id<1> idx) {
                        auto id = idx[0];
                        ope(id, in_d, out_d);
                      })
        .wait();
  }

  queue.memcpy(out_gpu.data(), out_d, out_size_bytes).wait();

  sycl::free(in_d, queue);
  sycl::free(out_d, queue);

  queue.throw_asynchronous();

  // Run on CPU and compare the output
  if constexpr (singleTask == 1) {
    ope(in.data(), out_cpu.data());
  } else {
    for (size_t i = 0; i < num_elements; ++i) {
      ope(i, in.data(), out_cpu.data());
    }
  }
  if constexpr (verifyNan) {
    VERIFY_IS_CWISE_APPROX(out_gpu, out_cpu);
  } else {
    VERIFY_IS_APPROX(out_gpu, out_cpu);
  }
}

template <typename DataType, typename Input, typename Output>
void test_coeff_wise(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    DataType x1(in + i);
    DataType x2(in + i + 1);
    DataType x3(in + i + 2);
    Map<DataType> res(out + i * DataType::MaxSizeAtCompileTime);

    res.array() += (in[0] * x1 + x2).array() * x3.array();
  };

  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_complex_sqrt(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    typedef typename DataType::Scalar ComplexType;
    typedef typename DataType::Scalar::value_type ValueType;
    const int num_special_inputs = 18;

    if (i == 0) {
      const ValueType nan = std::numeric_limits<ValueType>::quiet_NaN();
      typedef Eigen::Vector<ComplexType, num_special_inputs> SpecialInputs;
      SpecialInputs special_in;
      special_in.setZero();
      int idx = 0;
      special_in[idx++] = ComplexType(0, 0);
      special_in[idx++] = ComplexType(-0, 0);
      special_in[idx++] = ComplexType(0, -0);
      special_in[idx++] = ComplexType(-0, -0);
      const ValueType inf = std::numeric_limits<ValueType>::infinity();
      special_in[idx++] = ComplexType(1.0, inf);
      special_in[idx++] = ComplexType(nan, inf);
      special_in[idx++] = ComplexType(1.0, -inf);
      special_in[idx++] = ComplexType(nan, -inf);
      special_in[idx++] = ComplexType(-inf, 1.0);
      special_in[idx++] = ComplexType(inf, 1.0);
      special_in[idx++] = ComplexType(-inf, -1.0);
      special_in[idx++] = ComplexType(inf, -1.0);
      special_in[idx++] = ComplexType(-inf, nan);
      special_in[idx++] = ComplexType(inf, nan);
      special_in[idx++] = ComplexType(1.0, nan);
      special_in[idx++] = ComplexType(nan, 1.0);
      special_in[idx++] = ComplexType(nan, -1.0);
      special_in[idx++] = ComplexType(nan, nan);

      Map<SpecialInputs> special_out(out);
      special_out = special_in.cwiseSqrt();
    }

    DataType x1(in + i);
    Map<DataType> res(out + num_special_inputs + i * DataType::MaxSizeAtCompileTime);
    res = x1.cwiseSqrt();
  };
  run_and_verify<true>(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_complex_operators(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    typedef typename DataType::Scalar ComplexType;
    typedef typename DataType::Scalar::value_type ValueType;
    const int num_scalar_operators = 24;
    const int num_vector_operators = 23;  // no unary + operator.
    size_t out_idx = i * (num_scalar_operators + num_vector_operators * DataType::MaxSizeAtCompileTime);

    // Scalar operators.
    const ComplexType a = in[i];
    const ComplexType b = in[i + 1];

    out[out_idx++] = +a;
    out[out_idx++] = -a;

    out[out_idx++] = a + b;
    out[out_idx++] = a + numext::real(b);
    out[out_idx++] = numext::real(a) + b;
    out[out_idx++] = a - b;
    out[out_idx++] = a - numext::real(b);
    out[out_idx++] = numext::real(a) - b;
    out[out_idx++] = a * b;
    out[out_idx++] = a * numext::real(b);
    out[out_idx++] = numext::real(a) * b;
    out[out_idx++] = a / b;
    out[out_idx++] = a / numext::real(b);
    out[out_idx++] = numext::real(a) / b;

    out[out_idx] = a;
    out[out_idx++] += b;
    out[out_idx] = a;
    out[out_idx++] -= b;
    out[out_idx] = a;
    out[out_idx++] *= b;
    out[out_idx] = a;
    out[out_idx++] /= b;

    const ComplexType true_value = ComplexType(ValueType(1), ValueType(0));
    const ComplexType false_value = ComplexType(ValueType(0), ValueType(0));
    out[out_idx++] = (a == b ? true_value : false_value);
    out[out_idx++] = (a == numext::real(b) ? true_value : false_value);
    out[out_idx++] = (numext::real(a) == b ? true_value : false_value);
    out[out_idx++] = (a != b ? true_value : false_value);
    out[out_idx++] = (a != numext::real(b) ? true_value : false_value);
    out[out_idx++] = (numext::real(a) != b ? true_value : false_value);

    // Vector versions.
    DataType x1(in + i);
    DataType x2(in + i + 1);
    const int res_size = DataType::MaxSizeAtCompileTime * num_scalar_operators;
    const int size = DataType::MaxSizeAtCompileTime;
    int block_idx = 0;

    Map<VectorX<ComplexType>> res(out + out_idx, res_size);
    res.segment(block_idx, size) = -x1;
    block_idx += size;

    res.segment(block_idx, size) = x1 + x2;
    block_idx += size;
    res.segment(block_idx, size) = x1 + x2.real();
    block_idx += size;
    res.segment(block_idx, size) = x1.real() + x2;
    block_idx += size;
    res.segment(block_idx, size) = x1 - x2;
    block_idx += size;
    res.segment(block_idx, size) = x1 - x2.real();
    block_idx += size;
    res.segment(block_idx, size) = x1.real() - x2;
    block_idx += size;
    res.segment(block_idx, size) = x1.array() * x2.array();
    block_idx += size;
    res.segment(block_idx, size) = x1.array() * x2.real().array();
    block_idx += size;
    res.segment(block_idx, size) = x1.real().array() * x2.array();
    block_idx += size;
    res.segment(block_idx, size) = x1.array() / x2.array();
    block_idx += size;
    res.segment(block_idx, size) = x1.array() / x2.real().array();
    block_idx += size;
    res.segment(block_idx, size) = x1.real().array() / x2.array();
    block_idx += size;

    res.segment(block_idx, size) = x1;
    res.segment(block_idx, size) += x2;
    block_idx += size;
    res.segment(block_idx, size) = x1;
    res.segment(block_idx, size) -= x2;
    block_idx += size;
    res.segment(block_idx, size) = x1;
    res.segment(block_idx, size).array() *= x2.array();
    block_idx += size;
    res.segment(block_idx, size) = x1;
    res.segment(block_idx, size).array() /= x2.array();
    block_idx += size;

    const DataType true_vector = DataType::Constant(true_value);
    const DataType false_vector = DataType::Constant(false_value);
    res.segment(block_idx, size) = (x1 == x2 ? true_vector : false_vector);
    block_idx += size;
    res.segment(block_idx, size) = (x1 == x2.real() ? true_vector : false_vector);
    block_idx += size;
    //        res.segment(block_idx, size) = (x1.real() == x2) ? true_vector : false_vector;
    //        block_idx += size;
    res.segment(block_idx, size) = (x1 != x2 ? true_vector : false_vector);
    block_idx += size;
    res.segment(block_idx, size) = (x1 != x2.real() ? true_vector : false_vector);
    block_idx += size;
    //        res.segment(block_idx, size) = (x1.real() != x2 ? true_vector : false_vector);
    //        block_idx += size;
  };
  run_and_verify<true>(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_redux(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    int N = 10;
    DataType x1(in + i);
    out[i * N + 0] = x1.minCoeff();
    out[i * N + 1] = x1.maxCoeff();
    out[i * N + 2] = x1.sum();
    out[i * N + 3] = x1.prod();
    out[i * N + 4] = x1.matrix().squaredNorm();
    out[i * N + 5] = x1.matrix().norm();
    out[i * N + 6] = x1.colwise().sum().maxCoeff();
    out[i * N + 7] = x1.rowwise().maxCoeff().sum();
    out[i * N + 8] = x1.matrix().colwise().squaredNorm().sum();
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_replicate(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    DataType x1(in + i);
    int step = x1.size() * 4;
    int stride = 3 * step;

    typedef Map<Array<typename DataType::Scalar, Dynamic, Dynamic>> MapType;
    MapType(out + i * stride + 0 * step, x1.rows() * 2, x1.cols() * 2) = x1.replicate(2, 2);
    MapType(out + i * stride + 1 * step, x1.rows() * 3, x1.cols()) = in[i] * x1.colwise().replicate(3);
    MapType(out + i * stride + 2 * step, x1.rows(), x1.cols() * 3) = in[i] * x1.rowwise().replicate(3);
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType1, typename DataType2, typename Input, typename Output>
void test_product(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType1::Scalar* in, typename DataType1::Scalar* out) {
    using namespace Eigen;
    typedef Matrix<typename DataType1::Scalar, DataType1::RowsAtCompileTime, DataType2::ColsAtCompileTime> DataType3;
    DataType1 x1(in + i);
    DataType2 x2(in + i + 1);
    Map<DataType3> res(out + i * DataType3::MaxSizeAtCompileTime);
    res += in[i] * x1 * x2;
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType1, typename DataType2, typename Input, typename Output>
void test_diagonal(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType1::Scalar* in, typename DataType1::Scalar* out) {
    using namespace Eigen;
    DataType1 x1(in + i);
    Map<DataType2> res(out + i * DataType2::MaxSizeAtCompileTime);
    res += x1.diagonal();
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_eigenvalues_direct(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    typedef Matrix<typename DataType::Scalar, DataType::RowsAtCompileTime, 1> Vec;
    DataType M(in + i);
    Map<Vec> res(out + i * Vec::MaxSizeAtCompileTime);
    DataType A = M * M.adjoint();
    SelfAdjointEigenSolver<DataType> eig;
    eig.computeDirect(A);
    res = eig.eigenvalues();
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_matrix_inverse(size_t num_elements, const Input& in, Output& out) {
  auto operation = [](size_t i, const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    using namespace Eigen;
    DataType M(in + i);
    Map<DataType> res(out + i * DataType::MaxSizeAtCompileTime);
    res = M.inverse();
  };
  run_and_verify(operation, num_elements, in, out);
}

template <typename DataType, typename Input, typename Output>
void test_numeric_limits(const Input& in, Output& out) {
  auto operation = [](const typename DataType::Scalar* in, typename DataType::Scalar* out) {
    EIGEN_UNUSED_VARIABLE(in)
    out[0] = numext::numeric_limits<float>::epsilon();
    out[1] = (numext::numeric_limits<float>::max)();
    out[2] = (numext::numeric_limits<float>::min)();
    out[3] = numext::numeric_limits<float>::infinity();
    out[4] = numext::numeric_limits<float>::quiet_NaN();
  };
  run_and_verify<true, true>(operation, 1, in, out);
}

EIGEN_DECLARE_TEST(sycl_basic) {
  Eigen::VectorXf in, out;
  Eigen::VectorXcf cfin, cfout;

  constexpr size_t num_elements = 100;
  constexpr size_t data_size = num_elements * 512;
  in.setRandom(data_size);
  out.setConstant(data_size, -1);
  cfin.setRandom(data_size);
  cfout.setConstant(data_size, -1);

  CALL_SUBTEST(test_coeff_wise<Vector3f>(num_elements, in, out));
  CALL_SUBTEST(test_coeff_wise<Array44f>(num_elements, in, out));

  CALL_SUBTEST(test_complex_operators<Vector3cf>(num_elements, cfin, cfout));
  CALL_SUBTEST(test_complex_sqrt<Vector3cf>(num_elements, cfin, cfout));

  CALL_SUBTEST(test_redux<Array4f>(num_elements, in, out));
  CALL_SUBTEST(test_redux<Matrix3f>(num_elements, in, out));

  CALL_SUBTEST(test_replicate<Array4f>(num_elements, in, out));
  CALL_SUBTEST(test_replicate<Array33f>(num_elements, in, out));

  auto test_prod_mm = [&]() { test_product<Matrix3f, Matrix3f>(num_elements, in, out); };
  auto test_prod_mv = [&]() { test_product<Matrix4f, Vector4f>(num_elements, in, out); };
  CALL_SUBTEST(test_prod_mm());
  CALL_SUBTEST(test_prod_mv());

  auto test_diagonal_mv3f = [&]() { test_diagonal<Matrix3f, Vector3f>(num_elements, in, out); };
  auto test_diagonal_mv4f = [&]() { test_diagonal<Matrix4f, Vector4f>(num_elements, in, out); };
  CALL_SUBTEST(test_diagonal_mv3f());
  CALL_SUBTEST(test_diagonal_mv4f());

  CALL_SUBTEST(test_eigenvalues_direct<Matrix3f>(num_elements, in, out));
  CALL_SUBTEST(test_eigenvalues_direct<Matrix2f>(num_elements, in, out));

  CALL_SUBTEST(test_matrix_inverse<Matrix2f>(num_elements, in, out));
  CALL_SUBTEST(test_matrix_inverse<Matrix3f>(num_elements, in, out));
  CALL_SUBTEST(test_matrix_inverse<Matrix4f>(num_elements, in, out));

  CALL_SUBTEST(test_numeric_limits<Vector3f>(in, out));
}
