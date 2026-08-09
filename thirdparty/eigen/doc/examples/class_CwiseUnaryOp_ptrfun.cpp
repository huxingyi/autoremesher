#include <Eigen/Core>
#include <iostream>

// define function to be applied coefficient-wise
double ramp(double x) {
  if (x > 0)
    return x;
  else
    return 0;
}

int main(int, char**) {
  Eigen::Matrix4d m1 = Eigen::Matrix4d::Random();
  std::cout << m1 << std::endl << "becomes: " << std::endl << m1.unaryExpr(std::ptr_fun(ramp)) << std::endl;
  return 0;
}
