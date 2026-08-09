#ifndef GPU_TEST_HELPER_H
#define GPU_TEST_HELPER_H

#include <Eigen/Core>

// Allow gpu** macros for generic tests.
#include <Eigen/src/Core/util/GpuHipCudaDefines.inc>

// std::tuple cannot be used on device, and there is a bug in cuda < 9.2 that
// doesn't allow std::tuple to compile for host code either. In these cases,
// use our custom implementation.
#if defined(EIGEN_GPU_COMPILE_PHASE) || (defined(EIGEN_CUDACC) && EIGEN_CUDA_SDK_VER < 92000)
#define EIGEN_USE_CUSTOM_TUPLE 1
#else
#define EIGEN_USE_CUSTOM_TUPLE 0
#endif

#if EIGEN_USE_CUSTOM_TUPLE
#include "../Eigen/src/Core/arch/GPU/Tuple.h"
#else
#include <tuple>
#endif
namespace Eigen {

namespace internal {

// Note: cannot re-use tuple_impl, since that will cause havoc for
// tuple_test.
namespace test_detail {
// Use std::tuple on CPU, otherwise use the GPU-specific versions.
#if !EIGEN_USE_CUSTOM_TUPLE
using std::get;
using std::make_tuple;
using std::tie;
using std::tuple;
#else
using tuple_impl::get;
using tuple_impl::make_tuple;
using tuple_impl::tie;
using tuple_impl::tuple;
#endif
#undef EIGEN_USE_CUSTOM_TUPLE
}  // namespace test_detail

template <size_t N, size_t Idx, typename OutputIndexSequence, typename... Ts>
struct extract_output_indices_helper;

/**
 * Extracts a set of indices corresponding to non-const l-value reference
 * output types.
 *
 * \internal
 * \tparam N the number of types {T1, Ts...}.
 * \tparam Idx the "index" to append if T1 is an output type.
 * \tparam OutputIndices the current set of output indices.
 * \tparam T1 the next type to consider, with index Idx.
 * \tparam Ts the remaining types.
 */
template <size_t N, size_t Idx, size_t... OutputIndices, typename T1, typename... Ts>
struct extract_output_indices_helper<N, Idx, std::index_sequence<OutputIndices...>, T1, Ts...> {
  using type = typename extract_output_indices_helper<
      N - 1, Idx + 1,
      typename std::conditional<
          // If is a non-const l-value reference, append index.
          std::is_lvalue_reference<T1>::value && !std::is_const<std::remove_reference_t<T1>>::value,
          std::index_sequence<OutputIndices..., Idx>, std::index_sequence<OutputIndices...>>::type,
      Ts...>::type;
};

// Base case.
template <size_t Idx, size_t... OutputIndices>
struct extract_output_indices_helper<0, Idx, std::index_sequence<OutputIndices...>> {
  using type = std::index_sequence<OutputIndices...>;
};

// Extracts a set of indices into Types... that correspond to non-const
// l-value references.
template <typename... Types>
using extract_output_indices =
    typename extract_output_indices_helper<sizeof...(Types), 0, std::index_sequence<>, Types...>::type;

// Helper struct for dealing with Generic functors that may return void.
struct void_helper {
  struct Void {};

  // Converts void -> Void, T otherwise.
  template <typename T>
  using ReturnType = typename std::conditional<std::is_same<T, void>::value, Void, T>::type;

  // Non-void return value.
  template <typename Func, typename... Args>
  static EIGEN_ALWAYS_INLINE EIGEN_DEVICE_FUNC auto call(Func&& func, Args&&... args)
      -> std::enable_if_t<!std::is_same<decltype(func(args...)), void>::value, decltype(func(args...))> {
    return func(std::forward<Args>(args)...);
  }

  // Void return value.
  template <typename Func, typename... Args>
  static EIGEN_ALWAYS_INLINE EIGEN_DEVICE_FUNC auto call(Func&& func, Args&&... args)
      -> std::enable_if_t<std::is_same<decltype(func(args...)), void>::value, Void> {
    func(std::forward<Args>(args)...);
    return Void{};
  }

  // Restores the original return type, Void -> void, T otherwise.
  template <typename T>
  static EIGEN_ALWAYS_INLINE EIGEN_DEVICE_FUNC
      std::enable_if_t<!std::is_same<typename std::decay<T>::type, Void>::value, T>
      restore(T&& val) {
    return val;
  }

