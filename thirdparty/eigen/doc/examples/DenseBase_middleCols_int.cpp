#include <Eigen/Core>
#include <iostream>

int main() {
  int const N = 5;
  Eigen::MatrixXi A(N, N);
  A.setRandom();
  std::cout << "A =\n" << A << '\n' << std::endl;
  std::cout << "A(1..3,:) =\n" << A.middleCols(1, 3) << std::endl;
  return 0;
}
