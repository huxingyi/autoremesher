// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2025 The Eigen Authors
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename Scalar, typename Packet>
void verify_data(const Scalar* data_in, const Scalar* data_out, const Packet& a, Index begin, Index count) {
  constexpr int PacketSize = internal::unpacket_traits<Packet>::size;
  bool ok = true;
  for (Index i = begin; i < begin + count; i++) {
    ok = ok && numext::equal_strict(data_in[i], data_out[i]);
  }
  if (!ok) {
    std::cout << "begin: " << begin << ", count: " << count << "\n";
    std::cout << "Scalar type: " << type_name(Scalar()) << " x " << PacketSize << "\n";
    std::cout << "data in:  {";
    for (Index i = 0; i < PacketSize; i++) {
      if (i > 0) std::cout << ",";
      if (i < begin || i >= begin + count) {
        std::cout << "MASK";
      } else {
        std::cout << data_in[i];
      }
    }
    std::cout << "}\n";
    std::cout << "data out: {";
    for (Index i = 0; i < PacketSize; i++) {
      if (i > 0) std::cout << ",";
      if (i < begin || i >= begin + count) {
        std::cout << "MASK";
      } else {
        std::cout << data_out[i];
      }
    }
    std::cout << "}\n";
    std::cout << "packet:   ";
    std::cout << internal::postream(a) << "\n";
  }
  VERIFY(ok);
}

template <typename Scalar, int PacketSize, bool Run = internal::find_packet_by_size<Scalar, PacketSize>::value>
struct packet_segment_test_impl {
  using Packet = typename internal::find_packet_by_size<Scalar, PacketSize>::type;
  static void test_unaligned() {
    // test loading a packet segment from unaligned memory that includes unallocated memory

    // | X   X   X   X | *   *   *   X | X   X   X   X |
    //    begin -> { X | *   *   *   } <- begin + count

    VectorX<Scalar> data_in(PacketSize), data_out(PacketSize);
    data_in.setRandom();
    data_out.setRandom();

    Scalar* unaligned_data_in = data_in.data() - 1;
    Scalar* unaligned_data_out = data_out.data() - 1;

    Index begin = 1;
    Index count = PacketSize - 1;

    Packet a = internal::ploaduSegment<Packet>(unaligned_data_in, begin, count);
    internal::pstoreuSegment<Scalar, Packet>(unaligned_data_out, a, begin, count);

    verify_data(unaligned_data_in, unaligned_data_out, a, begin, count);

    // test loading the entire packet

    data_in.setRandom();
    data_out.setRandom();

    unaligned_data_in = data_in.data();
    unaligned_data_out = data_out.data();

    begin = 0;
    count = PacketSize;

    Packet b = internal::ploaduSegment<Packet>(unaligned_data_in, begin, count);
    internal::pstoreuSegment<Scalar, Packet>(unaligned_data_out, b, begin, count);

    verify_data(unaligned_data_in, unaligned_data_out, b, begin, count);

    // test loading an empty packet segment in unallocated memory
    count = 0;

    for (begin = 0; begin < PacketSize; begin++) {
      data_in.setRandom();
      data_out = data_in;
      Packet c = internal::ploaduSegment<Packet>(data_in.data(), begin, count);
      internal::pstoreuSegment<Scalar, Packet>(data_out.data(), c, begin, count);
      // verify that ploaduSegment / pstoreuSegment did nothing
      VERIFY_IS_CWISE_EQUAL(data_in, data_out);
    }
  }
  static void test_aligned() {
    // test loading a packet segment from aligned memory that includes unallocated memory

    // | X   X   X   X | *   *   *   X | X   X   X   X |
    //        begin -> { *   *   *   X } <- begin + count

    VectorX<Scalar> data_in(PacketSize - 1), data_out(PacketSize - 1);
    data_in.setRandom();
    data_out.setRandom();

    Scalar* aligned_data_in = data_in.data();
    Scalar* aligned_data_out = data_out.data();

    Index begin = 0;
    Index count = PacketSize - 1;

    Packet b = internal::ploadSegment<Packet>(aligned_data_in, begin, count);
    internal::pstoreSegment<Scalar, Packet>(aligned_data_out, b, begin, count);

    verify_data(aligned_data_in, aligned_data_out, b, begin, count);
  }
  static void run() {
    test_unaligned();
    test_aligned();
  }
};

template <typename Scalar, int PacketSize>
struct packet_segment_test_impl<Scalar, PacketSize, false> {
  static void run() {}
};

template <typename Scalar, int PacketSize>
struct packet_segment_test_driver {
  static void run() {
    packet_segment_test_impl<Scalar, PacketSize>::run();
    packet_segment_test_driver<Scalar, PacketSize / 2>::run();
  }
};

template <typename Scalar>
struct packet_segment_test_driver<Scalar, 1> {
  static void run() {}
};

template <bool Enable = internal::packet_traits<half>::Vectorizable>
void testReverseEdgeCase() {
  // this reversed cast uses a non-zero offset for ploadSegment
  Index size = 16 * internal::packet_traits<half>::size + 1;
  VectorX<half> v1(size);
  VectorX<float> v2(size), v3(size);
  v1.setRandom();
  v2 = v1.reverse().cast<float>();
  v3 = v1.cast<float>().reverse();
  VERIFY_IS_EQUAL(v2, v3);
}

template <>
void testReverseEdgeCase<false>() {}

template <typename Scalar>
void test_packet_segment() {
  packet_segment_test_driver<Scalar, internal::packet_traits<Scalar>::size>::run();
}

EIGEN_DECLARE_TEST(packet_segment) {
  for (int i = 0; i < g_repeat; i++) {
    test_packet_segment<bool>();
    test_packet_segment<int8_t>();
    test_packet_segment<uint8_t>();
    test_packet_segment<int16_t>();
    test_packet_segment<uint16_t>();
    test_packet_segment<int32_t>();
    test_packet_segment<uint32_t>();
    test_packet_segment<int64_t>();
    test_packet_segment<uint64_t>();
    test_packet_segment<bfloat16>();
    test_packet_segment<half>();
    test_packet_segment<float>();
    test_packet_segment<double>();
    test_packet_segment<std::complex<float>>();
    test_packet_segment<std::complex<double>>();
    testReverseEdgeCase();
  }
}
