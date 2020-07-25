// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_COMBING_H
#define DIRECTIONAL_COMBING_H


#include <Eigen/Core>
#include <queue>
#include <vector>
#include <cmath>
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/edge_topology.h>
#include <directional/tree.h>
#include <directional/representative_to_raw.h>
#include <directional/principal_matching.h>

namespace directional
{
  // Reorders the vectors in a face (preserving CCW) so that the prescribed matching across most edges, except a small set (called a cut), is an identity, making it ready for cutting and parameterization.
  // Important: if the Raw field in not CCW ordered, the result is unpredictable.
  // Input:
  //  V:        #V x 3 vertex coordinates
  //  F:        #F x 3 face vertex indices
  //  EV:       #E x 2 edges to vertices indices
  //  EF:       #E x 2 edges to faces indices
  //  rawField: #F by 3*N  The directional field, assumed to be ordered CCW, and in xyzxyz raw format. The degree is inferred by the size.
  //  matching: #E matching function, where vector k in EF(i,0) matches to vector (k+matching(k))%N in EF(i,1). In case of boundary, there is a -1.
  // Output:
  //  combedField: #F by 3*N reindexed field
  IGL_INLINE void combing(const Eigen::MatrixXd& V,
                          const Eigen::MatrixXi& F,
                          const Eigen::MatrixXi& EV,
                          const Eigen::MatrixXi& EF,
                          const Eigen::MatrixXi& FE,
                          const Eigen::MatrixXd& rawField,
                          const Eigen::VectorXi& matching,
                          Eigen::MatrixXd& combedField)
  {
    using namespace Eigen;
    //flood-filling through the matching to comb field
    combedField.conservativeResize(rawField.rows(), rawField.cols());
    int N=rawField.cols()/3;
    //dual tree to find combing routes
    VectorXi visitedFaces=VectorXi::Constant(F.rows(),1,0);
    std::queue<std::pair<int,int> > faceMatchingQueue;
    faceMatchingQueue.push(std::pair<int,int>(0,0));
    do{
      std::pair<int,int> currFaceMatching=faceMatchingQueue.front();
      faceMatchingQueue.pop();
      if (visitedFaces(currFaceMatching.first))
        continue;
      visitedFaces(currFaceMatching.first)=1;
      
      //combing field to start from the matching index
      combedField.block(currFaceMatching.first, 0, 1, 3*(N-currFaceMatching.second))=rawField.block(currFaceMatching.first, 3*currFaceMatching.second, 1, 3*(N-currFaceMatching.second));
      combedField.block(currFaceMatching.first, 3*(N-currFaceMatching.second), 1, 3*currFaceMatching.second)=rawField.block(currFaceMatching.first, 0, 1, 3*currFaceMatching.second);
      
      for (int i=0;i<3;i++){
        int nextMatching=(matching(FE(currFaceMatching.first,i)));
        int nextFace=(EF(FE(currFaceMatching.first,i),0)==currFaceMatching.first ? EF(FE(currFaceMatching.first,i),1) : EF(FE(currFaceMatching.first,i),0));
        nextMatching*=(EF(FE(currFaceMatching.first,i),0)==currFaceMatching.first ? 1.0 : -1.0);
        nextMatching=(nextMatching+currFaceMatching.second+10*N)%N;  //killing negatives
        if ((nextFace!=-1)&&(!visitedFaces(nextFace)))
          faceMatchingQueue.push(std::pair<int,int>(nextFace, nextMatching));
        
      }
      
    }while (!faceMatchingQueue.empty());
  }
  
  //version for input in representative format (for N-RoSy directionals).
  IGL_INLINE void combing(const Eigen::MatrixXd& V,
                          const Eigen::MatrixXi& F,
                          const Eigen::MatrixXi& EV,
                          const Eigen::MatrixXi& EF,
                          const Eigen::MatrixXi& FE,
                          const Eigen::MatrixXd& representativeField,
                          const int N,
                          const Eigen::VectorXi& matching,
                          Eigen::MatrixXd& combedField)
  {
    Eigen::MatrixXd rawField;
    representative_to_raw(V, F, representativeField, N, rawField);
    combing(V, F, EV, EF, FE, rawField, matching, combedField);
  }
  
  //version with prescribed cuts from faces
  IGL_INLINE void combing(const Eigen::MatrixXd& V,
                          const Eigen::MatrixXi& F,
                          const Eigen::MatrixXi& EV,
                          const Eigen::MatrixXi& EF,
                          const Eigen::MatrixXi& FE,
                          const Eigen::MatrixXi& faceIsCut,
                          const Eigen::MatrixXd& rawField,
                          const Eigen::VectorXi& matching,
                          Eigen::MatrixXd& combedField,
                          Eigen::VectorXi& combedMatching)
  {
    using namespace Eigen;
    //flood-filling through the matching to comb field
    combedField.conservativeResize(rawField.rows(), rawField.cols());
    combedMatching.conservativeResize(EF.rows());
    int N=rawField.cols()/3;
    //dual tree to find combing routes
    VectorXi visitedFaces=VectorXi::Constant(F.rows(),1,0);
    std::queue<std::pair<int,int> > faceMatchingQueue;
    faceMatchingQueue.push(std::pair<int,int>(0,0));
    VectorXi faceTurns(rawField.rows());
    do{
      std::pair<int,int> currFaceMatching=faceMatchingQueue.front();
      faceMatchingQueue.pop();
      if (visitedFaces(currFaceMatching.first))
        continue;
      visitedFaces(currFaceMatching.first)=1;
      
      //combing field to start from the matching index
      combedField.block(currFaceMatching.first, 0, 1, 3*(N-currFaceMatching.second))=rawField.block(currFaceMatching.first, 3*currFaceMatching.second, 1, 3*(N-currFaceMatching.second));
      combedField.block(currFaceMatching.first, 3*(N-currFaceMatching.second), 1, 3*currFaceMatching.second)=rawField.block(currFaceMatching.first, 0, 1, 3*currFaceMatching.second);
      
      faceTurns(currFaceMatching.first)=currFaceMatching.second;
      
      for (int i=0;i<3;i++){
        int nextMatching=(matching(FE(currFaceMatching.first,i)));
        int nextFace=(EF(FE(currFaceMatching.first,i),0)==currFaceMatching.first ? EF(FE(currFaceMatching.first,i),1) : EF(FE(currFaceMatching.first,i),0));
        nextMatching*=(EF(FE(currFaceMatching.first,i),0)==currFaceMatching.first ? 1.0 : -1.0);
        nextMatching=(nextMatching+currFaceMatching.second+10*N)%N;  //killing negatives
        if ((nextFace!=-1)&&(!visitedFaces(nextFace))&&(!faceIsCut(currFaceMatching.first,i)))
          faceMatchingQueue.push(std::pair<int,int>(nextFace, nextMatching));
        
      }
      
    }while (!faceMatchingQueue.empty());
    
    //giving combed matching
    for (int i=0;i<EF.rows();i++){
      if ((EF(i,0)==-1)||(EF(i,1)==-1))
        combedMatching(i)=-1;
      else
        combedMatching(i)=(faceTurns(EF(i,0))-faceTurns(EF(i,1))+matching(i)+1000000*N)%N;
    }
  }
  
}




#endif


