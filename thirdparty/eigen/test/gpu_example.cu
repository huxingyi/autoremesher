// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2021 The Eigen Team.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// The following is an example GPU test.

#define EIGEN_USE_GPU
#include "main.h"  // Include the main test utilities.

// Define a kernel functor.
//
// The kernel must be a POD type and implement operator().
struct AddKernel {
  // Parameters must be POD or serializable Eigen types (e.g. Matrix,
  // Array). The return value must be a POD or serializable value type.
  template <typename Type1, typename Type2, typename Type3>
  EIGEN_DEVICE_FUNC Type3 operator()(const Type1& A, const Type2& B, Type3& C) const {
    C = A + B;        // Populate output parameter.
    Type3 D = A + B;  // Populate return value.
    return D;
  }
};

// Define a sub-test that uses the kernel.
template <typename T>
void test_add(const T& type) {
  const Index rows = type.rows();
  const Index cols = type.cols();

  // Create random inputs.
  const T A = T::Random(rows, cols);
  const T B = T::Random(rows, cols);
  T C;  // Output parameter.

  // Create kernel.
  AddKernel add_kernel;

  // Run add_kernel(A, B, C) via run(...).
  // This will run on the GPU if using a GPU compiler, or CPU otherwise,
  // facilitating generic tests that can run on either.
  T D = run(add_kernel, A, B, C);

  // Check that both output parameter and return value are correctly populated.
  const T expected = A + B;
  VERIFY_IS_CWISE_EQUAL(C, expected);
  VERIFY_IS_CWISE_EQUAL(D, expected);

  // In a GPU-only test, we can verify that the CPU and GPU produce the
  // same results.
  T C_cpu, C_gpu;
  T D_cpu = run_on_cpu(add_kernel, A, B, C_cpu);  // Runs on CPU.
  T D_gpu = run_on_gpu(add_kernel, A, B, C_gpu);  // Runs on GPU.
  VERIFY_IS_CWISE_EQUAL(C_cpu, C_gpu);
  VERIFY_IS_CWISE_EQUAL(D_cpu, D_gpu);
};

struct MultiplyKernel {
  template <typename Type1, typename Type2, typename Type3>
  EIGEN_DEVICE_FUNC Type3 operator()(const Type1& A, const Type2& B, Type3& C) const {
    C = A * B;
    return A * B;
  }
};

template <typename T1, typename T2, typename T3>
void test_multiply(const T1& type1, const T2& type2, const T3& type3) {
  const T1 A = T1::Random(type1.rows(), type1.cols());
  const T2 B = T2::Random(type2.rows(), type2.cols());
  T3 C;

  MultiplyKernel multiply_kernel;

  // The run(...) family of functions uses a memory buffer to transfer data back
  // and forth to and from the device.  The size of this buffer is estimated
  // from the size of all input parameters.  If the estimated buffer size is
  // not sufficient for transferring outputs from device-to-host, then an
  // explicit buffer size needs to be specified.

  // 2 outputs of size (A * B). For each matrix output, the buffer will store
  // the number of rows, columns, and the data.
  size_t buffer_capacity_hint = 2 * (                                                          // 2 output parameters
                                        2 * sizeof(typename T3::Index)                         // # Rows, # Cols
                                        + A.rows() * B.cols() * sizeof(typename T3::Scalar));  // Output data

  T3 D = run_with_hint(buffer_capacity_hint, multiply_kernel, A, B, C);

  const T3 expected = A * B;
  VERIFY_IS_CWISE_APPROX(C, expected);
  VERIFY_IS_CWISE_APPROX(D, expected);

  T3 C_cpu, C_gpu;
  T3 D_cpu = run_on_cpu(multiply_kernel, A, B, C_cpu);
  T3 D_gpu = run_on_gpu_with_hint(buffer_capacity_hint, multiply_kernel, A, B, C_gpu);
  VERIFY_IS_CWISE_APPROX(C_cpu, C_gpu);
  VERIFY_IS_CWISE_APPROX(D_cpu, D_gpu);
}

// Declare the test fixture.
EIGEN_DECLARE_TEST(gpu_example) {
  // For the number of repeats, call the desired subtests.
  for (int i = 0; i < g_repeat; i++) {
    // Call subtests with different sized/typed inputs.
    CALL_SUBTEST(test_add(Eigen::Vector3f()));
    CALL_SUBTEST(test_add(Eigen::Matrix3d()));
    CALL_SUBTEST(test_add(Eigen::MatrixX<int>(10, 10)));

    CALL_SUBTEST(test_add(Eigen::Array44f()));
    CALL_SUBTEST(test_add(Eigen::ArrayXd(20)));
    CALL_SUBTEST(test_add(Eigen::ArrayXXi(13, 17)));

    CALL_SUBTEST(test_multiply(Eigen::Matrix3d(), Eigen::Matrix3d(), Eigen::Matrix3d()));
    CALL_SUBTEST(test_multiply(Eigen::MatrixX<int>(10, 10), Eigen::MatrixX<int>(10, 10), Eigen::MatrixX<int>()));
    CALL_SUBTEST(test_multiply(Eigen::MatrixXf(12, 1), Eigen::MatrixXf(1, 32), Eigen::MatrixXf()));
  }
}
