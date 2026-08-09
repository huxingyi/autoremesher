#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix2f A, b;
  A << 2, -1, -1, 3;
  b << 1, 2, 3, 1;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  std::cout << "Here is the right hand side b:\n" << b << std::endl;
  Eigen::Matrix2f x = A.ldlt().solve(b);
  std::cout << "The solution is:\n" << x << std::endl;
}
