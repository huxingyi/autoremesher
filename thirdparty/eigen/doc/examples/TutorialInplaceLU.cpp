#include <iostream>
#include <Eigen/Dense>

int main() {
  std::cout << "[init]\n";
  Eigen::MatrixXd A(2, 2);
  A << 2, -1, 1, 3;
  std::cout << "Here is the input matrix A before decomposition:\n" << A << "\n";
  std::cout << "[init]\n";

  std::cout << "[declaration]\n";
  Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd> > lu(A);
  std::cout << "Here is the input matrix A after decomposition:\n" << A << "\n";
  std::cout << "[declaration]\n";

  std::cout << "[matrixLU]\n";
  std::cout << "Here is the matrix storing the L and U factors:\n" << lu.matrixLU() << "\n";
  std::cout << "[matrixLU]\n";

  std::cout << "[solve]\n";
  Eigen::MatrixXd A0(2, 2);
  A0 << 2, -1, 1, 3;
  Eigen::VectorXd b(2);
  b << 1, 2;
  Eigen::VectorXd x = lu.solve(b);
  std::cout << "Residual: " << (A0 * x - b).norm() << "\n";
  std::cout << "[solve]\n";

  std::cout << "[modifyA]\n";
  A << 3, 4, -2, 1;
  x = lu.solve(b);
  std::cout << "Residual: " << (A0 * x - b).norm() << "\n";
  std::cout << "[modifyA]\n";

  std::cout << "[recompute]\n";
  A0 = A;  // save A
  lu.compute(A);
  x = lu.solve(b);
  std::cout << "Residual: " << (A0 * x - b).norm() << "\n";
  std::cout << "[recompute]\n";

  std::cout << "[recompute_bis0]\n";
  Eigen::MatrixXd A1(2, 2);
  A1 << 5, -2, 3, 4;
  lu.compute(A1);
  std::cout << "Here is the input matrix A1 after decomposition:\n" << A1 << "\n";
  std::cout << "[recompute_bis0]\n";

  std::cout << "[recompute_bis1]\n";
  x = lu.solve(b);
  std::cout << "Residual: " << (A1 * x - b).norm() << "\n";
  std::cout << "[recompute_bis1]\n";
}
