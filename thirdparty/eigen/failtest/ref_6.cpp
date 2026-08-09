#include "../Eigen/Core"

using namespace Eigen;

void call_ref(Ref<const VectorXf, 0, InnerStride<2>>) {}

int main() {
  VectorXf a(10);
  Map<const VectorXf, 0, InnerStride<2>> m(a.data(), 5);
#ifdef EIGEN_SHOULD_FAIL_TO_BUILD
  call_ref(a);
#else
  call_ref(m);
#endif
}
