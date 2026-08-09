#include <iostream>
#include <Eigen/Dense>

using Eigen::MatrixXd;

int main() {
  MatrixXd A = MatrixXd::Random(100, 100);
  MatrixXd b = MatrixXd::Random(100, 50);
  MatrixXd x = A.fullPivLu().solve(b);
  double relative_error = (A * x - b).norm() / b.norm();  // norm() is L2 norm
  std::cout << "The relative error is:\n" << relative_error << std::endl;
}
