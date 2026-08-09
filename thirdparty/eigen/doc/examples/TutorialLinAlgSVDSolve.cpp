#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 2);
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  Eigen::VectorXf b = Eigen::VectorXf::Random(3);
  std::cout << "Here is the right hand side b:\n" << b << std::endl;
  std::cout << "The least-squares solution is:\n"
            << A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b) << std::endl;
}
