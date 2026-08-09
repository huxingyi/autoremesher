// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2013 Hauke Heibel <hauke.heibel@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_TESTING_PLAINOBJECT_CTOR

#include "main.h"
#include "AnnoyingScalar.h"
#include "MovableScalar.h"
#include "SafeScalar.h"

#include <Eigen/Core>

using DenseStorageD3x3 = Eigen::DenseStorage<double, 9, 3, 3, 0>;
#if !defined(EIGEN_DENSE_STORAGE_CTOR_PLUGIN)
static_assert(std::is_trivially_copy_constructible<DenseStorageD3x3>::value,
              "DenseStorage not trivially_copy_constructible");
static_assert(std::is_trivially_move_constructible<DenseStorageD3x3>::value,
              "DenseStorage not trivially_move_constructible");
static_assert(std::is_trivially_copy_assignable<DenseStorageD3x3>::value, "DenseStorage not trivially_copy_assignable");
static_assert(std::is_trivially_move_assignable<DenseStorageD3x3>::value, "DenseStorage not trivially_move_assignable");
#endif
// all plain object types conform to standard layout
static_assert(std::is_standard_layout<Matrix4f>::value, "Matrix4f not standard_layout");
static_assert(std::is_standard_layout<Array4f>::value, "Array4f not standard_layout");
static_assert(std::is_standard_layout<VectorXf>::value, "VectorXf not standard_layout");
static_assert(std::is_standard_layout<ArrayXf>::value, "ArrayXf not standard_layout");
static_assert(std::is_standard_layout<MatrixXf>::value, "MatrixXf not standard_layout");
static_assert(std::is_standard_layout<ArrayXXf>::value, "ArrayXXf not standard_layout");
// all fixed-size, fixed-dimension plain object types are trivially default constructible
static_assert(std::is_trivially_default_constructible<Matrix4f>::value, "Matrix4f not trivially_default_constructible");
static_assert(std::is_trivially_default_constructible<Array4f>::value, "Array4f not trivially_default_constructible");
// all fixed-size, fixed-dimension plain object types are trivially move constructible
static_assert(std::is_trivially_move_constructible<Matrix4f>::value, "Matrix4f not trivially_move_constructible");
static_assert(std::is_trivially_move_constructible<Array4f>::value, "Array4f not trivially_move_constructible");
// all statically-allocated plain object types are trivially destructible
static_assert(std::is_trivially_destructible<Matrix4f>::value, "Matrix4f not trivially_destructible");
static_assert(std::is_trivially_destructible<Array4f>::value, "Array4f not trivially_destructible");
static_assert(std::is_trivially_destructible<Matrix<float, 4, Dynamic, 0, 4, 4>>::value,
              "Matrix4X44 not trivially_destructible");
static_assert(std::is_trivially_destructible<Array<float, 4, Dynamic, 0, 4, 4>>::value,
              "Array4X44 not trivially_destructible");
#if !defined(EIGEN_DENSE_STORAGE_CTOR_PLUGIN)
// all fixed-size, fixed-dimension plain object types are trivially copy constructible
static_assert(std::is_trivially_copy_constructible<Matrix4f>::value, "Matrix4f not trivially_copy_constructible");
static_assert(std::is_trivially_copy_constructible<Array4f>::value, "Array4f not trivially_copy_constructible");
#endif

template <typename T, int Size, int Rows, int Cols>
void dense_storage_copy(int rows, int cols) {
  typedef DenseStorage<T, Size, Rows, Cols, 0> DenseStorageType;

  const int size = rows * cols;
  DenseStorageType reference(size, rows, cols);
  T* raw_reference = reference.data();
  for (int i = 0; i < size; ++i) raw_reference[i] = internal::random<T>();

  DenseStorageType copied_reference(reference);
  const T* raw_copied_reference = copied_reference.data();
  for (int i = 0; i < size; ++i) VERIFY_IS_EQUAL(raw_reference[i], raw_copied_reference[i]);
}

