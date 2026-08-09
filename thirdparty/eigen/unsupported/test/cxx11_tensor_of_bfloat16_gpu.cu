// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2021 Rohit Santhanam <rohit.santhanam@amd.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TEST_NO_LONGDOUBLE
#define EIGEN_TEST_NO_COMPLEX

#define EIGEN_DEFAULT_DENSE_INDEX_TYPE int
#define EIGEN_USE_GPU

#include "main.h"
#include <unsupported/Eigen/CXX11/Tensor>

using Eigen::Tensor;

template <typename>
void test_gpu_numext() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  bool* d_res_bfloat16 = (bool*)gpu_device.allocate(num_elem * sizeof(bool));
  bool* d_res_float = (bool*)gpu_device.allocate(num_elem * sizeof(bool));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<bool, 1>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<bool, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float.device(gpu_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_res_float.device(gpu_device) = gpu_float.unaryExpr(Eigen::internal::scalar_isnan_op<float>());
  // Test bfloat16 specific isnan op.
  gpu_res_bfloat16.device(gpu_device) =
      gpu_float.cast<Eigen::bfloat16>().unaryExpr(Eigen::internal::scalar_isnan_op<Eigen::bfloat16>());

  Tensor<bool, 1> bfloat16_prec(num_elem);
  Tensor<bool, 1> full_prec(num_elem);
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, num_elem * sizeof(bool));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(bool));
  gpu_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_EQUAL(full_prec(i), bfloat16_prec(i));
  }

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

#ifdef EIGEN_HAS_GPU_BF16

template <typename>
void test_gpu_conversion() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  Eigen::bfloat16* d_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  float* d_conv = (float*)gpu_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_bfloat16(d_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_conv(d_conv, num_elem);

  gpu_float.device(gpu_device) = gpu_float.random();
  gpu_bfloat16.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>();
  gpu_conv.device(gpu_device) = gpu_bfloat16.cast<float>();

  Tensor<float, 1> initial(num_elem);
  Tensor<float, 1> final(num_elem);
  gpu_device.memcpyDeviceToHost(initial.data(), d_float, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(final.data(), d_conv, num_elem * sizeof(float));

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(static_cast<Eigen::bfloat16>(initial(i)), static_cast<Eigen::bfloat16>(final(i)));
  }

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_bfloat16);
  gpu_device.deallocate(d_conv);
}

template <typename>
void test_gpu_unary() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_bfloat16 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_float = (float*)gpu_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float.device(gpu_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_float.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().cast<float>();
  gpu_res_float.device(gpu_device) = gpu_float.abs();
  gpu_res_bfloat16.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().abs().cast<float>();

  Tensor<float, 1> bfloat16_prec(num_elem);
  Tensor<float, 1> full_prec(num_elem);
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));
  gpu_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(full_prec(i), bfloat16_prec(i));
  }

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

template <typename>
void test_gpu_elementwise() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float1 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_float2 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_bfloat16 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_float = (float*)gpu_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float1(d_float1, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float2(d_float2, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  gpu_float1.device(gpu_device) = gpu_float1.random();
  gpu_float1.device(gpu_device) = gpu_float1.cast<Eigen::bfloat16>().cast<float>();
  gpu_float2.device(gpu_device) = gpu_float2.random();
  gpu_float2.device(gpu_device) = gpu_float2.cast<Eigen::bfloat16>().cast<float>();
  gpu_res_float.device(gpu_device) = (gpu_float1 + gpu_float2) * gpu_float1;
  gpu_res_bfloat16.device(gpu_device) =
      ((gpu_float1.cast<Eigen::bfloat16>() + gpu_float2.cast<Eigen::bfloat16>()) * gpu_float1.cast<Eigen::bfloat16>())
          .cast<float>();

  Tensor<float, 1> bfloat16_prec(num_elem);
  Tensor<float, 1> full_prec(num_elem);
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));
  gpu_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(static_cast<Eigen::bfloat16>(full_prec(i)), static_cast<Eigen::bfloat16>(bfloat16_prec(i)));
  }

  gpu_device.deallocate(d_float1);
  gpu_device.deallocate(d_float2);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

