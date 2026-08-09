#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::MatrixXf m(2, 4);
  Eigen::VectorXf v(2);

  m << 1, 23, 6, 9, 3, 11, 7, 2;

  v << 2, 3;

  Eigen::Index index;
  // find nearest neighbour
  (m.colwise() - v).colwise().squaredNorm().minCoeff(&index);

  std::cout << "Nearest neighbour is column " << index << ":" << std::endl;
  std::cout << m.col(index) << std::endl;
}
