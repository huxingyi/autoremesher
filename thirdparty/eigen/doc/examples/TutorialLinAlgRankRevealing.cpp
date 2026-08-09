#include <iostream>
#include <Eigen/Dense>

int main() {
  Eigen::Matrix3f A;
  A << 1, 2, 5, 2, 1, 4, 3, 0, 3;
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  Eigen::FullPivLU<Eigen::Matrix3f> lu_decomp(A);
  std::cout << "The rank of A is " << lu_decomp.rank() << std::endl;
  std::cout << "Here is a matrix whose columns form a basis of the null-space of A:\n"
            << lu_decomp.kernel() << std::endl;
  std::cout << "Here is a matrix whose columns form a basis of the column-space of A:\n"
            << lu_decomp.image(A) << std::endl;  // yes, have to pass the original A
}
