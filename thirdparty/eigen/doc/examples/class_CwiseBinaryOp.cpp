#include <Eigen/Core>
#include <iostream>

using Eigen::Matrix4d;

// define a custom template binary functor
template <typename Scalar>
struct MakeComplexOp {
  typedef std::complex<Scalar> result_type;
  result_type operator()(const Scalar& a, const Scalar& b) const { return result_type(a, b); }
};

int main(int, char**) {
  Matrix4d m1 = Matrix4d::Random(), m2 = Matrix4d::Random();
  std::cout << m1.binaryExpr(m2, MakeComplexOp<double>()) << std::endl;
  return 0;
}