  // Void case.
  template <typename T = void>
  static EIGEN_ALWAYS_INLINE EIGEN_DEVICE_FUNC void restore(const Void&) {}
};

// Runs a kernel via serialized buffer.  Does this by deserializing the buffer
// to construct the arguments, calling the kernel, then re-serialing the outputs.
// The buffer contains
//     [ input_buffer_size, args ]
// After the kernel call, it is then populated with
//     [ output_buffer_size, output_parameters, return_value ]
// If the output_buffer_size exceeds the buffer's capacity, then only the
// output_buffer_size is populated.
template <typename Kernel, typename... Args, size_t... Indices, size_t... OutputIndices>
EIGEN_DEVICE_FUNC void run_serialized(std::index_sequence<Indices...>, std::index_sequence<OutputIndices...>,
                                      Kernel kernel, uint8_t* buffer, size_t capacity) {
  using test_detail::get;
  using test_detail::make_tuple;
  using test_detail::tuple;
  // Deserialize input size and inputs.
  size_t input_size;
  const uint8_t* read_ptr = buffer;
  const uint8_t* read_end = buffer + capacity;
  read_ptr = Eigen::deserialize(read_ptr, read_end, input_size);
  // Create value-type instances to populate.
  auto args = make_tuple(typename std::decay<Args>::type{}...);
  EIGEN_UNUSED_VARIABLE(args)  // Avoid NVCC compile warning.
  // NVCC 9.1 requires us to spell out the template parameters explicitly.
  read_ptr = Eigen::deserialize(read_ptr, read_end, get<Indices, typename std::decay<Args>::type...>(args)...);

  // Call function, with void->Void conversion so we are guaranteed a complete
  // output type.
  auto result = void_helper::call(kernel, get<Indices, typename std::decay<Args>::type...>(args)...);

  // Determine required output size.
  size_t output_size = Eigen::serialize_size(capacity);
  output_size += Eigen::serialize_size(get<OutputIndices, typename std::decay<Args>::type...>(args)...);
  output_size += Eigen::serialize_size(result);

  // Always serialize required buffer size.
  uint8_t* write_ptr = buffer;
  uint8_t* write_end = buffer + capacity;
  write_ptr = Eigen::serialize(write_ptr, write_end, output_size);
  // Null `write_ptr` can be safely passed along.
  // Serialize outputs if they fit in the buffer.
  if (output_size <= capacity) {
    // Collect outputs and result.
    write_ptr = Eigen::serialize(write_ptr, write_end, get<OutputIndices, typename std::decay<Args>::type...>(args)...);
    write_ptr = Eigen::serialize(write_ptr, write_end, result);
  }
}

template <typename Kernel, typename... Args>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE void run_serialized(Kernel kernel, uint8_t* buffer, size_t capacity) {
  run_serialized<Kernel, Args...>(std::make_index_sequence<sizeof...(Args)>{}, extract_output_indices<Args...>{},
                                  kernel, buffer, capacity);
}

#ifdef EIGEN_GPUCC

// Checks for GPU errors and asserts / prints the error message.
#define GPU_CHECK(expr)                                                 \
  do {                                                                  \
    gpuError_t err = expr;                                              \
    if (err != gpuSuccess) {                                            \
      printf("%s: %s\n", gpuGetErrorName(err), gpuGetErrorString(err)); \
      gpu_assert(false);                                                \
    }                                                                   \
  } while (0)

// Calls run_serialized on the GPU.
template <typename Kernel, typename... Args>
__global__ EIGEN_HIP_LAUNCH_BOUNDS_1024 void run_serialized_on_gpu_meta_kernel(const Kernel kernel, uint8_t* buffer,
                                                                               size_t capacity) {
  run_serialized<Kernel, Args...>(kernel, buffer, capacity);
}

// Runs kernel(args...) on the GPU via the serialization mechanism.
//
// Note: this may end up calling the kernel multiple times if the initial output
// buffer is not large enough to hold the outputs.
template <typename Kernel, typename... Args, size_t... Indices, size_t... OutputIndices>
auto run_serialized_on_gpu(size_t buffer_capacity_hint, std::index_sequence<Indices...>,
                           std::index_sequence<OutputIndices...>, Kernel kernel, Args&&... args)
    -> decltype(kernel(args...)) {
  // Compute the required serialization buffer capacity.
  // Round up input size to next power of two to give a little extra room
  // for outputs.
  size_t input_data_size = sizeof(size_t) + Eigen::serialize_size(args...);

  size_t capacity;
  if (buffer_capacity_hint == 0) {
    // Estimate as the power of two larger than the total input size.
    capacity = sizeof(size_t);
    while (capacity <= input_data_size) {
      capacity *= 2;
    }
  } else {
    // Use the larger of the hint and the total input size.
    // Add sizeof(size_t) to the hint to account for storing the buffer capacity
    // itself so the user doesn't need to think about this.
    capacity = std::max<size_t>(buffer_capacity_hint + sizeof(size_t), input_data_size);
  }
  std::vector<uint8_t> buffer(capacity);

  uint8_t* host_data = nullptr;
  uint8_t* host_data_end = nullptr;
  uint8_t* host_ptr = nullptr;
  uint8_t* device_data = nullptr;
  size_t output_data_size = 0;

  // Allocate buffers and copy input data.
  capacity = std::max<size_t>(capacity, output_data_size);
  buffer.resize(capacity);
  host_data = buffer.data();
  host_data_end = buffer.data() + capacity;
  host_ptr = Eigen::serialize(host_data, host_data_end, input_data_size);
  host_ptr = Eigen::serialize(host_ptr, host_data_end, args...);

  // Copy inputs to host.
  gpuMalloc((void**)(&device_data), capacity);
  gpuMemcpy(device_data, buffer.data(), input_data_size, gpuMemcpyHostToDevice);
  GPU_CHECK(gpuDeviceSynchronize());

// Run kernel.
#ifdef EIGEN_USE_HIP
  hipLaunchKernelGGL(HIP_KERNEL_NAME(run_serialized_on_gpu_meta_kernel<Kernel, Args...>), 1, 1, 0, 0, kernel,
                     device_data, capacity);
#else
  run_serialized_on_gpu_meta_kernel<Kernel, Args...><<<1, 1>>>(kernel, device_data, capacity);
#endif
  // Check pre-launch and kernel execution errors.
  GPU_CHECK(gpuGetLastError());
  GPU_CHECK(gpuDeviceSynchronize());
  // Copy back new output to host.
  gpuMemcpy(host_data, device_data, capacity, gpuMemcpyDeviceToHost);
  gpuFree(device_data);
  GPU_CHECK(gpuDeviceSynchronize());

  // Determine output buffer size.
  const uint8_t* c_host_ptr = Eigen::deserialize(host_data, host_data_end, output_data_size);
  // If the output doesn't fit in the buffer, spit out warning and fail.
  if (output_data_size > capacity) {
    std::cerr << "The serialized output does not fit in the output buffer, " << output_data_size << " vs capacity "
              << capacity << "." << std::endl
              << "Try specifying a minimum buffer capacity: " << std::endl
              << "  run_with_hint(" << output_data_size << ", ...)" << std::endl;
    VERIFY(false);
  }

  // Deserialize outputs.
  auto args_tuple = test_detail::tie(args...);
  EIGEN_UNUSED_VARIABLE(args_tuple)  // Avoid NVCC compile warning.
  c_host_ptr = Eigen::deserialize(c_host_ptr, host_data_end, test_detail::get<OutputIndices, Args&...>(args_tuple)...);

  // Maybe deserialize return value, properly handling void.
  typename void_helper::ReturnType<decltype(kernel(args...))> result;
  c_host_ptr = Eigen::deserialize(c_host_ptr, host_data_end, result);
  return void_helper::restore(result);
}

#endif  // EIGEN_GPUCC

}  // namespace internal

/**
 * Runs a kernel on the CPU, returning the results.
 * \param kernel kernel to run.
 * \param args ... input arguments.
 * \return kernel(args...).
 */
template <typename Kernel, typename... Args>
auto run_on_cpu(Kernel kernel, Args&&... args) -> decltype(kernel(args...)) {
  return kernel(std::forward<Args>(args)...);
}

#ifdef EIGEN_GPUCC

/**
 * Runs a kernel on the GPU, returning the results.
 *
 * The kernel must be able to be passed directly as an input to a global
 * function (i.e. empty or POD).  Its inputs must be "Serializable" so we
 * can transfer them to the device, and the output must be a Serializable value
 * type so it can be transferred back from the device.
 *
 * \param kernel kernel to run.
 * \param args ... input arguments, must be "Serializable".
 * \return kernel(args...).
 */
template <typename Kernel, typename... Args>
auto run_on_gpu(Kernel kernel, Args&&... args) -> decltype(kernel(args...)) {
  return internal::run_serialized_on_gpu<Kernel, Args...>(
      /*buffer_capacity_hint=*/0, std::make_index_sequence<sizeof...(Args)>{},
      internal::extract_output_indices<Args...>{}, kernel, std::forward<Args>(args)...);
}

/**
 * Runs a kernel on the GPU, returning the results.
 *
 * This version allows specifying a minimum buffer capacity size required for
 * serializing the puts to transfer results from device to host.  Use this when
 * `run_on_gpu(...)` fails to determine an appropriate capacity by default.
 *
 * \param buffer_capacity_hint minimum required buffer size for serializing
 *        outputs.
 * \param kernel kernel to run.
 * \param args ... input arguments, must be "Serializable".
 * \return kernel(args...).
 * \sa run_on_gpu
 */
template <typename Kernel, typename... Args>
auto run_on_gpu_with_hint(size_t buffer_capacity_hint, Kernel kernel, Args&&... args) -> decltype(kernel(args...)) {
  return internal::run_serialized_on_gpu<Kernel, Args...>(
      buffer_capacity_hint, std::make_index_sequence<sizeof...(Args)>{}, internal::extract_output_indices<Args...>{},
      kernel, std::forward<Args>(args)...);
}

/**
 * Kernel for determining basic Eigen compile-time information
 * (i.e. the cuda/hip arch)
 */
struct CompileTimeDeviceInfoKernel {
  struct Info {
    int cuda;
    int hip;
  };

