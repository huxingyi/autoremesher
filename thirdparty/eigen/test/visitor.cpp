// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "main.h"

template <typename MatrixType>
void matrixVisitor_impl(MatrixType& m) {
  typedef typename MatrixType::Scalar Scalar;

  Index rows = m.rows();
  Index cols = m.cols();

  Scalar minc = Scalar(1000), maxc = Scalar(-1000);
  Index minrow = 0, mincol = 0, maxrow = 0, maxcol = 0;
  for (Index j = 0; j < cols; j++)
    for (Index i = 0; i < rows; i++) {
      if (m(i, j) < minc) {
        minc = m(i, j);
        minrow = i;
        mincol = j;
      }
      if (m(i, j) > maxc) {
        maxc = m(i, j);
        maxrow = i;
        maxcol = j;
      }
    }
  Index eigen_minrow, eigen_mincol, eigen_maxrow, eigen_maxcol;
  Scalar eigen_minc, eigen_maxc;
  eigen_minc = m.minCoeff(&eigen_minrow, &eigen_mincol);
  eigen_maxc = m.maxCoeff(&eigen_maxrow, &eigen_maxcol);
  VERIFY(minrow == eigen_minrow);
  VERIFY(maxrow == eigen_maxrow);
  VERIFY(mincol == eigen_mincol);
  VERIFY(maxcol == eigen_maxcol);
  VERIFY_IS_APPROX(minc, eigen_minc);
  VERIFY_IS_APPROX(maxc, eigen_maxc);
  VERIFY_IS_APPROX(minc, m.minCoeff());
  VERIFY_IS_APPROX(maxc, m.maxCoeff());

  eigen_maxc = (m.adjoint() * m).maxCoeff(&eigen_maxrow, &eigen_maxcol);
  Index maxrow2 = 0, maxcol2 = 0;
  eigen_maxc = (m.adjoint() * m).eval().maxCoeff(&maxrow2, &maxcol2);
  VERIFY(maxrow2 == eigen_maxrow);
  VERIFY(maxcol2 == eigen_maxcol);

  if (!NumTraits<Scalar>::IsInteger && m.size() > 2) {
    // Test NaN propagation by replacing an element with NaN.
    bool stop = false;
    for (Index j = 0; j < cols && !stop; ++j) {
      for (Index i = 0; i < rows && !stop; ++i) {
        if (!(j == mincol && i == minrow) && !(j == maxcol && i == maxrow)) {
          m(i, j) = NumTraits<Scalar>::quiet_NaN();
          stop = true;
          break;
        }
      }
    }

    eigen_minc = m.template minCoeff<PropagateNumbers>(&eigen_minrow, &eigen_mincol);
    eigen_maxc = m.template maxCoeff<PropagateNumbers>(&eigen_maxrow, &eigen_maxcol);
    VERIFY(minrow == eigen_minrow);
    VERIFY(maxrow == eigen_maxrow);
    VERIFY(mincol == eigen_mincol);
    VERIFY(maxcol == eigen_maxcol);
    VERIFY_IS_APPROX(minc, eigen_minc);
    VERIFY_IS_APPROX(maxc, eigen_maxc);
    VERIFY_IS_APPROX(minc, m.template minCoeff<PropagateNumbers>());
    VERIFY_IS_APPROX(maxc, m.template maxCoeff<PropagateNumbers>());

    eigen_minc = m.template minCoeff<PropagateNaN>(&eigen_minrow, &eigen_mincol);
    eigen_maxc = m.template maxCoeff<PropagateNaN>(&eigen_maxrow, &eigen_maxcol);
    VERIFY(minrow != eigen_minrow || mincol != eigen_mincol);
    VERIFY(maxrow != eigen_maxrow || maxcol != eigen_maxcol);
    VERIFY((numext::isnan)(eigen_minc));
    VERIFY((numext::isnan)(eigen_maxc));

    // Test matrix of all NaNs.
    m.fill(NumTraits<Scalar>::quiet_NaN());
    eigen_minc = m.template minCoeff<PropagateNumbers>(&eigen_minrow, &eigen_mincol);
    eigen_maxc = m.template maxCoeff<PropagateNumbers>(&eigen_maxrow, &eigen_maxcol);
    VERIFY(eigen_minrow == 0);
    VERIFY(eigen_maxrow == 0);
    VERIFY(eigen_mincol == 0);
    VERIFY(eigen_maxcol == 0);
    VERIFY((numext::isnan)(eigen_minc));
    VERIFY((numext::isnan)(eigen_maxc));

    eigen_minc = m.template minCoeff<PropagateNaN>(&eigen_minrow, &eigen_mincol);
    eigen_maxc = m.template maxCoeff<PropagateNaN>(&eigen_maxrow, &eigen_maxcol);
    VERIFY(eigen_minrow == 0);
    VERIFY(eigen_maxrow == 0);
    VERIFY(eigen_mincol == 0);
    VERIFY(eigen_maxcol == 0);
    VERIFY((numext::isnan)(eigen_minc));
    VERIFY((numext::isnan)(eigen_maxc));

    eigen_minc = m.template minCoeff<PropagateFast>(&eigen_minrow, &eigen_mincol);
    eigen_maxc = m.template maxCoeff<PropagateFast>(&eigen_maxrow, &eigen_maxcol);
    VERIFY(eigen_minrow == 0);
    VERIFY(eigen_maxrow == 0);
    VERIFY(eigen_mincol == 0);
    VERIFY(eigen_maxcol == 0);
    VERIFY((numext::isnan)(eigen_minc));
    VERIFY((numext::isnan)(eigen_maxc));
  }
}
template <typename MatrixType>
void matrixVisitor(const MatrixType& p) {
  MatrixType m(p.rows(), p.cols());
  // construct a random matrix where all coefficients are different
  m.setRandom();
  for (Index i = 0; i < m.size(); i++)
    for (Index i2 = 0; i2 < i; i2++)
      while (numext::equal_strict(m(i), m(i2)))  // yes, strict equality
        m(i) = internal::random<typename DenseBase<MatrixType>::Scalar>();
  MatrixType n = m;
  matrixVisitor_impl(m);
  // force outer-inner access pattern
  using BlockType = Block<MatrixType, Dynamic, Dynamic>;
  BlockType m_block = n.block(0, 0, n.rows(), n.cols());
  matrixVisitor_impl(m_block);
}

