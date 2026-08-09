// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <g.gael@free.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdlib>
#include <string>

#include "sparse.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <Eigen/SparseExtra>

// Read from an environment variable TEST_TMPDIR, if available,
// and append the provided filename. Defaults to local directory.
std::string GetTestTempFilename(const char* filename) {
  const char* test_tmpdir = std::getenv("TEST_TMPDIR");
  if (test_tmpdir == nullptr) {
    return std::string(filename);
  }
  return std::string(test_tmpdir) + std::string("/") + std::string(filename);
}

template <typename SetterType, typename DenseType, typename Scalar, int Options>
bool test_random_setter(SparseMatrix<Scalar, Options>& sm, const DenseType& ref,
                        const std::vector<Vector2i>& nonzeroCoords) {
  {
    sm.setZero();
    SetterType w(sm);
    std::vector<Vector2i> remaining = nonzeroCoords;
    while (!remaining.empty()) {
      int i = internal::random<int>(0, static_cast<int>(remaining.size()) - 1);
      w(remaining[i].x(), remaining[i].y()) = ref.coeff(remaining[i].x(), remaining[i].y());
      remaining[i] = remaining.back();
      remaining.pop_back();
    }
  }
  return sm.isApprox(ref);
}

template <typename SparseMatrixType>
void sparse_extra(const SparseMatrixType& ref) {
  const Index rows = ref.rows();
  const Index cols = ref.cols();
  typedef typename SparseMatrixType::Scalar Scalar;
  enum { Flags = SparseMatrixType::Flags };

  double density = (std::max)(8. / (rows * cols), 0.01);
  typedef Matrix<Scalar, Dynamic, Dynamic> DenseMatrix;
  typedef Matrix<Scalar, Dynamic, 1> DenseVector;
  Scalar eps = 1e-6;

  SparseMatrixType m(rows, cols);
  DenseMatrix refMat = DenseMatrix::Zero(rows, cols);
  DenseVector vec1 = DenseVector::Random(rows);

  std::vector<Vector2i> zeroCoords;
  std::vector<Vector2i> nonzeroCoords;
  initSparse<Scalar>(density, refMat, m, 0, &zeroCoords, &nonzeroCoords);

  if (zeroCoords.size() == 0 || nonzeroCoords.size() == 0) return;

  // test coeff and coeffRef
  for (int i = 0; i < (int)zeroCoords.size(); ++i) {
    VERIFY_IS_MUCH_SMALLER_THAN(m.coeff(zeroCoords[i].x(), zeroCoords[i].y()), eps);
    if (internal::is_same<SparseMatrixType, SparseMatrix<Scalar, Flags> >::value)
      VERIFY_RAISES_ASSERT(m.coeffRef(zeroCoords[0].x(), zeroCoords[0].y()) = 5);
  }
  VERIFY_IS_APPROX(m, refMat);

  m.coeffRef(nonzeroCoords[0].x(), nonzeroCoords[0].y()) = Scalar(5);
  refMat.coeffRef(nonzeroCoords[0].x(), nonzeroCoords[0].y()) = Scalar(5);

  VERIFY_IS_APPROX(m, refMat);

  // random setter
  //   {
  //     m.setZero();
  //     VERIFY_IS_NOT_APPROX(m, refMat);
  //     SparseSetter<SparseMatrixType, RandomAccessPattern> w(m);
  //     std::vector<Vector2i> remaining = nonzeroCoords;
  //     while(!remaining.empty())
  //     {
  //       int i = internal::random<int>(0,remaining.size()-1);
  //       w->coeffRef(remaining[i].x(),remaining[i].y()) = refMat.coeff(remaining[i].x(),remaining[i].y());
  //       remaining[i] = remaining.back();
  //       remaining.pop_back();
  //     }
  //   }
  //   VERIFY_IS_APPROX(m, refMat);

  VERIFY((test_random_setter<RandomSetter<SparseMatrixType, StdMapTraits> >(m, refMat, nonzeroCoords)));
  VERIFY((test_random_setter<RandomSetter<SparseMatrixType, StdUnorderedMapTraits> >(m, refMat, nonzeroCoords)));
#ifdef EIGEN_GOOGLEHASH_SUPPORT
  VERIFY((test_random_setter<RandomSetter<SparseMatrixType, GoogleDenseHashMapTraits> >(m, refMat, nonzeroCoords)));
  VERIFY((test_random_setter<RandomSetter<SparseMatrixType, GoogleSparseHashMapTraits> >(m, refMat, nonzeroCoords)));
#endif

  // test RandomSetter
  /*{
    SparseMatrixType m1(rows,cols), m2(rows,cols);
    DenseMatrix refM1 = DenseMatrix::Zero(rows, rows);
    initSparse<Scalar>(density, refM1, m1);
    {
      Eigen::RandomSetter<SparseMatrixType > setter(m2);
      for (int j=0; j<m1.outerSize(); ++j)
        for (typename SparseMatrixType::InnerIterator i(m1,j); i; ++i)
          setter(i.index(), j) = i.value();
    }
    VERIFY_IS_APPROX(m1, m2);
  }*/
}

template <typename SparseMatrixType>
void check_marketio() {
  typedef Matrix<typename SparseMatrixType::Scalar, Dynamic, Dynamic> DenseMatrix;
  Index rows = internal::random<Index>(1, 100);
  Index cols = internal::random<Index>(1, 100);
  SparseMatrixType m1, m2;
  m1 = DenseMatrix::Random(rows, cols).sparseView();
  std::string filename = GetTestTempFilename("sparse_extra.mtx");
  saveMarket(m1, filename);
  loadMarket(m2, filename);
  VERIFY_IS_EQUAL(DenseMatrix(m1), DenseMatrix(m2));
}

