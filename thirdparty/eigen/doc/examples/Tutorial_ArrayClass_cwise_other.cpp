#include <Eigen/Dense>
#include <iostream>

int main() {
  Eigen::ArrayXf a = Eigen::ArrayXf::Random(5);
  a *= 2;
  std::cout << "a =" << std::endl << a << std::endl;
  std::cout << "a.abs() =" << std::endl << a.abs() << std::endl;
  std::cout << "a.abs().sqrt() =" << std::endl << a.abs().sqrt() << std::endl;
  std::cout << "a.min(a.abs().sqrt()) =" << std::endl << a.min(a.abs().sqrt()) << std::endl;
}