template <typename T, int Size, int Rows, int Cols>
void dense_storage_assignment(int rows, int cols) {
  typedef DenseStorage<T, Size, Rows, Cols, 0> DenseStorageType;

  const int size = rows * cols;
  DenseStorageType reference(size, rows, cols);
  T* raw_reference = reference.data();
  for (int i = 0; i < size; ++i) raw_reference[i] = internal::random<T>();

  DenseStorageType copied_reference;
  copied_reference = reference;
  const T* raw_copied_reference = copied_reference.data();
  for (int i = 0; i < size; ++i) VERIFY_IS_EQUAL(raw_reference[i], raw_copied_reference[i]);
}

template <typename T, int Size, int Rows, int Cols>
void dense_storage_swap(int rowsa, int colsa, int rowsb, int colsb) {
  typedef DenseStorage<T, Size, Rows, Cols, 0> DenseStorageType;

  const int sizea = rowsa * colsa;
  ArrayX<T> referencea(sizea);
  referencea.setRandom();
  DenseStorageType a(sizea, rowsa, colsa);
  for (int i = 0; i < sizea; ++i) a.data()[i] = referencea(i);

  const int sizeb = rowsb * colsb;
  ArrayX<T> referenceb(sizeb);
  referenceb.setRandom();
  DenseStorageType b(sizeb, rowsb, colsb);
  for (int i = 0; i < sizeb; ++i) b.data()[i] = referenceb(i);

  a.swap(b);

  for (int i = 0; i < sizea; i++) VERIFY_IS_EQUAL(b.data()[i], referencea(i));
  for (int i = 0; i < sizeb; i++) VERIFY_IS_EQUAL(a.data()[i], referenceb(i));
}

template <typename T, int Size, std::size_t Alignment>
void dense_storage_alignment() {
  struct alignas(Alignment) Empty1 {};
  VERIFY_IS_EQUAL(std::alignment_of<Empty1>::value, Alignment);

  struct EIGEN_ALIGN_TO_BOUNDARY(Alignment) Empty2 {};
  VERIFY_IS_EQUAL(std::alignment_of<Empty2>::value, Alignment);

  struct Nested1 {
    EIGEN_ALIGN_TO_BOUNDARY(Alignment) T data[Size];
  };
  VERIFY_IS_EQUAL(std::alignment_of<Nested1>::value, Alignment);

  VERIFY_IS_EQUAL((std::alignment_of<internal::plain_array<T, Size, AutoAlign, Alignment>>::value), Alignment);

  const std::size_t default_alignment = internal::compute_default_alignment<T, Size>::value;
  if (default_alignment > 0) {
    VERIFY_IS_EQUAL((std::alignment_of<DenseStorage<T, Size, 1, 1, AutoAlign>>::value), default_alignment);
    VERIFY_IS_EQUAL((std::alignment_of<Matrix<T, Size, 1, AutoAlign>>::value), default_alignment);
    struct Nested2 {
      Matrix<T, Size, 1, AutoAlign> mat;
    };
    VERIFY_IS_EQUAL(std::alignment_of<Nested2>::value, default_alignment);
  }
}

