#include <Eigen/Dense>
#include <iostream>

int main() {
  Eigen::ArrayXXf m(2, 2);

  // assign some values coefficient by coefficient
  m(0, 0) = 1.0;
  m(0, 1) = 2.0;
  m(1, 0) = 3.0;
  m(1, 1) = m(0, 1) + m(1, 0);

  // print values to standard output
  std::cout << m << std::endl << std::endl;

  // using the comma-initializer is also allowed
  m << 1.0, 2.0, 3.0, 4.0;

  // print values to standard output
  std::cout << m << std::endl;
}