template <typename VectorType>
void vectorVisitor(const VectorType& w) {
  typedef typename VectorType::Scalar Scalar;

  Index size = w.size();

  // construct a random vector where all coefficients are different
  VectorType v;
  v = VectorType::Random(size);
  for (Index i = 0; i < size; i++)
    for (Index i2 = 0; i2 < i; i2++)
      while (v(i) == v(i2))  // yes, ==
        v(i) = internal::random<Scalar>();

  Scalar minc = v(0), maxc = v(0);
  Index minidx = 0, maxidx = 0;
  for (Index i = 0; i < size; i++) {
    if (v(i) < minc) {
      minc = v(i);
      minidx = i;
    }
    if (v(i) > maxc) {
      maxc = v(i);
      maxidx = i;
    }
  }
  Index eigen_minidx, eigen_maxidx;
  Scalar eigen_minc, eigen_maxc;
  eigen_minc = v.minCoeff(&eigen_minidx);
  eigen_maxc = v.maxCoeff(&eigen_maxidx);
  VERIFY(minidx == eigen_minidx);
  VERIFY(maxidx == eigen_maxidx);
  VERIFY_IS_APPROX(minc, eigen_minc);
  VERIFY_IS_APPROX(maxc, eigen_maxc);
  VERIFY_IS_APPROX(minc, v.minCoeff());
  VERIFY_IS_APPROX(maxc, v.maxCoeff());

  Index idx0 = internal::random<Index>(0, size - 1);
  Index idx1 = eigen_minidx;
  Index idx2 = eigen_maxidx;
  VectorType v1(v), v2(v);
  v1(idx0) = v1(idx1);
  v2(idx0) = v2(idx2);
  v1.minCoeff(&eigen_minidx);
  v2.maxCoeff(&eigen_maxidx);
  VERIFY(eigen_minidx == (std::min)(idx0, idx1));
  VERIFY(eigen_maxidx == (std::min)(idx0, idx2));

  if (!NumTraits<Scalar>::IsInteger && size > 2) {
    // Test NaN propagation by replacing an element with NaN.
    for (Index i = 0; i < size; ++i) {
      if (i != minidx && i != maxidx) {
        v(i) = NumTraits<Scalar>::quiet_NaN();
        break;
      }
    }
    eigen_minc = v.template minCoeff<PropagateNumbers>(&eigen_minidx);
    eigen_maxc = v.template maxCoeff<PropagateNumbers>(&eigen_maxidx);
    VERIFY(minidx == eigen_minidx);
    VERIFY(maxidx == eigen_maxidx);
    VERIFY_IS_APPROX(minc, eigen_minc);
    VERIFY_IS_APPROX(maxc, eigen_maxc);
    VERIFY_IS_APPROX(minc, v.template minCoeff<PropagateNumbers>());
    VERIFY_IS_APPROX(maxc, v.template maxCoeff<PropagateNumbers>());

    eigen_minc = v.template minCoeff<PropagateNaN>(&eigen_minidx);
    eigen_maxc = v.template maxCoeff<PropagateNaN>(&eigen_maxidx);
    VERIFY(minidx != eigen_minidx);
    VERIFY(maxidx != eigen_maxidx);
    VERIFY((numext::isnan)(eigen_minc));
    VERIFY((numext::isnan)(eigen_maxc));
  }
}

