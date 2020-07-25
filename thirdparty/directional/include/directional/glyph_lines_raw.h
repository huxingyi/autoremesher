// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_GLYPH_LINES_RAW_H
#define DIRECTIONAL_GLYPH_LINES_RAW_H

#include <igl/igl_inline.h>
#include <igl/barycenter.h>
#include <igl/per_face_normals.h>
#include <igl/avg_edge_length.h>
#include <directional/representative_to_raw.h>
#include <directional/point_spheres.h>
#include <directional/line_boxes.h>
#include <Eigen/Core>


namespace directional
{
  // Creates mesh elements that comprise glyph drawing of a directional field.
  // Inputs:
  //  V:          #V X 3 vertex coordinates.
  //  F:          #F by 3 face vertex indices.
  //  rawField:   A directional field in raw xyzxyz form
  //  glyphColor: An array of either 1 by 3 color values for each vector, #F by 3 colors for each individual directional or #F*N by 3 colours for each individual vector, ordered by #F times vector 1, followed by #F times vector 2 etc.
  //  width, length, height: of the glyphs depicting the directionals
  //  N:        The degree of the field.
  
  // Outputs:
  //  fieldV: The vertices of the field mesh
  //  fieldF: The faces of the field mesh
  //  fieldC: The colors of the field mesh
  
  void IGL_INLINE glyph_lines_raw(const Eigen::MatrixXd &V,
                                  const Eigen::MatrixXi &F,
                                  const Eigen::MatrixXd &rawField,
                                  const Eigen::MatrixXd &glyphColor,
                                  double width,
                                  double length,
                                  double height,
                                  Eigen::MatrixXd &fieldV,
                                  Eigen::MatrixXi &fieldF,
                                  Eigen::MatrixXd &fieldC)
  {
    Eigen::MatrixXd normals;
    igl::per_face_normals(V, F, normals);
    int N=rawField.cols()/3;
    
    Eigen::MatrixXd vectNormals = normals.replicate(N,1);
    
    Eigen::MatrixXd barycenters, vectorColors, P1, P2;
    igl::barycenter(V, F, barycenters);
    
    P1.resize(F.rows() * N, 3);
    P2.resize(F.rows() * N, 3);
    vectorColors.resize(F.rows() * N, 3);
    
    normals.array() *= width;
    //barycenters += normals;
    P1 = barycenters.replicate(N, 1);
    
    for (int i = 0; i < N; i++)
      P2.middleRows(F.rows()*i, F.rows()) = rawField.middleCols(3*i, 3);
    
    P2.array() *= length;
    P2 += P1;
    
    // Duplicate colors so each cylinder gets the proper color
    if (glyphColor.rows() == 1)
      vectorColors = glyphColor.replicate(P1.rows(), 1);
    else if ((glyphColor.rows() == F.rows())&&(glyphColor.cols()==3))
      vectorColors = glyphColor.replicate(N, 1);
    else{
      for (int i=0;i<N;i++)
        vectorColors.block(i*F.rows(),0,F.rows(),3)=glyphColor.block(0,3*i,F.rows(),3);
    }
    
    Eigen::MatrixXd Vc, Cc, Vs, Cs;
    Eigen::MatrixXi Fc, Fs;
    // Draw boxes
    directional::line_boxes(P1, P2, vectNormals, width, height, vectorColors, fieldV, fieldF, fieldC);
    
    
  }
  
  //version without specification of glyph dimensions
  void IGL_INLINE glyph_lines_raw(const Eigen::MatrixXd &V,
                                  const Eigen::MatrixXi &F,
                                  const Eigen::MatrixXd &rawField,
                                  const Eigen::MatrixXd &glyphColors,
                                  Eigen::MatrixXd &fieldV,
                                  Eigen::MatrixXi &fieldF,
                                  Eigen::MatrixXd &fieldC,
                                  const double sizeRatio = 1.25)
  {
    double l = sizeRatio*igl::avg_edge_length(V, F);
    glyph_lines_raw(V, F, rawField, glyphColors, l/30, l/6, l/50, fieldV, fieldF, fieldC);
  }
  
}

#endif
