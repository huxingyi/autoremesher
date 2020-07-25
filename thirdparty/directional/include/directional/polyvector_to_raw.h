// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_POLYVECTOR_TO_RAW_H
#define DIRECTIONAL_POLYVECTOR_TO_RAW_H

#include <iostream>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/local_basis.h>
#include <unsupported/Eigen/Polynomials>
#include <Eigen/Geometry>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/Eigenvalues>


namespace directional
{
  // Converts a field in PolyVector representation to raw represenation.
  // Inputs:
  //  B1, B2:           #F by 3 matrices representing the local base of each face.
  //  polyVectorField:  #F by N complex PolyVectors
  //  N:                The degree of the field.
  // Outputs:
  //  raw:              #F by 3*N matrix with all N explicit vectors of each directional in raw format xyzxyz
  IGL_INLINE void polyvector_to_raw(const Eigen::MatrixXd& B1,
                                    const Eigen::MatrixXd& B2,
                                    const Eigen::MatrixXcd& polyVectorField,
                                    const int N,
                                    Eigen::MatrixXd& rawField)
  {
    rawField.resize(B1.rows(), 3 * N);
    for (int f = 0; f < B1.rows(); f++)
    {
      // Find the roots of p(t) = (t - c0)^n using
      // https://en.wikipedia.org/wiki/Companion_matrix
      Eigen::MatrixXcd M = Eigen::MatrixXcd::Zero(N, N);
      for (int i = 1; i < N; ++i)
        M(i, i - 1) = std::complex<double>(1, 0);
      M.col(N - 1) = -polyVectorField.row(f).transpose();
      Eigen::VectorXcd roots = M.eigenvalues();
      
      std::sort(roots.data(), roots.data() + roots.size(), [](std::complex<double> a, std::complex<double> b){return arg(a) > arg(b);});
      for (int i = 0; i < N; i++)
      {
        std::complex<double> root = roots(i);
        rawField.block<1, 3>(f, 3 * i) = B1.row(f) * root.real() + B2.row(f) * root.imag();
      }
    }
  }
  
  
  //Version without explicit bases
  IGL_INLINE void polyvector_to_raw(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::MatrixXcd& polyVectorField,
                                    const int N,
                                    Eigen::MatrixXd& rawField)
  {
    Eigen::MatrixXd B1, B2, x;
    igl::local_basis(V, F, B1, B2, x);
    polyvector_to_raw(B1, B2, polyVectorField, N, rawField);
  }
}
#endif
