// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can

// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_CURL_MATCHING_H
#define DIRECTIONAL_CURL_MATCHING_H

#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/edge_topology.h>
#include <directional/representative_to_raw.h>

namespace directional
{
  // Takes a field in raw form and computes both the curl-matching effort and the consequent curl matching on every edge.
  // Important: if the Raw field in not CCW ordered, the result is meaningless.
  // Input:
  //  V:      #V x 3 vertex coordinates
  //  F:      #F x 3 face vertex indices
  //  EV:     #E x 2 edges to vertices indices
  //  EF:     #E x 2 edges to faces indices
  //  raw:    The directional field, assumed to be ordered CCW, and in xyzxyzxyz...xyz (3*N cols) form. The degree is inferred by the size.
  // Output:
  // matching: #E matching function, where vector k in EF(i,0) matches to vector (k+matching(k))%N in EF(i,1). In case of boundary, there is a -1.
  //  effort: #E principal matching efforts.
  // curlNorm: the L2-norm of the curl vector

  IGL_INLINE void curl_matching(const Eigen::MatrixXd& V,
                                const Eigen::MatrixXi& F,
                                const Eigen::MatrixXi& EV,
                                const Eigen::MatrixXi& EF,
                                const Eigen::MatrixXi& FE,
                                const Eigen::MatrixXd& rawField,
                                Eigen::VectorXi& matching,
                                Eigen::VectorXd& effort,
                                Eigen::VectorXd& curlNorm)
  {
    
    typedef std::complex<double> Complex;
    using namespace Eigen;
    using namespace std;
    
    MatrixXd B1, B2, B3;
    igl::local_basis(V, F, B1, B2, B3);
    
    int N = rawField.cols() / 3;
    
    matching.conservativeResize(EF.rows());
    matching.setConstant(-1);
    curlNorm.conservativeResize(EF.rows());
    
    VectorXcd edgeTransport(EF.rows());  //the difference in the angle representation of edge i from EF(i,0) to EF(i,1)
    MatrixXd edgeVectors(EF.rows(), 3);
    for (int i = 0; i < EF.rows(); i++) {
      if (EF(i, 0) == -1 || EF(i, 1) == -1)
        continue;
      edgeVectors.row(i) = (V.row(EV(i, 1)) - V.row(EV(i, 0))).normalized();
      Complex ef(edgeVectors.row(i).dot(B1.row(EF(i, 0))), edgeVectors.row(i).dot(B2.row(EF(i, 0))));
      Complex eg(edgeVectors.row(i).dot(B1.row(EF(i, 1))), edgeVectors.row(i).dot(B2.row(EF(i, 1))));
      edgeTransport(i) = eg / ef;
    }
    
    effort = VectorXd::Zero(EF.rows());
    for (int i = 0; i < EF.rows(); i++) {
      if (EF(i, 0) == -1 || EF(i, 1) == -1)
        continue;
      //computing free coefficient effort (a.k.a. [Diamanti et al. 2014])
      Complex freeCoeffEffort(1.0, 0.0);
      int indexMinFromZero=0;
      //finding where the 0 vector in EF(i,0) goes to with smallest rotation angle in EF(i,1), computing the effort, and then adjusting the matching to have principal effort.
      double minCurl = 32767000.0;
      for (int j = 0; j < N; j++) {
        double currCurl = 0;
        for (int k=0;k<N;k++){
          RowVector3d vecDiff =rawField.block(EF(i, 1), 3 * ((j+k)%N), 1, 3)-rawField.block(EF(i, 0), 3*k, 1, 3);
          currCurl +=pow(edgeVectors.row(i).dot(vecDiff),2.0);
        }
        
        if (currCurl < minCurl){
          indexMinFromZero=j;
          minCurl=currCurl;
        }
      }
      
      matching(i) =indexMinFromZero;
      curlNorm(i)= sqrt(minCurl);
      
      //computing the full effort for 0->indexMinFromZero, and readjusting the matching to fit principal effort
      double currEffort=0;
      for (int j = 0; j < N; j++) {
        RowVector3d vecjf = rawField.block(EF(i, 0), 3*j, 1, 3);
        Complex vecjfc = Complex(vecjf.dot(B1.row(EF(i, 0))), vecjf.dot(B2.row(EF(i, 0))));
        RowVector3d vecjg = rawField.block(EF(i, 1), 3 * ((matching(i)+j+N)%N), 1, 3);
        Complex vecjgc = Complex(vecjg.dot(B1.row(EF(i, 1))), vecjg.dot(B2.row(EF(i, 1))));
        Complex transvecjfc = vecjfc*edgeTransport(i);
         //cout<<"transvecjfc, vecjgc: "<<transvecjfc<<","<<vecjgc<<endl;
        currEffort+= arg(vecjgc / transvecjfc);
        //cout<<"arg(vecjgc / transvecjfc): "<<arg(vecjgc / transvecjfc)<<endl;
      }
      
      effort(i) = currEffort;

    }
    
  }
  
  //version with representative vector (for N-RoSy) as input.
  IGL_INLINE void curl_matching(const Eigen::MatrixXd& V,
                                     const Eigen::MatrixXi& F,
                                     const Eigen::MatrixXi& EV,
                                     const Eigen::MatrixXi& EF,
                                     const Eigen::MatrixXi& FE,
                                     const Eigen::MatrixXd& representativeField,
                                     const int N,
                                     Eigen::VectorXi& matching,
                                     Eigen::VectorXd& effort,
                                     Eigen::VectorXd& curlNorm)
  {
    Eigen::MatrixXd rawField;
    representative_to_raw(V, F, representativeField, N, rawField);
    curl_matching(V, F, EV, EF, FE, rawField, matching, effort, curlNorm);
  }
}




#endif


