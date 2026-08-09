#include <Eigen/Core>
#include <iostream>

template <typename Derived>
Eigen::Block<Derived, 2, 2> topLeft2x2Corner(Eigen::MatrixBase<Derived>& m) {
  return Eigen::Block<Derived, 2, 2>(m.derived(), 0, 0);
}

template <typename Derived>
const Eigen::Block<const Derived, 2, 2> topLeft2x2Corner(const Eigen::MatrixBase<Derived>& m) {
  return Eigen::Block<const Derived, 2, 2>(m.derived(), 0, 0);
}

int main(int, char**) {
  Eigen::Matrix3d m = Eigen::Matrix3d::Identity();
  std::cout << topLeft2x2Corner(4 * m) << std::endl;  // calls the const version
  topLeft2x2Corner(m) *= 2;                           // calls the non-const version
  std::cout << "Now the matrix m is:" << std::endl << m << std::endl;
  return 0;
}
