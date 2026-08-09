// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "packetmath_test_shared.h"
#include "random_without_cast_overflow.h"

template <typename T>
inline T REF_ADD(const T& a, const T& b) {
  return a + b;
}
template <typename T>
inline T REF_SUB(const T& a, const T& b) {
  return a - b;
}
template <typename T>
inline T REF_MUL(const T& a, const T& b) {
  return a * b;
}

template <typename Scalar, typename EnableIf = void>
struct madd_impl {
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar madd(const Scalar& a, const Scalar& b, const Scalar& c) {
    return a * b + c;
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar msub(const Scalar& a, const Scalar& b, const Scalar& c) {
    return a * b - c;
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar nmadd(const Scalar& a, const Scalar& b, const Scalar& c) {
    return c - a * b;
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar nmsub(const Scalar& a, const Scalar& b, const Scalar& c) {
    return Scalar(0) - (a * b + c);
  }
};

template <typename Scalar>
struct madd_impl<Scalar,
                 std::enable_if_t<Eigen::internal::is_scalar<Scalar>::value && Eigen::NumTraits<Scalar>::IsSigned>> {
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar madd(const Scalar& a, const Scalar& b, const Scalar& c) {
    return numext::madd(a, b, c);
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar msub(const Scalar& a, const Scalar& b, const Scalar& c) {
    return numext::madd(a, b, Scalar(-c));
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar nmadd(const Scalar& a, const Scalar& b, const Scalar& c) {
    return numext::madd(Scalar(-a), b, c);
  }
  static EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE Scalar nmsub(const Scalar& a, const Scalar& b, const Scalar& c) {
    return -Scalar(numext::madd(a, b, c));
  }
};

template <typename T>
inline T REF_MADD(const T& a, const T& b, const T& c) {
  return madd_impl<T>::madd(a, b, c);
}
template <typename T>
inline T REF_MSUB(const T& a, const T& b, const T& c) {
  return madd_impl<T>::msub(a, b, c);
}
template <typename T>
inline T REF_NMADD(const T& a, const T& b, const T& c) {
  return madd_impl<T>::nmadd(a, b, c);
}
template <typename T>
inline T REF_NMSUB(const T& a, const T& b, const T& c) {
  return madd_impl<T>::nmsub(a, b, c);
}
template <typename T>
inline T REF_DIV(const T& a, const T& b) {
  return a / b;
}
template <typename T>
inline T REF_RECIPROCAL(const T& a) {
  return T(1) / a;
}
template <typename T>
inline T REF_ABS_DIFF(const T& a, const T& b) {
  return a > b ? a - b : b - a;
}

// Specializations for bool.
template <>
inline bool REF_ADD(const bool& a, const bool& b) {
  return a || b;
}
template <>
inline bool REF_SUB(const bool& a, const bool& b) {
  return a ^ b;
}
template <>
inline bool REF_MUL(const bool& a, const bool& b) {
  return a && b;
}
template <>
inline bool REF_MADD(const bool& a, const bool& b, const bool& c) {
  return (a && b) || c;
}
template <>
inline bool REF_DIV(const bool& a, const bool& b) {
  return a && b;
}
template <>
inline bool REF_RECIPROCAL(const bool& a) {
  return a;
}

template <typename T>
inline T REF_FREXP(const T& x, T& exp) {
  int iexp = 0;
  EIGEN_USING_STD(frexp)
  const T out = static_cast<T>(frexp(x, &iexp));
  exp = static_cast<T>(iexp);

  // The exponent value is unspecified if the input is inf or NaN, but MSVC
  // seems to set it to 1.  We need to set it back to zero for consistency.
  if (!(numext::isfinite)(x)) {
    exp = T(0);
  }
  return out;
}

template <typename T>
inline T REF_LDEXP(const T& x, const T& exp) {
  EIGEN_USING_STD(ldexp)
  return static_cast<T>(ldexp(x, static_cast<int>(exp)));
}

// provides a convenient function to take the absolute value of each component of a complex number to prevent
// catastrophic cancellation in randomly generated complex numbers
template <typename T, bool IsComplex = NumTraits<T>::IsComplex>
struct abs_helper_impl {
  static T run(T x) { return numext::abs(x); }
};
template <typename T>
struct abs_helper_impl<T, true> {
  static T run(T x) {
    T res = x;
    numext::real_ref(res) = numext::abs(numext::real(res));
    numext::imag_ref(res) = numext::abs(numext::imag(res));
    return res;
  }
};
template <typename T>
T abs_helper(T x) {
  return abs_helper_impl<T>::run(x);
}

// Uses pcast to cast from one array to another.
template <typename SrcPacket, typename TgtPacket, int SrcCoeffRatio, int TgtCoeffRatio>
struct pcast_array;

template <typename SrcPacket, typename TgtPacket, int TgtCoeffRatio>
struct pcast_array<SrcPacket, TgtPacket, 1, TgtCoeffRatio> {
  typedef typename internal::unpacket_traits<SrcPacket>::type SrcScalar;
  typedef typename internal::unpacket_traits<TgtPacket>::type TgtScalar;
  static void cast(const SrcScalar* src, size_t size, TgtScalar* dst) {
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    size_t i;
    for (i = 0; i < size && i + SrcPacketSize <= size; i += TgtPacketSize) {
      internal::pstoreu(dst + i, internal::pcast<SrcPacket, TgtPacket>(internal::ploadu<SrcPacket>(src + i)));
    }
    // Leftovers that cannot be loaded into a packet.
    for (; i < size; ++i) {
      dst[i] = static_cast<TgtScalar>(src[i]);
    }
  }
};

template <typename SrcPacket, typename TgtPacket>
struct pcast_array<SrcPacket, TgtPacket, 2, 1> {
  static void cast(const typename internal::unpacket_traits<SrcPacket>::type* src, size_t size,
                   typename internal::unpacket_traits<TgtPacket>::type* dst) {
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    for (size_t i = 0; i < size; i += TgtPacketSize) {
      SrcPacket a = internal::ploadu<SrcPacket>(src + i);
      SrcPacket b = internal::ploadu<SrcPacket>(src + i + SrcPacketSize);
      internal::pstoreu(dst + i, internal::pcast<SrcPacket, TgtPacket>(a, b));
    }
  }
};

template <typename SrcPacket, typename TgtPacket>
struct pcast_array<SrcPacket, TgtPacket, 4, 1> {
  static void cast(const typename internal::unpacket_traits<SrcPacket>::type* src, size_t size,
                   typename internal::unpacket_traits<TgtPacket>::type* dst) {
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    for (size_t i = 0; i < size; i += TgtPacketSize) {
      SrcPacket a = internal::ploadu<SrcPacket>(src + i);
      SrcPacket b = internal::ploadu<SrcPacket>(src + i + SrcPacketSize);
      SrcPacket c = internal::ploadu<SrcPacket>(src + i + 2 * SrcPacketSize);
      SrcPacket d = internal::ploadu<SrcPacket>(src + i + 3 * SrcPacketSize);
      internal::pstoreu(dst + i, internal::pcast<SrcPacket, TgtPacket>(a, b, c, d));
    }
  }
};

template <typename SrcPacket, typename TgtPacket>
struct pcast_array<SrcPacket, TgtPacket, 8, 1> {
  static void cast(const typename internal::unpacket_traits<SrcPacket>::type* src, size_t size,
                   typename internal::unpacket_traits<TgtPacket>::type* dst) {
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    for (size_t i = 0; i < size; i += TgtPacketSize) {
      SrcPacket a = internal::ploadu<SrcPacket>(src + i);
      SrcPacket b = internal::ploadu<SrcPacket>(src + i + SrcPacketSize);
      SrcPacket c = internal::ploadu<SrcPacket>(src + i + 2 * SrcPacketSize);
      SrcPacket d = internal::ploadu<SrcPacket>(src + i + 3 * SrcPacketSize);
      SrcPacket e = internal::ploadu<SrcPacket>(src + i + 4 * SrcPacketSize);
      SrcPacket f = internal::ploadu<SrcPacket>(src + i + 5 * SrcPacketSize);
      SrcPacket g = internal::ploadu<SrcPacket>(src + i + 6 * SrcPacketSize);
      SrcPacket h = internal::ploadu<SrcPacket>(src + i + 7 * SrcPacketSize);
      internal::pstoreu(dst + i, internal::pcast<SrcPacket, TgtPacket>(a, b, c, d, e, f, g, h));
    }
  }
};

template <typename SrcPacket, typename TgtPacket, int SrcCoeffRatio, int TgtCoeffRatio, bool CanCast = false>
struct test_cast_helper;

template <typename SrcPacket, typename TgtPacket, int SrcCoeffRatio, int TgtCoeffRatio>
struct test_cast_helper<SrcPacket, TgtPacket, SrcCoeffRatio, TgtCoeffRatio, false> {
  static void run() {}
};

template <typename SrcPacket, typename TgtPacket, int SrcCoeffRatio, int TgtCoeffRatio>
struct test_cast_helper<SrcPacket, TgtPacket, SrcCoeffRatio, TgtCoeffRatio, true> {
  static void run() {
    typedef typename internal::unpacket_traits<SrcPacket>::type SrcScalar;
    typedef typename internal::unpacket_traits<TgtPacket>::type TgtScalar;
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    static const int BlockSize = SrcPacketSize * SrcCoeffRatio;
    eigen_assert(BlockSize == TgtPacketSize * TgtCoeffRatio && "Packet sizes and cast ratios are mismatched.");

    static const int DataSize = 10 * BlockSize;
    EIGEN_ALIGN_MAX SrcScalar data1[DataSize];
    EIGEN_ALIGN_MAX TgtScalar data2[DataSize];
    EIGEN_ALIGN_MAX TgtScalar ref[DataSize];

    // Construct a packet of scalars that will not overflow when casting
    for (int i = 0; i < DataSize; ++i) {
      data1[i] = internal::random_without_cast_overflow<SrcScalar, TgtScalar>::value();
    }

    for (int i = 0; i < DataSize; ++i) {
      ref[i] = static_cast<const TgtScalar>(data1[i]);
    }

    pcast_array<SrcPacket, TgtPacket, SrcCoeffRatio, TgtCoeffRatio>::cast(data1, DataSize, data2);

    VERIFY(test::areApprox(ref, data2, DataSize) && "internal::pcast<>");

    // Test that pcast<SrcScalar, TgtScalar> generates the same result.
    for (int i = 0; i < DataSize; ++i) {
      data2[i] = internal::pcast<SrcScalar, TgtScalar>(data1[i]);
    }
    VERIFY(test::areApprox(ref, data2, DataSize) && "internal::pcast<>");
  }
};

template <typename SrcPacket, typename TgtPacket>
struct test_cast {
  static void run() {
    typedef typename internal::unpacket_traits<SrcPacket>::type SrcScalar;
    typedef typename internal::unpacket_traits<TgtPacket>::type TgtScalar;
    typedef typename internal::type_casting_traits<SrcScalar, TgtScalar> TypeCastingTraits;
    static const int SrcCoeffRatio = TypeCastingTraits::SrcCoeffRatio;
    static const int TgtCoeffRatio = TypeCastingTraits::TgtCoeffRatio;
    static const int SrcPacketSize = internal::unpacket_traits<SrcPacket>::size;
    static const int TgtPacketSize = internal::unpacket_traits<TgtPacket>::size;
    static const bool HasCast =
        internal::unpacket_traits<SrcPacket>::vectorizable && internal::unpacket_traits<TgtPacket>::vectorizable &&
        TypeCastingTraits::VectorizedCast && (SrcPacketSize * SrcCoeffRatio == TgtPacketSize * TgtCoeffRatio);
    test_cast_helper<SrcPacket, TgtPacket, SrcCoeffRatio, TgtCoeffRatio, HasCast>::run();
  }
};

template <typename SrcPacket, typename TgtScalar,
          typename TgtPacket = typename internal::packet_traits<TgtScalar>::type,
          bool Vectorized = internal::packet_traits<TgtScalar>::Vectorizable,
          bool HasHalf = !internal::is_same<typename internal::unpacket_traits<TgtPacket>::half, TgtPacket>::value>
struct test_cast_runner;

template <typename SrcPacket, typename TgtScalar, typename TgtPacket>
struct test_cast_runner<SrcPacket, TgtScalar, TgtPacket, true, false> {
  static void run() { test_cast<SrcPacket, TgtPacket>::run(); }
};

template <typename SrcPacket, typename TgtScalar, typename TgtPacket>
struct test_cast_runner<SrcPacket, TgtScalar, TgtPacket, true, true> {
  static void run() {
    test_cast<SrcPacket, TgtPacket>::run();
    test_cast_runner<SrcPacket, TgtScalar, typename internal::unpacket_traits<TgtPacket>::half>::run();
  }
};

template <typename SrcPacket, typename TgtScalar, typename TgtPacket>
struct test_cast_runner<SrcPacket, TgtScalar, TgtPacket, false, false> {
  static void run() {}
};

template <typename Scalar, typename Packet, typename EnableIf = void>
struct packetmath_pcast_ops_runner {
  static void run() {
    test_cast_runner<Packet, float>::run();
    test_cast_runner<Packet, double>::run();
    test_cast_runner<Packet, int8_t>::run();
    test_cast_runner<Packet, uint8_t>::run();
    test_cast_runner<Packet, int16_t>::run();
    test_cast_runner<Packet, uint16_t>::run();
    test_cast_runner<Packet, int32_t>::run();
    test_cast_runner<Packet, uint32_t>::run();
    test_cast_runner<Packet, int64_t>::run();
    test_cast_runner<Packet, uint64_t>::run();
    test_cast_runner<Packet, bool>::run();
    test_cast_runner<Packet, std::complex<float>>::run();
    test_cast_runner<Packet, std::complex<double>>::run();
    test_cast_runner<Packet, half>::run();
    test_cast_runner<Packet, bfloat16>::run();
  }
};

// Only some types support cast from std::complex<>.
template <typename Scalar, typename Packet>
struct packetmath_pcast_ops_runner<Scalar, Packet, std::enable_if_t<NumTraits<Scalar>::IsComplex>> {
  static void run() {
    test_cast_runner<Packet, std::complex<float>>::run();
    test_cast_runner<Packet, std::complex<double>>::run();
    test_cast_runner<Packet, half>::run();
    test_cast_runner<Packet, bfloat16>::run();
  }
};

template <typename Scalar, typename Packet>
void packetmath_boolean_mask_ops() {
  using RealScalar = typename NumTraits<Scalar>::Real;
  const int PacketSize = internal::unpacket_traits<Packet>::size;
  const int size = 2 * PacketSize;
  EIGEN_ALIGN_MAX Scalar data1[size];
  EIGEN_ALIGN_MAX Scalar data2[size];
  EIGEN_ALIGN_MAX Scalar ref[size];

  for (int i = 0; i < size; ++i) {
    data1[i] = internal::random<Scalar>();
  }
  CHECK_CWISE1_MASK(internal::ptrue, internal::ptrue);
  CHECK_CWISE2_IF(true, internal::pandnot, internal::pandnot);
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = Scalar(RealScalar(i));
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }

  CHECK_CWISE2_MASK(internal::pcmp_eq, internal::pcmp_eq);

  // Test (-0) == (0) for signed operations
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = Scalar(-0.0);
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }
  CHECK_CWISE2_MASK(internal::pcmp_eq, internal::pcmp_eq);

  // Test NaN
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = NumTraits<Scalar>::quiet_NaN();
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }
  CHECK_CWISE2_MASK(internal::pcmp_eq, internal::pcmp_eq);
}

template <typename Scalar, typename Packet>
void packetmath_boolean_mask_ops_real() {
  const int PacketSize = internal::unpacket_traits<Packet>::size;
  const int size = 2 * PacketSize;
  EIGEN_ALIGN_MAX Scalar data1[size];
  EIGEN_ALIGN_MAX Scalar data2[size];

  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = internal::random<Scalar>();
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }

  CHECK_CWISE2_MASK(internal::pcmp_lt_or_nan, internal::pcmp_lt_or_nan);

  // Test (-0) <=/< (0) for signed operations
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = Scalar(-0.0);
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }
  CHECK_CWISE2_MASK(internal::pcmp_lt_or_nan, internal::pcmp_lt_or_nan);

