#include <Eigen/Dense>
#include <iostream>

using Eigen::MatrixXf;

int main() {
  MatrixXf m(2, 2);
  MatrixXf n(2, 2);
  MatrixXf result(2, 2);

  m << 1, 2, 3, 4;
  n << 5, 6, 7, 8;

  result = m * n;
  std::cout << "-- Matrix m*n: --\n" << result << "\n\n";
  result = m.array() * n.array();
  std::cout << "-- Array m*n: --\n" << result << "\n\n";
  result = m.cwiseProduct(n);
  std::cout << "-- With cwiseProduct: --\n" << result << "\n\n";
  result = m.array() + 4;
  std::cout << "-- Array m + 4: --\n" << result << "\n\n";
}
