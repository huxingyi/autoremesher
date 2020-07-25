//This file is part of Directional, a library for directional field processing.
// Copyright (C) 2016 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_DUAL_CYCLES_H
#define DIRECTIONAL_DUAL_CYCLES_H
#include <Eigen/Core>
#include <igl/boundary_loop.h>
#include <igl/local_basis.h>
#include <igl/gaussian_curvature.h>
#include <igl/colon.h>
#include <igl/setdiff.h>
#include <igl/slice.h>
#include <igl/unique.h>
#include <igl/edge_topology.h>
#include <vector>
#include <unordered_map>
#include "tree.h"


namespace directional
{
  // Creates the set of independent dual cycles (closed loops of connected faces that cannot be morphed to each other) on a mesh. Primarily used for index prescription.
  // The basis cycle matrix first contains #V-#b cycles for every inner vertex (by order), then #b boundary cycles, and finally 2*g generator cycles around all handles. Total #c cycles.The cycle matrix sums information on the dual edges between the faces, and is indexed into the inner edges alone (excluding boundary)
  //input:
  //  V: #V by 3 vertices.
  //  F: #F by 3 triangles.
  //  EV: #E by 2 matrix of edges (vertex indices)
  //  EF: #E by 2 matrix of oriented adjacent faces
  //output:
  //  basisCycles:    #c by #iE basis cycles
  //  cycleCurvature:   #c by 1 curvatures of each cycle (for inner-vertex cycles, simply the Gaussian curvature.
  //  vertex2cycle:     #v by 1 map between vertex and corresponding cycle (for comfort of input from the user's side; inner vertices map to their cycles, boundary vertices to the bigger boundary cycle.
  //  innerEdges:       #iE by 1 the subset of #EV that are inner edges, and with the same ordering as the columns of basisCycles.
  
