#include <Eigen/Dense>
#include <iostream>

template <typename Derived1, typename Derived2>
void copyUpperTriangularPart(Eigen::MatrixBase<Derived1>& dst, const Eigen::MatrixBase<Derived2>& src) {
  /* Note the 'template' keywords in the following line! */
  dst.template triangularView<Eigen::Upper>() = src.template triangularView<Eigen::Upper>();
}

int main() {
  Eigen::MatrixXi m1 = Eigen::MatrixXi::Ones(5, 5);
  Eigen::MatrixXi m2 = Eigen::MatrixXi::Random(4, 4);
  std::cout << "m2 before copy:" << std::endl;
  std::cout << m2 << std::endl << std::endl;
  copyUpperTriangularPart(m2, m1.topLeftCorner(4, 4));
  std::cout << "m2 after copy:" << std::endl;
  std::cout << m2 << std::endl << std::endl;
}
