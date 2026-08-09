#include <iostream>
#include <Eigen/Core>

template <typename Derived>
void print_size(const Eigen::EigenBase<Derived>& b) {
  std::cout << "size (rows, cols): " << b.size() << " (" << b.rows() << ", " << b.cols() << ")" << std::endl;
}

int main() {
  Eigen::Vector3f v;
  print_size(v);
  // v.asDiagonal() returns a 3x3 diagonal matrix pseudo-expression
  print_size(v.asDiagonal());
}
