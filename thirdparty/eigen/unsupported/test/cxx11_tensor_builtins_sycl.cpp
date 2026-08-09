// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2016
// Mehdi Goli    Codeplay Software Ltd.
// Ralph Potter  Codeplay Software Ltd.
// Luke Iwanski  Codeplay Software Ltd.
// Contact: <eigen@codeplay.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TEST_NO_LONGDOUBLE
#define EIGEN_TEST_NO_COMPLEX

#define EIGEN_DEFAULT_DENSE_INDEX_TYPE int64_t
#define EIGEN_USE_SYCL

#include "main.h"
#include <unsupported/Eigen/CXX11/Tensor>

using Eigen::array;
using Eigen::SyclDevice;
using Eigen::Tensor;
using Eigen::TensorMap;

// Functions used to compare the TensorMap implementation on the device with
// the equivalent on the host
namespace SYCL {

template <typename T>
T abs(T x) {
  return cl::sycl::abs(x);
}
template <>
Eigen::half abs(Eigen::half x) {
  return Eigen::half(cl::sycl::fabs(static_cast<cl::sycl::half>(x)));
}

template <>
float abs(float x) {
  return cl::sycl::fabs(x);
}

template <>
double abs(double x) {
  return cl::sycl::fabs(x);
}

template <typename T>
T square(T x) {
  return x * x;
}
template <typename T>
T cube(T x) {
  return x * x * x;
}
template <typename T>
T inverse(T x) {
  return T(1) / x;
}
template <typename T>
T cwiseMax(T x, T y) {
  return cl::sycl::max(x, y);
}
template <>
Eigen::half cwiseMax(Eigen::half x, Eigen::half y) {
  return Eigen::half(cl::sycl::max(static_cast<cl::sycl::half>(x), static_cast<cl::sycl::half>(y)));
}

template <typename T>
T cwiseMin(T x, T y) {
  return cl::sycl::min(x, y);
}
template <>
Eigen::half cwiseMin(Eigen::half x, Eigen::half y) {
  return Eigen::half(cl::sycl::min(static_cast<cl::sycl::half>(x), static_cast<cl::sycl::half>(y)));
}

template <typename T>
T sqrt(T x) {
  return cl::sycl::sqrt(x);
}
template <>
Eigen::half sqrt(Eigen::half x) {
  return Eigen::half(cl::sycl::sqrt(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T rsqrt(T x) {
  return cl::sycl::rsqrt(x);
}
template <>
Eigen::half rsqrt(Eigen::half x) {
  return Eigen::half(cl::sycl::rsqrt(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T tanh(T x) {
  return cl::sycl::tanh(x);
}
template <>
Eigen::half tanh(Eigen::half x) {
  return Eigen::half(cl::sycl::tanh(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T exp(T x) {
  return cl::sycl::exp(x);
}
template <>
Eigen::half exp(Eigen::half x) {
  return Eigen::half(cl::sycl::exp(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T expm1(T x) {
  return cl::sycl::expm1(x);
}
template <>
Eigen::half expm1(Eigen::half x) {
  return Eigen::half(cl::sycl::expm1(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T log(T x) {
  return cl::sycl::log(x);
}
template <>
Eigen::half log(Eigen::half x) {
  return Eigen::half(cl::sycl::log(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T ceil(T x) {
  return cl::sycl::ceil(x);
}
template <>
Eigen::half ceil(Eigen::half x) {
  return Eigen::half(cl::sycl::ceil(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T floor(T x) {
  return cl::sycl::floor(x);
}
template <>
Eigen::half floor(Eigen::half x) {
  return Eigen::half(cl::sycl::floor(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T round(T x) {
  return cl::sycl::round(x);
}
template <>
Eigen::half round(Eigen::half x) {
  return Eigen::half(cl::sycl::round(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T log1p(T x) {
  return cl::sycl::log1p(x);
}
template <>
Eigen::half log1p(Eigen::half x) {
  return Eigen::half(cl::sycl::log1p(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T sign(T x) {
  return cl::sycl::sign(x);
}
template <>
Eigen::half sign(Eigen::half x) {
  return Eigen::half(cl::sycl::sign(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T isnan(T x) {
  return cl::sycl::isnan(x);
}
template <>
Eigen::half isnan(Eigen::half x) {
  return Eigen::half(cl::sycl::isnan(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T isfinite(T x) {
  return cl::sycl::isfinite(x);
}
template <>
Eigen::half isfinite(Eigen::half x) {
  return Eigen::half(cl::sycl::isfinite(static_cast<cl::sycl::half>(x)));
}

template <typename T>
T isinf(T x) {
  return cl::sycl::isinf(x);
}
template <>
Eigen::half isinf(Eigen::half x) {
  return Eigen::half(cl::sycl::isinf(static_cast<cl::sycl::half>(x)));
}
}  // namespace SYCL

#define DECLARE_UNARY_STRUCT_NON_SYCL(FUNC)  \
  struct op_##FUNC {                         \
    template <typename T>                    \
    auto operator()(const T& x) {            \
      return SYCL::FUNC(x);                  \
    }                                        \
    template <typename T>                    \
    auto operator()(const TensorMap<T>& x) { \
      return x.FUNC();                       \
    }                                        \
  };

DECLARE_UNARY_STRUCT_NON_SYCL(abs)
DECLARE_UNARY_STRUCT_NON_SYCL(square)
DECLARE_UNARY_STRUCT_NON_SYCL(cube)
DECLARE_UNARY_STRUCT_NON_SYCL(inverse)

#define DECLARE_BINARY_STRUCT_NON_SYCL(FUNC)                          \
  struct op_##FUNC {                                                  \
    template <typename T1, typename T2>                               \
    auto operator()(const T1& x, const T2& y) {                       \
      return SYCL::FUNC(x, y);                                        \
    }                                                                 \
    template <typename T1, typename T2>                               \
    auto operator()(const TensorMap<T1>& x, const TensorMap<T2>& y) { \
      return x.FUNC(y);                                               \
    }                                                                 \
  };

DECLARE_BINARY_STRUCT_NON_SYCL(cwiseMax)
DECLARE_BINARY_STRUCT_NON_SYCL(cwiseMin)

struct EqualAssignment {
  template <typename Lhs, typename Rhs>
  void operator()(Lhs& lhs, const Rhs& rhs) {
    lhs = rhs;
  }
};

struct PlusEqualAssignment {
  template <typename Lhs, typename Rhs>
  void operator()(Lhs& lhs, const Rhs& rhs) {
    lhs += rhs;
  }
};

template <typename DataType, int DataLayout, typename Assignment, typename Operator>
void test_unary_builtins_for_scalar(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  Operator op;
  Assignment asgn;
  {
    /* Assignment(out, Operator(in)) */
    Tensor<DataType, 3, DataLayout, int64_t> in(tensor_range);
    Tensor<DataType, 3, DataLayout, int64_t> out(tensor_range);
    in = in.random() + DataType(0.01);
    out = out.random() + DataType(0.01);
    Tensor<DataType, 3, DataLayout, int64_t> reference(out);
    DataType* gpu_data = static_cast<DataType*>(sycl_device.allocate(in.size() * sizeof(DataType)));
    DataType* gpu_data_out = static_cast<DataType*>(sycl_device.allocate(out.size() * sizeof(DataType)));
    TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu(gpu_data, tensor_range);
    TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_out(gpu_data_out, tensor_range);
    sycl_device.memcpyHostToDevice(gpu_data, in.data(), (in.size()) * sizeof(DataType));
    sycl_device.memcpyHostToDevice(gpu_data_out, out.data(), (out.size()) * sizeof(DataType));
    auto device_expr = gpu_out.device(sycl_device);
    asgn(device_expr, op(gpu));
    sycl_device.memcpyDeviceToHost(out.data(), gpu_data_out, (out.size()) * sizeof(DataType));
    for (int64_t i = 0; i < out.size(); ++i) {
      DataType ver = reference(i);
      asgn(ver, op(in(i)));
      VERIFY_IS_APPROX(out(i), ver);
    }
    sycl_device.deallocate(gpu_data);
    sycl_device.deallocate(gpu_data_out);
  }
  {
    /* Assignment(out, Operator(out)) */
    Tensor<DataType, 3, DataLayout, int64_t> out(tensor_range);
    // Offset with 1 to avoid tiny output (< 1e-6) as they can easily fail.
    out = out.random() + DataType(1);
    Tensor<DataType, 3, DataLayout, int64_t> reference(out);
    DataType* gpu_data_out = static_cast<DataType*>(sycl_device.allocate(out.size() * sizeof(DataType)));
    TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_out(gpu_data_out, tensor_range);
    sycl_device.memcpyHostToDevice(gpu_data_out, out.data(), (out.size()) * sizeof(DataType));
    auto device_expr = gpu_out.device(sycl_device);
    asgn(device_expr, op(gpu_out));
    sycl_device.memcpyDeviceToHost(out.data(), gpu_data_out, (out.size()) * sizeof(DataType));
    for (int64_t i = 0; i < out.size(); ++i) {
      DataType ver = reference(i);
      asgn(ver, op(reference(i)));
      VERIFY_IS_APPROX(out(i), ver);
    }
    sycl_device.deallocate(gpu_data_out);
  }
}

#define DECLARE_UNARY_STRUCT(FUNC)                                 \
  struct op_##FUNC {                                               \
    template <typename T>                                          \
    auto operator()(const T& x) -> decltype(SYCL::FUNC(x)) {       \
      return SYCL::FUNC(x);                                        \
    }                                                              \
    template <typename T>                                          \
    auto operator()(const TensorMap<T>& x) -> decltype(x.FUNC()) { \
      return x.FUNC();                                             \
    }                                                              \
  };

DECLARE_UNARY_STRUCT(sqrt)
DECLARE_UNARY_STRUCT(rsqrt)
DECLARE_UNARY_STRUCT(tanh)
DECLARE_UNARY_STRUCT(exp)
DECLARE_UNARY_STRUCT(expm1)
DECLARE_UNARY_STRUCT(log)
DECLARE_UNARY_STRUCT(ceil)
DECLARE_UNARY_STRUCT(floor)
DECLARE_UNARY_STRUCT(round)
DECLARE_UNARY_STRUCT(log1p)
DECLARE_UNARY_STRUCT(sign)
DECLARE_UNARY_STRUCT(isnan)
DECLARE_UNARY_STRUCT(isfinite)
DECLARE_UNARY_STRUCT(isinf)

template <typename DataType, int DataLayout, typename Assignment>
void test_unary_builtins_for_assignement(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
#define RUN_UNARY_TEST(FUNC) \
  test_unary_builtins_for_scalar<DataType, DataLayout, Assignment, op_##FUNC>(sycl_device, tensor_range)
  RUN_UNARY_TEST(abs);
  RUN_UNARY_TEST(sqrt);
  RUN_UNARY_TEST(rsqrt);
  RUN_UNARY_TEST(square);
  RUN_UNARY_TEST(cube);
  RUN_UNARY_TEST(inverse);
  RUN_UNARY_TEST(tanh);
  RUN_UNARY_TEST(exp);
  RUN_UNARY_TEST(expm1);
  RUN_UNARY_TEST(log);
  RUN_UNARY_TEST(ceil);
  RUN_UNARY_TEST(floor);
  RUN_UNARY_TEST(round);
  RUN_UNARY_TEST(log1p);
  RUN_UNARY_TEST(sign);
}

template <typename DataType, int DataLayout, typename Operator>
void test_unary_builtins_return_bool(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  /* out = op(in) */
  Operator op;
  Tensor<DataType, 3, DataLayout, int64_t> in(tensor_range);
  Tensor<bool, 3, DataLayout, int64_t> out(tensor_range);
  in = in.random() + DataType(0.01);
  DataType* gpu_data = static_cast<DataType*>(sycl_device.allocate(in.size() * sizeof(DataType)));
  bool* gpu_data_out = static_cast<bool*>(sycl_device.allocate(out.size() * sizeof(bool)));
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu(gpu_data, tensor_range);
  TensorMap<Tensor<bool, 3, DataLayout, int64_t>> gpu_out(gpu_data_out, tensor_range);
  sycl_device.memcpyHostToDevice(gpu_data, in.data(), (in.size()) * sizeof(DataType));
  gpu_out.device(sycl_device) = op(gpu);
  sycl_device.memcpyDeviceToHost(out.data(), gpu_data_out, (out.size()) * sizeof(bool));
  for (int64_t i = 0; i < out.size(); ++i) {
    VERIFY_IS_EQUAL(out(i), op(in(i)));
  }
  sycl_device.deallocate(gpu_data);
  sycl_device.deallocate(gpu_data_out);
}

template <typename DataType, int DataLayout>
void test_unary_builtins(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  test_unary_builtins_for_assignement<DataType, DataLayout, PlusEqualAssignment>(sycl_device, tensor_range);
  test_unary_builtins_for_assignement<DataType, DataLayout, EqualAssignment>(sycl_device, tensor_range);
  test_unary_builtins_return_bool<DataType, DataLayout, op_isnan>(sycl_device, tensor_range);
  test_unary_builtins_return_bool<DataType, DataLayout, op_isfinite>(sycl_device, tensor_range);
  test_unary_builtins_return_bool<DataType, DataLayout, op_isinf>(sycl_device, tensor_range);
}

template <typename DataType>
static void test_builtin_unary_sycl(const Eigen::SyclDevice& sycl_device) {
  int64_t sizeDim1 = 10;
  int64_t sizeDim2 = 10;
  int64_t sizeDim3 = 10;
  array<int64_t, 3> tensor_range = {{sizeDim1, sizeDim2, sizeDim3}};

  test_unary_builtins<DataType, RowMajor>(sycl_device, tensor_range);
  test_unary_builtins<DataType, ColMajor>(sycl_device, tensor_range);
}

template <typename DataType, int DataLayout, typename Operator>
void test_binary_builtins_func(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  /* out = op(in_1, in_2) */
  Operator op;
  Tensor<DataType, 3, DataLayout, int64_t> in_1(tensor_range);
  Tensor<DataType, 3, DataLayout, int64_t> in_2(tensor_range);
  Tensor<DataType, 3, DataLayout, int64_t> out(tensor_range);
  in_1 = in_1.random() + DataType(0.01);
  in_2 = in_2.random() + DataType(0.01);
  Tensor<DataType, 3, DataLayout, int64_t> reference(out);
  DataType* gpu_data_1 = static_cast<DataType*>(sycl_device.allocate(in_1.size() * sizeof(DataType)));
  DataType* gpu_data_2 = static_cast<DataType*>(sycl_device.allocate(in_2.size() * sizeof(DataType)));
  DataType* gpu_data_out = static_cast<DataType*>(sycl_device.allocate(out.size() * sizeof(DataType)));
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_1(gpu_data_1, tensor_range);
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_2(gpu_data_2, tensor_range);
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_out(gpu_data_out, tensor_range);
  sycl_device.memcpyHostToDevice(gpu_data_1, in_1.data(), (in_1.size()) * sizeof(DataType));
  sycl_device.memcpyHostToDevice(gpu_data_2, in_2.data(), (in_2.size()) * sizeof(DataType));
  gpu_out.device(sycl_device) = op(gpu_1, gpu_2);
  sycl_device.memcpyDeviceToHost(out.data(), gpu_data_out, (out.size()) * sizeof(DataType));
  for (int64_t i = 0; i < out.size(); ++i) {
    VERIFY_IS_APPROX(out(i), op(in_1(i), in_2(i)));
  }
  sycl_device.deallocate(gpu_data_1);
  sycl_device.deallocate(gpu_data_2);
  sycl_device.deallocate(gpu_data_out);
}

template <typename DataType, int DataLayout, typename Operator>
void test_binary_builtins_fixed_arg2(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  /* out = op(in_1, 2) */
  Operator op;
  const DataType arg2(2);
  Tensor<DataType, 3, DataLayout, int64_t> in_1(tensor_range);
  Tensor<DataType, 3, DataLayout, int64_t> out(tensor_range);
  in_1 = in_1.random();
  Tensor<DataType, 3, DataLayout, int64_t> reference(out);
  DataType* gpu_data_1 = static_cast<DataType*>(sycl_device.allocate(in_1.size() * sizeof(DataType)));
  DataType* gpu_data_out = static_cast<DataType*>(sycl_device.allocate(out.size() * sizeof(DataType)));
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_1(gpu_data_1, tensor_range);
  TensorMap<Tensor<DataType, 3, DataLayout, int64_t>> gpu_out(gpu_data_out, tensor_range);
  sycl_device.memcpyHostToDevice(gpu_data_1, in_1.data(), (in_1.size()) * sizeof(DataType));
  gpu_out.device(sycl_device) = op(gpu_1, arg2);
  sycl_device.memcpyDeviceToHost(out.data(), gpu_data_out, (out.size()) * sizeof(DataType));
  for (int64_t i = 0; i < out.size(); ++i) {
    VERIFY_IS_APPROX(out(i), op(in_1(i), arg2));
  }
  sycl_device.deallocate(gpu_data_1);
  sycl_device.deallocate(gpu_data_out);
}

#define DECLARE_BINARY_STRUCT(FUNC)                                                          \
  struct op_##FUNC {                                                                         \
    template <typename T1, typename T2>                                                      \
    auto operator()(const T1& x, const T2& y) -> decltype(cl::sycl::FUNC(x, y)) {            \
      return cl::sycl::FUNC(x, y);                                                           \
    }                                                                                        \
    template <typename T1, typename T2>                                                      \
    auto operator()(const TensorMap<T1>& x, const TensorMap<T2>& y) -> decltype(x.FUNC(y)) { \
      return x.FUNC(y);                                                                      \
    }                                                                                        \
  };

#define DECLARE_BINARY_STRUCT_OP(NAME, OPERATOR)                          \
  struct op_##NAME {                                                      \
    template <typename T1, typename T2>                                   \
    auto operator()(const T1& x, const T2& y) -> decltype(x OPERATOR y) { \
      return x OPERATOR y;                                                \
    }                                                                     \
  };

DECLARE_BINARY_STRUCT_OP(plus, +)
DECLARE_BINARY_STRUCT_OP(minus, -)
DECLARE_BINARY_STRUCT_OP(times, *)
DECLARE_BINARY_STRUCT_OP(divide, /)
DECLARE_BINARY_STRUCT_OP(modulo, %)

template <typename DataType, int DataLayout>
void test_binary_builtins(const Eigen::SyclDevice& sycl_device, const array<int64_t, 3>& tensor_range) {
  test_binary_builtins_func<DataType, DataLayout, op_cwiseMax>(sycl_device, tensor_range);
  test_binary_builtins_func<DataType, DataLayout, op_cwiseMin>(sycl_device, tensor_range);
  test_binary_builtins_func<DataType, DataLayout, op_plus>(sycl_device, tensor_range);
  test_binary_builtins_func<DataType, DataLayout, op_minus>(sycl_device, tensor_range);
  test_binary_builtins_func<DataType, DataLayout, op_times>(sycl_device, tensor_range);
  test_binary_builtins_func<DataType, DataLayout, op_divide>(sycl_device, tensor_range);
}

template <typename DataType>
static void test_floating_builtin_binary_sycl(const Eigen::SyclDevice& sycl_device) {
  int64_t sizeDim1 = 10;
  int64_t sizeDim2 = 10;
  int64_t sizeDim3 = 10;
  array<int64_t, 3> tensor_range = {{sizeDim1, sizeDim2, sizeDim3}};
  test_binary_builtins<DataType, RowMajor>(sycl_device, tensor_range);
  test_binary_builtins<DataType, ColMajor>(sycl_device, tensor_range);
}

template <typename DataType>
static void test_integer_builtin_binary_sycl(const Eigen::SyclDevice& sycl_device) {
  int64_t sizeDim1 = 10;
  int64_t sizeDim2 = 10;
  int64_t sizeDim3 = 10;
  array<int64_t, 3> tensor_range = {{sizeDim1, sizeDim2, sizeDim3}};
  test_binary_builtins_fixed_arg2<DataType, RowMajor, op_modulo>(sycl_device, tensor_range);
  test_binary_builtins_fixed_arg2<DataType, ColMajor, op_modulo>(sycl_device, tensor_range);
}

EIGEN_DECLARE_TEST(cxx11_tensor_builtins_sycl) {
  for (const auto& device : Eigen::get_sycl_supported_devices()) {
    QueueInterface queueInterface(device);
    Eigen::SyclDevice sycl_device(&queueInterface);
    CALL_SUBTEST_1(test_builtin_unary_sycl<half>(sycl_device));
    CALL_SUBTEST_2(test_floating_builtin_binary_sycl<half>(sycl_device));
    CALL_SUBTEST_3(test_builtin_unary_sycl<float>(sycl_device));
    CALL_SUBTEST_4(test_floating_builtin_binary_sycl<float>(sycl_device));
    CALL_SUBTEST_5(test_integer_builtin_binary_sycl<int>(sycl_device));
  }
}
