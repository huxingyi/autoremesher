// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_SETUP_PARAMETERIZATION_H
#define DIRECTIONAL_SETUP_PARAMETERIZATION_H

#include <queue>
#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/edge_topology.h>
#include <directional/tree.h>
#include <directional/representative_to_raw.h>
#include <directional/principal_matching.h>
#include <directional/dcel.h>

namespace directional
{
  
  class ParameterizationData{
  public:
    Eigen::SparseMatrix<double> vertexTrans2CutMat;
    Eigen::SparseMatrix<double> constraintMat;
    Eigen::SparseMatrix<double> symmMat;
    Eigen::VectorXi constrainedVertices;
    Eigen::VectorXi integerVars;
    Eigen::MatrixXi face2cut;
    
    ParameterizationData(){}
    ~ParameterizationData(){}
  };
  
  
  // Setting up the seamless parameterization algorithm
  // Input:
  //  N:          The degree of the field.
  //  wholeV:     #V x 3 vertex coordinates
  //  wholeF:     #F x 3 face vertex indices
  //  EV:         #E x 2 edges to vertices indices
  //  EF:         #E x 2 edges to faces indices
  // matching:    #E matching function, where vector k in EF(i,0) matches to vector (k+matching(k))%N in EF(i,1). In case of boundary, there is a -1. Most matching should be zero due to prior combing.
  // singVertices:list of singular vertices in wholeV.
  // Output:
  //  pd:         parameterization data subsequently used in directional::parameterize();
  //  cutV:       the Vertices of the cut mesh.
  //  cutF:       The Vaces of the cut mesh.
  
