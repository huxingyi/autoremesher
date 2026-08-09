// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2025 The Eigen Authors.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_TEST_CUSTOM_COMPLEX_H
#define EIGEN_TEST_CUSTOM_COMPLEX_H

#include <ostream>
#include <sstream>

namespace custom_complex {

template <typename Real>
struct CustomComplex {
  CustomComplex() : re{0}, im{0} {}
  CustomComplex(const CustomComplex& other) = default;
  CustomComplex(CustomComplex&& other) = default;
  CustomComplex& operator=(const CustomComplex& other) = default;
  CustomComplex& operator=(CustomComplex&& other) = default;
  CustomComplex(Real x) : re{x}, im{0} {}
  CustomComplex(Real x, Real y) : re{x}, im{y} {}

  CustomComplex operator+(const CustomComplex& other) const { return CustomComplex(re + other.re, im + other.im); }

  CustomComplex operator-() const { return CustomComplex(-re, -im); }

  CustomComplex operator-(const CustomComplex& other) const { return CustomComplex(re - other.re, im - other.im); }

  CustomComplex operator*(const CustomComplex& other) const {
    return CustomComplex(re * other.re - im * other.im, re * other.im + im * other.re);
  }

  CustomComplex operator/(const CustomComplex& other) const {
    // Smith's complex division (https://arxiv.org/pdf/1210.4539.pdf),
    // guards against over/under-flow.
    const bool scale_imag = numext::abs(other.im) <= numext::abs(other.re);
    const Real rscale = scale_imag ? Real(1) : other.re / other.im;
    const Real iscale = scale_imag ? other.im / other.re : Real(1);
    const Real denominator = other.re * rscale + other.im * iscale;
    return CustomComplex((re * rscale + im * iscale) / denominator, (im * rscale - re * iscale) / denominator);
  }

  CustomComplex& operator+=(const CustomComplex& other) {
    *this = *this + other;
    return *this;
  }
  CustomComplex& operator-=(const CustomComplex& other) {
    *this = *this - other;
    return *this;
  }
  CustomComplex& operator*=(const CustomComplex& other) {
    *this = *this * other;
    return *this;
  }
  CustomComplex& operator/=(const CustomComplex& other) {
    *this = *this / other;
    return *this;
  }

  bool operator==(const CustomComplex& other) const {
    return numext::equal_strict(re, other.re) && numext::equal_strict(im, other.im);
  }
  bool operator!=(const CustomComplex& other) const { return !(*this == other); }

  friend CustomComplex operator+(const Real& a, const CustomComplex& b) { return CustomComplex(a + b.re, b.im); }

  friend CustomComplex operator-(const Real& a, const CustomComplex& b) { return CustomComplex(a - b.re, -b.im); }

  friend CustomComplex operator*(const Real& a, const CustomComplex& b) { return CustomComplex(a * b.re, a * b.im); }

  friend CustomComplex operator*(const CustomComplex& a, const Real& b) { return CustomComplex(a.re * b, a.im * b); }

  friend CustomComplex operator/(const CustomComplex& a, const Real& b) { return CustomComplex(a.re / b, a.im / b); }

  friend std::ostream& operator<<(std::ostream& stream, const CustomComplex& x) {
    std::stringstream ss;
    ss << "(" << x.re << ", " << x.im << ")";
    stream << ss.str();
    return stream;
  }

  Real re;
  Real im;
};

template <typename Real>
Real real(const CustomComplex<Real>& x) {
  return x.re;
}
template <typename Real>
Real imag(const CustomComplex<Real>& x) {
  return x.im;
}
template <typename Real>
CustomComplex<Real> conj(const CustomComplex<Real>& x) {
  return CustomComplex<Real>(x.re, -x.im);
}
template <typename Real>
CustomComplex<Real> sqrt(const CustomComplex<Real>& x) {
  return Eigen::internal::complex_sqrt(x);
}
template <typename Real>
Real abs(const CustomComplex<Real>& x) {
  return Eigen::numext::sqrt(x.re * x.re + x.im * x.im);
}

}  // namespace custom_complex

template <typename Real>
using CustomComplex = custom_complex::CustomComplex<Real>;

namespace Eigen {
template <typename Real>
struct NumTraits<CustomComplex<Real>> : NumTraits<Real> {
  enum { IsComplex = 1 };
};

namespace numext {
template <typename Real>
EIGEN_DEVICE_FUNC EIGEN_ALWAYS_INLINE bool(isfinite)(const CustomComplex<Real>& x) {
  return (numext::isfinite)(x.re) && (numext::isfinite)(x.im);
}

}  // namespace numext
}  // namespace Eigen

#endif  // EIGEN_TEST_CUSTOM_COMPLEX_H
