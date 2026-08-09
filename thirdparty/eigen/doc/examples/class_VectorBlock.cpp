#include <Eigen/Core>
#include <iostream>

template <typename Derived>
Eigen::VectorBlock<Derived> segmentFromRange(Eigen::MatrixBase<Derived>& v, int start, int end) {
  return Eigen::VectorBlock<Derived>(v.derived(), start, end - start);
}

template <typename Derived>
const Eigen::VectorBlock<const Derived> segmentFromRange(const Eigen::MatrixBase<Derived>& v, int start, int end) {
  return Eigen::VectorBlock<const Derived>(v.derived(), start, end - start);
}

int main(int, char**) {
  Eigen::Matrix<int, 1, 6> v;
  v << 1, 2, 3, 4, 5, 6;
  std::cout << segmentFromRange(2 * v, 2, 4) << std::endl;  // calls the const version
  segmentFromRange(v, 1, 3) *= 5;                           // calls the non-const version
  std::cout << "Now the vector v is:" << std::endl << v << std::endl;
  return 0;
}
