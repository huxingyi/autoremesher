#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix2d A;
  A << 2, 1, 2, 0.9999999999;
  Eigen::FullPivLU<Eigen::Matrix2d> lu(A);
  std::cout << "By default, the rank of A is found to be " << lu.rank() << std::endl;
  lu.setThreshold(1e-5);
  std::cout << "With threshold 1e-5, the rank of A is found to be " << lu.rank() << std::endl;
}