  IGL_INLINE void dual_cycles(const Eigen::MatrixXd& V,
                              const Eigen::MatrixXi& F,
                              const Eigen::MatrixXi& EV,
                              const Eigen::MatrixXi& EF,
                              Eigen::SparseMatrix<double>& basisCycles,
                              Eigen::VectorXd& cycleCurvature,
                              Eigen::VectorXi& vertex2cycle,
                              Eigen::VectorXi& innerEdges)
  {
    using namespace Eigen;
    using namespace std;
    int numV = F.maxCoeff() + 1;
    int eulerChar = numV - EV.rows() + F.rows();
    vertex2cycle.conservativeResize(V.rows());
    
    std::vector<std::vector<int>> boundaryLoops;
    
    igl::boundary_loop(F, boundaryLoops);
    int numBoundaries=boundaryLoops.size();
    int numGenerators=2-numBoundaries-eulerChar;
    
    vector<Triplet<double> > basisCycleTriplets(EV.rows() * 2);
    
    basisCycles.resize(numV+numBoundaries+numGenerators, EV.rows());
    
    //all 1-ring cycles, including boundaries
    for (int i = 0; i < EV.rows(); i++) {
      basisCycleTriplets.push_back(Triplet<double>(EV(i, 0), i, -1.0));
      basisCycleTriplets.push_back(Triplet<double>(EV(i, 1), i, 1.0));
    }
    
    //Creating boundary cycles by building a matrix the sums up boundary loops and zeros out boundary vertex cycles - it will be multiplied from the left to basisCyclesMat
    VectorXi isBoundary(V.rows()); isBoundary.setZero();
    for (int i=0;i<boundaryLoops.size();i++)
      for (int j=0;j<boundaryLoops[i].size();j++)
        isBoundary(boundaryLoops[i][j])=1;
    
    VectorXi pureInnerEdgeMask=VectorXi::Constant(EV.rows(),1);
    for (int i=0;i<EV.rows();i++)
      if ((isBoundary(EV(i,0)))||(isBoundary(EV(i,1))))
        pureInnerEdgeMask(i)=0;
    
    int currGeneratorCycle=0;
    int currBoundaryCycle=0;
    
    if ((numGenerators!=0)/*||(numBoundaries!=0)*/){
      MatrixXi reducedEV(EV);
      for (int i = 1; i < reducedEV.rows(); i++)
        if(isBoundary(reducedEV(i,0)) || isBoundary(reducedEV(i, 1)))
          reducedEV(i,0) = -1;
      
      VectorXi primalTreeEdges, primalTreeFathers;
      VectorXi dualTreeEdges, dualTreeFathers;
      tree(reducedEV, primalTreeEdges, primalTreeFathers);
      //creating a set of dual edges that do not cross edges in the primal tree
      VectorXi fullIndices = VectorXi::LinSpaced(EV.rows(), 0, EV.rows() - 1);
      VectorXi reducedEFIndices, inFullIndices;
      MatrixXi reducedEF;
      igl::setdiff(fullIndices, primalTreeEdges, reducedEFIndices, inFullIndices);
      VectorXi Two = VectorXi::LinSpaced(2, 0, 1);
      
      igl::slice(EF, reducedEFIndices, Two, reducedEF);
      tree(reducedEF, dualTreeEdges, dualTreeFathers);
      //converting dualTreeEdges from reducedEF to EF
      for (int i = 0; i < dualTreeEdges.size(); i++)
        dualTreeEdges(i) = inFullIndices(dualTreeEdges(i));
      
      for (int i = 0; i < dualTreeFathers.size(); i++)
        if (dualTreeFathers(i) != -1 && dualTreeFathers(i) != -2)
          dualTreeFathers(i) = inFullIndices(dualTreeFathers(i));
      
      //building tree co-tree based homological cycles
      //finding dual edge which are not in the tree, and following their faces to the end
      VectorXi isinTree = VectorXi::Zero(EF.rows());
      for (int i = 0; i < dualTreeEdges.size(); i++) {
        isinTree(dualTreeEdges(i)) = 1;
      }
      for (int i = 0; i < primalTreeEdges.size(); i++) {
        isinTree(primalTreeEdges(i)) = 1;
      }
      
      for (int i = 0; i < isinTree.size(); i++) {
        if (isinTree(i))
          continue;
        
        //std::cout<<"New Cycle"<<std::endl;
        //otherwise, follow both end faces to the root and this is the dual cycle
        if (EF(i, 0) == -1 || EF(i, 1) == -1)
          continue;
        std::vector<Triplet<double> > candidateTriplets;
        //candidateTriplets.push_back(Triplet<double>(0, i, 1.0));
        Vector2i currLeaves; currLeaves << EF(i, 0), EF(i, 1);
        VectorXi visitedOnce = VectorXi::Zero(EF.rows());  //used to remove the tail from the LCA to the root
        bool isBoundaryCycle=true;
        for (int i = 0; i < 2; i++) { //on leaves
          int currTreeEdge = -1;  //indexing within dualTreeEdges
          int currFace = currLeaves(i);
          currTreeEdge = dualTreeFathers(currFace);
          if (currTreeEdge == -2)
          {
            break;
          }

          while (currTreeEdge != -1) {
            //std::cout<<"currTreeEdge: "<<currTreeEdge<<"\n"<<std::endl;
            //determining orientation of current edge vs. face
            double sign = ((EF(currTreeEdge, 0) == currFace) != (i == 0) ? 1.0 : -1.0);
            visitedOnce(currTreeEdge) = 1 - visitedOnce(currTreeEdge);
            candidateTriplets.push_back(Triplet<double>(0, currTreeEdge, sign));
            currFace = (EF(currTreeEdge, 0) == currFace ? EF(currTreeEdge, 1) : EF(currTreeEdge, 0));
            currTreeEdge = dualTreeFathers(currFace);
          }
        }

        //only putting in dual edges that are below the LCA
        for (int i=0;i<candidateTriplets.size();i++)
          if ((visitedOnce(candidateTriplets[i].col()))&&(pureInnerEdgeMask(candidateTriplets[i].col())))
            isBoundaryCycle=false;
        
        int currRow = (isBoundaryCycle ? numV+currBoundaryCycle : numV+numBoundaries+currGeneratorCycle);
        (isBoundaryCycle ? currBoundaryCycle++ : currGeneratorCycle++);
        
        basisCycleTriplets.push_back(Triplet<double>(currRow, i, 1.0));
        for (size_t i = 0; i < candidateTriplets.size(); i++)
          if (visitedOnce(candidateTriplets[i].col())){
            Triplet<double> trueTriplet(currRow,candidateTriplets[i].col(), candidateTriplets[i].value());
            basisCycleTriplets.push_back(trueTriplet);
          }
      }
      //assert(currBoundaryCycle==numBoundaries && currGeneratorCycle==numGenerators);
    }
    
    SparseMatrix<double> sumBoundaryLoops(numV+numBoundaries+numGenerators,numV+numBoundaries+numGenerators);
    vector<Triplet<double>> sumBoundaryLoopsTriplets;
    vector<int> innerVerticesList, innerEdgesList;
    VectorXi remainRows, remainColumns;
    
    for (int i=0;i<numV;i++){
      sumBoundaryLoopsTriplets.push_back(Triplet<double>(i, i,1.0-isBoundary[i]));
      if (!isBoundary(i)){
        innerVerticesList.push_back(i);
        vertex2cycle(i)=innerVerticesList.size()-1;
      }
    }
    
    for (int i=0;i<EV.rows();i++)
      if (!((isBoundary(EV(i,0)))&&(isBoundary(EV(i,1)))))
        innerEdgesList.push_back(i);
    
    //summing up boundary loops
    for (int i=0;i<boundaryLoops.size();i++)
      for (int j=0;j<boundaryLoops[i].size();j++){
        sumBoundaryLoopsTriplets.push_back(Triplet<double>(numV+i, boundaryLoops[i][j],1.0));
        vertex2cycle(boundaryLoops[i][j])=i;
      }
    
    
    //just passing generators through;
    for (int i=numV+numBoundaries;i<numV+numBoundaries+numGenerators;i++)
      sumBoundaryLoopsTriplets.push_back(Triplet<double>(i, i,1.0));
    
    sumBoundaryLoops.setFromTriplets(sumBoundaryLoopsTriplets.begin(), sumBoundaryLoopsTriplets.end());
    
    basisCycles.resize(numV+numBoundaries+numGenerators, EV.rows());
    basisCycles.setFromTriplets(basisCycleTriplets.begin(), basisCycleTriplets.end());
    basisCycles=sumBoundaryLoops*basisCycles;
    
    //removing rows and columns
    remainRows.resize(innerVerticesList.size()+numBoundaries+numGenerators);
    remainColumns.resize(innerEdgesList.size());
    for (int i=0;i<innerVerticesList.size();i++)
      remainRows(i)=innerVerticesList[i];
    
    for (int i=0;i<numBoundaries+numGenerators;i++)
      remainRows(innerVerticesList.size()+i)=numV+i;
    
    for (int i=0;i<innerEdgesList.size();i++)
      remainColumns(i)=innerEdgesList[i];
    
    //creating slicing matrices
    std::vector<Triplet<double> > rowSliceTriplets, colSliceTriplets;
    for (int i=0;i<remainRows.size();i++)
      rowSliceTriplets.push_back(Triplet<double>(i, remainRows(i), 1.0));
    for (int i=0;i<remainColumns.size();i++)
      colSliceTriplets.push_back(Triplet<double>(remainColumns(i), i, 1.0));
    
    SparseMatrix<double> rowSliceMat(remainRows.rows(), basisCycles.rows());
    rowSliceMat.setFromTriplets(rowSliceTriplets.begin(), rowSliceTriplets.end());
    
    SparseMatrix<double> colSliceMat(basisCycles.cols(), remainColumns.rows());
    colSliceMat.setFromTriplets(colSliceTriplets.begin(), colSliceTriplets.end());
    
    basisCycles=rowSliceMat*basisCycles*colSliceMat;
    
    innerEdges.conservativeResize(innerEdgesList.size());
    for (int i=0;i<innerEdgesList.size();i++)
      innerEdges(i)=innerEdgesList[i];
    
    //computing cycle curvatures
    Eigen::MatrixXd B1, B2, B3;
    igl::local_basis(V, F, B1, B2, B3);
    
    
    /************************SHOULD BE DEPRECATED****************************/
    //Explanation: currently computing holonomy as curvature.
    VectorXd edgeParallelAngleChange(basisCycles.cols());  //the difference in the angle representation of edge i from EF(i,0) to EF(i,1)
    //MatrixXd edgeVectors(columns(columns.size() - 1), 3);
    
    for (int i = 0; i < innerEdges.rows(); i++) {
    
      int currEdge=innerEdges(i);
      
      RowVectorXd edgeVectors = (V.row(EV(currEdge, 1)) - V.row(EV(currEdge, 0))).normalized();
      double x1 = edgeVectors.dot(B1.row(EF(currEdge, 0)));
      double y1 = edgeVectors.dot(B2.row(EF(currEdge, 0)));
      double x2 = edgeVectors.dot(B1.row(EF(currEdge, 1)));
      double y2 = edgeVectors.dot(B2.row(EF(currEdge, 1)));
      edgeParallelAngleChange(i) = atan2(y2, x2) - atan2(y1, x1);
    }
    
    
    cycleCurvature = basisCycles*edgeParallelAngleChange;
    for (int i = 0; i < cycleCurvature.size(); i++) {
      while (cycleCurvature(i) >= M_PI) cycleCurvature(i) -= 2.0*M_PI;
      while (cycleCurvature(i) < -M_PI) cycleCurvature(i) += 2.0*M_PI;
    }
    
    /***************End of "SHOULD BE DEPRECATED"****************/
  
  }
}
#endif