template <typename Derived, bool Vectorizable>
struct TrackedVisitor {
  using Scalar = typename DenseBase<Derived>::Scalar;
  static constexpr int PacketSize = Eigen::internal::packet_traits<Scalar>::size;
  static constexpr bool RowMajor = Derived::IsRowMajor;

  void init(Scalar v, Index i, Index j) { return this->operator()(v, i, j); }
  template <typename Packet>
  void initpacket(Packet p, Index i, Index j) {
    return this->packet(p, i, j);
  }
  void operator()(Scalar v, Index i, Index j) {
    EIGEN_UNUSED_VARIABLE(v)
    visited.emplace_back(i, j);
    scalarOps++;
  }

  template <typename Packet>
  void packet(Packet p, Index i, Index j) {
    EIGEN_UNUSED_VARIABLE(p)
    for (int k = 0; k < PacketSize; k++)
      if (RowMajor)
        visited.emplace_back(i, j + k);
      else
        visited.emplace_back(i + k, j);
    vectorOps++;
  }
  std::vector<std::pair<Index, Index>> visited;
  Index scalarOps = 0;
  Index vectorOps = 0;
};

namespace Eigen {
namespace internal {

template <typename T, bool Vectorizable>
struct functor_traits<TrackedVisitor<T, Vectorizable>> {
  enum { PacketAccess = Vectorizable, LinearAccess = false, Cost = 1 };
};

}  // namespace internal
}  // namespace Eigen

template <typename Derived, bool Vectorized>
void checkOptimalTraversal_impl(const DenseBase<Derived>& mat) {
  using Scalar = typename DenseBase<Derived>::Scalar;
  static constexpr int PacketSize = Eigen::internal::packet_traits<Scalar>::size;
  static constexpr bool RowMajor = Derived::IsRowMajor;
  Derived X(mat.rows(), mat.cols());
  X.setRandom();
  TrackedVisitor<Derived, Vectorized> visitor;
  visitor.visited.reserve(X.size());
  X.visit(visitor);
  Index count = 0;
  for (Index j = 0; j < X.outerSize(); ++j) {
    for (Index i = 0; i < X.innerSize(); ++i) {
      Index r = RowMajor ? j : i;
      Index c = RowMajor ? i : j;
      VERIFY_IS_EQUAL(visitor.visited[count].first, r);
      VERIFY_IS_EQUAL(visitor.visited[count].second, c);
      ++count;
    }
  }
  Index vectorOps = Vectorized ? ((X.innerSize() / PacketSize) * X.outerSize()) : 0;
  Index scalarOps = X.size() - (vectorOps * PacketSize);
  VERIFY_IS_EQUAL(vectorOps, visitor.vectorOps);
  VERIFY_IS_EQUAL(scalarOps, visitor.scalarOps);
}

