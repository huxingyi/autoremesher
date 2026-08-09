#include <Eigen/Dense>
#include <iostream>

int main() {
  Eigen::Array22f m;
  m << 1, 2, 3, 4;
  Eigen::Array44f a = Eigen::Array44f::Constant(0.6);
  std::cout << "Here is the array a:\n" << a << "\n\n";
  a.block<2, 2>(1, 1) = m;
  std::cout << "Here is now a with m copied into its central 2x2 block:\n" << a << "\n\n";
  a.block(0, 0, 2, 3) = a.block(2, 1, 2, 3);
  std::cout << "Here is now a with bottom-right 2x3 block copied into top-left 2x3 block:\n" << a << "\n\n";
}
