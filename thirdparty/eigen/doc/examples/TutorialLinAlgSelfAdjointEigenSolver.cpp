#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix2f A;
  A << 1, 2, 2, 3;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> eigensolver(A);
  if (eigensolver.info() != Eigen::Success) abort();
  std::cout << "The eigenvalues of A are:\n" << eigensolver.eigenvalues() << std::endl;
  std::cout << "Here's a matrix whose columns are eigenvectors of A \n"
            << "corresponding to these eigenvalues:\n"
            << eigensolver.eigenvectors() << std::endl;
}
