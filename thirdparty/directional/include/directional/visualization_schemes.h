// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_VISUALIZATION_SCHEMES_H
#define DIRECTIONAL_VISUALIZATION_SCHEMES_H

#include <Eigen/Core>


//This file contains the default libdirectional visualization paradigms
namespace directional
{
  
  //Mesh colors
  Eigen::RowVector3d IGL_INLINE default_mesh_color(){
    return Eigen::RowVector3d::Constant(1.0);
  }
  
  //Color for faces that are selected for editing and constraints
  Eigen::RowVector3d IGL_INLINE selected_face_color(){
    return Eigen::RowVector3d(0.7,0.2,0.2);
  }
  
  //Glyph colors
  Eigen::RowVector3d IGL_INLINE default_glyph_color(){
    return Eigen::RowVector3d(0.0,0.2,1.0);
  }
  
  //Glyphs in selected faces
  Eigen::RowVector3d IGL_INLINE selected_face_glyph_color(){
    return Eigen::RowVector3d(223.0/255.0, 210.0/255.0, 16.0/255.0);
  }
  
  //The selected glyph currently edited from a selected face
  Eigen::RowVector3d IGL_INLINE selected_vector_glyph_color(){
    return Eigen::RowVector3d(0.0,1.0,0.5);
  }
  
  //Colors by indices in each directional object. If the field is combed they will appear coherent across faces.
  Eigen::MatrixXd IGL_INLINE indexed_glyph_colors(const Eigen::MatrixXd& field){
    
    Eigen::Matrix<double, 9,3> glyphPrincipalColors;
    glyphPrincipalColors<<1.0,0.0,0.5,
    1.0,0.5,0.0,
    0.0,1.0,0.5,
    0.0,0.5,1.0,
    0.5,0.0,1.0,
    0.5,1.0,0.0,
    1.0,0.5,0.5,
    0.5,1.0,0.5,
    0.5,0.5,1.0;
    
    Eigen::MatrixXd fullGlyphColors(field.rows(),field.cols());
    int N = field.cols()/3;
    for (int i=0;i<field.rows();i++)
      for (int j=0;j<N;j++)
        fullGlyphColors.block(i,3*j,1,3)<<glyphPrincipalColors.row(j);
    
    return fullGlyphColors;
  }
  
  //Jet-based singularity colors
  Eigen::MatrixXd IGL_INLINE default_singularity_colors(const int N){
    Eigen::MatrixXd fullColors;
    Eigen::VectorXd NList(2*N);
    for (int i=0;i<N;i++){
      NList(i)=-N+i;
      NList(N+i)=i+1;
    }
    igl::jet(-NList,true,fullColors);
    return fullColors;
  }
  
  //Colors for emphasized edges, mostly seams and cuts
  Eigen::RowVector3d IGL_INLINE default_seam_color(){
    return Eigen::RowVector3d(0.0,0.0,0.0);
  }
}

#endif