  EIGEN_DEVICE_FUNC Info operator()() const {
    Info info = {-1, -1};
#if defined(__CUDA_ARCH__)
    info.cuda = static_cast<int>(__CUDA_ARCH__ + 0);
#endif
#if defined(EIGEN_HIP_DEVICE_COMPILE)
    info.hip = static_cast<int>(EIGEN_HIP_DEVICE_COMPILE + 0);
#endif
    return info;
  }
};

/**
 * Queries and prints the compile-time and runtime GPU info.
 */
void print_gpu_device_info() {
  int device = 0;
  gpuDeviceProp_t deviceProp;
  gpuGetDeviceProperties(&deviceProp, device);

  auto info = run_on_gpu(CompileTimeDeviceInfoKernel());

  std::cout << "GPU compile-time info:\n";

#ifdef EIGEN_CUDACC
  std::cout << "  EIGEN_CUDACC:                " << int(EIGEN_CUDACC) << std::endl;
#endif

#ifdef EIGEN_CUDA_SDK_VER
  std::cout << "  EIGEN_CUDA_SDK_VER:          " << int(EIGEN_CUDA_SDK_VER) << std::endl;
#endif

#if EIGEN_COMP_NVCC
  std::cout << "  EIGEN_COMP_NVCC:             " << int(EIGEN_COMP_NVCC) << std::endl;
#endif

#ifdef EIGEN_HIPCC
  std::cout << "  EIGEN_HIPCC:                 " << int(EIGEN_HIPCC) << std::endl;
#endif

  std::cout << "  EIGEN_CUDA_ARCH:             " << info.cuda << std::endl;
  std::cout << "  EIGEN_HIP_DEVICE_COMPILE:    " << info.hip << std::endl;

  std::cout << "GPU device info:\n";
  std::cout << "  name:                        " << deviceProp.name << std::endl;
  std::cout << "  capability:                  " << deviceProp.major << "." << deviceProp.minor << std::endl;
  std::cout << "  multiProcessorCount:         " << deviceProp.multiProcessorCount << std::endl;
  std::cout << "  maxThreadsPerMultiProcessor: " << deviceProp.maxThreadsPerMultiProcessor << std::endl;
  std::cout << "  warpSize:                    " << deviceProp.warpSize << std::endl;
  std::cout << "  regsPerBlock:                " << deviceProp.regsPerBlock << std::endl;
  std::cout << "  concurrentKernels:           " << deviceProp.concurrentKernels << std::endl;
}

#endif  // EIGEN_GPUCC

/**
 * Runs a kernel on the GPU (if EIGEN_GPUCC), or CPU otherwise.
 *
 * This is to better support creating generic tests.
 *
 * The kernel must be able to be passed directly as an input to a global
 * function (i.e. empty or POD).  Its inputs must be "Serializable" so we
 * can transfer them to the device, and the output must be a Serializable value
 * type so it can be transferred back from the device.
 *
 * \param kernel kernel to run.
 * \param args ... input arguments, must be "Serializable".
 * \return kernel(args...).
 */
template <typename Kernel, typename... Args>
auto run(Kernel kernel, Args&&... args) -> decltype(kernel(args...)) {
#ifdef EIGEN_GPUCC
  return run_on_gpu(kernel, std::forward<Args>(args)...);
#else
  return run_on_cpu(kernel, std::forward<Args>(args)...);
#endif
}

/**
 * Runs a kernel on the GPU (if EIGEN_GPUCC), or CPU otherwise.
 *
 * This version allows specifying a minimum buffer capacity size required for
 * serializing the puts to transfer results from device to host.  Use this when
 * `run(...)` fails to determine an appropriate capacity by default.
 *
 * \param buffer_capacity_hint minimum required buffer size for serializing
 *        outputs.
 * \param kernel kernel to run.
 * \param args ... input arguments, must be "Serializable".
 * \return kernel(args...).
 * \sa run
 */
template <typename Kernel, typename... Args>
auto run_with_hint(size_t buffer_capacity_hint, Kernel kernel, Args&&... args) -> decltype(kernel(args...)) {
#ifdef EIGEN_GPUCC
  return run_on_gpu_with_hint(buffer_capacity_hint, kernel, std::forward<Args>(args)...);
#else
  EIGEN_UNUSED_VARIABLE(buffer_capacity_hint)
  return run_on_cpu(kernel, std::forward<Args>(args)...);
#endif
}

}  // namespace Eigen

#endif  // GPU_TEST_HELPER_H
