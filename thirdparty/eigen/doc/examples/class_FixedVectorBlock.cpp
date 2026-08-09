#include <Eigen/Core>
#include <iostream>

template <typename Derived>
Eigen::VectorBlock<Derived, 2> firstTwo(Eigen::MatrixBase<Derived>& v) {
  return Eigen::VectorBlock<Derived, 2>(v.derived(), 0);
}

template <typename Derived>
const Eigen::VectorBlock<const Derived, 2> firstTwo(const Eigen::MatrixBase<Derived>& v) {
  return Eigen::VectorBlock<const Derived, 2>(v.derived(), 0);
}

int main(int, char**) {
  Eigen::Matrix<int, 1, 6> v;
  v << 1, 2, 3, 4, 5, 6;
  std::cout << firstTwo(4 * v) << std::endl;  // calls the const version
  firstTwo(v) *= 2;                           // calls the non-const version
  std::cout << "Now the vector v is:" << std::endl << v << std::endl;
  return 0;
}