  // Test NaN
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = NumTraits<Scalar>::quiet_NaN();
    data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
  }
  CHECK_CWISE2_MASK(internal::pcmp_lt_or_nan, internal::pcmp_lt_or_nan);
}

template <typename Scalar, typename Packet, typename EnableIf = void>
struct packetmath_boolean_mask_ops_notcomplex_test {
  static void run() {}
};

template <typename Scalar, typename Packet>
struct packetmath_boolean_mask_ops_notcomplex_test<
    Scalar, Packet,
    std::enable_if_t<internal::packet_traits<Scalar>::HasCmp && !internal::is_same<Scalar, bool>::value>> {
  static void run() {
    const int PacketSize = internal::unpacket_traits<Packet>::size;
    const int size = 2 * PacketSize;
    EIGEN_ALIGN_MAX Scalar data1[size];
    EIGEN_ALIGN_MAX Scalar data2[size];

    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = internal::random<Scalar>();
      data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
    }

    CHECK_CWISE2_MASK(internal::pcmp_le, internal::pcmp_le);
    CHECK_CWISE2_MASK(internal::pcmp_lt, internal::pcmp_lt);

    // Test (-0) <=/< (0) for signed operations
    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = Scalar(-0.0);
      data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
    }
    CHECK_CWISE2_MASK(internal::pcmp_le, internal::pcmp_le);
    CHECK_CWISE2_MASK(internal::pcmp_lt, internal::pcmp_lt);

    // Test NaN
    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = NumTraits<Scalar>::quiet_NaN();
      data1[i + PacketSize] = internal::random<bool>() ? data1[i] : Scalar(0);
    }
    CHECK_CWISE2_MASK(internal::pcmp_le, internal::pcmp_le);
    CHECK_CWISE2_MASK(internal::pcmp_lt, internal::pcmp_lt);
  }
};

template <typename Scalar, typename Packet, typename EnableIf = void>
struct packetmath_minus_zero_add_test {
  static void run() {}
};

template <typename Scalar, typename Packet>
struct packetmath_minus_zero_add_test<Scalar, Packet, std::enable_if_t<!NumTraits<Scalar>::IsInteger>> {
  static void run() {
    const int PacketSize = internal::unpacket_traits<Packet>::size;
    const int size = 2 * PacketSize;
    EIGEN_ALIGN_MAX Scalar data1[size] = {};
    EIGEN_ALIGN_MAX Scalar data2[size] = {};
    EIGEN_ALIGN_MAX Scalar ref[size] = {};

    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = Scalar(-0.0);
      data1[i + PacketSize] = Scalar(-0.0);
    }
    CHECK_CWISE2_IF(internal::packet_traits<Scalar>::HasAdd, REF_ADD, internal::padd);
  }
};

// Ensure optimization barrier compiles and doesn't modify contents.
// Only applies to raw types, so will not work for std::complex, Eigen::half
// or Eigen::bfloat16. For those you would need to refer to an underlying
// storage element.
template <typename Packet, typename EnableIf = void>
struct eigen_optimization_barrier_test {
  static void run() {}
};

template <typename Packet>
struct eigen_optimization_barrier_test<
    Packet, std::enable_if_t<!NumTraits<Packet>::IsComplex && !internal::is_same<Packet, Eigen::half>::value &&
                             !internal::is_same<Packet, Eigen::bfloat16>::value>> {
  static void run() {
    typedef typename internal::unpacket_traits<Packet>::type Scalar;
    Scalar s = internal::random<Scalar>();
    Packet barrier = internal::pset1<Packet>(s);
    EIGEN_OPTIMIZATION_BARRIER(barrier);
    eigen_assert(s == internal::pfirst(barrier) && "EIGEN_OPTIMIZATION_BARRIER");
  }
};

template <typename Scalar, typename Packet, bool HasNegate = internal::packet_traits<Scalar>::HasNegate>
struct negate_test_impl {
  static void run_negate(Scalar* data1, Scalar* data2, Scalar* ref, int PacketSize) {
    CHECK_CWISE1_IF(HasNegate, test::negate, internal::pnegate);
  }
  static void run_nmsub(Scalar* data1, Scalar* data2, Scalar* ref, int PacketSize) {
    CHECK_CWISE3_IF(HasNegate, REF_NMSUB, internal::pnmsub);
  }
};

template <typename Scalar, typename Packet>
struct negate_test_impl<Scalar, Packet, false> {
  static void run_negate(Scalar*, Scalar*, Scalar*, int) {}
  static void run_nmsub(Scalar*, Scalar*, Scalar*, int) {}
};

template <typename Scalar, typename Packet>
void negate_test(Scalar* data1, Scalar* data2, Scalar* ref, int size) {
  negate_test_impl<Scalar, Packet>::run_negate(data1, data2, ref, size);
}

template <typename Scalar, typename Packet>
void nmsub_test(Scalar* data1, Scalar* data2, Scalar* ref, int size) {
  negate_test_impl<Scalar, Packet>::run_nmsub(data1, data2, ref, size);
}

