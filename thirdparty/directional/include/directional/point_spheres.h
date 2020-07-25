// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_POINT_SPHERES_H
#define DIRECTIONAL_POINT_SPHERES_H

#include <string>
#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>


namespace directional
{
  // creates small spheres to visualize points on the overlay of the mesh
  // Input:
  //  P:      #P by 3 coordinates of the centers of spheres
  //  radius: radii of the spheres
  //  C:      #P by 3 - RBG colors per sphere
  //  res:    the resolution of the sphere discretization
  // extendMesh if to extend the V,T,TC, or to overwrite them
  // Output:
  //  V:    #V by 3 sphere mesh coordinates
  //  T     #T by 3 sphere mesh triangles
  //  C:    #T by 3 face-based colors
  IGL_INLINE bool point_spheres(const Eigen::MatrixXd& points,
                                const double& radius,
                                const Eigen::MatrixXd& colors,
                                const int res,
                                Eigen::MatrixXd& V,
                                Eigen::MatrixXi& T,
                                Eigen::MatrixXd& C)
  {
    using namespace Eigen;
    V.resize(res*res*points.rows(),3);
    T.resize(2*(res-1)*res*points.rows(),3);
    C.resize(T.rows(),3);
    
    for (int i=0;i<points.rows();i++){
      RowVector3d center=points.row(i);
      
      //creating vertices
      for (int j=0;j<res;j++){
        double z=center(2)+radius*cos(M_PI*(double)j/(double(res-1)));
        for (int k=0;k<res;k++){
          double x=center(0)+radius*sin(M_PI*(double)j/(double(res-1)))*cos(2*M_PI*(double)k/(double(res-1)));
          double y=center(1)+radius*sin(M_PI*(double)j/(double(res-1)))*sin(2*M_PI*(double)k/(double(res-1)));
          V.row((res*res)*i+j*res+k)<<x,y,z;
        }
      }
      
      
      //creating faces
      for (int j=0;j<res-1;j++){
        for (int k=0;k<res;k++){
          int v1=(res*res)*i+j*res+k;
          int v2=(res*res)*i+(j+1)*res+k;
          int v3=(res*res)*i+(j+1)*res+(k+1)%res;
          int v4=(res*res)*i+j*res+(k+1)%res;
          T.row(2*(((res-1)*res)*i+res*j+k))<<v1,v2,v3;
          T.row(2*(((res-1)*res)*i+res*j+k)+1)<<v4,v1,v3;
          C.row(2*(((res-1)*res)*i+res*j+k))<<colors.row(i);
          C.row(2*(((res-1)*res)*i+res*j+k)+1)<<colors.row(i);
        }
      }
    }
    
    return true;
  }
}




#endif


