//This file is part of Directional, a library for directional field processing.
// Copyright (C) 2017 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_EFFORT_TO_INDICES_H
#define DIRECTIONAL_EFFORT_TO_INDICES_H

#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>
#include <igl/edge_topology.h>
#include <igl/parallel_transport_angles.h>
#include <igl/per_face_normals.h>
#include <igl/parallel_transport_angles.h>
#include <directional/dual_cycles.h>


namespace directional
{
  // Computes cycle-based indices from dual-edge-based efforts.
  // Note: input is effort (sum of rotation angles), and not individual rotation angles
  // Input:
  //  basisCycles:    #c by #iE (inner edges of the mesh) the oriented basis cycles around which the indices are measured
  //  effort:         #iE the effort (sum of rotation angles) of matched vectors across the dual edge. Equal to N*rotation angles for N-RoSy fields.
  //  cycleCurvature: #c the cycle curvature (for instance, from directional::dual_cycles)
  //  N:              The degree of the field
  // Output:
  //  indices:     #c the index of the cycle x N (always an integer).
  IGL_INLINE void effort_to_indices(const Eigen::SparseMatrix<double>& basisCycles,
                                    const Eigen::VectorXd& effort,
                                    const Eigen::VectorXi& matching,
                                    const Eigen::VectorXd& cycleCurvature,
                                    const int N,
                                    Eigen::VectorXi& indices)
  {
    using namespace std;
    Eigen::VectorXd dIndices = ((basisCycles * effort + N*cycleCurvature).array() / (2.0*igl::PI));  //this should already be an integer up to numerical precision
    
    //Eigen::VectorXd matchingIndices = basisCycles*matching.cast<double>();
    
    //cout<<"dIndices - matchingIndices: "<<dIndices-matchingIndices<<endl;
    
    /*for (int i=0;i<dIndices.size();i++){
      int diff = (int)(std::round((dIndices(i) - matchingIndices(i))))%4;
      if (diff!=0)
        cout<<"index of difference: "<<i<<" and difference: "<<(dIndices(i) - matchingIndices(i))<<endl;
    }*/
    
    indices.conservativeResize(dIndices.size());
    for (int i=0;i<indices.size();i++)
      indices(i)=std::round(dIndices(i));
  
  }
  
  
  // minimal version without precomputed cycles or inner edges, returning only vertex singularities
  IGL_INLINE void effort_to_indices(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::MatrixXi& EV,
                                    const Eigen::MatrixXi& EF,
                                    const Eigen::VectorXd& effort,
                                    const Eigen::VectorXi& matching,
                                    const int N,
                                    Eigen::VectorXi& singVertices,
                                    Eigen::VectorXi& singIndices)
  {
    Eigen::SparseMatrix<double> basisCycles;
    Eigen::VectorXd cycleCurvature;
    Eigen::VectorXi vertex2cycle;
    Eigen::VectorXi innerEdges;
    directional::dual_cycles(V, F,EV, EF, basisCycles, cycleCurvature, vertex2cycle, innerEdges);
    Eigen::VectorXd effortInner(innerEdges.size());
    for (int i=0;i<innerEdges.size();i++)
      effortInner(i)=effort(innerEdges(i));
    Eigen::VectorXi fullIndices;
    directional::effort_to_indices(basisCycles, effortInner, matching, cycleCurvature, N, fullIndices);
    Eigen::VectorXi indices(V.size());
    for (int i=0;i<V.rows();i++)
      indices(i)=fullIndices(vertex2cycle(i));
    
    std::vector<int> singVerticesList;
    std::vector<int> singIndicesList;
    for (int i=0;i<V.rows();i++)
      if (indices(i)!=0){
        singVerticesList.push_back(i);
        singIndicesList.push_back(indices(i));
      }
    
    singVertices.conservativeResize(singVerticesList.size());
    singIndices.conservativeResize(singIndicesList.size());
    for (int i=0;i<singVerticesList.size();i++){
      singVertices(i)=singVerticesList[i];
      singIndices(i)=singIndicesList[i];
    }
  }
}

#endif


