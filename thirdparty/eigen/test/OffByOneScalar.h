
// A Scalar with internal representation T+1 so that zero is internally
// represented by T(1). This is used to test memory fill.
//
#pragma once
template <typename T>
class OffByOneScalar {
 public:
  OffByOneScalar() : val_(1) {}
  OffByOneScalar(const OffByOneScalar& other) = default;
  OffByOneScalar& operator=(const OffByOneScalar& other) = default;

  OffByOneScalar(T val) : val_(val + 1) {}
  OffByOneScalar& operator=(T val) { val_ = val + 1; }

  operator T() const { return val_ - 1; }

 private:
  T val_;
};