template <typename Scalar, typename Packet>
void packetmath() {
  typedef internal::packet_traits<Scalar> PacketTraits;
  const int PacketSize = internal::unpacket_traits<Packet>::size;
  typedef typename NumTraits<Scalar>::Real RealScalar;

  if (g_first_pass)
    std::cerr << "=== Testing packet of type '" << typeid(Packet).name() << "' and scalar type '"
              << typeid(Scalar).name() << "' and size '" << PacketSize << "' ===\n";

  const int max_size = PacketSize > 4 ? PacketSize : 4;
  const int size = PacketSize * max_size;
  EIGEN_ALIGN_MAX Scalar data1[size];
  EIGEN_ALIGN_MAX Scalar data2[size];
  EIGEN_ALIGN_MAX Scalar data3[size];
  EIGEN_ALIGN_MAX Scalar ref[size];
  RealScalar refvalue = RealScalar(0);

  eigen_optimization_barrier_test<Packet>::run();
  eigen_optimization_barrier_test<Scalar>::run();

  for (int i = 0; i < size; ++i) {
    data1[i] = internal::random<Scalar>();
    data2[i] = internal::random<Scalar>();
    refvalue = (std::max)(refvalue, numext::abs(data1[i]));
  }

  internal::pstore(data2, internal::pload<Packet>(data1));
  VERIFY(test::areApprox(data1, data2, PacketSize) && "aligned load/store");

  for (int offset = 0; offset < PacketSize; ++offset) {
    internal::pstore(data2, internal::ploadu<Packet>(data1 + offset));
    VERIFY(test::areApprox(data1 + offset, data2, PacketSize) && "internal::ploadu");
  }

  for (int offset = 0; offset < PacketSize; ++offset) {
    internal::pstoreu(data2 + offset, internal::pload<Packet>(data1));
    VERIFY(test::areApprox(data1, data2 + offset, PacketSize) && "internal::pstoreu");
  }

  for (int M = 0; M < PacketSize; ++M) {
    for (int N = 0; N <= PacketSize; ++N) {
      for (int j = 0; j < size; ++j) {
        data1[j] = internal::random<Scalar>();
        data2[j] = internal::random<Scalar>();
        refvalue = (std::max)(refvalue, numext::abs(data1[j]));
      }

      if (M == 0) {
        internal::pstore_partial(data2, internal::pload_partial<Packet>(data1, N), N);
        VERIFY(test::areApprox(data1, data2, N) && "aligned loadN/storeN");

        for (int offset = 0; offset < PacketSize; ++offset) {
          internal::pstore_partial(data2, internal::ploadu_partial<Packet>(data1 + offset, N), N);
          VERIFY(test::areApprox(data1 + offset, data2, N) && "internal::ploadu_partial");
        }

        for (int offset = 0; offset < PacketSize; ++offset) {
          internal::pstoreu_partial(data2 + offset, internal::pload_partial<Packet>(data1, N), N);
          VERIFY(test::areApprox(data1, data2 + offset, N) && "internal::pstoreu_partial");
        }
      }

      if (N + M > PacketSize) continue;  // Don't read or write past end of Packet

      internal::pstore_partial(data2, internal::pload_partial<Packet>(data1, N, M), N, M);
      VERIFY(test::areApprox(data1, data2, N) && "aligned offset loadN/storeN");
    }
  }

  if (internal::unpacket_traits<Packet>::masked_load_available) {
    test::packet_helper<internal::unpacket_traits<Packet>::masked_load_available, Packet> h;
    unsigned long long max_umask = (0x1ull << PacketSize);

    for (int offset = 0; offset < PacketSize; ++offset) {
      for (unsigned long long umask = 0; umask < max_umask; ++umask) {
        h.store(data2, h.load(data1 + offset, umask));
        for (int k = 0; k < PacketSize; ++k) data3[k] = ((umask & (0x1ull << k)) >> k) ? data1[k + offset] : Scalar(0);
        VERIFY(test::areApprox(data3, data2, PacketSize) && "internal::ploadu masked");
      }
    }
  }

  if (internal::unpacket_traits<Packet>::masked_store_available) {
    test::packet_helper<internal::unpacket_traits<Packet>::masked_store_available, Packet> h;
    unsigned long long max_umask = (0x1ull << PacketSize);

    for (int offset = 0; offset < PacketSize; ++offset) {
      for (unsigned long long umask = 0; umask < max_umask; ++umask) {
        internal::pstore(data2, internal::pset1<Packet>(Scalar(0)));
        h.store(data2, h.loadu(data1 + offset), umask);
        for (int k = 0; k < PacketSize; ++k) data3[k] = ((umask & (0x1ull << k)) >> k) ? data1[k + offset] : Scalar(0);
        VERIFY(test::areApprox(data3, data2, PacketSize) && "internal::pstoreu masked");
      }
    }
  }

  VERIFY((!PacketTraits::Vectorizable) || PacketTraits::HasAdd);
  VERIFY((!PacketTraits::Vectorizable) || PacketTraits::HasSub);
  VERIFY((!PacketTraits::Vectorizable) || PacketTraits::HasMul);

  CHECK_CWISE2_IF(PacketTraits::HasAdd, REF_ADD, internal::padd);
  CHECK_CWISE2_IF(PacketTraits::HasSub, REF_SUB, internal::psub);
  CHECK_CWISE2_IF(PacketTraits::HasMul, REF_MUL, internal::pmul);
  CHECK_CWISE2_IF(PacketTraits::HasDiv, REF_DIV, internal::pdiv);

  negate_test<Scalar, Packet>(data1, data2, ref, PacketSize);
  CHECK_CWISE1_IF(PacketTraits::HasReciprocal, REF_RECIPROCAL, internal::preciprocal);
  CHECK_CWISE1(numext::conj, internal::pconj);

  CHECK_CWISE1_IF(PacketTraits::HasSign, numext::sign, internal::psign);

  for (int offset = 0; offset < 3; ++offset) {
    for (int i = 0; i < PacketSize; ++i) ref[i] = data1[offset];
    internal::pstore(data2, internal::pset1<Packet>(data1[offset]));
    VERIFY(test::areApprox(ref, data2, PacketSize) && "internal::pset1");
  }

  {
    for (int i = 0; i < PacketSize * 4; ++i) ref[i] = data1[i / PacketSize];
    Packet A0, A1, A2, A3;
    internal::pbroadcast4<Packet>(data1, A0, A1, A2, A3);
    internal::pstore(data2 + 0 * PacketSize, A0);
    internal::pstore(data2 + 1 * PacketSize, A1);
    internal::pstore(data2 + 2 * PacketSize, A2);
    internal::pstore(data2 + 3 * PacketSize, A3);
    VERIFY(test::areApprox(ref, data2, 4 * PacketSize) && "internal::pbroadcast4");
  }

  {
    for (int i = 0; i < PacketSize * 2; ++i) ref[i] = data1[i / PacketSize];
    Packet A0, A1;
    internal::pbroadcast2<Packet>(data1, A0, A1);
    internal::pstore(data2 + 0 * PacketSize, A0);
    internal::pstore(data2 + 1 * PacketSize, A1);
    VERIFY(test::areApprox(ref, data2, 2 * PacketSize) && "internal::pbroadcast2");
  }

  VERIFY(internal::isApprox(data1[0], internal::pfirst(internal::pload<Packet>(data1))) && "internal::pfirst");

  if (PacketSize > 1) {
    // apply different offsets to check that ploaddup is robust to unaligned inputs
    for (int offset = 0; offset < 4; ++offset) {
      for (int i = 0; i < PacketSize / 2; ++i) ref[2 * i + 0] = ref[2 * i + 1] = data1[offset + i];
      internal::pstore(data2, internal::ploaddup<Packet>(data1 + offset));
      VERIFY(test::areApprox(ref, data2, PacketSize) && "ploaddup");
    }
  }

  if (PacketSize > 2) {
    // apply different offsets to check that ploadquad is robust to unaligned inputs
    for (int offset = 0; offset < 4; ++offset) {
      for (int i = 0; i < PacketSize / 4; ++i)
        ref[4 * i + 0] = ref[4 * i + 1] = ref[4 * i + 2] = ref[4 * i + 3] = data1[offset + i];
      internal::pstore(data2, internal::ploadquad<Packet>(data1 + offset));
      VERIFY(test::areApprox(ref, data2, PacketSize) && "ploadquad");
    }
  }

  ref[0] = Scalar(0);
  for (int i = 0; i < PacketSize; ++i) ref[0] += data1[i];
  VERIFY(test::isApproxAbs(ref[0], internal::predux(internal::pload<Packet>(data1)), refvalue) && "internal::predux");

  if (!internal::is_same<Packet, typename internal::unpacket_traits<Packet>::half>::value) {
    int HalfPacketSize = PacketSize > 4 ? PacketSize / 2 : PacketSize;
    for (int i = 0; i < HalfPacketSize; ++i) ref[i] = Scalar(0);
    for (int i = 0; i < PacketSize; ++i) ref[i % HalfPacketSize] += data1[i];
    internal::pstore(data2, internal::predux_half_dowto4(internal::pload<Packet>(data1)));
    VERIFY(test::areApprox(ref, data2, HalfPacketSize) && "internal::predux_half_dowto4");
  }

  // Avoid overflows.
  if (NumTraits<Scalar>::IsInteger && NumTraits<Scalar>::IsSigned &&
      Eigen::internal::unpacket_traits<Packet>::size > 1) {
    Scalar limit = static_cast<Scalar>(
        static_cast<RealScalar>(std::pow(static_cast<double>(numext::real(NumTraits<Scalar>::highest())),
                                         1.0 / static_cast<double>(Eigen::internal::unpacket_traits<Packet>::size))));
    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = internal::random<Scalar>(Scalar(0) - limit, limit);
    }
  } else if (!NumTraits<Scalar>::IsInteger && !NumTraits<Scalar>::IsComplex && !std::is_same<Scalar, bool>::value) {
    // Prevent very small product results by adjusting range.  Otherwise,
    // we may end up with multiplying e.g. 32 Eigen::halfs with values < 1.
    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = REF_MUL(internal::random<Scalar>(Scalar(0.5), Scalar(1)),
                         (internal::random<bool>() ? Scalar(-1) : Scalar(1)));
    }
  }
  ref[0] = Scalar(1);
  for (int i = 0; i < PacketSize; ++i) ref[0] = REF_MUL(ref[0], data1[i]);
  VERIFY(internal::isApprox(ref[0], internal::predux_mul(internal::pload<Packet>(data1))) && "internal::predux_mul");

  for (int i = 0; i < PacketSize; ++i) ref[i] = data1[PacketSize - i - 1];
  internal::pstore(data2, internal::preverse(internal::pload<Packet>(data1)));
  VERIFY(test::areApprox(ref, data2, PacketSize) && "internal::preverse");

  internal::PacketBlock<Packet> kernel;
  for (int i = 0; i < PacketSize; ++i) {
    kernel.packet[i] = internal::pload<Packet>(data1 + i * PacketSize);
  }
  ptranspose(kernel);
  for (int i = 0; i < PacketSize; ++i) {
    internal::pstore(data2, kernel.packet[i]);
    for (int j = 0; j < PacketSize; ++j) {
      VERIFY(test::isApproxAbs(data2[j], data1[i + j * PacketSize], refvalue) && "ptranspose");
    }
  }

  // GeneralBlockPanelKernel also checks PacketBlock<Packet,(PacketSize%4)==0?4:PacketSize>;
  if (PacketSize > 4 && PacketSize % 4 == 0) {
    internal::PacketBlock<Packet, PacketSize % 4 == 0 ? 4 : PacketSize> kernel2;
    for (int i = 0; i < 4; ++i) {
      kernel2.packet[i] = internal::pload<Packet>(data1 + i * PacketSize);
    }
    ptranspose(kernel2);
    int data_counter = 0;
    for (int i = 0; i < PacketSize; ++i) {
      for (int j = 0; j < 4; ++j) {
        data2[data_counter++] = data1[j * PacketSize + i];
      }
    }
    for (int i = 0; i < 4; ++i) {
      internal::pstore(data3, kernel2.packet[i]);
      for (int j = 0; j < PacketSize; ++j) {
        VERIFY(test::isApproxAbs(data3[j], data2[i * PacketSize + j], refvalue) && "ptranspose");
      }
    }
  }

  if (PacketTraits::HasBlend) {
    Packet thenPacket = internal::pload<Packet>(data1);
    Packet elsePacket = internal::pload<Packet>(data2);
    EIGEN_ALIGN_MAX internal::Selector<PacketSize> selector;
    for (int i = 0; i < PacketSize; ++i) {
      selector.select[i] = i;
    }

    Packet blend = internal::pblend(selector, thenPacket, elsePacket);
    EIGEN_ALIGN_MAX Scalar result[size];
    internal::pstore(result, blend);
    for (int i = 0; i < PacketSize; ++i) {
      VERIFY(test::isApproxAbs(result[i], (selector.select[i] ? data1[i] : data2[i]), refvalue));
    }
  }

  {
    for (int i = 0; i < PacketSize; ++i) {
      // "if" mask
      // Note: it's UB to load 0xFF directly into a `bool`.
      uint8_t v =
          internal::random<bool>() ? (std::is_same<Scalar, bool>::value ? static_cast<uint8_t>(true) : 0xff) : 0;
      // Avoid strict aliasing violation by using memset.
      memset(static_cast<void*>(data1 + i), v, sizeof(Scalar));
      // "then" packet
      data1[i + PacketSize] = internal::random<Scalar>();
      // "else" packet
      data1[i + 2 * PacketSize] = internal::random<Scalar>();
    }
    CHECK_CWISE3_IF(true, internal::pselect, internal::pselect);
  }

  for (int i = 0; i < size; ++i) {
    data1[i] = internal::random<Scalar>();
  }
  CHECK_CWISE1(internal::pzero, internal::pzero);
  CHECK_CWISE2_IF(true, internal::por, internal::por);
  CHECK_CWISE2_IF(true, internal::pxor, internal::pxor);
  CHECK_CWISE2_IF(true, internal::pand, internal::pand);

  packetmath_boolean_mask_ops<Scalar, Packet>();
  packetmath_pcast_ops_runner<Scalar, Packet>::run();
  packetmath_minus_zero_add_test<Scalar, Packet>::run();

  CHECK_CWISE3_IF(true, REF_MADD, internal::pmadd);
  if (!std::is_same<Scalar, bool>::value && NumTraits<Scalar>::IsSigned) {
    nmsub_test<Scalar, Packet>(data1, data2, ref, PacketSize);
  }

  // For pmsub, pnmadd, the values can cancel each other to become near zero,
  // which can lead to very flaky tests. Here we ensure the signs are such that
  // they do not cancel.
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = abs_helper(internal::random<Scalar>());
    data1[i + PacketSize] = abs_helper(internal::random<Scalar>());
    data1[i + 2 * PacketSize] = Scalar(0) - abs_helper(internal::random<Scalar>());
  }
  if (!std::is_same<Scalar, bool>::value && NumTraits<Scalar>::IsSigned) {
    CHECK_CWISE3_IF(true, REF_MSUB, internal::pmsub);
    CHECK_CWISE3_IF(true, REF_NMADD, internal::pnmadd);
  }

  CHECK_CWISE1_IF(PacketTraits::HasSqrt, numext::sqrt, internal::psqrt);
  CHECK_CWISE1_IF(PacketTraits::HasRsqrt, numext::rsqrt, internal::prsqrt);
  CHECK_CWISE1_IF(PacketTraits::HasCbrt, numext::cbrt, internal::pcbrt);
}

