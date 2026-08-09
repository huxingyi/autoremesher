// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2021 Kolja Brix <kolja.brix@rwth-aachen.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_RANDOM_MATRIX_HELPER
#define EIGEN_RANDOM_MATRIX_HELPER

#include <typeinfo>
#include <Eigen/QR>  // required for createRandomPIMatrixOfRank and generateRandomMatrixSvs

// Forward declarations to avoid ICC warnings
#if EIGEN_COMP_ICC

namespace Eigen {

template <typename MatrixType>
void createRandomPIMatrixOfRank(Index desired_rank, Index rows, Index cols, MatrixType& m);

template <typename PermutationVectorType>
void randomPermutationVector(PermutationVectorType& v, Index size);

template <typename MatrixType>
MatrixType generateRandomUnitaryMatrix(const Index dim);

template <typename MatrixType, typename RealScalarVectorType>
void generateRandomMatrixSvs(const RealScalarVectorType& svs, const Index rows, const Index cols, MatrixType& M);

template <typename VectorType, typename RealScalar>
VectorType setupRandomSvs(const Index dim, const RealScalar max);

template <typename VectorType, typename RealScalar>
VectorType setupRangeSvs(const Index dim, const RealScalar min, const RealScalar max);

}  // end namespace Eigen

#endif  // EIGEN_COMP_ICC

