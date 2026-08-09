#include <iostream>
#include <Eigen/SVD>

float inv_cond(const Eigen::Ref<const Eigen::MatrixXf>& a) {
  const Eigen::VectorXf sing_vals = a.jacobiSvd().singularValues();
  return sing_vals(sing_vals.size() - 1) / sing_vals(0);
}

int main() {
  Eigen::MatrixXf m = Eigen::MatrixXf::Random(4, 4);
  std::cout << "matrix m:\n" << m << "\n\n";
  std::cout << "inv_cond(m):          " << inv_cond(m) << "\n";
  std::cout << "inv_cond(m(1:3,1:3)): " << inv_cond(m.topLeftCorner(3, 3)) << "\n";
  std::cout << "inv_cond(m+I):        " << inv_cond(m + Eigen::MatrixXf::Identity(4, 4)) << "\n";
}