template <typename T>
void dense_storage_tests() {
  // Dynamic Storage.
  dense_storage_copy<T, Dynamic, Dynamic, Dynamic>(4, 3);
  dense_storage_copy<T, Dynamic, Dynamic, 3>(4, 3);
  dense_storage_copy<T, Dynamic, 4, Dynamic>(4, 3);
  // Fixed Storage.
  dense_storage_copy<T, 12, 4, 3>(4, 3);
  dense_storage_copy<T, 12, Dynamic, Dynamic>(4, 3);
  dense_storage_copy<T, 12, 4, Dynamic>(4, 3);
  dense_storage_copy<T, 12, Dynamic, 3>(4, 3);
  // Fixed Storage with Uninitialized Elements.
  dense_storage_copy<T, 18, Dynamic, Dynamic>(4, 3);
  dense_storage_copy<T, 18, 4, Dynamic>(4, 3);
  dense_storage_copy<T, 18, Dynamic, 3>(4, 3);

  // Dynamic Storage.
  dense_storage_assignment<T, Dynamic, Dynamic, Dynamic>(4, 3);
  dense_storage_assignment<T, Dynamic, Dynamic, 3>(4, 3);
  dense_storage_assignment<T, Dynamic, 4, Dynamic>(4, 3);
  // Fixed Storage.
  dense_storage_assignment<T, 12, 4, 3>(4, 3);
  dense_storage_assignment<T, 12, Dynamic, Dynamic>(4, 3);
  dense_storage_assignment<T, 12, 4, Dynamic>(4, 3);
  dense_storage_assignment<T, 12, Dynamic, 3>(4, 3);
  // Fixed Storage with Uninitialized Elements.
  dense_storage_assignment<T, 18, Dynamic, Dynamic>(4, 3);
  dense_storage_assignment<T, 18, 4, Dynamic>(4, 3);
  dense_storage_assignment<T, 18, Dynamic, 3>(4, 3);

  // Dynamic Storage.
  dense_storage_swap<T, Dynamic, Dynamic, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, Dynamic, Dynamic, Dynamic>(4, 3, 2, 1);
  dense_storage_swap<T, Dynamic, Dynamic, Dynamic>(2, 1, 4, 3);
  dense_storage_swap<T, Dynamic, Dynamic, 3>(4, 3, 4, 3);
  dense_storage_swap<T, Dynamic, Dynamic, 3>(4, 3, 2, 3);
  dense_storage_swap<T, Dynamic, Dynamic, 3>(2, 3, 4, 3);
  dense_storage_swap<T, Dynamic, 4, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, Dynamic, 4, Dynamic>(4, 3, 4, 1);
  dense_storage_swap<T, Dynamic, 4, Dynamic>(4, 1, 4, 3);
  // Fixed Storage.
  dense_storage_swap<T, 12, 4, 3>(4, 3, 4, 3);
  dense_storage_swap<T, 12, Dynamic, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, 12, Dynamic, Dynamic>(4, 3, 2, 1);
  dense_storage_swap<T, 12, Dynamic, Dynamic>(2, 1, 4, 3);
  dense_storage_swap<T, 12, 4, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, 12, 4, Dynamic>(4, 3, 4, 1);
  dense_storage_swap<T, 12, 4, Dynamic>(4, 1, 4, 3);
  dense_storage_swap<T, 12, Dynamic, 3>(4, 3, 4, 3);
  dense_storage_swap<T, 12, Dynamic, 3>(4, 3, 2, 3);
  dense_storage_swap<T, 12, Dynamic, 3>(2, 3, 4, 3);
  // Fixed Storage with Uninitialized Elements.
  dense_storage_swap<T, 18, Dynamic, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, 18, Dynamic, Dynamic>(4, 3, 2, 1);
  dense_storage_swap<T, 18, Dynamic, Dynamic>(2, 1, 4, 3);
  dense_storage_swap<T, 18, 4, Dynamic>(4, 3, 4, 3);
  dense_storage_swap<T, 18, 4, Dynamic>(4, 3, 4, 1);
  dense_storage_swap<T, 18, 4, Dynamic>(4, 1, 4, 3);
  dense_storage_swap<T, 18, Dynamic, 3>(4, 3, 4, 3);
  dense_storage_swap<T, 18, Dynamic, 3>(4, 3, 2, 3);
  dense_storage_swap<T, 18, Dynamic, 3>(2, 3, 4, 3);

  dense_storage_alignment<T, 16, 8>();
  dense_storage_alignment<T, 16, 16>();
  dense_storage_alignment<T, 16, 32>();
  dense_storage_alignment<T, 16, 64>();
}