namespace Eigen {

/**
 * Creates a random partial isometry matrix of given rank.
 *
 * A partial isometry is a matrix all of whose singular values are either 0 or 1.
 * This is very useful to test rank-revealing algorithms.
 *
 * @tparam MatrixType type of random partial isometry matrix
 * @param desired_rank rank requested for the random partial isometry matrix
 * @param rows row dimension of requested random partial isometry matrix
 * @param cols column dimension of requested random partial isometry matrix
 * @param m random partial isometry matrix
 */
template <typename MatrixType>
void createRandomPIMatrixOfRank(Index desired_rank, Index rows, Index cols, MatrixType& m) {
  typedef typename internal::traits<MatrixType>::Scalar Scalar;
  enum { Rows = MatrixType::RowsAtCompileTime, Cols = MatrixType::ColsAtCompileTime };

  typedef Matrix<Scalar, Dynamic, 1> VectorType;
  typedef Matrix<Scalar, Rows, Rows> MatrixAType;
  typedef Matrix<Scalar, Cols, Cols> MatrixBType;

  if (desired_rank == 0) {
    m.setZero(rows, cols);
    return;
  }

  if (desired_rank == 1) {
    // here we normalize the vectors to get a partial isometry
    m = VectorType::Random(rows).normalized() * VectorType::Random(cols).normalized().transpose();
    return;
  }

  MatrixAType a = MatrixAType::Random(rows, rows);
  MatrixType d = MatrixType::Identity(rows, cols);
  MatrixBType b = MatrixBType::Random(cols, cols);

  // set the diagonal such that only desired_rank non-zero entries remain
  const Index diag_size = (std::min)(d.rows(), d.cols());
  if (diag_size != desired_rank)
    d.diagonal().segment(desired_rank, diag_size - desired_rank) = VectorType::Zero(diag_size - desired_rank);

  HouseholderQR<MatrixAType> qra(a);
  HouseholderQR<MatrixBType> qrb(b);
  m = qra.householderQ() * d * qrb.householderQ();
}

/**
 * Generate random permutation vector.
 *
 * @tparam PermutationVectorType type of vector used to store permutation
 * @param v permutation vector
 * @param size length of permutation vector
 */
template <typename PermutationVectorType>
void randomPermutationVector(PermutationVectorType& v, Index size) {
  typedef typename PermutationVectorType::Scalar Scalar;
  v.resize(size);
  for (Index i = 0; i < size; ++i) v(i) = Scalar(i);
  if (size == 1) return;
  for (Index n = 0; n < 3 * size; ++n) {
    Index i = internal::random<Index>(0, size - 1);
    Index j;
    do j = internal::random<Index>(0, size - 1);
    while (j == i);
    std::swap(v(i), v(j));
  }
}

/**
 * Generate a random unitary matrix of prescribed dimension.
 *
 * The algorithm is using a random Householder sequence to produce
 * a random unitary matrix.
 *
 * @tparam MatrixType type of matrix to generate
 * @param dim row and column dimension of the requested square matrix
 * @return random unitary matrix
 */
template <typename MatrixType>
MatrixType generateRandomUnitaryMatrix(const Index dim) {
  typedef typename internal::traits<MatrixType>::Scalar Scalar;
  typedef Matrix<Scalar, Dynamic, 1> VectorType;

  MatrixType v = MatrixType::Identity(dim, dim);
  VectorType h = VectorType::Zero(dim);
  for (Index i = 0; i < dim; ++i) {
    v.col(i).tail(dim - i - 1) = VectorType::Random(dim - i - 1);
    h(i) = 2 / v.col(i).tail(dim - i).squaredNorm();
  }

  const Eigen::HouseholderSequence<MatrixType, VectorType> HSeq(v, h);
  return MatrixType(HSeq);
}

/**
 * Generation of random matrix with prescribed singular values.
 *
 * We generate random matrices with given singular values by setting up
 * a singular value decomposition. By choosing the number of zeros as
 * singular values we can specify the rank of the matrix.
 * Moreover, we also control its spectral norm, which is the largest
 * singular value, as well as its condition number with respect to the
 * l2-norm, which is the quotient of the largest and smallest singular
 * value.
 *
 * Reference: For details on the method see e.g. Section 8.1 (pp. 62 f) in
 *
 *   C. C. Paige, M. A. Saunders,
 *   LSQR: An algorithm for sparse linear equations and sparse least squares.
 *   ACM Transactions on Mathematical Software 8(1), pp. 43-71, 1982.
 *   https://web.stanford.edu/group/SOL/software/lsqr/lsqr-toms82a.pdf
 *
 * and also the LSQR webpage https://web.stanford.edu/group/SOL/software/lsqr/.
 *
 * @tparam MatrixType matrix type to generate
 * @tparam RealScalarVectorType vector type with real entries used for singular values
 * @param svs vector of desired singular values
 * @param rows row dimension of requested random matrix
 * @param cols column dimension of requested random matrix
 * @param M generated matrix with prescribed singular values
 */
template <typename MatrixType, typename RealScalarVectorType>
void generateRandomMatrixSvs(const RealScalarVectorType& svs, const Index rows, const Index cols, MatrixType& M) {
  enum { Rows = MatrixType::RowsAtCompileTime, Cols = MatrixType::ColsAtCompileTime };
  typedef typename internal::traits<MatrixType>::Scalar Scalar;
  typedef Matrix<Scalar, Rows, Rows> MatrixAType;
  typedef Matrix<Scalar, Cols, Cols> MatrixBType;

  const Index min_dim = (std::min)(rows, cols);

  const MatrixAType U = generateRandomUnitaryMatrix<MatrixAType>(rows);
  const MatrixBType V = generateRandomUnitaryMatrix<MatrixBType>(cols);

  M = U.block(0, 0, rows, min_dim) * svs.asDiagonal() * V.block(0, 0, cols, min_dim).transpose();
}

/**
 * Setup a vector of random singular values with prescribed upper limit.
 * For use with generateRandomMatrixSvs().
 *
 * Singular values are non-negative real values. By convention (to be consistent with
 * singular value decomposition) we sort them in decreasing order.
 *
 * This strategy produces random singular values in the range [0, max], in particular
 * the singular values can be zero or arbitrarily close to zero.
 *
 * @tparam VectorType vector type with real entries used for singular values
 * @tparam RealScalar data type used for real entry
 * @param dim number of singular values to generate
 * @param max upper bound for singular values
 * @return vector of singular values
 */
template <typename VectorType, typename RealScalar>
VectorType setupRandomSvs(const Index dim, const RealScalar max) {
  VectorType svs = max / RealScalar(2) * (VectorType::Random(dim) + VectorType::Ones(dim));
  std::sort(svs.begin(), svs.end(), std::greater<RealScalar>());
  return svs;
}

/**
 * Setup a vector of random singular values with prescribed range.
 * For use with generateRandomMatrixSvs().
 *
 * Singular values are non-negative real values. By convention (to be consistent with
 * singular value decomposition) we sort them in decreasing order.
 *
 * For dim > 1 this strategy generates a vector with largest entry max, smallest entry
 * min, and remaining entries in the range [min, max]. For dim == 1 the only entry is
 * min.
 *
 * @tparam VectorType vector type with real entries used for singular values
 * @tparam RealScalar data type used for real entry
 * @param dim number of singular values to generate
 * @param min smallest singular value to use
 * @param max largest singular value to use
 * @return vector of singular values
 */
template <typename VectorType, typename RealScalar>
VectorType setupRangeSvs(const Index dim, const RealScalar min, const RealScalar max) {
  VectorType svs = VectorType::Random(dim);
  if (dim == 0) return svs;
  if (dim == 1) {
    svs(0) = min;
    return svs;
  }
  std::sort(svs.begin(), svs.end(), std::greater<RealScalar>());

  // scale to range [min, max]
  const RealScalar c_min = svs(dim - 1), c_max = svs(0);
  svs = (svs - VectorType::Constant(dim, c_min)) / (c_max - c_min);
  return min * (VectorType::Ones(dim) - svs) + max * svs;
}

}  // end namespace Eigen

#endif  // EIGEN_RANDOM_MATRIX_HELPER
