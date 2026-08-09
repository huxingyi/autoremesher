#include <Eigen/Dense>
#include <iostream>

using Eigen::MatrixXf;

void copyUpperTriangularPart(MatrixXf& dst, const MatrixXf& src) {
  dst.triangularView<Eigen::Upper>() = src.triangularView<Eigen::Upper>();
}

int main() {
  MatrixXf m1 = MatrixXf::Ones(4, 4);
  MatrixXf m2 = MatrixXf::Random(4, 4);
  std::cout << "m2 before copy:" << std::endl;
  std::cout << m2 << std::endl << std::endl;
  copyUpperTriangularPart(m2, m1);
  std::cout << "m2 after copy:" << std::endl;
  std::cout << m2 << std::endl << std::endl;
}