// Notice that this definition works for complex types as well.
// c++11 has std::log2 for real, but not for complex types.
template <typename Scalar>
Scalar log2(Scalar x) {
  return Scalar(EIGEN_LOG2E) * std::log(x);
}

// Create a functor out of a function so it can be passed (with overloads)
// to another function as an input argument.
#define CREATE_FUNCTOR(Name, Func)     \
  struct Name {                        \
    template <typename T>              \
    T operator()(const T& val) const { \
      return Func(val);                \
    }                                  \
  }

CREATE_FUNCTOR(psqrt_functor, internal::psqrt);
CREATE_FUNCTOR(prsqrt_functor, internal::prsqrt);
CREATE_FUNCTOR(pcbrt_functor, internal::pcbrt);

// TODO(rmlarsen): Run this test for more functions.
template <bool Cond, typename Scalar, typename Packet, typename RefFunctorT, typename FunctorT>
void packetmath_test_IEEE_corner_cases(const RefFunctorT& ref_fun, const FunctorT& fun) {
  const int PacketSize = internal::unpacket_traits<Packet>::size;
  const Scalar norm_min = (std::numeric_limits<Scalar>::min)();
  const Scalar norm_max = (std::numeric_limits<Scalar>::max)();

  constexpr int size = PacketSize * 2;
  EIGEN_ALIGN_MAX Scalar data1[size];
  EIGEN_ALIGN_MAX Scalar data2[size];
  EIGEN_ALIGN_MAX Scalar ref[size];
  for (int i = 0; i < size; ++i) {
    data1[i] = data2[i] = ref[i] = Scalar(0);
  }

  // Test for subnormals.
  if (Cond && std::numeric_limits<Scalar>::has_denorm == std::denorm_present && !EIGEN_ARCH_ARM) {
    for (int scale = 1; scale < 5; ++scale) {
      // When EIGEN_FAST_MATH is 1 we relax the conditions slightly, and allow the function
      // to return the same value for subnormals as the reference would return for zero with
      // the same sign as the input.
#if EIGEN_FAST_MATH
      data1[0] = Scalar(scale) * std::numeric_limits<Scalar>::denorm_min();
      data1[1] = -data1[0];
      test::packet_helper<Cond, Packet> h;
      h.store(data2, fun(h.load(data1)));
      for (int i = 0; i < PacketSize; ++i) {
        const Scalar ref_zero = ref_fun(data1[i] < 0 ? -Scalar(0) : Scalar(0));
        const Scalar ref_val = ref_fun(data1[i]);
        VERIFY(((std::isnan)(data2[i]) && (std::isnan)(ref_val)) || data2[i] == ref_zero ||
               verifyIsApprox(data2[i], ref_val));
      }
#else
      CHECK_CWISE1_IF(Cond, ref_fun, fun);
#endif
    }
  }

  // Test for smallest normalized floats.
  data1[0] = norm_min;
  data1[1] = -data1[0];
  CHECK_CWISE1_IF(Cond, ref_fun, fun);

  // Test for largest floats.
  data1[0] = norm_max;
  data1[1] = -data1[0];
  CHECK_CWISE1_IF(Cond, ref_fun, fun);

  // Test for zeros.
  data1[0] = Scalar(0.0);
  data1[1] = -data1[0];
  CHECK_CWISE1_IF(Cond, ref_fun, fun);

  // Test for infinities.
  data1[0] = NumTraits<Scalar>::infinity();
  data1[1] = -data1[0];
  CHECK_CWISE1_IF(Cond, ref_fun, fun);

  // Test for quiet NaNs.
  data1[0] = std::numeric_limits<Scalar>::quiet_NaN();
  data1[1] = -std::numeric_limits<Scalar>::quiet_NaN();
  CHECK_CWISE1_IF(Cond, ref_fun, fun);
}

