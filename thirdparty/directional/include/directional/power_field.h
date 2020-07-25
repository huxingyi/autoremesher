// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_POWER_FIELD_H
#define DIRECTIONAL_POWER_FIELD_H

#include <iostream>
#include <Eigen/Geometry>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/Eigenvalues>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/local_basis.h>
#include <directional/polyvector_field.h>


namespace directional
{
  
  // Precalculate the power-field LDLt solvers. Must be recalculated whenever
  // bc changes or the mesh changes.
  // Inputs:
  //  V: #V by 3 vertex coordinates.
  //  F: #F by 3 face vertex indices.
  //  EV: #E by 2 matrix of edges (vertex indices)
  //  EF: #E by 2 matrix of oriented adjacent faces
  //  B1, B2: #F by 3 matrices representing the local base of each face.
  //  bc: The face ids where the pv is prescribed.
  //  N: The degree of the field.
  // Outputs:
  //  solver: with prefactorized left-hand side
  //  AFull, AVar: The resulting left-hand side matrices
  IGL_INLINE void power_field_precompute(const Eigen::MatrixXd& V,
                                        const Eigen::MatrixXi& F,
                                        const Eigen::MatrixXi& EV,
                                        const Eigen::MatrixXi& EF,
                                        const Eigen::MatrixXd& B1,
                                        const Eigen::MatrixXd& B2,
                                        const Eigen::VectorXi& bc,
                                        const int N, Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>>& solver,
                                        Eigen::SparseMatrix<std::complex<double>>& Afull,
                                        Eigen::SparseMatrix<std::complex<double>>& AVar)
  {
    polyvector_precompute(V,F,EV,EF,B1,B2,bc,N,solver,Afull,AVar);
  }
  
  // Computes a power field on the entire mesh from given values at the prescribed indices.
  // powerfield_precompute must be called in advance, and "b" must be on the given "bc"
  // If no constraints are given the zero field will be returned.
  // Inputs:
  //  B1, B2: #F by 3 matrices representing the local base of each face.
  //  bc: the faces on which the polyvector is prescribed.
  //  b: #F by 3 in representative form of the N-RoSy's on the faces indicated by bc.
  //  solver: with prefactorized left-hand side
  //  Afull, AVar: left-hand side matrices (with and without constraints) of the system.
  //  N: The degree of the field.
  // Outputs:
  //  powerField: #F by 2 The output interpolated field, in complex numbers.
  IGL_INLINE void power_field(const Eigen::MatrixXd& B1,
                              const Eigen::MatrixXd& B2,
                              const Eigen::VectorXi& bc,
                              const Eigen::MatrixXd& b,
                              const Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>>& solver,
                              const Eigen::SparseMatrix<std::complex<double>>& Afull,
                              const Eigen::SparseMatrix<std::complex<double>>& AVar,
                              const int N,
                              Eigen::MatrixXcd& powerField)
  {
    polyvector_field(B1,B2,bc,b,solver,Afull,AVar,N,powerField);
  }
  
  // Minimal version without auxiliary data.
  IGL_INLINE void power_field(const Eigen::MatrixXd& V,
                              const Eigen::MatrixXi& F,
                              const Eigen::VectorXi& bc,
                              const Eigen::MatrixXd& b,
                              const int N,
                              Eigen::MatrixXcd& powerField)
  {
    Eigen::MatrixXi EV, xi, EF;
    igl::edge_topology(V, F, EV, xi, EF);
    Eigen::MatrixXd B1, B2, xd;
    igl::local_basis(V, F, B1, B2, xd);
    Eigen::SparseMatrix<std::complex<double>> Afull, AVar;
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>> solver;
    power_field_precompute(V,F,EV,EF,B1,B2,bc,N, solver,Afull,AVar);
    power_field(B1, B2, bc, b, solver, Afull, AVar, N, powerField);
  }
}


#endif