template <typename>
void test_gpu_trancendental() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float1 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_float2 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_float3 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  Eigen::bfloat16* d_res1_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res1_float = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res2_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res2_float = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res3_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res3_float = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float1(d_float1, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float2(d_float2, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float3(d_float3, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res1_bfloat16(d_res1_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res1_float(d_res1_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res2_bfloat16(d_res2_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res2_float(d_res2_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res3_bfloat16(d_res3_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res3_float(d_res3_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res4_bfloat16(d_res3_bfloat16, num_elem);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res4_float(d_res3_float, num_elem);

  gpu_float1.device(gpu_device) = gpu_float1.random() - gpu_float1.constant(0.5f);
  gpu_float1.device(gpu_device) = gpu_float1.cast<Eigen::bfloat16>().cast<float>();
  gpu_float2.device(gpu_device) = gpu_float2.random() + gpu_float1.constant(0.5f);
  gpu_float2.device(gpu_device) = gpu_float2.cast<Eigen::bfloat16>().cast<float>();
  gpu_float3.device(gpu_device) = gpu_float3.random();
  gpu_float3.device(gpu_device) = gpu_float3.cast<Eigen::bfloat16>().cast<float>();
  gpu_res1_float.device(gpu_device) = gpu_float1.exp().cast<Eigen::bfloat16>();
  gpu_res2_float.device(gpu_device) = gpu_float2.log().cast<Eigen::bfloat16>();
  gpu_res3_float.device(gpu_device) = gpu_float3.log1p().cast<Eigen::bfloat16>();
  gpu_res4_float.device(gpu_device) = gpu_float3.expm1().cast<Eigen::bfloat16>();

  gpu_res1_bfloat16.device(gpu_device) = gpu_float1.cast<Eigen::bfloat16>();
  gpu_res1_bfloat16.device(gpu_device) = gpu_res1_bfloat16.exp();

  gpu_res2_bfloat16.device(gpu_device) = gpu_float2.cast<Eigen::bfloat16>();
  gpu_res2_bfloat16.device(gpu_device) = gpu_res2_bfloat16.log();

  gpu_res3_bfloat16.device(gpu_device) = gpu_float3.cast<Eigen::bfloat16>();
  gpu_res3_bfloat16.device(gpu_device) = gpu_res3_bfloat16.log1p();

  gpu_res3_bfloat16.device(gpu_device) = gpu_float3.cast<Eigen::bfloat16>();
  gpu_res3_bfloat16.device(gpu_device) = gpu_res3_bfloat16.expm1();

  Tensor<float, 1> input1(num_elem);
  Tensor<Eigen::bfloat16, 1> bfloat16_prec1(num_elem);
  Tensor<Eigen::bfloat16, 1> full_prec1(num_elem);
  Tensor<float, 1> input2(num_elem);
  Tensor<Eigen::bfloat16, 1> bfloat16_prec2(num_elem);
  Tensor<Eigen::bfloat16, 1> full_prec2(num_elem);
  Tensor<float, 1> input3(num_elem);
  Tensor<Eigen::bfloat16, 1> bfloat16_prec3(num_elem);
  Tensor<Eigen::bfloat16, 1> full_prec3(num_elem);
  gpu_device.memcpyDeviceToHost(input1.data(), d_float1, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(input2.data(), d_float2, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(input3.data(), d_float3, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(bfloat16_prec1.data(), d_res1_bfloat16, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec1.data(), d_res1_float, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(bfloat16_prec2.data(), d_res2_bfloat16, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec2.data(), d_res2_float, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(bfloat16_prec3.data(), d_res3_bfloat16, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec3.data(), d_res3_float, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(full_prec1(i), bfloat16_prec1(i));
  }
  for (int i = 0; i < num_elem; ++i) {
    if (std::abs(input2(i) - 1.f) < 0.05f)  // log lacks accuracy nearby 1
      VERIFY_IS_APPROX(full_prec2(i) + Eigen::bfloat16(0.1f), bfloat16_prec2(i) + Eigen::bfloat16(0.1f));
    else
      VERIFY_IS_APPROX(full_prec2(i), bfloat16_prec2(i));
  }
  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(full_prec3(i), bfloat16_prec3(i));
  }
  gpu_device.deallocate(d_float1);
  gpu_device.deallocate(d_float2);
  gpu_device.deallocate(d_float3);
  gpu_device.deallocate(d_res1_bfloat16);
  gpu_device.deallocate(d_res1_float);
  gpu_device.deallocate(d_res2_bfloat16);
  gpu_device.deallocate(d_res2_float);
  gpu_device.deallocate(d_res3_float);
  gpu_device.deallocate(d_res3_bfloat16);
}

template <typename>
void test_gpu_contractions() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int rows = 23;
  int cols = 23;
  int num_elem = rows * cols;

  float* d_float1 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_float2 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  Eigen::bfloat16* d_res_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res_float = (Eigen::bfloat16*)gpu_device.allocate(num_elem * sizeof(Eigen::bfloat16));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float1(d_float1, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float2(d_float2, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 2>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16, rows, cols);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 2>, Eigen::Aligned> gpu_res_float(d_res_float, rows, cols);

  gpu_float1.device(gpu_device) = gpu_float1.random() - gpu_float1.constant(0.5f);
  gpu_float2.device(gpu_device) = gpu_float2.random() - gpu_float2.constant(0.5f);

  typedef Tensor<float, 2>::DimensionPair DimPair;
  Eigen::array<DimPair, 1> dims(DimPair(1, 0));
  gpu_res_float.device(gpu_device) = gpu_float1.contract(gpu_float2, dims).cast<Eigen::bfloat16>();
  gpu_res_bfloat16.device(gpu_device) =
      gpu_float1.cast<Eigen::bfloat16>().contract(gpu_float2.cast<Eigen::bfloat16>(), dims);

  Tensor<Eigen::bfloat16, 2> bfloat16_prec(rows, cols);
  Tensor<Eigen::bfloat16, 2> full_prec(rows, cols);
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(Eigen::bfloat16));
  gpu_device.synchronize();

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      if (numext::abs(full_prec(i, j) - bfloat16_prec(i, j)) > Eigen::bfloat16(1e-2f)) {
        VERIFY_IS_APPROX(full_prec(i, j), bfloat16_prec(i, j));
      }
    }
  }

  gpu_device.deallocate(d_float1);
  gpu_device.deallocate(d_float2);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

template <typename>
void test_gpu_reductions(int size1, int size2, int redux) {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = size1 * size2;
  int result_size = (redux == 1 ? size1 : size2);

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  Eigen::bfloat16* d_res_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(result_size * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res_float = (Eigen::bfloat16*)gpu_device.allocate(result_size * sizeof(Eigen::bfloat16));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float(d_float, size1, size2);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16, result_size);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 1>, Eigen::Aligned> gpu_res_float(d_res_float, result_size);

  gpu_float.device(gpu_device) = gpu_float.random() * 2.0f;

  Eigen::array<int, 1> redux_dim = {redux};
  gpu_res_float.device(gpu_device) = gpu_float.sum(redux_dim).cast<Eigen::bfloat16>();
  gpu_res_bfloat16.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().sum(redux_dim);

  Tensor<Eigen::bfloat16, 1> bfloat16_prec(result_size);
  Tensor<Eigen::bfloat16, 1> full_prec(result_size);
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, result_size * sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, result_size * sizeof(Eigen::bfloat16));
  gpu_device.synchronize();

  for (int i = 0; i < result_size; ++i) {
    VERIFY_IS_APPROX(full_prec(i), bfloat16_prec(i));
  }

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

template <typename>
void test_gpu_reductions() {
  test_gpu_reductions<void>(13, 13, 0);
  test_gpu_reductions<void>(13, 13, 1);

  test_gpu_reductions<void>(35, 36, 0);
  test_gpu_reductions<void>(35, 36, 1);

  test_gpu_reductions<void>(36, 35, 0);
  test_gpu_reductions<void>(36, 35, 1);
}

template <typename>
void test_gpu_full_reductions() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int size = 13;
  int num_elem = size * size;

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  Eigen::bfloat16* d_res_bfloat16 = (Eigen::bfloat16*)gpu_device.allocate(1 * sizeof(Eigen::bfloat16));
  Eigen::bfloat16* d_res_float = (Eigen::bfloat16*)gpu_device.allocate(1 * sizeof(Eigen::bfloat16));

  Eigen::TensorMap<Eigen::Tensor<float, 2>, Eigen::Aligned> gpu_float(d_float, size, size);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 0>, Eigen::Aligned> gpu_res_bfloat16(d_res_bfloat16);
  Eigen::TensorMap<Eigen::Tensor<Eigen::bfloat16, 0>, Eigen::Aligned> gpu_res_float(d_res_float);

  gpu_float.device(gpu_device) = gpu_float.random();

  gpu_res_float.device(gpu_device) = gpu_float.sum().cast<Eigen::bfloat16>();
  gpu_res_bfloat16.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().sum();

  Tensor<Eigen::bfloat16, 0> bfloat16_prec;
  Tensor<Eigen::bfloat16, 0> full_prec;
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, sizeof(Eigen::bfloat16));
  gpu_device.synchronize();

  VERIFY_IS_APPROX(full_prec(), bfloat16_prec());

  gpu_res_float.device(gpu_device) = gpu_float.maximum().cast<Eigen::bfloat16>();
  gpu_res_bfloat16.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().maximum();
  gpu_device.memcpyDeviceToHost(bfloat16_prec.data(), d_res_bfloat16, sizeof(Eigen::bfloat16));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, sizeof(Eigen::bfloat16));
  gpu_device.synchronize();

  VERIFY_IS_APPROX(full_prec(), bfloat16_prec());

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_res_bfloat16);
  gpu_device.deallocate(d_res_float);
}

template <typename>
void test_gpu_forced_evals() {
  Eigen::GpuStreamDevice stream;
  Eigen::GpuDevice gpu_device(&stream);
  int num_elem = 101;

  float* d_float = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_bfloat16_1 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_bfloat16_2 = (float*)gpu_device.allocate(num_elem * sizeof(float));
  float* d_res_float = (float*)gpu_device.allocate(num_elem * sizeof(float));

  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_float(d_float, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_bfloat16_1(d_res_bfloat16_1, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Unaligned> gpu_res_bfloat16_2(d_res_bfloat16_2, num_elem);
  Eigen::TensorMap<Eigen::Tensor<float, 1>, Eigen::Aligned> gpu_res_float(d_res_float, num_elem);

  Eigen::array<int, 1> no_bcast;
  no_bcast[0] = 1;

  gpu_float.device(gpu_device) = gpu_float.random() - gpu_float.constant(0.5f);
  gpu_float.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().cast<float>();
  gpu_res_float.device(gpu_device) = gpu_float.abs();
  gpu_res_bfloat16_1.device(gpu_device) = gpu_float.cast<Eigen::bfloat16>().abs().eval().cast<float>();
  gpu_res_bfloat16_2.device(gpu_device) =
      gpu_float.cast<Eigen::bfloat16>().abs().broadcast(no_bcast).eval().cast<float>();

  Tensor<float, 1> bfloat16_prec1(num_elem);
  Tensor<float, 1> bfloat16_prec2(num_elem);
  Tensor<float, 1> full_prec(num_elem);
  gpu_device.memcpyDeviceToHost(bfloat16_prec1.data(), d_res_bfloat16_1, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(bfloat16_prec2.data(), d_res_bfloat16_2, num_elem * sizeof(float));
  gpu_device.memcpyDeviceToHost(full_prec.data(), d_res_float, num_elem * sizeof(float));
  gpu_device.synchronize();

  for (int i = 0; i < num_elem; ++i) {
    VERIFY_IS_APPROX(full_prec(i), bfloat16_prec1(i));
    VERIFY_IS_APPROX(full_prec(i), bfloat16_prec2(i));
  }

  gpu_device.deallocate(d_float);
  gpu_device.deallocate(d_res_bfloat16_1);
  gpu_device.deallocate(d_res_bfloat16_2);
  gpu_device.deallocate(d_res_float);
}

#endif

EIGEN_DECLARE_TEST(cxx11_tensor_of_bfloat16_gpu) {
  CALL_SUBTEST_1(test_gpu_numext<void>());

// The reduction unit tests have been excluded until a working
// implementation to expand the accumulator data type to float32
// is available.
// TODO: add reduction unit tests
#ifdef EIGEN_HAS_GPU_BF16
  CALL_SUBTEST_2(test_gpu_conversion<void>());
  CALL_SUBTEST_3(test_gpu_unary<void>());
  CALL_SUBTEST_4(test_gpu_elementwise<void>());
  CALL_SUBTEST_5(test_gpu_trancendental<void>());
  CALL_SUBTEST_6(test_gpu_contractions<void>());
  CALL_SUBTEST_7(test_gpu_reductions<void>());
  CALL_SUBTEST_8(test_gpu_full_reductions<void>());
  CALL_SUBTEST_9(test_gpu_forced_evals<void>());
#else
  std::cout << "bfloat16 floats are not supported by this version of gpu: skipping the test" << std::endl;
#endif
}
