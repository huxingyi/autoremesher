// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_INDEX_PRESCRIPTION_H
#define DIRECTIONAL_INDEX_PRESCRIPTION_H

#include <Eigen/Core>
#include <vector>
#include <cmath>
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/parallel_transport_angles.h>
#include <igl/edge_topology.h>
#include <igl/boundary_loop.h>


namespace directional
{    
  // Computes the dual-edge-based rotation angles that are required to reproduce a prescribed set of indices on the dual cycles of the mesh.
  // In case the sum of curvature is not consistent with the topology, the system is solved in least squares and unexpected singularities may appear elsewhere. linfError will mostl like be far from zero.
  // Inputs:
  //  V:          #V by 3 vertex coordinates
  //  F:          #F by 3 face vertex indices
  //  EV:         #E by 3 edges
  //  innerEdges: #iE the subset from EV of inner (non-boundary) edges.
  //  basisCycles:#c X #E the basis cycles matrix (obtained from directional::dual_cycles
  //  indices:    #c the prescribed index around each cycle. They should add up to N*Euler_characteristic of the mesh.
  //  cycleCurvature: #c the original curvature for each basis cycle.
  //  solver: The Simplicial LDLT solver used to solver the problem. It will only prefactor the matrix once upon the first call to the function; the state of  the solver solely depends on the basisCycles, therefore it only needs to be reset if the basisCycles matrix changed.
  //  N: the degree of the field.
  // Output:
  //  rotationAngles: #iE rotation angles (difference from parallel transport) per inner dual edge
  //  linfError: l_infinity error of the computation. If this is not approximately 0, the prescribed indices are likely inconsistent (don't add up to the correct sum).
  IGL_INLINE void index_prescription(const Eigen::MatrixXd& V,
                                     const Eigen::MatrixXi& F,
                                     const Eigen::MatrixXi& EV,
                                     const Eigen::VectorXi& innerEdges,
                                     const Eigen::SparseMatrix<double>& basisCycles,
                                     const Eigen::VectorXd& cycleCurvature,
                                     const Eigen::VectorXi& cycleIndices,
                                     Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> >& ldltSolver,
                                     const int N,
                                     Eigen::VectorXd& rotationAngles,
                                     double &linfError)
  {
    using namespace Eigen;
    using namespace std;
    
    VectorXd cycleNewCurvature = cycleIndices.cast<double>()*(2.0*igl::PI/(double)N);
    
    //Initialize solver if never before
    if (!ldltSolver.rows())
    {
      SparseMatrix<double> AAt = basisCycles*basisCycles.transpose();
      ldltSolver.compute(AAt);
    }
    
    VectorXd innerRotationAngles = basisCycles.transpose()*ldltSolver.solve(-cycleCurvature + cycleNewCurvature);
    rotationAngles.conservativeResize(EV.rows());
    rotationAngles.setZero();
    for (int i=0;i<innerEdges.rows();i++)
      rotationAngles(innerEdges(i))=innerRotationAngles(i);
    
    linfError = (basisCycles*innerRotationAngles - (-cycleCurvature + cycleNewCurvature)).lpNorm<Infinity>();
  }
  
  //Minimal version: no provided solver
  IGL_INLINE void index_prescription(const Eigen::MatrixXd& V,
                                     const Eigen::MatrixXi& F,
                                     const Eigen::VectorXi& innerEdges,
                                     const Eigen::SparseMatrix<double>& basisCycles,
                                     const Eigen::VectorXd& cycleCurvature,
                                     const Eigen::VectorXi& cycleIndices,
                                     const int N,
                                     Eigen::VectorXd& rotationAngles,
                                     double &error)
  {
    Eigen::MatrixXi EV, x, EF;
    igl::edge_topology(V, F, EV, x, EF);
    Eigen::MatrixXd B1, B2, B3;
    igl::local_basis(V, F, B1, B2, B3);
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > ldltSolver;
    index_prescription(V, F,EV, innerEdges, basisCycles,cycleCurvature, cycleIndices, ldltSolver, N, rotationAngles, error);
  }
}




#endif


