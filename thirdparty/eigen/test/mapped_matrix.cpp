// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2006-2010 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

#define EIGEN_TESTMAP_MAX_SIZE 256

template <typename VectorType>
void map_class_vector(const VectorType& m) {
  typedef typename VectorType::Scalar Scalar;

  Index size = m.size();

  Scalar* array1 = internal::aligned_new<Scalar>(size);
  Scalar* array2 = internal::aligned_new<Scalar>(size);
  Scalar* array3 = new Scalar[size + 1];
  // In case of no alignment, avoid division by zero.
  constexpr int alignment = (std::max<int>)(EIGEN_MAX_ALIGN_BYTES, 1);
  Scalar* array3unaligned = (std::uintptr_t(array3) % alignment) == 0 ? array3 + 1 : array3;
  Scalar array4[EIGEN_TESTMAP_MAX_SIZE];

  Map<VectorType, AlignedMax>(array1, size) = VectorType::Random(size);
  Map<VectorType, AlignedMax>(array2, size) = Map<VectorType, AlignedMax>(array1, size);
  Map<VectorType>(array3unaligned, size) = Map<VectorType>(array1, size);
  Map<VectorType>(array4, size) = Map<VectorType, AlignedMax>(array1, size);
  VectorType ma1 = Map<VectorType, AlignedMax>(array1, size);
  VectorType ma2 = Map<VectorType, AlignedMax>(array2, size);
  VectorType ma3 = Map<VectorType>(array3unaligned, size);
  VectorType ma4 = Map<VectorType>(array4, size);
  VERIFY_IS_EQUAL(ma1, ma2);
  VERIFY_IS_EQUAL(ma1, ma3);
  VERIFY_IS_EQUAL(ma1, ma4);
#ifdef EIGEN_VECTORIZE
  if (internal::packet_traits<Scalar>::Vectorizable && size >= AlignedMax)
    VERIFY_RAISES_ASSERT((Map<VectorType, AlignedMax>(array3unaligned, size)))
#endif

  internal::aligned_delete(array1, size);
  internal::aligned_delete(array2, size);
  delete[] array3;
}

template <typename MatrixType>
void map_class_matrix(const MatrixType& m) {
  typedef typename MatrixType::Scalar Scalar;

  Index rows = m.rows(), cols = m.cols(), size = rows * cols;
  Scalar s1 = internal::random<Scalar>();

  // array1 and array2 -> aligned heap allocation
  Scalar* array1 = internal::aligned_new<Scalar>(size);
  for (int i = 0; i < size; i++) array1[i] = Scalar(1);
  Scalar* array2 = internal::aligned_new<Scalar>(size);
  for (int i = 0; i < size; i++) array2[i] = Scalar(1);
  // array3unaligned -> unaligned pointer to heap
  Scalar* array3 = new Scalar[size + 1];
  Index sizep1 = size + 1;  // <- without this temporary MSVC 2103 generates bad code
  for (Index i = 0; i < sizep1; i++) array3[i] = Scalar(1);
  // In case of no alignment, avoid division by zero.
  constexpr int alignment = (std::max<int>)(EIGEN_MAX_ALIGN_BYTES, 1);
  Scalar* array3unaligned = (std::uintptr_t(array3) % alignment) == 0 ? array3 + 1 : array3;
  Scalar array4[256];
  if (size <= 256)
    for (int i = 0; i < size; i++) array4[i] = Scalar(1);

  Map<MatrixType> map1(array1, rows, cols);
  Map<MatrixType, AlignedMax> map2(array2, rows, cols);
  Map<MatrixType> map3(array3unaligned, rows, cols);
  Map<MatrixType> map4(array4, rows, cols);

  VERIFY_IS_EQUAL(map1, MatrixType::Ones(rows, cols));
  map1.setConstant(s1);
  VERIFY_IS_EQUAL(map1, MatrixType::Constant(rows, cols, s1));
  map1.setZero();
  VERIFY_IS_EQUAL(map1, MatrixType::Zero(rows, cols));

  VERIFY_IS_EQUAL(map2, MatrixType::Ones(rows, cols));
  map2.setConstant(s1);
  VERIFY_IS_EQUAL(map2, MatrixType::Constant(rows, cols, s1));
  map2.setZero();
  VERIFY_IS_EQUAL(map2, MatrixType::Zero(rows, cols));

  VERIFY_IS_EQUAL(map3, MatrixType::Ones(rows, cols));
  map3.setConstant(s1);
  VERIFY_IS_EQUAL(map3, MatrixType::Constant(rows, cols, s1));
  map3.setZero();
  VERIFY_IS_EQUAL(map3, MatrixType::Zero(rows, cols));

  map1 = MatrixType::Random(rows, cols);
  map2 = map1;
  map3 = map1;
  MatrixType ma1 = map1;
  MatrixType ma2 = map2;
  MatrixType ma3 = map3;
  VERIFY_IS_EQUAL(map1, map2);
  VERIFY_IS_EQUAL(map1, map3);
  VERIFY_IS_EQUAL(ma1, ma2);
  VERIFY_IS_EQUAL(ma1, ma3);
  VERIFY_IS_EQUAL(ma1, map3);

  VERIFY_IS_APPROX(s1 * map1, s1 * map2);
  VERIFY_IS_APPROX(s1 * ma1, s1 * ma2);
  VERIFY_IS_EQUAL(s1 * ma1, s1 * ma3);
  VERIFY_IS_APPROX(s1 * map1, s1 * map3);

  map2 *= s1;
  map3 *= s1;
  VERIFY_IS_APPROX(s1 * map1, map2);
  VERIFY_IS_APPROX(s1 * map1, map3);

  if (size <= 256) {
    VERIFY_IS_EQUAL(map4, MatrixType::Ones(rows, cols));
    map4 = map1;
    MatrixType ma4 = map4;
    VERIFY_IS_EQUAL(map1, map4);
    VERIFY_IS_EQUAL(ma1, map4);
    VERIFY_IS_EQUAL(ma1, ma4);
    VERIFY_IS_APPROX(s1 * map1, s1 * map4);

    map4 *= s1;
    VERIFY_IS_APPROX(s1 * map1, map4);
  }

  internal::aligned_delete(array1, size);
  internal::aligned_delete(array2, size);
  delete[] array3;
}

