// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2017 Daniele Panozzo <daniele.panozzo@gmail.com>, Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_POWER_TO_REPRESENTATIVE_H
#define DIRECTIONAL_POWER_TO_REPRESENTATIVE_H

#include <Eigen/Geometry>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/Eigenvalues>
#include <igl/local_basis.h>
#include <iostream>

namespace directional
{
  // Converts a field in Cartesian power representation ("u^N") to representative vector.
  // Input:
  //  B1, B2:         #F x 3 matrices representing the local base of each face.
  //  Y:              #F x 1 representing the field to the Nth power: Y=U^N where U is any representative.
  //  N:              The degree of the field.
  // Output:
  //  U:              #F x 3 representative vectors on the faces.
  IGL_INLINE void power_to_representative(const Eigen::MatrixXd& B1,
                                          const Eigen::MatrixXd& B2,
                                          const Eigen::MatrixXcd& powerField,
                                          const int N,
                                          Eigen::MatrixXd& representativeField)
  {
    // Convert the interpolated polyvector into Euclidean vectors
    representativeField.conservativeResize(B1.rows(), 3);
    for (int f = 0; f < B1.rows(); ++f)
    {
      // Find the roots of p(t) = (t - c0)^n using
      // https://en.wikipedia.org/wiki/Companion_matrix
      Eigen::MatrixXcd M = Eigen::MatrixXcd::Zero(N, N);
      for (int i = 1; i < N; ++i)
        M(i, i - 1) = std::complex<double>(1, 0);
      M(0, N - 1) = powerField(f, 0);
      std::complex<double> root = M.eigenvalues()(0);
      representativeField.row(f) = B1.row(f) * root.real() + B2.row(f) * root.imag();
    }
  }
  
  
  // Version with only (V,F) as input
  IGL_INLINE void power_to_representative(const Eigen::MatrixXd& V,
                                          const Eigen::MatrixXi& F,
                                          const Eigen::MatrixXcd& powerField,
                                          const int N,
                                          Eigen::MatrixXd& representativeField)
  {
    Eigen::MatrixXd B1, B2, x;
    igl::local_basis(V, F, B1, B2, x);
    power_to_representative(B1, B2, powerField, N, representativeField);
  }
}
#endif
