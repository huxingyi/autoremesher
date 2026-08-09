#include <Eigen/Core>
#include <iostream>

template <typename Derived>
const Eigen::Reshaped<const Derived> reshape_helper(const Eigen::MatrixBase<Derived>& m, int rows, int cols) {
  return Eigen::Reshaped<const Derived>(m.derived(), rows, cols);
}

int main(int, char**) {
  Eigen::MatrixXd m(3, 4);
  m << 1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12;
  std::cout << m << std::endl;
  Eigen::Ref<const Eigen::MatrixXd> n = reshape_helper(m, 2, 6);
  std::cout << "Matrix m is:" << std::endl << m << std::endl;
  std::cout << "Matrix n is:" << std::endl << n << std::endl;
}
