#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix3f A;
  A << 1, 2, 1, 2, 1, 0, -1, 1, 2;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  std::cout << "The determinant of A is " << A.determinant() << std::endl;
  std::cout << "The inverse of A is:\n" << A.inverse() << std::endl;
}
