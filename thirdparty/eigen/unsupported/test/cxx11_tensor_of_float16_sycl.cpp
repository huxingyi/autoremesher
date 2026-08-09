// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2023
// Alejandro Acosta  Codeplay Software Ltd.
// Contact: <eigen@codeplay.com>
// Benoit Steiner <benoit.steiner.goog@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TEST_NO_LONGDOUBLE
#define EIGEN_TEST_NO_COMPLEX

#define EIGEN_DEFAULT_DENSE_INDEX_TYPE int
#define EIGEN_USE_SYCL
#define EIGEN_SYCL_HALF_SUPPORT

#include "main.h"
#include <unsupported/Eigen/CXX11/Tensor>

using Eigen::SyclDevice;
using Eigen::Tensor;

void test_gpu_numext(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));
  bool* d_res_half = static_cast<bool*>(sycl_device.allocate(num_elem * sizeof(bool)));
  bool* d_res_float = static_cast<bool*>(sycl_device.allocate(num_elem * sizeof(bool)));

  Eigen::TensorMap<Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Tensor<bool, 1>, Eigen::Aligned> gpu_res_half(d_res_half, num_elem);
  Eigen::TensorMap<Tensor<bool, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float.device(sycl_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_res_float.device(sycl_device) = gpu_float.unaryExpr(Eigen::internal::scalar_isnan_op<float>());
  gpu_res_half.device(sycl_device) =
      gpu_float.cast<Eigen::half>().unaryExpr(Eigen::internal::scalar_isnan_op<Eigen::half>());

  Tensor<bool, 1> half_prec(num_elem);
  Tensor<bool, 1> full_prec(num_elem);

  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, num_elem * sizeof(bool));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(bool));

  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking numext " << i << std::endl;
    VERIFY_IS_EQUAL(full_prec(i), half_prec(i));
  }
}

void test_gpu_conversion(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));
  Eigen::half* d_half = static_cast<Eigen::half*>(sycl_device.allocate(num_elem * sizeof(Eigen::half)));
  float* d_conv = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_half(d_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_conv(d_conv, num_elem);

  gpu_float.device(sycl_device) = gpu_float.random();
  gpu_half.device(sycl_device) = gpu_float.cast<Eigen::half>();
  gpu_conv.device(sycl_device) = gpu_half.cast<float>();

  Tensor<float, 1> initial(num_elem);
  Tensor<float, 1> final(num_elem);
  sycl_device.memcpyDeviceToHost(initial.data(), d_float, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(final.data(), d_conv, num_elem * sizeof(float));

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(initial(i), final(i));
  }
}

void test_gpu_unary(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_res_half = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_res_float = (float*)sycl_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_half(d_res_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float.device(sycl_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_res_float.device(sycl_device) = gpu_float.abs();
  gpu_res_half.device(sycl_device) = gpu_float.cast<Eigen::half>().abs().cast<float>();

  Tensor<float, 1> half_prec(num_elem);
  Tensor<float, 1> full_prec(num_elem);
  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));
  sycl_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking unary " << i << std::endl;
    VERIFY_IS_APPROX(full_prec(i), half_prec(i));
  }
}

void test_gpu_elementwise(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float1 = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));
  float* d_float2 = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));
  float* d_res_half = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));
  float* d_res_float = static_cast<float*>(sycl_device.allocate(num_elem * sizeof(float)));

  Eigen::TensorMap<Tensor<float, 1>, Eigen::Aligned> gpu_float1(d_float1, num_elem);
  Eigen::TensorMap<Tensor<float, 1>, Eigen::Aligned> gpu_float2(d_float2, num_elem);
  Eigen::TensorMap<Tensor<float, 1>, Eigen::Aligned> gpu_res_half(d_res_half, num_elem);
  Eigen::TensorMap<Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float1.device(sycl_device) = gpu_float1.random();
  gpu_float2.device(sycl_device) = gpu_float2.random();
  gpu_res_float.device(sycl_device) = (gpu_float1 + gpu_float2) * gpu_float1;
  gpu_res_half.device(sycl_device) =
      ((gpu_float1.cast<Eigen::half>() + gpu_float2.cast<Eigen::half>()) * gpu_float1.cast<Eigen::half>())
          .cast<float>();

  Tensor<float, 1> half_prec(num_elem);
  Tensor<float, 1> full_prec(num_elem);

  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));

  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking elemwise " << i << ": full prec = " << full_prec(i) << " vs half prec = " << half_prec(i)
              << std::endl;
    VERIFY_IS_APPROX(static_cast<Eigen::half>(full_prec(i)), static_cast<Eigen::half>(half_prec(i)));
  }
}