template <typename VectorType>
void check_marketio_vector() {
  Index size = internal::random<Index>(1, 100);
  VectorType v1, v2;
  v1 = VectorType::Random(size);
  std::string filename = GetTestTempFilename("vector_extra.mtx");
  saveMarketVector(v1, filename);
  loadMarketVector(v2, filename);
  VERIFY_IS_EQUAL(v1, v2);
}

template <typename DenseMatrixType>
void check_marketio_dense() {
  Index rows = DenseMatrixType::MaxRowsAtCompileTime;
  if (DenseMatrixType::MaxRowsAtCompileTime == Dynamic) {
    rows = internal::random<Index>(1, 100);
  } else if (DenseMatrixType::RowsAtCompileTime == Dynamic) {
    rows = internal::random<Index>(1, DenseMatrixType::MaxRowsAtCompileTime);
  }

  Index cols = DenseMatrixType::MaxColsAtCompileTime;
  if (DenseMatrixType::MaxColsAtCompileTime == Dynamic) {
    cols = internal::random<Index>(1, 100);
  } else if (DenseMatrixType::ColsAtCompileTime == Dynamic) {
    cols = internal::random<Index>(1, DenseMatrixType::MaxColsAtCompileTime);
  }

  DenseMatrixType m1, m2;
  m1 = DenseMatrixType::Random(rows, cols);
  std::string filename = GetTestTempFilename("dense_extra.mtx");
  saveMarketDense(m1, filename);
  loadMarketDense(m2, filename);
  VERIFY_IS_EQUAL(m1, m2);
}

template <typename Scalar>
void check_sparse_inverse() {
  typedef SparseMatrix<Scalar> MatrixType;

  Matrix<Scalar, -1, -1> A;
  A.resize(1000, 1000);
  A.fill(0);
  A.setIdentity();
  A.col(0).array() += 1;
  A.row(0).array() += 2;
  A.col(2).array() += 3;
  A.row(7).array() += 3;
  A.col(9).array() += 3;
  A.block(3, 4, 4, 2).array() += 9;
  A.middleRows(10, 50).array() += 3;
  A.middleCols(50, 50).array() += 40;
  A.block(500, 300, 40, 20).array() += 10;
  A.transposeInPlace();

  Eigen::SparseLU<MatrixType> slu;
  slu.compute(A.sparseView());
  Matrix<Scalar, -1, -1> Id(A.rows(), A.cols());
  Id.setIdentity();
  Matrix<Scalar, -1, -1> inv = slu.solve(Id);

  const MatrixType sparseInv = Eigen::SparseInverse<Scalar>().compute(A.sparseView()).inverse();

  Scalar sumdiff = 0;  // Check the diff only of the non-zero elements
  for (Eigen::Index j = 0; j < A.cols(); j++) {
    for (typename MatrixType::InnerIterator iter(sparseInv, j); iter; ++iter) {
      const Scalar diff = std::abs(inv(iter.row(), iter.col()) - iter.value());
      VERIFY_IS_APPROX_OR_LESS_THAN(diff, 1e-11);

      if (iter.value() != 0) {
        sumdiff += diff;
      }
    }
  }

  VERIFY_IS_APPROX_OR_LESS_THAN(sumdiff, 1e-10);
}

EIGEN_DECLARE_TEST(sparse_extra) {
  for (int i = 0; i < g_repeat; i++) {
    int s = Eigen::internal::random<int>(1, 50);
    CALL_SUBTEST_1(sparse_extra(SparseMatrix<double>(8, 8)));
    CALL_SUBTEST_2(sparse_extra(SparseMatrix<std::complex<double> >(s, s)));
    CALL_SUBTEST_1(sparse_extra(SparseMatrix<double>(s, s)));

    CALL_SUBTEST_3((check_marketio<SparseMatrix<float, ColMajor, int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<double, ColMajor, int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<std::complex<float>, ColMajor, int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<std::complex<double>, ColMajor, int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<float, ColMajor, long int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<double, ColMajor, long int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<std::complex<float>, ColMajor, long int> >()));
    CALL_SUBTEST_3((check_marketio<SparseMatrix<std::complex<double>, ColMajor, long int> >()));

    CALL_SUBTEST_4((check_marketio_dense<Matrix<float, Dynamic, Dynamic> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<float, Dynamic, Dynamic, RowMajor> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<double, Dynamic, Dynamic> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<std::complex<float>, Dynamic, Dynamic> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<std::complex<double>, Dynamic, Dynamic> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<float, Dynamic, 3> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<double, 3, Dynamic> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<double, 3, 4> >()));
    CALL_SUBTEST_4((check_marketio_dense<Matrix<double, Dynamic, Dynamic, ColMajor, 5, 5> >()));

    CALL_SUBTEST_5((check_marketio_vector<Matrix<float, 1, Dynamic> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<double, 1, Dynamic> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<std::complex<float>, 1, Dynamic> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<std::complex<double>, 1, Dynamic> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<float, Dynamic, 1> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<double, Dynamic, 1> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<std::complex<float>, Dynamic, 1> >()));
    CALL_SUBTEST_5((check_marketio_vector<Matrix<std::complex<double>, Dynamic, 1> >()));

    CALL_SUBTEST_6((check_sparse_inverse<double>()));

    TEST_SET_BUT_UNUSED_VARIABLE(s);
  }
}