void checkOptimalTraversal() {
  using Scalar = float;
  constexpr int PacketSize = Eigen::internal::packet_traits<Scalar>::size;
  // use sizes that mix vector and scalar ops
  constexpr int Rows = 3 * PacketSize + 1;
  constexpr int Cols = 4 * PacketSize + 1;
  int rows = internal::random(PacketSize + 1, EIGEN_TEST_MAX_SIZE);
  int cols = internal::random(PacketSize + 1, EIGEN_TEST_MAX_SIZE);

  using UnrollColMajor = Matrix<Scalar, Rows, Cols, ColMajor>;
  using UnrollRowMajor = Matrix<Scalar, Rows, Cols, RowMajor>;
  using DynamicColMajor = Matrix<Scalar, Dynamic, Dynamic, ColMajor>;
  using DynamicRowMajor = Matrix<Scalar, Dynamic, Dynamic, RowMajor>;

  // Scalar-only visitors
  checkOptimalTraversal_impl<UnrollColMajor, false>(UnrollColMajor(Rows, Cols));
  checkOptimalTraversal_impl<UnrollRowMajor, false>(UnrollRowMajor(Rows, Cols));
  checkOptimalTraversal_impl<DynamicColMajor, false>(DynamicColMajor(rows, cols));
  checkOptimalTraversal_impl<DynamicRowMajor, false>(DynamicRowMajor(rows, cols));

  // Vectorized visitors
  checkOptimalTraversal_impl<UnrollColMajor, true>(UnrollColMajor(Rows, Cols));
  checkOptimalTraversal_impl<UnrollRowMajor, true>(UnrollRowMajor(Rows, Cols));
  checkOptimalTraversal_impl<DynamicColMajor, true>(DynamicColMajor(rows, cols));
  checkOptimalTraversal_impl<DynamicRowMajor, true>(DynamicRowMajor(rows, cols));

  const Eigen::Array<bool, Eigen::Dynamic, 1> a = Eigen::Array<bool, 2, 1>{false, true};
  Eigen::Index i = -1;

  VERIFY(!a.minCoeff(&i));
  VERIFY(i == 0);

  VERIFY(!(!a).minCoeff(&i));
  VERIFY(i == 1);

  Eigen::Index j = -1;

  VERIFY(a.maxCoeff(&j));
  VERIFY(j == 1);

  VERIFY((!a).maxCoeff(&j));
  VERIFY(j == 0);
}

EIGEN_DECLARE_TEST(visitor) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(matrixVisitor(Matrix<float, 1, 1>()));
    CALL_SUBTEST_2(matrixVisitor(Matrix2f()));
    CALL_SUBTEST_3(matrixVisitor(Matrix4d()));
    CALL_SUBTEST_4(matrixVisitor(MatrixXd(8, 12)));
    CALL_SUBTEST_5(matrixVisitor(Matrix<double, Dynamic, Dynamic, RowMajor>(20, 20)));
    CALL_SUBTEST_6(matrixVisitor(MatrixXi(8, 12)));
  }
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_7(vectorVisitor(Vector4f()));
    CALL_SUBTEST_7(vectorVisitor(Matrix<int, 12, 1>()));
    CALL_SUBTEST_8(vectorVisitor(VectorXd(10)));
    CALL_SUBTEST_9(vectorVisitor(RowVectorXd(10)));
    CALL_SUBTEST_10(vectorVisitor(VectorXf(33)));
  }
  CALL_SUBTEST_11(checkOptimalTraversal());
}