void test_gpu_trancendental(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float1 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_float2 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_float3 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  Eigen::half* d_res1_half = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res1_float = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res2_half = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res2_float = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res3_half = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res3_float = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float1(d_float1, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float2(d_float2, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float3(d_float3, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res1_half(d_res1_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res1_float(d_res1_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res2_half(d_res2_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res2_float(d_res2_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res3_half(d_res3_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res3_float(d_res3_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res4_half(d_res3_half, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res4_float(d_res3_float, num_elem);

  gpu_float1.device(sycl_device) = gpu_float1.random() - gpu_float1.constant(0.5f);
  gpu_float2.device(sycl_device) = gpu_float2.random() + gpu_float1.constant(0.5f);
  gpu_float3.device(sycl_device) = gpu_float3.random();
  gpu_res1_float.device(sycl_device) = gpu_float1.exp().cast<Eigen::half>();
  gpu_res2_float.device(sycl_device) = gpu_float2.log().cast<Eigen::half>();
  gpu_res3_float.device(sycl_device) = gpu_float3.log1p().cast<Eigen::half>();
  gpu_res4_float.device(sycl_device) = gpu_float3.expm1().cast<Eigen::half>();

  gpu_res1_half.device(sycl_device) = gpu_float1.cast<Eigen::half>();
  gpu_res1_half.device(sycl_device) = gpu_res1_half.exp();

  gpu_res2_half.device(sycl_device) = gpu_float2.cast<Eigen::half>();
  gpu_res2_half.device(sycl_device) = gpu_res2_half.log();

  gpu_res3_half.device(sycl_device) = gpu_float3.cast<Eigen::half>();
  gpu_res3_half.device(sycl_device) = gpu_res3_half.log1p();

  gpu_res3_half.device(sycl_device) = gpu_float3.cast<Eigen::half>();
  gpu_res3_half.device(sycl_device) = gpu_res3_half.expm1();

  Tensor<float, 1> input1(num_elem);
  Tensor<Eigen::half, 1> half_prec1(num_elem);
  Tensor<Eigen::half, 1> full_prec1(num_elem);
  Tensor<float, 1> input2(num_elem);
  Tensor<Eigen::half, 1> half_prec2(num_elem);
  Tensor<Eigen::half, 1> full_prec2(num_elem);
  Tensor<float, 1> input3(num_elem);
  Tensor<Eigen::half, 1> half_prec3(num_elem);
  Tensor<Eigen::half, 1> full_prec3(num_elem);
  sycl_device.memcpyDeviceToHost(input1.data(), d_float1, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(input2.data(), d_float2, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(input3.data(), d_float3, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(half_prec1.data(), d_res1_half, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec1.data(), d_res1_float, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(half_prec2.data(), d_res2_half, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec2.data(), d_res2_float, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(half_prec3.data(), d_res3_half, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec3.data(), d_res3_float, num_elem * sizeof(Eigen::half));
  sycl_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking elemwise exp " << i << " input = " << input1(i) << " full = " << full_prec1(i)
              << " half = " << half_prec1(i) << std::endl;
    VERIFY_IS_APPROX(full_prec1(i), half_prec1(i));
  }
  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking elemwise log " << i << " input = " << input2(i) << " full = " << full_prec2(i)
              << " half = " << half_prec2(i) << std::endl;
    if (std::abs(input2(i) - 1.f) < 0.05f)  // log lacks accuracy nearby 1
      VERIFY_IS_APPROX(full_prec2(i) + Eigen::half(0.1f), half_prec2(i) + Eigen::half(0.1f));
    else
      VERIFY_IS_APPROX(full_prec2(i), half_prec2(i));
  }
  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking elemwise plog1 " << i << " input = " << input3(i) << " full = " << full_prec3(i)
              << " half = " << half_prec3(i) << std::endl;
    VERIFY_IS_APPROX(full_prec3(i), half_prec3(i));
  }
}

void test_gpu_contractions(const Eigen::SyclDevice& sycl_device) {
  int rows = 23;
  int cols = 23;
  int num_elem = rows * cols;

  float* d_float1 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_float2 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  Eigen::half* d_res_half = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));
  Eigen::half* d_res_float = (Eigen::half*)sycl_device.allocate(num_elem * sizeof(Eigen::half));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float1(d_float1, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float2(d_float2, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 2>, Eigen::Aligned> gpu_res_half(d_res_half, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 2>, Eigen::Aligned> gpu_res_float(d_res_float, rows, cols);

  gpu_float1.device(sycl_device) = gpu_float1.random() - gpu_float1.constant(0.5f);
  gpu_float2.device(sycl_device) = gpu_float2.random() - gpu_float2.constant(0.5f);

  typedef typename Tensor<float, 2>::DimensionPair DimPair;
  Eigen::array<DimPair, 1> dims;
  gpu_res_float.device(sycl_device) = gpu_float1.contract(gpu_float2, dims).cast<Eigen::half>();
  gpu_res_half.device(sycl_device) = gpu_float1.cast<Eigen::half>().contract(gpu_float2.cast<Eigen::half>(), dims);

  Tensor<Eigen::half, 2> half_prec(rows, cols);
  Tensor<Eigen::half, 2> full_prec(rows, cols);
  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, num_elem * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(Eigen::half));
  sycl_device.synchronize();

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      std::cout << "Checking contract " << i << " " << j << full_prec(i, j) << " " << half_prec(i, j) << std::endl;
      if (numext::abs(full_prec(i, j) - half_prec(i, j)) > Eigen::half(1e-2f)) {
        VERIFY_IS_APPROX(full_prec(i, j), half_prec(i, j));
      }
    }
  }
}

void test_gpu_reductions(const Eigen::SyclDevice& sycl_device, int size1, int size2, int redux) {
  std::cout << "Reducing " << size1 << " by " << size2 << " tensor along dim " << redux << std::endl;

  int num_elem = size1 * size2;
  int result_size = (redux == 1 ? size1 : size2);

  float* d_float = (float*)sycl_device.allocate(num_elem * sizeof(float));
  Eigen::half* d_res_half = (Eigen::half*)sycl_device.allocate(result_size * sizeof(Eigen::half));
  Eigen::half* d_res_float = (Eigen::half*)sycl_device.allocate(result_size * sizeof(Eigen::half));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float(d_float, size1, size2);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res_half(d_res_half, result_size);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 1>, Eigen::Aligned> gpu_res_float(d_res_float, result_size);

  gpu_float.device(sycl_device) = gpu_float.random() * 2.0f;

  Eigen::array<int, 1> redux_dim = {redux};
  gpu_res_float.device(sycl_device) = gpu_float.sum(redux_dim).cast<Eigen::half>();
  gpu_res_half.device(sycl_device) = gpu_float.cast<Eigen::half>().sum(redux_dim);

  Tensor<Eigen::half, 1> half_prec(result_size);
  Tensor<Eigen::half, 1> full_prec(result_size);
  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, result_size * sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, result_size * sizeof(Eigen::half));
  sycl_device.synchronize();

  for (int i = 0; i < result_size; ++i) {
    std::cout << "EXPECTED " << full_prec(i) << " GOT " << half_prec(i) << std::endl;
    VERIFY_IS_APPROX(full_prec(i), half_prec(i));
  }
}

void test_gpu_reductions(const Eigen::SyclDevice& sycl_device) {
  test_gpu_reductions(sycl_device, 13, 13, 0);
  test_gpu_reductions(sycl_device, 13, 13, 1);

  test_gpu_reductions(sycl_device, 35, 36, 0);
  test_gpu_reductions(sycl_device, 35, 36, 1);

  test_gpu_reductions(sycl_device, 36, 35, 0);
  test_gpu_reductions(sycl_device, 36, 35, 1);
}

void test_gpu_full_reductions(const Eigen::SyclDevice& sycl_device) {
  int size = 13;
  int num_elem = size * size;

  float* d_float = (float*)sycl_device.allocate(num_elem * sizeof(float));
  Eigen::half* d_res_half = (Eigen::half*)sycl_device.allocate(1 * sizeof(Eigen::half));
  Eigen::half* d_res_float = (Eigen::half*)sycl_device.allocate(1 * sizeof(Eigen::half));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float(d_float, size, size);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 0>, Eigen::Aligned> gpu_res_half(d_res_half);
  Eigen::TensorMap<Eigen::Tensor<Eigen::half, 0>, Eigen::Aligned> gpu_res_float(d_res_float);

  gpu_float.device(sycl_device) = gpu_float.random();

  gpu_res_float.device(sycl_device) = gpu_float.sum().cast<Eigen::half>();
  gpu_res_half.device(sycl_device) = gpu_float.cast<Eigen::half>().sum();

  Tensor<Eigen::half, 0> half_prec;
  Tensor<Eigen::half, 0> full_prec;
  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, sizeof(Eigen::half));
  sycl_device.synchronize();

  VERIFY_IS_APPROX(full_prec(), half_prec());

  gpu_res_float.device(sycl_device) = gpu_float.maximum().cast<Eigen::half>();
  gpu_res_half.device(sycl_device) = gpu_float.cast<Eigen::half>().maximum();
  sycl_device.memcpyDeviceToHost(half_prec.data(), d_res_half, sizeof(Eigen::half));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, sizeof(Eigen::half));
  sycl_device.synchronize();

  VERIFY_IS_APPROX(full_prec(), half_prec());
}

void test_gpu_forced_evals(const Eigen::SyclDevice& sycl_device) {
  int num_elem = 101;

  float* d_float = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_res_half1 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_res_half2 = (float*)sycl_device.allocate(num_elem * sizeof(float));
  float* d_res_float = (float*)sycl_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_half1(d_res_half1, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Unaligned> gpu_res_half2(d_res_half2, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  Eigen::array<int, 1> no_bcast;
  no_bcast[0] = 1;

  gpu_float.device(sycl_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_res_float.device(sycl_device) = gpu_float.abs();
  gpu_res_half1.device(sycl_device) = gpu_float.cast<Eigen::half>().abs().eval().cast<float>();
  gpu_res_half2.device(sycl_device) = gpu_float.cast<Eigen::half>().abs().broadcast(no_bcast).eval().cast<float>();

  Tensor<float, 1> half_prec1(num_elem);
  Tensor<float, 1> half_prec2(num_elem);
  Tensor<float, 1> full_prec(num_elem);
  sycl_device.memcpyDeviceToHost(half_prec1.data(), d_res_half1, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(half_prec2.data(), d_res_half2, num_elem * sizeof(float));
  sycl_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));
  sycl_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    std::cout << "Checking forced eval " << i << full_prec(i) << " vs " << half_prec1(i) << " vs " << half_prec2(i)
              << std::endl;
    VERIFY_IS_APPROX(full_prec(i), half_prec1(i));
    VERIFY_IS_APPROX(full_prec(i), half_prec2(i));
  }
}

EIGEN_DECLARE_TEST(cxx11_tensor_of_float16_sycl) {
  for (const auto& s : Eigen::get_sycl_supported_devices()) {
    QueueInterface queueInterface(s);
    auto sycl_device = Eigen::SyclDevice(&queueInterface);

    CALL_SUBTEST_1(test_gpu_numext(sycl_device));
    CALL_SUBTEST_1(test_gpu_conversion(sycl_device));
    CALL_SUBTEST_1(test_gpu_unary(sycl_device));
    CALL_SUBTEST_1(test_gpu_elementwise(sycl_device));
    CALL_SUBTEST_1(test_gpu_trancendental(sycl_device));
    CALL_SUBTEST_2(test_gpu_contractions(sycl_device));
    CALL_SUBTEST_3(test_gpu_reductions(sycl_device));
    CALL_SUBTEST_4(test_gpu_full_reductions(sycl_device));
    CALL_SUBTEST_5(test_gpu_forced_evals(sycl_device));
  }
}
