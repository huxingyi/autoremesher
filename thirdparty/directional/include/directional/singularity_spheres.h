// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_SINGULARITY_SPHERES_H
#define DIRECTIONAL_SINGULARITY_SPHERES_H

#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>
#include <igl/barycenter.h>
#include <igl/per_face_normals.h>
#include <igl/avg_edge_length.h>
#include <igl/jet.h>
#include <directional/visualization_schemes.h>
#include <directional/representative_to_raw.h>
#include <directional/point_spheres.h>

namespace directional
{
    
  // Returns a list of faces, vertices and color values that can be used to draw singularities for non-zero index values.
  // Input:
  //  V:              #V X 3 vertex coordinates.
  //  F:              #F X 3 mesh triangles.
  //  indices:        #V x 1 index (/N) per vertex (must be 0<index<N-1)
  //  singularityColors: 2*N x 3 colos per positive index in order [-N,..-1, 1, N]
  // Output:
  //  singV:          The vertices of the singularity spheres.
  //  singF:          The faces of the singularity spheres.
  //  singC:         The colors of the singularity spheres.
  void IGL_INLINE singularity_spheres(const Eigen::MatrixXd& V,
                                      const Eigen::MatrixXi& F,
                                      const int N,
                                      const Eigen::VectorXi& singVertices,
                                      const Eigen::VectorXi& singIndices,
                                      Eigen::MatrixXd& singV,
                                      Eigen::MatrixXi& singF,
                                      Eigen::MatrixXd& singC,
                                      const double radiusRatio=1.25)
  
  {

    Eigen::MatrixXd points(singVertices.size(), 3);
    Eigen::MatrixXd colors(singIndices.size(), 3);
    Eigen::MatrixXd singularityColors=directional::default_singularity_colors(N);
    Eigen::MatrixXd positiveColors=singularityColors.block(singularityColors.rows()/2,0,singularityColors.rows()/2,3);
    Eigen::MatrixXd negativeColors=singularityColors.block(0,0,singularityColors.rows()/2,3);
    for (int i = 0; i < singIndices.rows(); i++)
    {
      points.row(i) = V.row(singVertices(i));
      if (singIndices(i) > 0)
        colors.row(i) = positiveColors.row((singIndices(i)-1 > positiveColors.rows()-1 ? positiveColors.rows()-1  : singIndices(i)-1) );
      else if (singIndices(i)<0)
        colors.row(i) = negativeColors.row((negativeColors.rows()+singIndices(i) > 0 ? negativeColors.rows()+singIndices(i) : 0));
      else
        colors.row(i).setZero(); //this shouldn't have been input
      
    }
    double radius = radiusRatio*igl::avg_edge_length(V, F)/5.0;
    directional::point_spheres(points, radius, colors, 8, singV, singF, singC);
  
  }
  

  //version that provides all vertex indices instead of only singularities
  /*void IGL_INLINE singularity_spheres(const Eigen::MatrixXd& V,
                                      const Eigen::MatrixXi& F,
                                      const int N,
                                      const Eigen::VectorXi& fullIndices,
                                      Eigen::MatrixXd& singV,
                                      Eigen::MatrixXi& singF,
                                      Eigen::MatrixXd& singC)
  
  {
    
    Eigen::MatrixXd singularityColors=directional::default_singularity_colors(N);
    std::vector<int> singVerticesList;
    std::vector<int> singIndicesList;
    for (int i=0;i<V.rows();i++)
      if (fullIndices(i)!=0){
        singVerticesList.push_back(i);
        singIndicesList.push_back(fullIndices(i));
      }
    
    Eigen::VectorXi singVertices(singVerticesList.size());
    Eigen::VectorXi singIndices(singIndicesList.size());
    for (int i=0;i<singVerticesList.size();i++){
      singVertices(i)=singVerticesList[i];
      singIndices(i)=singIndicesList[i];
    }
    
    singularity_spheres(V,F, singVertices,singIndices,singularityColors,singV, singF, singC);
  }*/
  
}

#endif