template <typename Scalar, typename Packet>
void packetmath_real() {
  typedef internal::packet_traits<Scalar> PacketTraits;
  const int PacketSize = internal::unpacket_traits<Packet>::size;

  const int size = PacketSize * 4;
  EIGEN_ALIGN_MAX Scalar data1[PacketSize * 4] = {};
  EIGEN_ALIGN_MAX Scalar data2[PacketSize * 4] = {};
  EIGEN_ALIGN_MAX Scalar ref[PacketSize * 4] = {};

  // Negate with -0.
  if (PacketTraits::HasNegate) {
    test::packet_helper<PacketTraits::HasNegate, Packet> h;
    data1[0] = Scalar{-0};
    h.store(data2, internal::pnegate(h.load(data1)));
    typedef typename internal::make_unsigned<typename internal::make_integer<Scalar>::type>::type Bits;
    Bits bits = numext::bit_cast<Bits>(data2[0]);
    VERIFY_IS_EQUAL(bits, static_cast<Bits>(Bits(1) << (sizeof(Scalar) * CHAR_BIT - 1)));
  }

  for (int i = 0; i < size; ++i) {
    data1[i] = Scalar(internal::random<double>(0, 1) * std::pow(10., internal::random<double>(-6, 6)));
    data2[i] = Scalar(internal::random<double>(0, 1) * std::pow(10., internal::random<double>(-6, 6)));
  }

  if (internal::random<float>(0, 1) < 0.1f) data1[internal::random<int>(0, PacketSize)] = Scalar(0);

  CHECK_CWISE1_IF(PacketTraits::HasLog, std::log, internal::plog);
  CHECK_CWISE1_IF(PacketTraits::HasLog, log2, internal::plog2);
  CHECK_CWISE1_IF(PacketTraits::HasRsqrt, numext::rsqrt, internal::prsqrt);

  for (int i = 0; i < size; ++i) {
    data1[i] = Scalar(internal::random<double>(-1, 1) * std::pow(10., internal::random<double>(-3, 3)));
    data2[i] = Scalar(internal::random<double>(-1, 1) * std::pow(10., internal::random<double>(-3, 3)));
  }
  CHECK_CWISE1_IF(PacketTraits::HasSin, std::sin, internal::psin);
  CHECK_CWISE1_IF(PacketTraits::HasCos, std::cos, internal::pcos);
  CHECK_CWISE1_IF(PacketTraits::HasTan, std::tan, internal::ptan);

  CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::round, internal::pround);
  CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::ceil, internal::pceil);
  CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::floor, internal::pfloor);
  CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::rint, internal::print);
  CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::trunc, internal::ptrunc);
  CHECK_CWISE1_IF(PacketTraits::HasSign, numext::sign, internal::psign);

  packetmath_boolean_mask_ops_real<Scalar, Packet>();

  // Rounding edge cases.
  if (PacketTraits::HasRound) {
    typedef typename internal::make_integer<Scalar>::type IntType;
    // Start with values that cannot fit inside an integer, work down to less than one.
    Scalar val =
        numext::mini(Scalar(2) * static_cast<Scalar>(NumTraits<IntType>::highest()), NumTraits<Scalar>::highest());
    std::vector<Scalar> values;
    while (val > Scalar(0.25)) {
      // Cover both even and odd, positive and negative cases.
      values.push_back(val);
      values.push_back(val + Scalar(0.3));
      values.push_back(val + Scalar(0.5));
      values.push_back(val + Scalar(0.8));
      values.push_back(val + Scalar(1));
      values.push_back(val + Scalar(1.3));
      values.push_back(val + Scalar(1.5));
      values.push_back(val + Scalar(1.8));
      values.push_back(-val);
      values.push_back(-val - Scalar(0.3));
      values.push_back(-val - Scalar(0.5));
      values.push_back(-val - Scalar(0.8));
      values.push_back(-val - Scalar(1));
      values.push_back(-val - Scalar(1.3));
      values.push_back(-val - Scalar(1.5));
      values.push_back(-val - Scalar(1.8));
      values.push_back(Scalar(-1.5) + val);  // Bug 1785.
      val = val / Scalar(2);
    }
    values.push_back(NumTraits<Scalar>::infinity());
    values.push_back(-NumTraits<Scalar>::infinity());
    values.push_back(NumTraits<Scalar>::quiet_NaN());

    for (size_t k = 0; k < values.size(); ++k) {
      data1[0] = values[k];
      CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::round, internal::pround);
      CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::ceil, internal::pceil);
      CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::floor, internal::pfloor);
      CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::rint, internal::print);
      CHECK_CWISE1_EXACT_IF(PacketTraits::HasRound, numext::trunc, internal::ptrunc);
    }
  }

  for (int i = 0; i < size; ++i) {
    data1[i] = Scalar(internal::random<double>(-1, 1));
    data2[i] = Scalar(internal::random<double>(-1, 1));
  }
  CHECK_CWISE1_IF(PacketTraits::HasASin, std::asin, internal::pasin);
  CHECK_CWISE1_IF(PacketTraits::HasACos, std::acos, internal::pacos);
  CHECK_CWISE1_IF(PacketTraits::HasATan, std::atan, internal::patan);
  CHECK_CWISE1_IF(PacketTraits::HasATanh, std::atanh, internal::patanh);

  for (int i = 0; i < size; ++i) {
    data1[i] = Scalar(internal::random<double>(-87, 88));
    data2[i] = Scalar(internal::random<double>(-87, 88));
    data1[0] = -NumTraits<Scalar>::infinity();
  }
  CHECK_CWISE1_IF(PacketTraits::HasExp, std::exp, internal::pexp);
  CHECK_CWISE1_IF(PacketTraits::HasExp, std::exp2, internal::pexp2);

  CHECK_CWISE1_BYREF1_IF(PacketTraits::HasExp, REF_FREXP, internal::pfrexp);
  if (PacketTraits::HasExp) {
// Check denormals:
#if !EIGEN_ARCH_ARM
    for (int j = 0; j < 3; ++j) {
      data1[0] = Scalar(std::ldexp(1, NumTraits<Scalar>::min_exponent() - j));
      CHECK_CWISE1_BYREF1_IF(PacketTraits::HasExp, REF_FREXP, internal::pfrexp);
      data1[0] = -data1[0];
      CHECK_CWISE1_BYREF1_IF(PacketTraits::HasExp, REF_FREXP, internal::pfrexp);
    }
#endif

    // zero
    data1[0] = Scalar(0);
    CHECK_CWISE1_BYREF1_IF(PacketTraits::HasExp, REF_FREXP, internal::pfrexp);

    // inf and NaN only compare output fraction, not exponent.
    test::packet_helper<PacketTraits::HasExp, Packet> h;
    Packet pout;
    Scalar sout;
    Scalar special[] = {NumTraits<Scalar>::infinity(), -NumTraits<Scalar>::infinity(), NumTraits<Scalar>::quiet_NaN()};
    for (int i = 0; i < 3; ++i) {
      data1[0] = special[i];
      ref[0] = Scalar(REF_FREXP(data1[0], ref[PacketSize]));
      h.store(data2, internal::pfrexp(h.load(data1), h.forward_reference(pout, sout)));
      VERIFY(test::areApprox(ref, data2, 1) && "internal::pfrexp");
    }
  }

  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = Scalar(internal::random<double>(-1, 1));
    data2[i] = Scalar(internal::random<double>(-1, 1));
  }
  for (int i = 0; i < PacketSize; ++i) {
    data1[i + PacketSize] = Scalar(internal::random<int>(-4, 4));
    data2[i + PacketSize] = Scalar(internal::random<double>(-4, 4));
  }
  CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
  if (PacketTraits::HasExp) {
    data1[0] = Scalar(-1);
    // underflow to zero
    data1[PacketSize] = Scalar(NumTraits<Scalar>::min_exponent() - 55);
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    // overflow to inf
    data1[PacketSize] = Scalar(NumTraits<Scalar>::max_exponent() + 10);
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    // NaN stays NaN
    data1[0] = NumTraits<Scalar>::quiet_NaN();
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    VERIFY((numext::isnan)(data2[0]));
    // inf stays inf
    data1[0] = NumTraits<Scalar>::infinity();
    data1[PacketSize] = Scalar(NumTraits<Scalar>::min_exponent() - 10);
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    // zero stays zero
    data1[0] = Scalar(0);
    data1[PacketSize] = Scalar(NumTraits<Scalar>::max_exponent() + 10);
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    // Small number big exponent.
    data1[0] = Scalar(std::ldexp(Scalar(1.0), NumTraits<Scalar>::min_exponent() - 1));
    data1[PacketSize] = Scalar(-NumTraits<Scalar>::min_exponent() + NumTraits<Scalar>::max_exponent());
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
    // Big number small exponent.
    data1[0] = Scalar(std::ldexp(Scalar(1.0), NumTraits<Scalar>::max_exponent() - 1));
    data1[PacketSize] = Scalar(+NumTraits<Scalar>::min_exponent() - NumTraits<Scalar>::max_exponent());
    CHECK_CWISE2_IF(PacketTraits::HasExp, REF_LDEXP, internal::pldexp);
  }

  for (int i = 0; i < size; ++i) {
    data1[i] = Scalar(internal::random<double>(-1, 1) * std::pow(10., internal::random<double>(-6, 6)));
    data2[i] = Scalar(internal::random<double>(-1, 1) * std::pow(10., internal::random<double>(-6, 6)));
  }
  data1[0] = Scalar(1e-20);
  CHECK_CWISE1_IF(PacketTraits::HasTanh, std::tanh, internal::ptanh);
  if (PacketTraits::HasExp && PacketSize >= 2) {
    const Scalar small = NumTraits<Scalar>::epsilon();
    data1[0] = NumTraits<Scalar>::quiet_NaN();
    data1[1] = small;
    test::packet_helper<PacketTraits::HasExp, Packet> h;
    h.store(data2, internal::pexp(h.load(data1)));
    VERIFY((numext::isnan)(data2[0]));
    // TODO(rmlarsen): Re-enable for bfloat16.
    if (!internal::is_same<Scalar, bfloat16>::value) {
      VERIFY_IS_APPROX(std::exp(small), data2[1]);
    }

    data1[0] = -small;
    data1[1] = Scalar(0);
    h.store(data2, internal::pexp(h.load(data1)));
    // TODO(rmlarsen): Re-enable for bfloat16.
    if (!internal::is_same<Scalar, bfloat16>::value) {
      VERIFY_IS_APPROX(std::exp(-small), data2[0]);
    }
    VERIFY_IS_EQUAL(std::exp(Scalar(0)), data2[1]);

    data1[0] = (std::numeric_limits<Scalar>::min)();
    data1[1] = -(std::numeric_limits<Scalar>::min)();
    h.store(data2, internal::pexp(h.load(data1)));
    VERIFY_IS_APPROX(std::exp((std::numeric_limits<Scalar>::min)()), data2[0]);
    VERIFY_IS_APPROX(std::exp(-(std::numeric_limits<Scalar>::min)()), data2[1]);

    data1[0] = std::numeric_limits<Scalar>::denorm_min();
    data1[1] = -std::numeric_limits<Scalar>::denorm_min();
    h.store(data2, internal::pexp(h.load(data1)));
    VERIFY_IS_APPROX(std::exp(std::numeric_limits<Scalar>::denorm_min()), data2[0]);
    VERIFY_IS_APPROX(std::exp(-std::numeric_limits<Scalar>::denorm_min()), data2[1]);
  }

  if (PacketTraits::HasTanh) {
    // NOTE this test migh fail with GCC prior to 6.3, see MathFunctionsImpl.h for details.
    data1[0] = NumTraits<Scalar>::quiet_NaN();
    test::packet_helper<internal::packet_traits<Scalar>::HasTanh, Packet> h;
    h.store(data2, internal::ptanh(h.load(data1)));
    VERIFY((numext::isnan)(data2[0]));
  }

  if (PacketTraits::HasExp) {
    internal::scalar_logistic_op<Scalar> logistic;
    for (int i = 0; i < size; ++i) {
      data1[i] = Scalar(internal::random<double>(-20, 20));
    }

    test::packet_helper<PacketTraits::HasExp, Packet> h;
    h.store(data2, logistic.packetOp(h.load(data1)));
    for (int i = 0; i < PacketSize; ++i) {
      VERIFY_IS_APPROX(data2[i], logistic(data1[i]));
    }
  }

#if EIGEN_HAS_C99_MATH
  data1[0] = NumTraits<Scalar>::infinity();
  data1[1] = Scalar(-1);
  CHECK_CWISE1_IF(PacketTraits::HasLog1p, std::log1p, internal::plog1p);
  data1[0] = NumTraits<Scalar>::infinity();
  data1[1] = -NumTraits<Scalar>::infinity();
  CHECK_CWISE1_IF(PacketTraits::HasExpm1, std::expm1, internal::pexpm1);
