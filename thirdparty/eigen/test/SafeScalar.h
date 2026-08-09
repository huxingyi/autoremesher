
// A Scalar that asserts for uninitialized access.
template <typename T>
class SafeScalar {
 public:
  SafeScalar() : initialized_(false) {}

  SafeScalar(const T& val) : val_(val), initialized_(true) {}

  template <typename Source>
  explicit SafeScalar(const Source& val) : SafeScalar(T(val)) {}

  operator T() const {
    VERIFY(initialized_ && "Uninitialized access.");
    return val_;
  }

  template <typename Target>
  explicit operator Target() const {
    return Target(this->operator T());
  }

 private:
  T val_;
  bool initialized_;
};

namespace Eigen {
template <typename T>
struct NumTraits<SafeScalar<T>> : GenericNumTraits<T> {
  enum { RequireInitialization = 1 };
};
}  // namespace Eigen