// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2016 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_TREE_H
#define DIRECTIONAL_TREE_H
#include <igl/igl_inline.h>
#include <Eigen/Core>
#include <vector>
#include <queue>


namespace directional
{
  // Creates a tree from a graph given by the edges in EV
  // edges containing a negative vertex are skipped.
  // Input:
  //  EV:   #E by 2 list of edges in the graph
  // Output:
  //  tE:   #Te vector of edges (within EV) in the graph.
  //  tEf:  #V the edges leading to each vertex. -1 for the root
  IGL_INLINE void tree(const Eigen::MatrixXi& EV,
                       Eigen::VectorXi& tE,
                       Eigen::VectorXi& tEf)
  {
    using namespace Eigen;
    int numV=EV.maxCoeff()+1;
    VectorXi Valences=VectorXi::Zero(numV);
    for (int i=0;i<EV.rows();i++){
      if (EV(i, 0) == -1 || EV(i, 1) == -1)
        continue;
      Valences(EV(i,0))++;
      Valences(EV(i,1))++;
    }
    
    MatrixXi VE(numV, Valences.maxCoeff());
    Valences.setZero();
    for (int i=0;i<EV.rows();i++){
      if (EV(i, 0) == -1 || EV(i, 1) == -1)
        continue;
      VE(EV(i,0), Valences(EV(i,0))++)=i;
      VE(EV(i,1), Valences(EV(i,1))++)=i;
    }
    
    Eigen::VectorXi usedVertices=VectorXi::Zero(numV);
    int numUsed=0;
    
    //queue and initial possible edges
    std::queue<std::pair<int,int> > edgeVertices;
    
    tE.resize(numV-1);
    tEf.resize(numV);
    tEf.setConstant(-2);
    int currEdgeIndex=0;
    int start = 0;
    
    //Try to find initial possible root for the tree.
    while (Valences[start] == 0)
      if (++start > Valences.size())
        return;
    
    edgeVertices.push(std::pair<int, int>(-1, start));
    do{
      std::pair<int, int> currEdgeVertex=edgeVertices.front();
      edgeVertices.pop();
      if (usedVertices(currEdgeVertex.second))
        continue;
      
      if (currEdgeVertex.first!=-1)
        tE(currEdgeIndex++)=currEdgeVertex.first;
      tEf(currEdgeVertex.second)=currEdgeVertex.first;
      usedVertices(currEdgeVertex.second)=1;
      
      //inserting the new unused vertices
      for (int i=0;i<Valences(currEdgeVertex.second);i++){
        int nextEdge=VE(currEdgeVertex.second, i);
        int nextVertex=(EV(nextEdge, 0)==currEdgeVertex.second ? EV(nextEdge, 1) : EV(nextEdge, 0));
        if (!usedVertices(nextVertex))
          edgeVertices.push(std::pair<int, int>(nextEdge, nextVertex));
      }
    }while (edgeVertices.size()!=0);
    
    tE.conservativeResize(usedVertices.sum() - 1);
  }
  
}

#endif


