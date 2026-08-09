#include "../Eigen/Core"

using namespace Eigen;

void call_ref(Ref<const Matrix3f, 0, OuterStride<2>>) {}

int main() {
  MatrixXf a(6, 2);
  Map<const Matrix3f, 0, OuterStride<Dynamic>> md(a.data(), OuterStride<Dynamic>(2));
  Map<const Matrix3f, 0, OuterStride<2>> m2(a.data());
#ifdef EIGEN_SHOULD_FAIL_TO_BUILD
  call_ref(md);
#else
  call_ref(m2);
#endif
}
