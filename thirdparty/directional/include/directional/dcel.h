// This file is part of libhedra, a library for polyhedral mesh processing
//
// Copyright (C) 2016 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef HEDRA_DCEL_H
#define HEDRA_DCEL_H

#include <igl/igl_inline.h>
#include <Eigen/Core>
#include <vector>


namespace hedra
{
    // Created a Double-Connected Edge-List (a.k.a. "halfedge structure") from the usual
    // libhedra mesh representation. This data structure is very convenient for mesh editing
    // and traversing, and the data structure is again only Eigen vectors and matrices.
    
    //input:
    //  D           #F by 1 - face degrees
    //  F           #F by max(D) - vertex indices in face
    //  EV          #E by 2 - edge vertex indices
    //  EF          #E by 2 - edge face indices (EF(i,0) is left face, EF(i,1)=-1 if boundary
    //  EFi         #E by 2 - position of edge in face by EF
    //  innerEdges  vector of inner edges into EV
    
    // Output:
    // the number of halfedges can be determined by H=|HV/HE/HF|. It is 2*[Inner edges]+[Boundary Edges]
    // VH   #V by 1 - Vertex to outgoing halfedge (into HE)
    // EH   #E by 2 - edge to halfedge, where EH(i,0) halfedge is positively oriented, and EH(i,1)=-1 when boundary.
    // FH   #F by max(D) - face to (correctly oriented) halfedge s.t. the origin vertex of FH(i,j) is F(i,j)
    // HV   #H by 1 - origin vertex of the halfedge
    // HE   #H by 1 - edge carrying this halfedge. It does not say which direction.
    // HF   #F by 1 - face containing halfedge
    // nextH, prevH, twinH - #H by 1 DCEL traversing operations. twinH(i)=-1 for boundary edges.
    
    IGL_INLINE void dcel(const Eigen::MatrixXi& D,
                         const Eigen::MatrixXi& F,
                         const Eigen::MatrixXi& EV,
                         const Eigen::MatrixXi& EF,
                         const Eigen::MatrixXi& EFi,
                         const Eigen::VectorXi& innerEdges,
                         Eigen::VectorXi& VH,
                         Eigen::MatrixXi& EH,
                         Eigen::MatrixXi& FH,
                         Eigen::VectorXi& HV,
                         Eigen::VectorXi& HE,
                         Eigen::VectorXi& HF,
                         Eigen::VectorXi& nextH,
                         Eigen::VectorXi& prevH,
                         Eigen::VectorXi& twinH)
    {
        //doing a local halfedge structure for polygonal meshes
        EH=Eigen::MatrixXi::Constant(EV.rows(),2,-1);
        int numH=0;
        
        for (int i=0;i<EF.rows();i++){
            if (EF(i,0)!=-1)
                EH(i,0)=numH++;
            if (EF(i,1)!=-1)
                EH(i,1)=numH++;
        }
        
        
        //halfedges to edge
        HE.conservativeResize(numH);
        for (int i=0;i<EH.rows();i++){
            if (EH(i,0)!=-1)
                HE(EH(i,0))=i;
            if (EH(i,1)!=-1)
                HE(EH(i,1))=i;
        }
        
        //halfedge to vertex and vice versa
        HV.conservativeResize(numH);
        VH.conservativeResize(EV.maxCoeff()+1);
        for (int i=0;i<EV.rows();i++){
            if (EH(i,0)!=-1){
                HV(EH(i,0))=EV(i,0);
                VH(EV(i,0))=EH(i,0);
            }
            if (EH(i,1)!=-1){
                HV(EH(i,1))=EV(i,1);
                VH(EV(i,1))=EH(i,1);
            }
        }
        
        //halfedge to twin
        twinH=Eigen::VectorXi::Constant(numH, -1);
        for (int i=0;i<EH.rows();i++)
            if ((EH(i,0)!=-1)&&(EH(i,1)!=-1)){
                twinH(EH(i,0))=EH(i,1);
                twinH(EH(i,1))=EH(i,0);
            }
        
        //faces to halfedges and vice versa
        FH.resize(F.rows(), F.cols());
        HF.resize(numH);
        for (int i=0;i<EF.rows();i++){
            if (EF(i,0)!=-1){
                FH(EF(i,0),EFi(i,0))=EH(i,0);
                HF(EH(i,0))=EF(i,0);
            }
            if (EF(i,1)!=-1){
                FH(EF(i,1),EFi(i,1))=EH(i,1);
                HF(EH(i,1))=EF(i,1);
            }
        }
        
        //halfedge to next and prev
        nextH.conservativeResize(HE.rows());
        prevH.conservativeResize(HE.rows());
        for (int i=0;i<D.rows();i++){
            for (int j=0;j<D(i);j++){
                nextH(FH(i,j))=FH(i,(j+1)%D(i));
                prevH(FH(i,(j+1)%D(i)))=FH(i,j);
            }
        }
        
    }

}


#endif
