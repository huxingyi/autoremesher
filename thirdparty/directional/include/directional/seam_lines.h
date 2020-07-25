// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_SEAM_LINES_H
#define DIRECTIONAL_SEAM_LINES_H

#include <string>
#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>
#include <igl/avg_edge_length.h>
#include <directional/visualization_schemes.h>
#include <directional/line_cylinders.h>

namespace directional
{
  
  //creating meshes for seam lines, as curves of cylinders
  // Input:
  //  V:      #V X 3 vertex coordinates.
  //  F:      #F X 3 mesh triangles.
  //  EV:     #E x 2 edges to vertices indices
  //  seams:  which edges of EV are seams
  //  width:  the width of the seam lines
  //  res:    the resolution of the cylinder bases.
  // Output:
  //  VSeams:          The vertices of the seam cylinders.
  //  FSeams:          The faces of the seam cylinders.
  //  CSeams:         The colors of the seam cylinders (per face).
  void IGL_INLINE seam_lines(const Eigen::MatrixXd &V,
                             const Eigen::MatrixXi &F,
                             const Eigen::MatrixXi &EV,
                             const Eigen::VectorXi seams,
                             double width,
                             int res,
                             Eigen::MatrixXd &VSeams,
                             Eigen::MatrixXi &FSeams,
                             Eigen::MatrixXd &CSeams)
  {
    
    std::vector<int> seamEdges;
    for (int i=0;i<EV.rows();i++)
      if (seams(i)!=0)
        seamEdges.push_back(i);
    
    Eigen::MatrixXd P1(seamEdges.size(),3), P2(seamEdges.size(),3);
    for (int i=0;i<seamEdges.size();i++){
      P1.row(i)=V.row(EV(seamEdges[i],0));
      P2.row(i)=V.row(EV(seamEdges[i],1));
    }
    
    directional::line_cylinders(P1, P2, width, directional::default_seam_color().replicate(P1.rows(),1), res, VSeams, FSeams, CSeams);
  }
  
  
  //A version with default width and resolution.
  void IGL_INLINE seam_lines(const Eigen::MatrixXd &V,
                             const Eigen::MatrixXi &F,
                             const Eigen::MatrixXi &EV,
                             const Eigen::VectorXi seams,
                             Eigen::MatrixXd &VSeams,
                             Eigen::MatrixXi &FSeams,
                             Eigen::MatrixXd &CSeams,
                             const double lengthRatio=1.25)
  {
    double l = lengthRatio*igl::avg_edge_length(V, F);
    seam_lines(V,F,EV, seams, l/25.0,6, VSeams, FSeams, CSeams);
  }
  
}

#endif


