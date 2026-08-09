#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix2f A, b;
  Eigen::LLT<Eigen::Matrix2f> llt;
  A << 2, -1, -1, 3;
  b << 1, 2, 3, 1;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  std::cout << "Here is the right hand side b:\n" << b << std::endl;
  std::cout << "Computing LLT decomposition..." << std::endl;
  llt.compute(A);
  std::cout << "The solution is:\n" << llt.solve(b) << std::endl;
  A(1, 1)++;
  std::cout << "The matrix A is now:\n" << A << std::endl;
  std::cout << "Computing LLT decomposition..." << std::endl;
  llt.compute(A);
  std::cout << "The solution is now:\n" << llt.solve(b) << std::endl;
}