template <typename VectorType>
void map_static_methods(const VectorType& m) {
  typedef typename VectorType::Scalar Scalar;

  Index size = m.size();

  Scalar* array1 = internal::aligned_new<Scalar>(size);
  Scalar* array2 = internal::aligned_new<Scalar>(size);
  Scalar* array3 = new Scalar[size + 1];
  // In case of no alignment, avoid division by zero.
  constexpr int alignment = (std::max<int>)(EIGEN_MAX_ALIGN_BYTES, 1);
  Scalar* array3unaligned = (std::uintptr_t(array3) % alignment) == 0 ? array3 + 1 : array3;

  VectorType::MapAligned(array1, size) = VectorType::Random(size);
  VectorType::Map(array2, size) = VectorType::Map(array1, size);
  VectorType::Map(array3unaligned, size) = VectorType::Map(array1, size);
  VectorType ma1 = VectorType::Map(array1, size);
  VectorType ma2 = VectorType::MapAligned(array2, size);
  VectorType ma3 = VectorType::Map(array3unaligned, size);
  VERIFY_IS_EQUAL(ma1, ma2);
  VERIFY_IS_EQUAL(ma1, ma3);

  internal::aligned_delete(array1, size);
  internal::aligned_delete(array2, size);
  delete[] array3;
}

template <typename PlainObjectType>
void check_const_correctness(const PlainObjectType&) {
  // there's a lot that we can't test here while still having this test compile!
  // the only possible approach would be to run a script trying to compile stuff and checking that it fails.
  // CMake can help with that.

  // verify that map-to-const don't have LvalueBit
  typedef std::add_const_t<PlainObjectType> ConstPlainObjectType;
  VERIFY(!(internal::traits<Map<ConstPlainObjectType> >::Flags & LvalueBit));
  VERIFY(!(internal::traits<Map<ConstPlainObjectType, AlignedMax> >::Flags & LvalueBit));
  VERIFY(!(Map<ConstPlainObjectType>::Flags & LvalueBit));
  VERIFY(!(Map<ConstPlainObjectType, AlignedMax>::Flags & LvalueBit));
}

EIGEN_DECLARE_TEST(mapped_matrix) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(map_class_vector(Matrix<float, 1, 1>()));
    CALL_SUBTEST_1(check_const_correctness(Matrix<float, 1, 1>()));
    CALL_SUBTEST_2(map_class_vector(Vector4d()));
    CALL_SUBTEST_2(map_class_vector(VectorXd(13)));
    CALL_SUBTEST_2(check_const_correctness(Matrix4d()));
    CALL_SUBTEST_3(map_class_vector(RowVector4f()));
    CALL_SUBTEST_4(map_class_vector(VectorXcf(8)));
    CALL_SUBTEST_5(map_class_vector(VectorXi(12)));
    CALL_SUBTEST_5(check_const_correctness(VectorXi(12)));

    CALL_SUBTEST_1(map_class_matrix(Matrix<float, 1, 1>()));
    CALL_SUBTEST_2(map_class_matrix(Matrix4d()));
    CALL_SUBTEST_11(map_class_matrix(Matrix<float, 3, 5>()));
    CALL_SUBTEST_4(map_class_matrix(MatrixXcf(internal::random<int>(1, 10), internal::random<int>(1, 10))));
    CALL_SUBTEST_5(map_class_matrix(MatrixXi(internal::random<int>(1, 10), internal::random<int>(1, 10))));

    CALL_SUBTEST_6(map_static_methods(Matrix<double, 1, 1>()));
    CALL_SUBTEST_7(map_static_methods(Vector3f()));
    CALL_SUBTEST_8(map_static_methods(RowVector3d()));
    CALL_SUBTEST_9(map_static_methods(VectorXcd(8)));
    CALL_SUBTEST_10(map_static_methods(VectorXf(12)));
  }
}
