#include <Eigen/Core>
#include <iostream>

template <typename Derived>
Eigen::Block<Derived> topLeftCorner(Eigen::MatrixBase<Derived>& m, int rows, int cols) {
  return Eigen::Block<Derived>(m.derived(), 0, 0, rows, cols);
}

template <typename Derived>
const Eigen::Block<const Derived> topLeftCorner(const Eigen::MatrixBase<Derived>& m, int rows, int cols) {
  return Eigen::Block<const Derived>(m.derived(), 0, 0, rows, cols);
}

int main(int, char**) {
  Eigen::Matrix4d m = Eigen::Matrix4d::Identity();
  std::cout << topLeftCorner(4 * m, 2, 3) << std::endl;  // calls the const version
  topLeftCorner(m, 2, 3) *= 5;                           // calls the non-const version
  std::cout << "Now the matrix m is:" << std::endl << m << std::endl;
  return 0;
}
