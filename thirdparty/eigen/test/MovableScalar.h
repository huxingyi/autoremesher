// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2020 Sebastien Boisvert <seb@boisvert.info>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_MISC_MOVABLE_SCALAR_H
#define EIGEN_MISC_MOVABLE_SCALAR_H

namespace Eigen {
template <typename Scalar>
struct MovableScalar {
  MovableScalar() : m_data(new Scalar) {}
  ~MovableScalar() { delete m_data; }
  MovableScalar(const MovableScalar& other) : m_data(new Scalar) { set(other.get()); }
  MovableScalar(MovableScalar&& other) noexcept : m_data(other.m_data) { other.m_data = nullptr; }
  MovableScalar& operator=(const MovableScalar& other) {
    set(other.get());
    return *this;
  }
  MovableScalar& operator=(MovableScalar&& other) noexcept {
    m_data = other.m_data;
    other.m_data = nullptr;
    return *this;
  }
  MovableScalar(const Scalar& scalar) : m_data(new Scalar) { set(scalar); }

  operator Scalar() const { return get(); }

 private:
  void set(const Scalar& value) {
    eigen_assert(m_data != nullptr);
    // suppress compiler warnings
    if (m_data != nullptr) *m_data = value;
  }
  Scalar get() const {
    eigen_assert(m_data != nullptr);
    // suppress compiler warnings
    return m_data == nullptr ? Scalar() : *m_data;
  }
  Scalar* m_data = nullptr;
};

template <typename Scalar>
struct NumTraits<MovableScalar<Scalar>> : NumTraits<Scalar> {
  enum { RequireInitialization = 1 };
};

}  // namespace Eigen

#endif