  IGL_INLINE void setup_parameterization(const int N,
                                         const Eigen::MatrixXd& wholeV,
                                         const Eigen::MatrixXi& wholeF,
                                         const Eigen::MatrixXi& EV,
                                         const Eigen::MatrixXi& EF,
                                         const Eigen::MatrixXi& FE,
                                         const Eigen::VectorXi& matching,
                                         const Eigen::VectorXi& singVertices,
                                         ParameterizationData& pd,
                                         Eigen::MatrixXd& cutV,
                                         Eigen::MatrixXi& cutF)
  {
    
    using namespace Eigen;
    using namespace std;
    
    assert(N==4 && "Seamless Parameterization currently only supports N=4");
    
    MatrixXi EFi,EH, FH;
    MatrixXd FEs;
    VectorXi VH, HV, HE, HF, nextH,prevH,twinH,innerEdges;
    
    VectorXi D=VectorXi::Constant(wholeF.rows(),3);
    VectorXi isSingular=VectorXi::Zero(wholeV.rows());
    for (int i=0;i<singVertices.size();i++)
      isSingular(singVertices(i))=1;
    
    pd.constrainedVertices=VectorXi::Zero(wholeV.rows());
    
    //computing extra topological information
    std::vector<int> innerEdgesVec;
    EFi=Eigen::MatrixXi::Constant(EF.rows(), 2,-1);
    FEs=Eigen::MatrixXd::Zero(FE.rows(),FE.cols());
    for (int i=0;i<EF.rows();i++)
      for (int k=0;k<2;k++){
        if (EF(i,k)==-1)
          continue;
        
        for (int j=0;j<D(EF(i,k));j++)
          if (FE(EF(i,k),j)==i)
            EFi(i,k)=j;
      }
    
    for (int i=0;i<EF.rows();i++){
      if (EFi(i,0)!=-1) FEs(EF(i,0),EFi(i,0))=1.0;
      if (EFi(i,1)!=-1) FEs(EF(i,1),EFi(i,1))=-1.0;
      if ((EF(i,0)!=-1)&&(EF(i,1)!=-1))
        innerEdgesVec.push_back(i);
    }
    
    innerEdges.resize(innerEdgesVec.size());
    for (int i=0;i<innerEdgesVec.size();i++)
      innerEdges(i)=innerEdgesVec[i];
    
    //hedra::polygonal_edge_topology(D,wholeF,EV,FE,EF,EFi, FEs,innerEdges);
    hedra::dcel(D,wholeF,EV,EF,EFi,innerEdges,VH,EH,FH,HV,HE,HF,nextH,prevH, twinH);
    
    VectorXi isBoundary = VectorXi::Zero(wholeV.rows());
    for (int i=0;i<HV.rows();i++)
      if (twinH(i)==-1)
        isBoundary(HV(i))=1;
    
    vector<MatrixXi> constParmMatrices(N);
    MatrixXi unitPermMatrix=MatrixXi::Zero(N,N);
    for (int i=0;i<N;i++)
      unitPermMatrix((i+1)%N,i)=1;
    
    constParmMatrices[0]=MatrixXi::Identity(N,N);
    for (int i=1;i<N;i++)
      constParmMatrices[i]=unitPermMatrix*constParmMatrices[i-1];
    
    
    VectorXi isSeam=VectorXi::Zero(EV.rows());
    for (int i=0;i<FE.rows();i++)
      for (int j=0;j<3;j++)
        if (pd.face2cut(i,j))
          isSeam(FE(i,j))=1;
    
    VectorXi isHEcut=VectorXi::Zero(HE.rows());
    
    for (int i=0;i<wholeF.rows();i++)
      for (int j=0;j<3;j++)
        if (pd.face2cut(i,j)){
          isHEcut(FH(i,j))=1;
        }
    
    VectorXi cutValence=VectorXi::Zero(wholeV.rows());
    for (int i=0;i<EV.rows();i++)
      if (isSeam(i)){
        cutValence(EV(i,0))++;
        cutValence(EV(i,1))++;
      }
    
    
    //establishing transition variables by tracing cut curves
    VectorXi Halfedge2TransitionIndices=VectorXi::Constant(HE.rows(),32767);
    VectorXi Halfedge2Matching(HE.rows());
    VectorXi isHEClaimed=VectorXi::Zero(HE.rows());
    //matching: EF(i,0)->EF(i,1), halfedge on i get the incoming matching, and so opposite.
    
    for (int i=0;i<HE.rows();i++){
      Halfedge2Matching(i)=(EH(HE(i),0)==i ? -matching(HE(i)) : matching(HE(i)));
      while (Halfedge2Matching(i)<0) Halfedge2Matching(i)+=N;
      //while (Halfedge2Matching(i)>=N/2) Halfedge2Matching(i)-=N;
    }
    
    int currTransition=1;
    
    //cutting mesh and creating map between wholeF and cutF
    //cutting the mesh
    vector<int> cut2whole;
    vector<RowVector3d> cutVlist;
    cutF.conservativeResize(wholeF.rows(),3);
    for (int i=0;i<VH.rows();i++){
      //creating corners whereever we have non-trivial matching
      int beginH=VH(i);
      int currH=beginH;
      
      //reseting to first cut or first boundary, if exists
      
      if (!isBoundary(i)){
        do{
          if (isHEcut(currH)!=0)
            break;
          currH=nextH(twinH(currH));
        }while (beginH!=currH);
      } else {
        do{
          if (twinH(currH)==-1)
            break;
          currH=nextH(twinH(currH));
        }while (twinH(currH)!=-1);
      }
      
      beginH=currH;
      
      do{
        if ((isHEcut(currH)!=0)||(beginH==currH)){
          cut2whole.push_back(i);
          cutVlist.push_back(wholeV.row(i));
        }
        
        for (int j=0;j<3;j++)
          if (wholeF(HF(currH),j)==i)
            cutF(HF(currH),j)=cut2whole.size()-1;
        
        currH=twinH(prevH(currH));
      }while ((beginH!=currH)&&(currH!=-1));
    }
    
    cutV.conservativeResize(cutVlist.size(),3);
    for (int i=0;i<cutVlist.size();i++)
      cutV.row(i)=cutVlist[i];
    
    //starting from each cut-graph node, we trace cut curves
    for (int i=0;i<wholeV.rows();i++){
      if (((cutValence(i)==2)&&(!isSingular(i)))||(cutValence(i)==0))
        continue;  //either mid-cut curve or non at all
      
      //if (isBoundary(i))
      //  continue;  //there is never a need to start with boundary vertices, and boundary curves don't get a transition variable
      
      //tracing curves until next node, if not already filled
      int beginH=VH(i);
      
      //reseting to first boundary
      int currH=beginH;
    
      if (isBoundary(i)){
        do{
          if (twinH(currH)==-1)
            break;
          currH=nextH(twinH(currH));
        }while (twinH(currH)!=-1);
      }
      
      beginH=currH;
    
      int nextHalfedgeInCut=-1;
      do{
        if ((isHEcut(currH)!=0)&&(isHEClaimed(currH)==0)&&(twinH(currH)!=-1)){ //unclaimed inner halfedge
          nextHalfedgeInCut=currH;
          Halfedge2TransitionIndices(nextHalfedgeInCut)=currTransition;
          Halfedge2TransitionIndices(twinH(nextHalfedgeInCut))=-currTransition;
          isHEClaimed(nextHalfedgeInCut)=1;
          isHEClaimed(twinH(nextHalfedgeInCut))=1;
          int nextCutVertex=HV(nextH(nextHalfedgeInCut));
          //advancing on the cut until next node
          while ((cutValence(nextCutVertex)==2)&&(!isSingular(nextCutVertex))&&(!isBoundary(nextCutVertex))){
            int beginH=VH(nextCutVertex);
            int currH=beginH;
            int nextHalfedgeInCut=-1;
            do{
              if ((isHEcut(currH)!=0)&&(isHEClaimed(currH)==0)) { //unclaimed cut halfedge
                nextHalfedgeInCut=currH;
                break;
              }
              currH=twinH(prevH(currH));
            }while (beginH!=currH);
            Halfedge2TransitionIndices(nextHalfedgeInCut)=currTransition;
            Halfedge2TransitionIndices(twinH(nextHalfedgeInCut))=-currTransition;
            isHEClaimed(nextHalfedgeInCut)=1;
            isHEClaimed(twinH(nextHalfedgeInCut))=1;
            nextCutVertex=HV(nextH(nextHalfedgeInCut));
          }
          
          currTransition++;
        }
        currH=twinH(prevH(currH));
      }while ((beginH!=currH)&&(currH!=-1));
    }
    
    int numTransitions=currTransition-1;
    
    vector<Triplet<double> > vertexTrans2CutTriplets, constTriplets;
  
    //forming the constraints and the singularity positions
    int currConst=0;
    for (int i=0;i<VH.rows();i++){
      std::vector<MatrixXi> permMatrices;
      std::vector<int> permIndices;  //in the space #V + #transitions
      //The initial corner gets the identity without any transition
      permMatrices.push_back(MatrixXi::Identity(N,N));
      permIndices.push_back(i);
      int beginH=VH(i);
      int currH=beginH;
      
      //reseting to first cut or boundary, if exists
      if (!isBoundary(i)){
        do{
          if (isHEcut(currH)!=0)
            break;
          currH=nextH(twinH(currH));
        }while (beginH!=currH);
      } else {
        do{
          if (twinH(currH)==-1)
            break;
          currH=nextH(twinH(currH));
        }while (twinH(currH)!=-1);
      }
      
      beginH=currH;
      
      int currCutVertex=-1;
      do{
        int currFace=HF(currH);
        int newCutVertex=-1;
        for (int j=0;j<3;j++)
          if (wholeF(currFace,j)==i)
            newCutVertex=cutF(currFace,j);
        
        //currCorner gets the permutations so far
        if (newCutVertex!=currCutVertex){
          currCutVertex=newCutVertex;
          
          for (int i=0;i<permIndices.size();i++)
            for (int j=0;j<N;j++)
              for (int k=0;k<N;k++)
                vertexTrans2CutTriplets.push_back(Triplet<double>(N*currCutVertex+j, N*permIndices[i]+k, (double)permMatrices[i](j,k)));
          
        }
        //updating the matrices for the next corner
        int nextHalfedge=twinH(prevH(currH));
        if (nextHalfedge==-1){  //reached a boundary
          currH=nextHalfedge;
          continue;
        }
        MatrixXi nextPermMatrix = constParmMatrices[Halfedge2Matching(nextHalfedge)%N];
        if (isHEcut(nextHalfedge)==0) { //no update needed
          currH=nextHalfedge;
          continue;
        }
        
        //otherwise, updating matrices with transition
        int nextTransition = Halfedge2TransitionIndices(nextHalfedge);
        if (nextTransition>0){  //Pe*f + Je
          for (int j=0;j<permMatrices.size();j++)
            permMatrices[j]=nextPermMatrix*permMatrices[j];
          
          //and identity on the fresh transition
          permMatrices.push_back(MatrixXi::Identity(N,N));
          permIndices.push_back(wholeV.rows()+nextTransition-1);
        } else { // (Pe*(f-Je))  matrix is already inverse since halfedge matching is minused
          //reverse order
          permMatrices.push_back(-MatrixXi::Identity(N,N));
          permIndices.push_back(wholeV.rows()-nextTransition-1);
          
          for (int j=0;j<permMatrices.size();j++)
            permMatrices[j]=nextPermMatrix*permMatrices[j];
          
        }
        currH=nextHalfedge;
      }while((currH!=beginH)&&(currH!=-1));
      
      //cleaning parmMatrices and permIndices to see if there is a constraint or reveal singularity-from-ransition
      std::set<int> cleanPermIndicesSet(permIndices.begin(), permIndices.end());
      std::vector<int> cleanPermIndices(cleanPermIndicesSet.begin(), cleanPermIndicesSet.end());
      std::vector<MatrixXi> cleanPermMatrices(cleanPermIndices.size());
      
      for (int j=0;j<cleanPermIndices.size();j++){
        cleanPermMatrices[j]=MatrixXi::Zero(N,N);
        for (int k=0;k<permIndices.size();k++)
          if (cleanPermIndices[j]==permIndices[k])
            cleanPermMatrices[j]+=permMatrices[k];
        if (cleanPermIndices[j]==i)
          cleanPermMatrices[j]-=MatrixXi::Identity(N,N);
      }
      
      //if not all matrices are zero, there is a constraint
      bool isConstraint=false;
      for (int j=0;j<cleanPermMatrices.size();j++)
        if (cleanPermMatrices[j].cwiseAbs().maxCoeff()!=0)
          isConstraint=true;
      
      if ((isConstraint)&&(!isBoundary(i))){
        for (int j=0;j<cleanPermMatrices.size();j++)
          for (int k=0;k<N;k++)
            for (int l=0;l<N;l++)
              constTriplets.push_back(Triplet<double>(N*currConst+k, N*cleanPermIndices[j]+l, (double)cleanPermMatrices[j](k,l)));
        currConst++;
        pd.constrainedVertices(i)=1;
      }
    }
    
    vector<Triplet<double>> cleanTriplets;
   
    pd.vertexTrans2CutMat.conservativeResize(N*cutV.rows(), N*(wholeV.rows()+numTransitions));
    cleanTriplets.clear();
    for (int i=0;i<vertexTrans2CutTriplets.size();i++)
      if (vertexTrans2CutTriplets[i].value()!=0.0)
        cleanTriplets.push_back(vertexTrans2CutTriplets[i]);
    pd.vertexTrans2CutMat.setFromTriplets(cleanTriplets.begin(), cleanTriplets.end());
    
    pd.constraintMat.conservativeResize(N*currConst, N*(wholeV.rows()+numTransitions));
    cleanTriplets.clear();
    for (int i=0;i<constTriplets.size();i++)
      if (constTriplets[i].value()!=0.0)
        cleanTriplets.push_back(constTriplets[i]);
    pd.constraintMat.setFromTriplets(cleanTriplets.begin(), cleanTriplets.end());
    
    //filtering out sign symmetry
    pd.symmMat.conservativeResize(N*(wholeV.rows()+numTransitions), N*(wholeV.rows()+numTransitions)/2);
    vector<Triplet<double>> symmMatTriplets;
    for (int i=0;i<N*(wholeV.rows()+numTransitions);i+=N){
      for (int j=0;j<N/2;j++){
        symmMatTriplets.push_back(Triplet<double>(i+j, i/2+j, 1.0));
        symmMatTriplets.push_back(Triplet<double>(i+j+N/2, i/2+j, -1.0));
      }
    }
    
    pd.symmMat.setFromTriplets(symmMatTriplets.begin(), symmMatTriplets.end());
    
    
    //in this case, also doing UV->UVW packing. This only works for N=6.
    /*if (N==6){
      SparseMatrix<double> baryMat(N*(wholeV.rows()+numTransitions)/2, N*(wholeV.rows()+numTransitions)/3);
      vector<Triplet<double>> baryMatTriplets;
      for (int i=0;i<N*(wholeV.rows()+numTransitions)/2;i+=N/2){
        baryMatTriplets.push_back(Triplet<double>(i, (i*2)/3, 1.0));
        baryMatTriplets.push_back(Triplet<double>(i+1, (i*2)/3+1, 1.0));
        baryMatTriplets.push_back(Triplet<double>(i+2, (i*2)/3, -1.0));
        baryMatTriplets.push_back(Triplet<double>(i+2, (i*2)/3+1, 1.0));
      }
      baryMat.setFromTriplets(baryMatTriplets.begin(), baryMatTriplets.end());
      symmMat = symmMat*baryMat;
      
      //cout<<"symmMat*VectorXd::Constant(symmMat.cols(),1.0): "<<symmMat*VectorXd::Constant(symmMat.cols(),1.0)<<endl;
      
      integerVars.conservativeResize(N*numTransitions/3);
      integerVars.setZero();
      for (int i=0;i<numTransitions;i++)
        for (int j=0;j<N/3;j++)
          integerVars(N*i/3+j) = N/3*(wholeV.rows()+i)+j;
    } else {*/
      pd.integerVars.conservativeResize(N*numTransitions/2);
      pd.integerVars.setZero();
      for (int i=0;i<numTransitions;i++)
        for (int j=0;j<N/2;j++)
          pd.integerVars(N*i/2+j) = N/2*(wholeV.rows()+i)+j;
    //}
  }
}




#endif


