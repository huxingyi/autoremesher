// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_LINE_BOXES_H
#define DIRECTIONAL_LINE_BOXES_H

#include <Eigen/Core>
#include <string>
#include <vector>
#include <cmath> 
#include <complex>
#include <igl/igl_inline.h>

namespace directional
{
  // creates a mesh of thin boxes to visualize lines on the overlay of the mesh
  // Input:
  //  P1,P2:          Each #P by 3 coordinates of the box endpoints
  //  normals:        Normals to the boxes (w.r.t. height).
  //  width, height:  Box dimensions
  //  boxColors:      #P by 3 RBG colors per box
  // Output:
  //  V:              #V by 3 box mesh coordinates
  //  T:              #T by 3 mesh triangles
  //  C:              #T by 3 colors
  
  IGL_INLINE bool line_boxes(const Eigen::MatrixXd& P1,
                             const Eigen::MatrixXd& P2,
                             const Eigen::MatrixXd& normals,
                             const double& width,
                             const double& height,
                             const Eigen::MatrixXd& boxColors,
                             Eigen::MatrixXd& V,
                             Eigen::MatrixXi& T,
                             Eigen::MatrixXd& C)
  {
    using namespace Eigen;
    
    //template mesh
    MatrixXd VBox(8,3);
    MatrixXi TBox(12,3);
    
    VBox<<0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 1.0,
    
    TBox<<0,1,2,
    2,3,1,
    2,1,5,
    5,6,2,
    7,6,5,
    5,4,7,
    4,5,1,
    1,0,4,
    3,7,4,
    4,0,3,
    3,2,6,
    6,7,3;
    
    V.resize(VBox.rows()*P1.rows(),3);
    T.resize(TBox.rows()*P1.rows(),3);
    int NewColorSize=T.rows();
    C.resize(NewColorSize,3);
 
    for (int i=0;i<P1.rows();i++){
      RowVector3d YAxis=(P2.row(i)-P1.row(i));
      RowVector3d ZAxis=normals.row(i);
      RowVector3d XAxis =YAxis.cross(ZAxis);
      XAxis.rowwise().normalize();
      XAxis*=width;
      ZAxis*=height;
      
      Matrix3d R; R<<XAxis, YAxis, ZAxis;
      
      RowVector3d P1onBox; P1onBox<<0.5, 0, 0.5;
      RowVector3d translation = P1.row(i) - P1onBox*R;
     
      V.block(VBox.rows()*i,0,VBox.rows(),3)=VBox*R+translation.replicate(VBox.rows(),1);
      T.block(TBox.rows()*i,0,TBox.rows(),3)=TBox.array()+VBox.rows()*i;
      C.block(TBox.rows()*i,0,TBox.rows(),3)=boxColors.row(i).replicate(TBox.rows(),1);
    }
    
    return true;
  }
  
}




#endif


