#define EIGEN_NO_DEBUG_SMALL_PRODUCT_BLOCKS
#include "sparse_solver.h"

#if defined(DEBUG)
#undef DEBUG
#endif

#include <Eigen/AccelerateSupport>

template <typename MatrixType, typename DenseMat>
int generate_sparse_rectangular_problem(MatrixType& A, DenseMat& dA, int maxRows = 300, int maxCols = 300) {
  typedef typename MatrixType::Scalar Scalar;
  int rows = internal::random<int>(1, maxRows);
  int cols = internal::random<int>(1, maxCols);
  double density = (std::max)(8.0 / (rows * cols), 0.01);

  A.resize(rows, cols);
  dA.resize(rows, cols);
  initSparse<Scalar>(density, dA, A, ForceNonZeroDiag);
  A.makeCompressed();
  return rows;
}

template <typename MatrixType, typename DenseMat>
int generate_sparse_square_symmetric_problem(MatrixType& A, DenseMat& dA, int maxSize = 300) {
  typedef typename MatrixType::Scalar Scalar;
  int rows = internal::random<int>(1, maxSize);
  int cols = rows;
  double density = (std::max)(8.0 / (rows * cols), 0.01);

  A.resize(rows, cols);
  dA.resize(rows, cols);
  initSparse<Scalar>(density, dA, A, ForceNonZeroDiag);
  dA = dA * dA.transpose();
  A = A * A.transpose();
  A.makeCompressed();
  return rows;
}

template <typename Scalar, typename Solver>
void test_accelerate_ldlt() {
  typedef SparseMatrix<Scalar> MatrixType;
  typedef Matrix<Scalar, Dynamic, 1> DenseVector;

  MatrixType A;
  Matrix<Scalar, Dynamic, Dynamic> dA;

  generate_sparse_square_symmetric_problem(A, dA);

  DenseVector b = DenseVector::Random(A.rows());

  Solver solver;
  solver.compute(A);

  if (solver.info() != Success) {
    std::cerr << "sparse LDLT factorization failed\n";
    exit(0);
    return;
  }

  DenseVector x = solver.solve(b);

  if (solver.info() != Success) {
    std::cerr << "sparse LDLT factorization failed\n";
    exit(0);
    return;
  }

  // Compare with a dense solver
  DenseVector refX = dA.ldlt().solve(b);
  VERIFY((A * x).isApprox(A * refX, test_precision<Scalar>()));
}

template <typename Scalar, typename Solver>
void test_accelerate_llt() {
  typedef SparseMatrix<Scalar> MatrixType;
  typedef Matrix<Scalar, Dynamic, 1> DenseVector;

  MatrixType A;
  Matrix<Scalar, Dynamic, Dynamic> dA;

  generate_sparse_square_symmetric_problem(A, dA);

  DenseVector b = DenseVector::Random(A.rows());

  Solver solver;
  solver.compute(A);

  if (solver.info() != Success) {
    std::cerr << "sparse LLT factorization failed\n";
    exit(0);
    return;
  }

  DenseVector x = solver.solve(b);

  if (solver.info() != Success) {
    std::cerr << "sparse LLT factorization failed\n";
    exit(0);
    return;
  }

  // Compare with a dense solver
  DenseVector refX = dA.llt().solve(b);
  VERIFY((A * x).isApprox(A * refX, test_precision<Scalar>()));
}

template <typename Scalar, typename Solver>
void test_accelerate_qr() {
  typedef SparseMatrix<Scalar> MatrixType;
  typedef Matrix<Scalar, Dynamic, 1> DenseVector;

  MatrixType A;
  Matrix<Scalar, Dynamic, Dynamic> dA;

  generate_sparse_rectangular_problem(A, dA);

  DenseVector b = DenseVector::Random(A.rows());

  Solver solver;
  solver.compute(A);

  if (solver.info() != Success) {
    std::cerr << "sparse QR factorization failed\n";
    exit(0);
    return;
  }

  DenseVector x = solver.solve(b);

  if (solver.info() != Success) {
    std::cerr << "sparse QR factorization failed\n";
    exit(0);
    return;
  }

  // Compare with a dense solver
  DenseVector refX = dA.colPivHouseholderQr().solve(b);
  VERIFY((A * x).isApprox(A * refX, test_precision<Scalar>()));
}

template <typename Scalar>
void run_tests() {
  typedef SparseMatrix<Scalar> MatrixType;

  test_accelerate_ldlt<Scalar, AccelerateLDLT<MatrixType, Lower> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTUnpivoted<MatrixType, Lower> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTSBK<MatrixType, Lower> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTTPP<MatrixType, Lower> >();

  test_accelerate_ldlt<Scalar, AccelerateLDLT<MatrixType, Upper> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTUnpivoted<MatrixType, Upper> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTSBK<MatrixType, Upper> >();
  test_accelerate_ldlt<Scalar, AccelerateLDLTTPP<MatrixType, Upper> >();

  test_accelerate_llt<Scalar, AccelerateLLT<MatrixType, Lower> >();

  test_accelerate_llt<Scalar, AccelerateLLT<MatrixType, Upper> >();

  test_accelerate_qr<Scalar, AccelerateQR<MatrixType> >();
}

EIGEN_DECLARE_TEST(accelerate_support) {
  CALL_SUBTEST_1(run_tests<float>());
  CALL_SUBTEST_2(run_tests<double>());
}
