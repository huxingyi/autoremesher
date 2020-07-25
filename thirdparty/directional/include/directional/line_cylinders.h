// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_LINE_CYLINDERS_H
#define DIRECTIONAL_LINE_CYLINDERS_H
#include <igl/igl_inline.h>
#include <Eigen/Core>
#include <string>
#include <vector>
#include <cmath> 
#include <complex>


namespace directional
{
  // creates a mesh of small cylinders to visualize lines on the overlay of the mesh
  // Inputs:
  //  P1,P2:      #P by 3 coordinates of the endpoints of the cylinders
  //  radius:     Cylinder base radii
  //  cyndColors: #P by 3 RBG colors per cylinder
  //  res:        The resolution of the cylinder (size of base polygon)
  // Outputs:
  //  V   #V by 3 cylinder mesh coordinates
  //  T   #T by 3 mesh triangles
  //  C   #T by 3 face-based colors
  IGL_INLINE bool line_cylinders(const Eigen::MatrixXd& P1,
                                 const Eigen::MatrixXd& P2,
                                 const double& radius,
                                 const Eigen::MatrixXd& cyndColors,
                                 const int res,
                                 Eigen::MatrixXd& V,
                                 Eigen::MatrixXi& T,
                                 Eigen::MatrixXd& C)
  {
    using namespace Eigen;
    int VOffset, TOffset, COffset;
    V.resize(2*res*P1.rows(),3);
    T.resize(2*res*P1.rows(),3);
    int NewColorSize=T.rows();
    C.resize(NewColorSize,3);
    VOffset=TOffset=COffset=0;
   
    RowVector3d ZAxis; ZAxis<<0.0,0.0,1.0;
    RowVector3d YAxis; YAxis<<0.0,1.0,0.0;
    
    MatrixXd PlanePattern(res,2);
    for (int i=0;i<res;i++){
      std::complex<double> CurrRoot=exp(2*M_PI*std::complex<double>(0,1)*(double)i/(double)res);
      PlanePattern.row(i)<<CurrRoot.real(), CurrRoot.imag();
    }
    
    for (int i=0;i<P1.rows();i++){
      RowVector3d NormAxis=(P2.row(i)-P1.row(i)).normalized();
      RowVector3d PlaneAxis1=NormAxis.cross(ZAxis);
      if (PlaneAxis1.norm()<10e-2)
        PlaneAxis1=NormAxis.cross(YAxis).normalized();
      else
        PlaneAxis1=PlaneAxis1.normalized();
      RowVector3d PlaneAxis2=NormAxis.cross(PlaneAxis1).normalized();
      for (int j=0;j<res;j++){
        int v1=2*res*i+2*j;
        int v2=2*res*i+2*j+1;
        int v3=2*res*i+2*((j+1)%res);
        int v4=2*res*i+2*((j+1)%res)+1;
        V.row(v1)<<P1.row(i)+(PlaneAxis1*PlanePattern(j,0)+PlaneAxis2*PlanePattern(j,1))*radius;
        V.row(v2)<<P2.row(i)+(PlaneAxis1*PlanePattern(j,0)+PlaneAxis2*PlanePattern(j,1))*radius;
        
        T.row(2*res*i+2*j)<<VOffset+v3,VOffset+v2,VOffset+v1;
        T.row(2*res*i+2*j+1)<<VOffset+v4,VOffset+v2,VOffset+v3;
        
        C.row(2*res*i+2*j)<<cyndColors.row(i);
        C.row(2*res*i+2*j+1)<<cyndColors.row(i);
        
      }
    }
    return true;
  }
  
}




#endif


