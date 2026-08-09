// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"
#include <typeinfo>

#if defined __GNUC__ && __GNUC__ >= 6
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif
// using namespace Eigen;

bool g_first_pass = true;

namespace Eigen {

namespace test {

template <typename T, std::enable_if_t<NumTraits<T>::IsSigned, bool> = true>
T negate(const T& x) {
  return -x;
}

template <typename T, std::enable_if_t<!NumTraits<T>::IsSigned, bool> = true>
T negate(const T& x) {
  return T(0) - x;
}

template <typename T>
Map<const Array<unsigned char, sizeof(T), 1> > bits(const T& x) {
  return Map<const Array<unsigned char, sizeof(T), 1> >(reinterpret_cast<const unsigned char*>(&x));
}

template <typename T>
bool biteq(T a, T b) {
  return (bits(a) == bits(b)).all();
}

// NOTE: we disable inlining for this function to workaround a GCC issue when using -O3 and the i387 FPU.
template <typename Scalar>
EIGEN_DONT_INLINE bool isApproxAbs(const Scalar& a, const Scalar& b, const typename NumTraits<Scalar>::Real& refvalue) {
  return internal::isMuchSmallerThan(a - b, refvalue);
}

template <typename Scalar>
inline void print_mismatch(const Scalar* ref, const Scalar* vec, int size) {
  std::cout << "ref: [" << Map<const Matrix<Scalar, 1, Dynamic> >(ref, size) << "]"
            << " != vec: [" << Map<const Matrix<Scalar, 1, Dynamic> >(vec, size) << "]\n";
}

template <typename Scalar>
bool areApproxAbs(const Scalar* a, const Scalar* b, int size, const typename NumTraits<Scalar>::Real& refvalue) {
  for (int i = 0; i < size; ++i) {
    if (!isApproxAbs(a[i], b[i], refvalue)) {
      print_mismatch(a, b, size);
      std::cout << std::setprecision(16) << "Values differ in position " << i << ": " << a[i] << " vs " << b[i]
                << std::endl;
      return false;
    }
  }
  return true;
}

template <typename Scalar>
bool areApprox(const Scalar* a, const Scalar* b, int size) {
  for (int i = 0; i < size; ++i) {
    if (numext::not_equal_strict(a[i], b[i]) && !internal::isApprox(a[i], b[i]) &&
        !((numext::isnan)(a[i]) && (numext::isnan)(b[i]))) {
      print_mismatch(a, b, size);
      std::cout << std::setprecision(16) << "Values differ in position " << i << ": " << a[i] << " vs " << b[i]
                << std::endl;
      return false;
    }
  }
  return true;
}

template <typename Scalar>
bool areEqual(const Scalar* a, const Scalar* b, int size) {
  for (int i = 0; i < size; ++i) {
    if (numext::not_equal_strict(a[i], b[i]) && !((numext::isnan)(a[i]) && (numext::isnan)(b[i]))) {
      print_mismatch(a, b, size);
      std::cout << std::setprecision(16) << "Values differ in position " << i << ": " << a[i] << " vs " << b[i]
                << std::endl;
      return false;
    }
  }
  return true;
}

template <typename Scalar>
bool areApprox(const Scalar* a, const Scalar* b, int size, const typename NumTraits<Scalar>::Real& precision) {
  for (int i = 0; i < size; ++i) {
    if (numext::not_equal_strict(a[i], b[i]) && !internal::isApprox(a[i], b[i], precision) &&
        !((numext::isnan)(a[i]) && (numext::isnan)(b[i]))) {
      print_mismatch(a, b, size);
      std::cout << std::setprecision(16) << "Values differ in position " << i << ": " << a[i] << " vs " << b[i]
                << std::endl;
      return false;
    }
  }
  return true;
}

#define CHECK_CWISE1(REFOP, POP)                                   \
  {                                                                \
    for (int i = 0; i < PacketSize; ++i) ref[i] = REFOP(data1[i]); \
    internal::pstore(data2, POP(internal::pload<Packet>(data1)));  \
    VERIFY(test::areApprox(ref, data2, PacketSize) && #POP);       \
  }

#define CHECK_CWISE1_MASK(REFOP, POP)                                \
  {                                                                  \
    bool ref_mask[PacketSize] = {};                                  \
    bool data_mask[PacketSize] = {};                                 \
    internal::pstore(data2, POP(internal::pload<Packet>(data1)));    \
    for (int i = 0; i < PacketSize; ++i) {                           \
      ref_mask[i] = numext::is_exactly_zero(REFOP(data1[i]));        \
      data_mask[i] = numext::is_exactly_zero(data2[i]);              \
    }                                                                \
    VERIFY(test::areEqual(ref_mask, data_mask, PacketSize) && #POP); \
  }

#define CHECK_CWISE2_MASK(REFOP, POP)                                                                          \
  {                                                                                                            \
    bool ref_mask[PacketSize] = {};                                                                            \
    bool data_mask[PacketSize] = {};                                                                           \
    internal::pstore(data2, POP(internal::pload<Packet>(data1), internal::pload<Packet>(data1 + PacketSize))); \
    for (int i = 0; i < PacketSize; ++i) {                                                                     \
      ref_mask[i] = numext::is_exactly_zero(REFOP(data1[i], data1[i + PacketSize]));                           \
      data_mask[i] = numext::is_exactly_zero(data2[i]);                                                        \
    }                                                                                                          \
    VERIFY(test::areEqual(ref_mask, data_mask, PacketSize) && #POP);                                           \
  }

// Checks component-wise for input of size N. All of data1, data2, and ref
// should have size at least ceil(N/PacketSize)*PacketSize to avoid memory
// access errors.
#define CHECK_CWISE1_N(REFOP, POP, N)                                                                             \
  {                                                                                                               \
    for (int i = 0; i < N; ++i) ref[i] = REFOP(data1[i]);                                                         \
    for (int j = 0; j < N; j += PacketSize) internal::pstore(data2 + j, POP(internal::pload<Packet>(data1 + j))); \
    VERIFY(test::areApprox(ref, data2, N) && #POP);                                                               \
  }

// Checks component-wise for input of complex type of size N. The real and
// the imaginary part are compared separately, with 1ULP relaxed condition
// for the imaginary part. All of data1 data2, ref, realdata1 and realref
// should have size at least ceil(N/PacketSize)*PacketSize to avoid
// memory access errors.
#define CHECK_CWISE1_IM1ULP_N(REFOP, POP, N)                                            \
  {                                                                                     \
    RealScalar eps_1ulp = RealScalar(1e1) * std::numeric_limits<RealScalar>::epsilon(); \
    for (int j = 0; j < N; j += PacketSize)                                             \
      internal::pstore(data2 + j, internal::plog(internal::pload<Packet>(data1 + j)));  \
    for (int i = 0; i < N; ++i) {                                                       \
      ref[i] = REFOP(data1[i]);                                                         \
      realref[i] = ref[i].imag();                                                       \
      realdata[i] = data2[i].imag();                                                    \
    }                                                                                   \
    VERIFY(test::areApprox(realdata, realref, N, eps_1ulp));                            \
    for (int i = 0; i < N; ++i) {                                                       \
      realdata[i] = data2[i].real();                                                    \
      realref[i] = ref[i].real();                                                       \
    }                                                                                   \
    VERIFY(test::areApprox(realdata, realref, N));                                      \
  }

template <bool Cond, typename Packet>
struct packet_helper {
  template <typename T>
  inline Packet load(const T* from) const {
    return internal::pload<Packet>(from);
  }

  template <typename T>
  inline Packet loadu(const T* from) const {
    return internal::ploadu<Packet>(from);
  }

  template <typename T>
  inline Packet load(const T* from, unsigned long long umask) const {
    using UMaskType = typename numext::get_integer_by_size<internal::plain_enum_max(
        internal::unpacket_traits<Packet>::size / CHAR_BIT, 1)>::unsigned_type;
    return internal::ploadu<Packet>(from, static_cast<UMaskType>(umask));
  }

  template <typename T>
  inline void store(T* to, const Packet& x) const {
    internal::pstore(to, x);
  }

  template <typename T>
  inline void store(T* to, const Packet& x, unsigned long long umask) const {
    using UMaskType = typename numext::get_integer_by_size<internal::plain_enum_max(
        internal::unpacket_traits<Packet>::size / CHAR_BIT, 1)>::unsigned_type;
    internal::pstoreu(to, x, static_cast<UMaskType>(umask));
  }

  template <typename T>
  inline Packet& forward_reference(Packet& packet, T& /*scalar*/) const {
    return packet;
  }
};

template <typename Packet>
struct packet_helper<false, Packet> {
  template <typename T>
  inline T load(const T* from) const {
    return *from;
  }

  template <typename T>
  inline T loadu(const T* from) const {
    return *from;
  }

  template <typename T>
  inline T load(const T* from, unsigned long long) const {
    return *from;
  }

  template <typename T>
  inline void store(T* to, const T& x) const {
    *to = x;
  }

  template <typename T>
  inline void store(T* to, const T& x, unsigned long long) const {
    *to = x;
  }

  template <typename T>
  inline T& forward_reference(Packet& /*packet*/, T& scalar) const {
    return scalar;
  }
};

#define CHECK_CWISE1_IF(COND, REFOP, POP)                                  \
  if (COND) {                                                              \
    test::packet_helper<COND, Packet> h;                                   \
    for (int i = 0; i < PacketSize; ++i) ref[i] = Scalar(REFOP(data1[i])); \
    h.store(data2, POP(h.load(data1)));                                    \
    VERIFY(test::areApprox(ref, data2, PacketSize) && #POP);               \
  }

#define CHECK_CWISE1_EXACT_IF(COND, REFOP, POP)                            \
  if (COND) {                                                              \
    test::packet_helper<COND, Packet> h;                                   \
    for (int i = 0; i < PacketSize; ++i) ref[i] = Scalar(REFOP(data1[i])); \
    h.store(data2, POP(h.load(data1)));                                    \
    VERIFY(test::areEqual(ref, data2, PacketSize) && #POP);                \
  }

#define CHECK_CWISE2_IF(COND, REFOP, POP)                                                         \
  if (COND) {                                                                                     \
    test::packet_helper<COND, Packet> h;                                                          \
    for (int i = 0; i < PacketSize; ++i) ref[i] = Scalar(REFOP(data1[i], data1[i + PacketSize])); \
    h.store(data2, POP(h.load(data1), h.load(data1 + PacketSize)));                               \
    VERIFY(test::areApprox(ref, data2, PacketSize) && #POP);                                      \
  }

// One input, one output by reference.
#define CHECK_CWISE1_BYREF1_IF(COND, REFOP, POP)                                                \
  if (COND) {                                                                                   \
    test::packet_helper<COND, Packet> h;                                                        \
    for (int i = 0; i < PacketSize; ++i) ref[i] = Scalar(REFOP(data1[i], ref[i + PacketSize])); \
    Packet pout;                                                                                \
    Scalar sout;                                                                                \
    h.store(data2, POP(h.load(data1), h.forward_reference(pout, sout)));                        \
    h.store(data2 + PacketSize, h.forward_reference(pout, sout));                               \
    VERIFY(test::areApprox(ref, data2, 2 * PacketSize) && #POP);                                \
  }

#define CHECK_CWISE3_IF(COND, REFOP, POP)                                                           \
  if (COND) {                                                                                       \
    test::packet_helper<COND, Packet> h;                                                            \
    for (int i = 0; i < PacketSize; ++i)                                                            \
      ref[i] = Scalar(REFOP(data1[i], data1[i + PacketSize], data1[i + 2 * PacketSize]));           \
    h.store(data2, POP(h.load(data1), h.load(data1 + PacketSize), h.load(data1 + 2 * PacketSize))); \
    VERIFY(test::areApprox(ref, data2, PacketSize) && #POP);                                        \
  }

// Specialize the runall struct in your test file by defining run().
template <typename Scalar, typename PacketType, bool IsComplex = NumTraits<Scalar>::IsComplex,
          bool IsInteger = NumTraits<Scalar>::IsInteger>
struct runall;

template <typename Scalar, typename PacketType = typename internal::packet_traits<Scalar>::type,
          bool Vectorized = internal::packet_traits<Scalar>::Vectorizable,
          bool HasHalf = !internal::is_same<typename internal::unpacket_traits<PacketType>::half, PacketType>::value>
struct runner;

template <typename Scalar, typename PacketType>
struct runner<Scalar, PacketType, true, true> {
  static void run() {
    runall<Scalar, PacketType>::run();
    runner<Scalar, typename internal::unpacket_traits<PacketType>::half>::run();
  }
};

template <typename Scalar, typename PacketType>
struct runner<Scalar, PacketType, true, false> {
  static void run() { runall<Scalar, PacketType>::run(); }
};

template <typename Scalar, typename PacketType>
struct runner<Scalar, PacketType, false, false> {
  static void run() { runall<Scalar, PacketType>::run(); }
};

}  // namespace test
}  // namespace Eigen