template <typename PlainType>
void plaintype_tests() {
  constexpr int RowsAtCompileTime = PlainType::RowsAtCompileTime;
  constexpr int ColsAtCompileTime = PlainType::ColsAtCompileTime;
  constexpr int MaxRowsAtCompileTime = PlainType::MaxRowsAtCompileTime;
  constexpr int MaxColsAtCompileTime = PlainType::MaxColsAtCompileTime;
  const Index expectedDefaultRows = RowsAtCompileTime == Dynamic ? 0 : RowsAtCompileTime;
  const Index expectedDefaultCols = ColsAtCompileTime == Dynamic ? 0 : ColsAtCompileTime;
  const Index minRows = RowsAtCompileTime == Dynamic ? 0 : RowsAtCompileTime;
  const Index minCols = ColsAtCompileTime == Dynamic ? 0 : ColsAtCompileTime;
  const Index maxRows = MaxRowsAtCompileTime == Dynamic ? 100 : MaxRowsAtCompileTime;
  const Index maxCols = MaxColsAtCompileTime == Dynamic ? 100 : MaxColsAtCompileTime;
  const Index rows = internal::random<Index>(minRows, maxRows);
  const Index cols = internal::random<Index>(minCols, maxCols);
  // default construction
  PlainType m0;
  VERIFY_IS_EQUAL(m0.rows(), expectedDefaultRows);
  VERIFY_IS_EQUAL(m0.cols(), expectedDefaultCols);
  m0.resize(rows, cols);
  m0.setRandom();
  // copy construction
  PlainType m1(m0);
  VERIFY_IS_EQUAL(m1.rows(), m0.rows());
  VERIFY_IS_EQUAL(m1.cols(), m0.cols());
  VERIFY_IS_CWISE_EQUAL(m1, m0);
  // move construction
  PlainType m2(std::move(m1));
  VERIFY_IS_EQUAL(m2.rows(), m0.rows());
  VERIFY_IS_EQUAL(m2.cols(), m0.cols());
  VERIFY_IS_CWISE_EQUAL(m2, m0);
  // check that object is usable after move construction
  m1.resize(minRows, minCols);
  m1.setRandom();
  // copy assignment
  m1 = m0;
  VERIFY_IS_EQUAL(m1.rows(), m0.rows());
  VERIFY_IS_EQUAL(m1.cols(), m0.cols());
  VERIFY_IS_CWISE_EQUAL(m1, m0);
  // move assignment
  m2.resize(minRows, minCols);
  m2.setRandom();
  m2 = std::move(m1);
  VERIFY_IS_EQUAL(m2.rows(), m0.rows());
  VERIFY_IS_EQUAL(m2.cols(), m0.cols());
  VERIFY_IS_CWISE_EQUAL(m2, m0);
  // check that object is usable after move assignment
  m1.resize(minRows, minCols);
  m1.setRandom();
  m1 = m2;
  VERIFY_IS_EQUAL(m1.rows(), m0.rows());
  VERIFY_IS_EQUAL(m1.cols(), m0.cols());
  VERIFY_IS_CWISE_EQUAL(m1, m0);
}

EIGEN_DECLARE_TEST(dense_storage) {
  dense_storage_tests<int>();
  dense_storage_tests<float>();
  dense_storage_tests<SafeScalar<float>>();
  dense_storage_tests<MovableScalar<float>>();
  dense_storage_tests<AnnoyingScalar>();
  for (int i = 0; i < g_repeat; i++) {
    plaintype_tests<Matrix<float, 0, 0, ColMajor>>();
    plaintype_tests<Matrix<float, Dynamic, Dynamic, ColMajor, 0, 0>>();

    plaintype_tests<Matrix<float, 16, 16, ColMajor>>();
    plaintype_tests<Matrix<float, 16, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<float, Dynamic, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<float, Dynamic, Dynamic, ColMajor, 16, 16>>();

    plaintype_tests<Matrix<SafeScalar<float>, 16, 16, ColMajor>>();
    plaintype_tests<Matrix<SafeScalar<float>, 16, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<SafeScalar<float>, Dynamic, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<SafeScalar<float>, Dynamic, Dynamic, ColMajor, 16, 16>>();

    plaintype_tests<Matrix<MovableScalar<float>, 16, 16, ColMajor>>();
    plaintype_tests<Matrix<MovableScalar<float>, 16, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<MovableScalar<float>, Dynamic, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<MovableScalar<float>, Dynamic, Dynamic, ColMajor, 16, 16>>();

    plaintype_tests<Matrix<AnnoyingScalar, 16, 16, ColMajor>>();
    plaintype_tests<Matrix<AnnoyingScalar, 16, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<AnnoyingScalar, Dynamic, Dynamic, ColMajor>>();
    plaintype_tests<Matrix<AnnoyingScalar, Dynamic, Dynamic, ColMajor, 16, 16>>();
  }
}

#undef EIGEN_TESTING_PLAINOBJECT_CTOR