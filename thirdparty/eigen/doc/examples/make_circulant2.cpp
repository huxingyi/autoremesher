#include <Eigen/Core>
#include <iostream>

// [circulant_func]
template <class ArgType>
class circulant_functor {
  const ArgType& m_vec;

 public:
  circulant_functor(const ArgType& arg) : m_vec(arg) {}

  const typename ArgType::Scalar& operator()(Eigen::Index row, Eigen::Index col) const {
    Eigen::Index index = row - col;
    if (index < 0) index += m_vec.size();
    return m_vec(index);
  }
};
// [circulant_func]

// [square]
template <class ArgType>
struct circulant_helper {
  typedef Eigen::Matrix<typename ArgType::Scalar, ArgType::SizeAtCompileTime, ArgType::SizeAtCompileTime,
                        Eigen::ColMajor, ArgType::MaxSizeAtCompileTime, ArgType::MaxSizeAtCompileTime>
      MatrixType;
};
// [square]

// [makeCirculant]
template <class ArgType>
Eigen::CwiseNullaryOp<circulant_functor<ArgType>, typename circulant_helper<ArgType>::MatrixType> makeCirculant(
    const Eigen::MatrixBase<ArgType>& arg) {
  typedef typename circulant_helper<ArgType>::MatrixType MatrixType;
  return MatrixType::NullaryExpr(arg.size(), arg.size(), circulant_functor<ArgType>(arg.derived()));
}
// [makeCirculant]

// [main]
int main() {
  Eigen::VectorXd vec(4);
  vec << 1, 2, 4, 8;
  Eigen::MatrixXd mat;
  mat = makeCirculant(vec);
  std::cout << mat << std::endl;
}
// [main]
