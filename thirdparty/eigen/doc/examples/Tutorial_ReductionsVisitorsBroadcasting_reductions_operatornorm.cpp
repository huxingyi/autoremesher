#include <Eigen/Dense>
#include <iostream>

int main() {
  Eigen::MatrixXf m(2, 2);
  m << 1, -2, -3, 4;

  std::cout << "1-norm(m)     = " << m.cwiseAbs().colwise().sum().maxCoeff()
            << " == " << m.colwise().lpNorm<1>().maxCoeff() << std::endl;

  std::cout << "infty-norm(m) = " << m.cwiseAbs().rowwise().sum().maxCoeff()
            << " == " << m.rowwise().lpNorm<1>().maxCoeff() << std::endl;
}