#endif

  if (PacketSize >= 2) {
    data1[0] = NumTraits<Scalar>::quiet_NaN();
    data1[1] = NumTraits<Scalar>::epsilon();
    if (PacketTraits::HasLog) {
      test::packet_helper<PacketTraits::HasLog, Packet> h;
      h.store(data2, internal::plog(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      // TODO(cantonios): Re-enable for bfloat16.
      if (!internal::is_same<Scalar, bfloat16>::value) {
        VERIFY_IS_APPROX(std::log(data1[1]), data2[1]);
      }

      data1[0] = -NumTraits<Scalar>::epsilon();
      data1[1] = Scalar(0);
      h.store(data2, internal::plog(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      VERIFY_IS_EQUAL(std::log(Scalar(0)), data2[1]);

      data1[0] = (std::numeric_limits<Scalar>::min)();
      data1[1] = -(std::numeric_limits<Scalar>::min)();
      h.store(data2, internal::plog(h.load(data1)));
      // TODO(cantonios): Re-enable for bfloat16.
      if (!internal::is_same<Scalar, bfloat16>::value) {
        VERIFY_IS_APPROX(std::log((std::numeric_limits<Scalar>::min)()), data2[0]);
      }
      VERIFY((numext::isnan)(data2[1]));

      // Note: 32-bit arm always flushes denorms to zero.
#if !EIGEN_ARCH_ARM
      if (std::numeric_limits<Scalar>::has_denorm == std::denorm_present) {
        data1[0] = std::numeric_limits<Scalar>::denorm_min();
        data1[1] = -std::numeric_limits<Scalar>::denorm_min();
        h.store(data2, internal::plog(h.load(data1)));
        // TODO(rmlarsen): Re-enable for bfloat16.
        if (!internal::is_same<Scalar, bfloat16>::value) {
          VERIFY_IS_APPROX(std::log(std::numeric_limits<Scalar>::denorm_min()), data2[0]);
        }
        VERIFY((numext::isnan)(data2[1]));
      }
#endif

      data1[0] = Scalar(-1.0f);
      h.store(data2, internal::plog(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));

      data1[0] = NumTraits<Scalar>::infinity();
      h.store(data2, internal::plog(h.load(data1)));
      VERIFY((numext::isinf)(data2[0]));
    }
    if (PacketTraits::HasLog1p) {
      test::packet_helper<PacketTraits::HasLog1p, Packet> h;
      data1[0] = Scalar(-2);
      data1[1] = -NumTraits<Scalar>::infinity();
      h.store(data2, internal::plog1p(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      VERIFY((numext::isnan)(data2[1]));
    }

    packetmath_test_IEEE_corner_cases<PacketTraits::HasSqrt, Scalar, Packet>(numext::sqrt<Scalar>, psqrt_functor());
    packetmath_test_IEEE_corner_cases<PacketTraits::HasRsqrt, Scalar, Packet>(numext::rsqrt<Scalar>, prsqrt_functor());
    packetmath_test_IEEE_corner_cases<PacketTraits::HasCbrt, Scalar, Packet>(numext::cbrt<Scalar>, pcbrt_functor());

    // TODO(rmlarsen): Re-enable for half and bfloat16.
    if (PacketTraits::HasCos && !internal::is_same<Scalar, half>::value &&
        !internal::is_same<Scalar, bfloat16>::value) {
      test::packet_helper<PacketTraits::HasCos, Packet> h;
      for (Scalar k = Scalar(1); k < Scalar(10000) / NumTraits<Scalar>::epsilon(); k *= Scalar(2)) {
        for (int k1 = 0; k1 <= 1; ++k1) {
          data1[0] = Scalar((2 * double(k) + k1) * double(EIGEN_PI) / 2 * internal::random<double>(0.8, 1.2));
          data1[1] = Scalar((2 * double(k) + 2 + k1) * double(EIGEN_PI) / 2 * internal::random<double>(0.8, 1.2));
          h.store(data2, internal::pcos(h.load(data1)));
          h.store(data2 + PacketSize, internal::psin(h.load(data1)));
          VERIFY(data2[0] <= Scalar(1.) && data2[0] >= Scalar(-1.));
          VERIFY(data2[1] <= Scalar(1.) && data2[1] >= Scalar(-1.));
          VERIFY(data2[PacketSize + 0] <= Scalar(1.) && data2[PacketSize + 0] >= Scalar(-1.));
          VERIFY(data2[PacketSize + 1] <= Scalar(1.) && data2[PacketSize + 1] >= Scalar(-1.));

          VERIFY_IS_APPROX(data2[0], std::cos(data1[0]));
          VERIFY_IS_APPROX(data2[1], std::cos(data1[1]));
          VERIFY_IS_APPROX(data2[PacketSize + 0], std::sin(data1[0]));
          VERIFY_IS_APPROX(data2[PacketSize + 1], std::sin(data1[1]));

          VERIFY_IS_APPROX(numext::abs2(data2[0]) + numext::abs2(data2[PacketSize + 0]), Scalar(1));
          VERIFY_IS_APPROX(numext::abs2(data2[1]) + numext::abs2(data2[PacketSize + 1]), Scalar(1));
        }
      }

      data1[0] = NumTraits<Scalar>::infinity();
      data1[1] = -NumTraits<Scalar>::infinity();
      h.store(data2, internal::psin(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      VERIFY((numext::isnan)(data2[1]));

      h.store(data2, internal::pcos(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      VERIFY((numext::isnan)(data2[1]));

      data1[0] = NumTraits<Scalar>::quiet_NaN();
      h.store(data2, internal::psin(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));
      h.store(data2, internal::pcos(h.load(data1)));
      VERIFY((numext::isnan)(data2[0]));

      data1[0] = -Scalar(0.);
      h.store(data2, internal::psin(h.load(data1)));
      VERIFY(test::biteq(data2[0], data1[0]));
      h.store(data2, internal::pcos(h.load(data1)));
      VERIFY_IS_EQUAL(data2[0], Scalar(1));
    }
  }
  if (PacketTraits::HasReciprocal && PacketSize >= 2) {
    test::packet_helper<PacketTraits::HasReciprocal, Packet> h;
    const Scalar inf = NumTraits<Scalar>::infinity();
    const Scalar zero = Scalar(0);
    data1[0] = zero;
    data1[1] = -zero;
    h.store(data2, internal::preciprocal(h.load(data1)));
    VERIFY_IS_EQUAL(data2[0], inf);
    VERIFY_IS_EQUAL(data2[1], -inf);

    data1[0] = inf;
    data1[1] = -inf;
    h.store(data2, internal::preciprocal(h.load(data1)));
    VERIFY_IS_EQUAL(data2[0], zero);
    VERIFY_IS_EQUAL(data2[1], -zero);
  }
}

#define CAST_CHECK_CWISE1_IF(COND, REFOP, POP, SCALAR, REFTYPE)                                  \
  if (COND) {                                                                                    \
    test::packet_helper<COND, Packet> h;                                                         \
    for (int i = 0; i < PacketSize; ++i) ref[i] = SCALAR(REFOP(static_cast<REFTYPE>(data1[i]))); \
    h.store(data2, POP(h.load(data1)));                                                          \
    VERIFY(test::areApprox(ref, data2, PacketSize) && #POP);                                     \
  }

template <typename Scalar>
Scalar propagate_nan_max(const Scalar& a, const Scalar& b) {
  if ((numext::isnan)(a)) return a;
  if ((numext::isnan)(b)) return b;
  return (numext::maxi)(a, b);
}

template <typename Scalar>
Scalar propagate_nan_min(const Scalar& a, const Scalar& b) {
  if ((numext::isnan)(a)) return a;
  if ((numext::isnan)(b)) return b;
  return (numext::mini)(a, b);
}

template <typename Scalar>
Scalar propagate_number_max(const Scalar& a, const Scalar& b) {
  if ((numext::isnan)(a)) return b;
  if ((numext::isnan)(b)) return a;
  return (numext::maxi)(a, b);
}

template <typename Scalar>
Scalar propagate_number_min(const Scalar& a, const Scalar& b) {
  if ((numext::isnan)(a)) return b;
  if ((numext::isnan)(b)) return a;
  return (numext::mini)(a, b);
}

template <typename Scalar, typename Packet>
void packetmath_notcomplex() {
  typedef internal::packet_traits<Scalar> PacketTraits;
  const int PacketSize = internal::unpacket_traits<Packet>::size;

  EIGEN_ALIGN_MAX Scalar data1[PacketSize * 4];
  EIGEN_ALIGN_MAX Scalar data2[PacketSize * 4];
  EIGEN_ALIGN_MAX Scalar ref[PacketSize * 4];

  Array<Scalar, Dynamic, 1>::Map(data1, PacketSize * 4).setRandom();

  VERIFY((!PacketTraits::Vectorizable) || PacketTraits::HasMin);
  VERIFY((!PacketTraits::Vectorizable) || PacketTraits::HasMax);

  CHECK_CWISE2_IF(PacketTraits::HasMin, (std::min), internal::pmin);
  CHECK_CWISE2_IF(PacketTraits::HasMax, (std::max), internal::pmax);

  CHECK_CWISE2_IF(PacketTraits::HasMin, propagate_number_min, internal::pmin<PropagateNumbers>);
  CHECK_CWISE2_IF(PacketTraits::HasMax, propagate_number_max, internal::pmax<PropagateNumbers>);
  CHECK_CWISE1(numext::abs, internal::pabs);
  // Vectorized versions may give a different result in the case of signed int overflow,
  // which is undefined behavior (e.g. NEON).
  // Also note that unsigned integers with size < sizeof(int) may be implicitly converted to a signed
  // int, which can also trigger UB.
  if (Eigen::NumTraits<Scalar>::IsInteger) {
    for (int i = 0; i < 2 * PacketSize; ++i) {
      data1[i] = data1[i] / Scalar(2);
    }
  }
  CHECK_CWISE2_IF(PacketTraits::HasAbsDiff, REF_ABS_DIFF, internal::pabsdiff);

  ref[0] = data1[0];
  for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmin(ref[0], data1[i]);
  VERIFY(internal::isApprox(ref[0], internal::predux_min(internal::pload<Packet>(data1))) && "internal::predux_min");
  ref[0] = data1[0];
  for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmax(ref[0], data1[i]);
  VERIFY(internal::isApprox(ref[0], internal::predux_max(internal::pload<Packet>(data1))) && "internal::predux_max");

  for (int i = 0; i < PacketSize; ++i) ref[i] = data1[0] + Scalar(i);
  internal::pstore(data2, internal::plset<Packet>(data1[0]));
  VERIFY(test::areApprox(ref, data2, PacketSize) && "internal::plset");

  {
    unsigned char* data1_bits = reinterpret_cast<unsigned char*>(data1);
    // predux_all - not needed yet
    // for (unsigned int i=0; i<PacketSize*sizeof(Scalar); ++i) data1_bits[i] = 0xff;
    // VERIFY(internal::predux_all(internal::pload<Packet>(data1)) && "internal::predux_all(1111)");
    // for(int k=0; k<PacketSize; ++k)
    // {
    //   for (unsigned int i=0; i<sizeof(Scalar); ++i) data1_bits[k*sizeof(Scalar)+i] = 0x0;
    //   VERIFY( (!internal::predux_all(internal::pload<Packet>(data1))) && "internal::predux_all(0101)");
    //   for (unsigned int i=0; i<sizeof(Scalar); ++i) data1_bits[k*sizeof(Scalar)+i] = 0xff;
    // }

    // predux_any
    for (unsigned int i = 0; i < PacketSize * sizeof(Scalar); ++i) data1_bits[i] = 0x0;
    VERIFY((!internal::predux_any(internal::pload<Packet>(data1))) && "internal::predux_any(0000)");
    for (int k = 0; k < PacketSize; ++k) {
      for (unsigned int i = 0; i < sizeof(Scalar); ++i) data1_bits[k * sizeof(Scalar) + i] = 0xff;
      VERIFY(internal::predux_any(internal::pload<Packet>(data1)) && "internal::predux_any(0101)");
      for (unsigned int i = 0; i < sizeof(Scalar); ++i) data1_bits[k * sizeof(Scalar) + i] = 0x00;
    }
  }

  // Test NaN propagation.
  if (!NumTraits<Scalar>::IsInteger) {
    // Test reductions with no NaNs.
    ref[0] = data1[0];
    for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmin<PropagateNumbers>(ref[0], data1[i]);
    VERIFY(internal::isApprox(ref[0], internal::predux_min<PropagateNumbers>(internal::pload<Packet>(data1))) &&
           "internal::predux_min<PropagateNumbers>");
    ref[0] = data1[0];
    for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmin<PropagateNaN>(ref[0], data1[i]);
    VERIFY(internal::isApprox(ref[0], internal::predux_min<PropagateNaN>(internal::pload<Packet>(data1))) &&
           "internal::predux_min<PropagateNaN>");
    ref[0] = data1[0];
    for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmax<PropagateNumbers>(ref[0], data1[i]);
    VERIFY(internal::isApprox(ref[0], internal::predux_max<PropagateNumbers>(internal::pload<Packet>(data1))) &&
           "internal::predux_max<PropagateNumbers>");
    ref[0] = data1[0];
    for (int i = 0; i < PacketSize; ++i) ref[0] = internal::pmax<PropagateNaN>(ref[0], data1[i]);
    VERIFY(internal::isApprox(ref[0], internal::predux_max<PropagateNaN>(internal::pload<Packet>(data1))) &&
           "internal::predux_max<PropagateNumbers>");
    // A single NaN.
    const size_t index = std::numeric_limits<size_t>::quiet_NaN() % PacketSize;
    data1[index] = NumTraits<Scalar>::quiet_NaN();
    VERIFY(PacketSize == 1 || !(numext::isnan)(internal::predux_min<PropagateNumbers>(internal::pload<Packet>(data1))));
    VERIFY((numext::isnan)(internal::predux_min<PropagateNaN>(internal::pload<Packet>(data1))));
    VERIFY(PacketSize == 1 || !(numext::isnan)(internal::predux_max<PropagateNumbers>(internal::pload<Packet>(data1))));
    VERIFY((numext::isnan)(internal::predux_max<PropagateNaN>(internal::pload<Packet>(data1))));
    // All NaNs.
    for (int i = 0; i < 4 * PacketSize; ++i) data1[i] = NumTraits<Scalar>::quiet_NaN();
    VERIFY((numext::isnan)(internal::predux_min<PropagateNumbers>(internal::pload<Packet>(data1))));
    VERIFY((numext::isnan)(internal::predux_min<PropagateNaN>(internal::pload<Packet>(data1))));
    VERIFY((numext::isnan)(internal::predux_max<PropagateNumbers>(internal::pload<Packet>(data1))));
    VERIFY((numext::isnan)(internal::predux_max<PropagateNaN>(internal::pload<Packet>(data1))));

    // Test NaN propagation for coefficient-wise min and max.
    for (int i = 0; i < PacketSize; ++i) {
      data1[i] = internal::random<bool>() ? NumTraits<Scalar>::quiet_NaN() : Scalar(0);
      data1[i + PacketSize] = internal::random<bool>() ? NumTraits<Scalar>::quiet_NaN() : Scalar(0);
    }
    // Note: NaN propagation is implementation defined for pmin/pmax, so we do not test it here.
    CHECK_CWISE2_IF(PacketTraits::HasMin, propagate_number_min, (internal::pmin<PropagateNumbers>));
    CHECK_CWISE2_IF(PacketTraits::HasMax, propagate_number_max, internal::pmax<PropagateNumbers>);
    CHECK_CWISE2_IF(PacketTraits::HasMin, propagate_nan_min, (internal::pmin<PropagateNaN>));
    CHECK_CWISE2_IF(PacketTraits::HasMax, propagate_nan_max, internal::pmax<PropagateNaN>);
  }

  packetmath_boolean_mask_ops_notcomplex_test<Scalar, Packet>::run();
}

template <typename Scalar, typename Packet, bool ConjLhs, bool ConjRhs>
void test_conj_helper(Scalar* data1, Scalar* data2, Scalar* ref, Scalar* pval) {
  const int PacketSize = internal::unpacket_traits<Packet>::size;

  internal::conj_if<ConjLhs> cj0;
  internal::conj_if<ConjRhs> cj1;
  internal::conj_helper<Scalar, Scalar, ConjLhs, ConjRhs> cj;
  internal::conj_helper<Packet, Packet, ConjLhs, ConjRhs> pcj;

  for (int i = 0; i < PacketSize; ++i) {
    ref[i] = cj0(data1[i]) * cj1(data2[i]);
    VERIFY(internal::isApprox(ref[i], cj.pmul(data1[i], data2[i])) && "conj_helper pmul");
  }
  internal::pstore(pval, pcj.pmul(internal::pload<Packet>(data1), internal::pload<Packet>(data2)));
  VERIFY(test::areApprox(ref, pval, PacketSize) && "conj_helper pmul");

  for (int i = 0; i < PacketSize; ++i) {
    Scalar tmp = ref[i];
    ref[i] += cj0(data1[i]) * cj1(data2[i]);
    VERIFY(internal::isApprox(ref[i], cj.pmadd(data1[i], data2[i], tmp)) && "conj_helper pmadd");
  }
  internal::pstore(
      pval, pcj.pmadd(internal::pload<Packet>(data1), internal::pload<Packet>(data2), internal::pload<Packet>(pval)));
  VERIFY(test::areApprox(ref, pval, PacketSize) && "conj_helper pmadd");
}

template <typename Scalar, typename Packet, bool HasExp = internal::packet_traits<Scalar>::HasExp>
struct exp_complex_test_impl {
  typedef typename Scalar::value_type RealScalar;

  static Scalar pexp1(const Scalar& x) {
    Packet px = internal::pset1<Packet>(x);
    Packet py = internal::pexp(px);
    return internal::pfirst(py);
  }

  static Scalar cis(const RealScalar& x) { return Scalar(numext::cos(x), numext::sin(x)); }

  // Verify equality with signed zero.
  static bool is_exactly_equal(RealScalar a, RealScalar b) {
    // NaNs are always unsigned, and always compare not equal directly.
    if ((numext::isnan)(a)) {
      return (numext::isnan)(b);
    }

    RealScalar zero(0);
#ifdef EIGEN_ARCH_ARM
    // ARM automatically flushes denormals to zero.
    // Preserve sign by multiplying by +0.
    if (numext::abs(a) < (std::numeric_limits<RealScalar>::min)()) {
      a = a * zero;
    }
    if (numext::abs(b) < (std::numeric_limits<RealScalar>::min)()) {
      b = b * zero;
    }
#endif

    // Signed zero.
    if (a == zero) {
      // Signs are either 0 or NaN, so verify that their comparisons to zero are equal.
      return (a == b) && ((numext::signbit(a) == zero) == (numext::signbit(b) == zero));
    }
    // Allow _some_ tolerance.
    return verifyIsApprox(a, b);
  }

  // Verify equality with signed zero.
  static bool is_exactly_equal(const Scalar& a, const Scalar& b) {
    bool result = is_exactly_equal(numext::real_ref(a), numext::real_ref(b)) &&
                  is_exactly_equal(numext::imag_ref(a), numext::imag_ref(b));
    if (!result) {
      std::cout << a << " != " << b << std::endl;
    }
    return result;
  }

  static bool is_sign_exp_unspecified(const Scalar& z) {
    const RealScalar inf = std::numeric_limits<RealScalar>::infinity();
    // If z is (-∞,±∞), the result is (±0,±0) (signs are unspecified)
    if (numext::real_ref(z) == -inf && (numext::isinf)(numext::imag_ref(z))) {
      return true;
    }
    // If z is (+∞,±∞), the result is (±∞,NaN) and FE_INVALID is raised (the sign of the real part is unspecified)
    if (numext::real_ref(z) == +inf && (numext::isinf)(numext::imag_ref(z))) {
      return true;
    }
    // If z is (-∞,NaN), the result is (±0,±0) (signs are unspecified)
    if (numext::real_ref(z) == -inf && (numext::isnan)(numext::imag_ref(z))) {
      return true;
    }
    // If z is (+∞,NaN), the result is (±∞,NaN) (the sign of the real part is unspecified)
    if (numext::real_ref(z) == +inf && (numext::isnan)(numext::imag_ref(z))) {
      return true;
    }
    return false;
  }

  static void run(Scalar* data1, Scalar* data2, Scalar* ref, int size) {
    const int PacketSize = internal::unpacket_traits<Packet>::size;

    for (int i = 0; i < size; ++i) {
      data1[i] = Scalar(internal::random<RealScalar>(), internal::random<RealScalar>());
    }
    CHECK_CWISE1_N(std::exp, internal::pexp, size);

    // Test all corner cases (and more).
    const RealScalar edges[] = {RealScalar(0),
                                RealScalar(1),
                                RealScalar(2),
                                RealScalar(EIGEN_PI / 2),
                                RealScalar(EIGEN_PI),
                                RealScalar(3 * EIGEN_PI / 2),
                                RealScalar(2 * EIGEN_PI),
                                numext::log(NumTraits<RealScalar>::highest()) - 1,
                                NumTraits<RealScalar>::highest(),
                                std::numeric_limits<RealScalar>::infinity(),
                                std::numeric_limits<RealScalar>::quiet_NaN(),
                                -RealScalar(0),
                                -RealScalar(1),
                                -RealScalar(2),
                                -RealScalar(EIGEN_PI / 2),
                                -RealScalar(EIGEN_PI),
                                -RealScalar(3 * EIGEN_PI / 2),
                                -RealScalar(2 * EIGEN_PI),
                                -numext::log(NumTraits<RealScalar>::highest()) + 1,
                                -NumTraits<RealScalar>::highest(),
                                -std::numeric_limits<RealScalar>::infinity(),
                                -std::numeric_limits<RealScalar>::quiet_NaN()};

    for (RealScalar x : edges) {
      for (RealScalar y : edges) {
        Scalar z = Scalar(x, y);
        Scalar w = pexp1(z);
        if (is_sign_exp_unspecified(z)) {
          Scalar abs_w = Scalar(numext::abs(numext::real_ref(w)), numext::abs(numext::imag_ref(w)));
          Scalar expected = numext::exp(z);
          Scalar abs_expected =
              Scalar(numext::abs(numext::real_ref(expected)), numext::abs(numext::imag_ref(expected)));
          VERIFY(is_exactly_equal(abs_w, abs_expected));
        } else {
          VERIFY(is_exactly_equal(w, numext::exp(z)));
        }
      }
    }
  }
};

template <typename Scalar, typename Packet>
struct exp_complex_test_impl<Scalar, Packet, false> {
  typedef typename Scalar::value_type RealScalar;
  static void run(Scalar*, Scalar*, Scalar*, int) {};
};

template <typename Scalar, typename Packet>
void exp_complex_test(Scalar* data1, Scalar* data2, Scalar* ref, int size) {
  exp_complex_test_impl<Scalar, Packet>::run(data1, data2, ref, size);
}

template <typename Scalar, typename Packet>
void packetmath_complex() {
  typedef internal::packet_traits<Scalar> PacketTraits;
  typedef typename Scalar::value_type RealScalar;
  const int PacketSize = internal::unpacket_traits<Packet>::size;

  const int size = PacketSize * 4;
  EIGEN_ALIGN_MAX Scalar data1[PacketSize * 4];
  EIGEN_ALIGN_MAX Scalar data2[PacketSize * 4];
  EIGEN_ALIGN_MAX Scalar ref[PacketSize * 4];
  EIGEN_ALIGN_MAX Scalar pval[PacketSize * 4];
  EIGEN_ALIGN_MAX RealScalar realdata[PacketSize * 4];
  EIGEN_ALIGN_MAX RealScalar realref[PacketSize * 4];

  for (int i = 0; i < size; ++i) {
    data1[i] = internal::random<Scalar>() * Scalar(1e2);
    data2[i] = internal::random<Scalar>() * Scalar(1e2);
  }

  test_conj_helper<Scalar, Packet, false, false>(data1, data2, ref, pval);
  test_conj_helper<Scalar, Packet, false, true>(data1, data2, ref, pval);
  test_conj_helper<Scalar, Packet, true, false>(data1, data2, ref, pval);
  test_conj_helper<Scalar, Packet, true, true>(data1, data2, ref, pval);

  // Test pcplxflip.
  {
    for (int i = 0; i < PacketSize; ++i) ref[i] = Scalar(std::imag(data1[i]), std::real(data1[i]));
    internal::pstore(pval, internal::pcplxflip(internal::pload<Packet>(data1)));
    VERIFY(test::areApprox(ref, pval, PacketSize) && "pcplxflip");
  }

  if (PacketTraits::HasSqrt) {
    for (int i = 0; i < size; ++i) {
      data1[i] = Scalar(internal::random<RealScalar>(), internal::random<RealScalar>());
    }
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, size);
    CHECK_CWISE1_IF(PacketTraits::HasSign, numext::sign, internal::psign);

    // Test misc. corner cases.
    const RealScalar zero = RealScalar(0);
    const RealScalar one = RealScalar(1);
    const RealScalar inf = std::numeric_limits<RealScalar>::infinity();
    const RealScalar nan = std::numeric_limits<RealScalar>::quiet_NaN();
    data1[0] = Scalar(zero, zero);
    data1[1] = Scalar(-zero, zero);
    data1[2] = Scalar(one, zero);
    data1[3] = Scalar(zero, one);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
    data1[0] = Scalar(-one, zero);
    data1[1] = Scalar(zero, -one);
    data1[2] = Scalar(one, one);
    data1[3] = Scalar(-one, -one);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
    data1[0] = Scalar(inf, zero);
    data1[1] = Scalar(zero, inf);
    data1[2] = Scalar(-inf, zero);
    data1[3] = Scalar(zero, -inf);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
    data1[0] = Scalar(inf, inf);
    data1[1] = Scalar(-inf, inf);
    data1[2] = Scalar(inf, -inf);
    data1[3] = Scalar(-inf, -inf);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
    data1[0] = Scalar(nan, zero);
    data1[1] = Scalar(zero, nan);
    data1[2] = Scalar(nan, one);
    data1[3] = Scalar(one, nan);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
    data1[0] = Scalar(nan, nan);
    data1[1] = Scalar(inf, nan);
    data1[2] = Scalar(nan, inf);
    data1[3] = Scalar(-inf, nan);
    CHECK_CWISE1_N(numext::sqrt, internal::psqrt, 4);
  }
  if (PacketTraits::HasLog) {
    for (int i = 0; i < size; ++i) {
      data1[i] = Scalar(internal::random<RealScalar>(), internal::random<RealScalar>());
    }
    CHECK_CWISE1_N(std::log, internal::plog, size);

    // Test misc. corner cases.
    const RealScalar zero = RealScalar(0);
    const RealScalar one = RealScalar(1);
    const RealScalar inf = std::numeric_limits<RealScalar>::infinity();
    const RealScalar nan = std::numeric_limits<RealScalar>::quiet_NaN();
    for (RealScalar x : {zero, one, inf}) {
      for (RealScalar y : {zero, one, inf}) {
        data1[0] = Scalar(x, y);
        data1[1] = Scalar(-x, y);
        data1[2] = Scalar(x, -y);
        data1[3] = Scalar(-x, -y);
        CHECK_CWISE1_IM1ULP_N(std::log, internal::plog, 4);
      }
    }
    // Set reference results to nan.
    // Some architectures don't handle IEEE edge cases correctly
    ref[0] = Scalar(nan, nan);
    ref[1] = Scalar(nan, nan);
    ref[2] = Scalar(nan, nan);
    ref[3] = Scalar(nan, nan);
    for (RealScalar x : {zero, one}) {
      data1[0] = Scalar(x, nan);
      data1[1] = Scalar(-x, nan);
      data1[2] = Scalar(nan, x);
      data1[3] = Scalar(nan, -x);
      for (int j = 0; j < size; j += PacketSize)
        internal::pstore(data2 + j, internal::plog(internal::pload<Packet>(data1 + j)));
      VERIFY(test::areApprox(ref, data2, 4));
    }
    data1[0] = Scalar(inf, nan);
    data1[1] = Scalar(-inf, nan);
    data1[2] = Scalar(nan, inf);
    data1[3] = Scalar(nan, -inf);
    CHECK_CWISE1_IM1ULP_N(numext::log, internal::plog, 4);
  }
  exp_complex_test<Scalar, Packet>(data1, data2, ref, size);
}

template <typename Scalar, typename Packet>
void packetmath_scatter_gather() {
  typedef typename NumTraits<Scalar>::Real RealScalar;
  const int PacketSize = internal::unpacket_traits<Packet>::size;
  EIGEN_ALIGN_MAX Scalar data1[PacketSize];
  RealScalar refvalue = RealScalar(0);
  for (int i = 0; i < PacketSize; ++i) {
    data1[i] = internal::random<Scalar>();
  }

  int stride = internal::random<int>(1, 20);

  // Buffer of zeros.
  EIGEN_ALIGN_MAX Scalar buffer[PacketSize * 20] = {};

  Packet packet = internal::pload<Packet>(data1);
  internal::pscatter<Scalar, Packet>(buffer, packet, stride);

  for (int i = 0; i < PacketSize * 20; ++i) {
    if ((i % stride) == 0 && i < stride * PacketSize) {
      VERIFY(test::isApproxAbs(buffer[i], data1[i / stride], refvalue) && "pscatter");
    } else {
      VERIFY(test::isApproxAbs(buffer[i], Scalar(0), refvalue) && "pscatter");
    }
  }

  for (int i = 0; i < PacketSize * 7; ++i) {
    buffer[i] = internal::random<Scalar>();
  }
  packet = internal::pgather<Scalar, Packet>(buffer, 7);
  internal::pstore(data1, packet);
  for (int i = 0; i < PacketSize; ++i) {
    VERIFY(test::isApproxAbs(data1[i], buffer[i * 7], refvalue) && "pgather");
  }

  for (Index N = 0; N <= PacketSize; ++N) {
    for (Index i = 0; i < N; ++i) {
      data1[i] = internal::random<Scalar>();
    }

    for (Index i = 0; i < N * 20; ++i) {
      buffer[i] = Scalar(0);
    }

    packet = internal::pload_partial<Packet>(data1, N);
    internal::pscatter_partial<Scalar, Packet>(buffer, packet, stride, N);

    for (Index i = 0; i < N * 20; ++i) {
      if ((i % stride) == 0 && i < stride * N) {
        VERIFY(test::isApproxAbs(buffer[i], data1[i / stride], refvalue) && "pscatter_partial");
      } else {
        VERIFY(test::isApproxAbs(buffer[i], Scalar(0), refvalue) && "pscatter_partial");
      }
    }

    for (Index i = 0; i < N * 7; ++i) {
      buffer[i] = internal::random<Scalar>();
    }
    packet = internal::pgather_partial<Scalar, Packet>(buffer, 7, N);
    internal::pstore_partial(data1, packet, N);
    for (Index i = 0; i < N; ++i) {
      VERIFY(test::isApproxAbs(data1[i], buffer[i * 7], refvalue) && "pgather_partial");
    }
  }
}

namespace Eigen {
namespace test {

template <typename Scalar, typename PacketType>
struct runall<Scalar, PacketType, false, false> {  // i.e. float or double
  static void run() {
    packetmath<Scalar, PacketType>();
    packetmath_scatter_gather<Scalar, PacketType>();
    packetmath_notcomplex<Scalar, PacketType>();
    packetmath_real<Scalar, PacketType>();
  }
};

template <typename Scalar, typename PacketType>
struct runall<Scalar, PacketType, false, true> {  // i.e. int
  static void run() {
    packetmath<Scalar, PacketType>();
    packetmath_scatter_gather<Scalar, PacketType>();
    packetmath_notcomplex<Scalar, PacketType>();
  }
};

template <typename Scalar, typename PacketType>
struct runall<Scalar, PacketType, true, false> {  // i.e. complex
  static void run() {
    packetmath<Scalar, PacketType>();
    packetmath_scatter_gather<Scalar, PacketType>();
    packetmath_complex<Scalar, PacketType>();
  }
};

}  // namespace test
}  // namespace Eigen

EIGEN_DECLARE_TEST(packetmath) {
  g_first_pass = true;
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(test::runner<float>::run());
    CALL_SUBTEST_2(test::runner<double>::run());
    CALL_SUBTEST_3(test::runner<int8_t>::run());
    CALL_SUBTEST_4(test::runner<uint8_t>::run());
    CALL_SUBTEST_5(test::runner<int16_t>::run());
    CALL_SUBTEST_6(test::runner<uint16_t>::run());
    CALL_SUBTEST_7(test::runner<int32_t>::run());
    CALL_SUBTEST_8(test::runner<uint32_t>::run());
    CALL_SUBTEST_9(test::runner<int64_t>::run());
    CALL_SUBTEST_10(test::runner<uint64_t>::run());
    CALL_SUBTEST_11(test::runner<std::complex<float>>::run());
    CALL_SUBTEST_12(test::runner<std::complex<double>>::run());
    CALL_SUBTEST_13(test::runner<half>::run());
    CALL_SUBTEST_14((packetmath<bool, internal::packet_traits<bool>::type>()));
    CALL_SUBTEST_14((packetmath_scatter_gather<bool, internal::packet_traits<bool>::type>()));
    CALL_SUBTEST_15(test::runner<bfloat16>::run());
    g_first_pass = false;
  }
}
