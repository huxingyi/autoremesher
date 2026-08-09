#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix3f A;
  Eigen::Vector3f b;
  A << 1, 2, 3, 4, 5, 6, 7, 8, 10;
  b << 3, 3, 4;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  std::cout << "Here is the vector b:\n" << b << std::endl;
  Eigen::Vector3f x = A.colPivHouseholderQr().solve(b);
  std::cout << "The solution is:\n" << x << std::endl;
}
