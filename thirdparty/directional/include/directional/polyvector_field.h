// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2017 Daniele Panozzo <daniele.panozzo@gmail.com>, Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_POLYVECTOR_FIELD_H
#define DIRECTIONAL_POLYVECTOR_FIELD_H

#include <Eigen/Geometry>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/Polynomials>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/local_basis.h>
#include <igl/edge_topology.h>
#include <igl/speye.h>
#include <igl/eigs.h>
#include <iostream>

namespace directional
{
  
  // Precalculate the polyvector LDLt solvers. Must be recalculated whenever
  // bc changes or the mesh changes.
  // Inputs:
  //  V:      #V by 3 vertex coordinates.
  //  F:      #F by 3 face vertex indices.
  //  EV:     #E by 2 matrix of edges (vertex indices)
  //  EF:     #E by 2 matrix of oriented adjacent faces
  //  B1, B2: #F by 3 matrices representing the local base of each face.
  //  bc:     The face ids where the pv is prescribed.
  //  N:      The degree of the field.
  // Outputs:
  //  solver:       with prefactorized left-hand side
  //  AFull, AVar:  The resulting left-hand side matrices
  IGL_INLINE void polyvector_precompute(const Eigen::MatrixXd& V,
                                        const Eigen::MatrixXi& F,
                                        const Eigen::MatrixXi& EV,
                                        const Eigen::MatrixXi& EF,
                                        const Eigen::MatrixXd& B1,
                                        const Eigen::MatrixXd& B2,
                                        const Eigen::VectorXi& bc,
                                        const int N,
                                        Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>>& solver,
                                        Eigen::SparseMatrix<std::complex<double>>& Afull,
                                        Eigen::SparseMatrix<std::complex<double>>& AVar)
  {
    
    using namespace std;
    using namespace Eigen;
    int rowCounter=0;
    // Build the sparse matrix, with an energy term for each edge and degree
    std::vector< Triplet<complex<double> > > AfullTriplets;
    for (int n = 0; n < N; n++)
    {
      for (int i=0;i<EF.rows();i++){
        
        if ((EF(i,0)==-1)||(EF(i,1)==-1))
          continue;  //boundary edge
        
        // Compute the complex representation of the common edge
        RowVector3d e = V.row(EV(i,1)) - V.row(EV(i,0));
        RowVector2d vef = Vector2d(e.dot(B1.row(EF(i,0))), e.dot(B2.row(EF(i,0)))).normalized();
        complex<double> ef(vef(0), vef(1));
        Vector2d veg = Vector2d(e.dot(B1.row(EF(i,1))), e.dot(B2.row(EF(i,1)))).normalized();
        complex<double> eg(veg(0), veg(1));
        
        // Add the term conj(f)^n*ui - conj(g)^n*uj to the energy matrix
        AfullTriplets.push_back(Triplet<complex<double> >(rowCounter, n*F.rows()+EF(i,0), pow(conj(ef), N-n)));
        AfullTriplets.push_back(Triplet<complex<double> >(rowCounter++, n*F.rows()+EF(i,1), -1.*pow(conj(eg), N-n)));
      }
    }
    
    
    Afull.conservativeResize(rowCounter, N*F.rows());
    Afull.setFromTriplets(AfullTriplets.begin(), AfullTriplets.end());
    
    VectorXi constIndices(N*bc.size());
    
    for (int n=0;n<N;n++)
      constIndices.segment(bc.rows()*n,bc.rows())=bc.array()+n*F.rows();
    
    //removing columns pertaining to constant indices
    VectorXi varMask=VectorXi::Constant(N*F.rows(),1);
    for (int i=0;i<constIndices.size();i++)
      varMask(constIndices(i))=0;
    
    VectorXi full2var=VectorXi::Constant(N*F.rows(),-1);
    int varCounter=0;
    for (int i=0;i<N*F.rows();i++)
      if (varMask(i))
        full2var(i)=varCounter++;
    
    assert(varCounter==N*(F.rows()-bc.size()));
    
    std::vector< Triplet<std::complex<double> > > AVarTriplets;
    for (int i=0;i<AfullTriplets.size();i++)
      if (full2var(AfullTriplets[i].col())!=-1)
        AVarTriplets.push_back(Triplet<complex<double>>(AfullTriplets[i].row(), full2var(AfullTriplets[i].col()), AfullTriplets[i].value()));
    
    AVar.conservativeResize(rowCounter, N*(F.rows()-bc.size()));
    AVar.setFromTriplets(AVarTriplets.begin(), AVarTriplets.end());
    solver.compute(AVar.adjoint()*AVar);
  }
  
  
  // Computes a polyvector on the entire mesh from given values at the prescribed indices.
  // polyvector_precompute must be called in advance, and "b" must be on the given "bc"
  // If no constraints are given the Fielder eigenvector field will be returned.
  // Inputs:
  //  B1, B2:       #F by 3 matrices representing the local base of each face.
  //  bc:           The faces on which the polyvector is prescribed.
  //  b:            The directionals on the faces indicated by bc. Should be given in either #bc by N raw format X1,Y1,Z1,X2,Y2,Z2,Xn,Yn,Zn, or representative #bc by 3 format (single xyz), implying N-RoSy
  //  solver:       With prefactorized left-hand side
  //  Afull, AVar:  Left-hand side matrices (with and without constraints) of the system
  //  N:            The degree of the field.
  // Outputs:
  //  polyVectorField: #F by N The output interpolated field, in polyvector (complex polynomial) format.
  IGL_INLINE void polyvector_field(const Eigen::MatrixXd& B1,
                                   const Eigen::MatrixXd& B2,
                                   const Eigen::VectorXi& bc,
                                   const Eigen::MatrixXd& b,
                                   const Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>>& solver,
                                   const Eigen::SparseMatrix<std::complex<double>>& Afull,
                                   const Eigen::SparseMatrix<std::complex<double>>& AVar,
                                   const int N,
                                   Eigen::MatrixXcd& polyVectorField)
  {
    using namespace std;
    using namespace Eigen;
    
    assert(bc.size()==b.rows());
    assert(solver.rows()!=0);
    
    if (bc.size() == 0)
    {
      //extracting first eigenvector into the field
      //Have to use reals because libigl does not currently support complex eigs.
      SparseMatrix<double> M; igl::speye(2*B1.rows(), 2*B1.rows(), M);
      //creating a matrix of only the N-rosy interpolation
      SparseMatrix<std::complex<double> > AfullNRosy(Afull.rows()/N,Afull.cols()/N);
      std::vector<Triplet<std::complex<double> > > AfullNRosyTriplets;
      for (int k=0; k<Afull.outerSize(); ++k)
        for (SparseMatrix<std::complex<double> >::InnerIterator it(Afull,k); it; ++it)
        {
          if ((it.row()<Afull.rows()/N)&&(it.col()<Afull.cols()/N))
            AfullNRosyTriplets.push_back(Triplet<std::complex<double> > (it.row(), it.col(), it.value()));
        }
      
      AfullNRosy.setFromTriplets(AfullNRosyTriplets.begin(), AfullNRosyTriplets.end());
      
      SparseMatrix<std::complex<double>> LComplex =AfullNRosy.adjoint()*AfullNRosy;
      SparseMatrix<double> L(2*B1.rows(),2*B1.rows());
      std::vector<Triplet<double> > LTriplets;
      for (int k=0; k<LComplex.outerSize(); ++k)
        for (SparseMatrix<std::complex<double>>::InnerIterator it(LComplex,k); it; ++it)
        {
          LTriplets.push_back(Triplet<double>(it.row(), it.col(), it.value().real()));
          LTriplets.push_back(Triplet<double>(it.row(), LComplex.cols()+it.col(), -it.value().imag()));
          LTriplets.push_back(Triplet<double>(LComplex.rows()+it.row(), it.col(), it.value().imag()));
          LTriplets.push_back(Triplet<double>(LComplex.rows()+it.row(), LComplex.cols()+it.col(), it.value().real()));
        }
      L.setFromTriplets(LTriplets.begin(), LTriplets.end());
      Eigen::MatrixXd U;
      Eigen::VectorXd S;
      igl::eigs(L,M,5,igl::EIGS_TYPE_SM,U,S);
      
      polyVectorField=MatrixXcd::Constant(B1.rows(), N, complex<double>());
      
      polyVectorField.col(0) = U.block(0,0,U.rows()/2,1).cast<std::complex<double> >().array()*std::complex<double>(1,0)+
     U.block(U.rows()/2,0,U.rows()/2,1).cast<std::complex<double> >().array()*std::complex<double>(0,1); 
      return;
    }
    
    MatrixXcd constValuesMat(b.rows(),N);
    assert((b.cols()==3*N)||(b.cols()==3));
    if (b.cols()==3)  //N-RoSy constraint
    {
      constValuesMat.setZero();
      for (int i=0;i<b.rows();i++){
        complex<double> bComplex=complex<double>(b.row(i).dot(B1.row(bc(i))), b.row(i).dot(B2.row(bc(i))));
        constValuesMat(i,0)=pow(bComplex, N);
      }
    } else {
      for (int i=0;i<b.rows();i++){
        RowVectorXcd poly,roots(N);
        for (int n=0;n<N;n++){
          RowVector3d vec=b.block(i,3*n,1,3);
          roots(n)=complex<double>(vec.dot(B1.row(bc(i))), vec.dot(B2.row(bc(i))));
        }
        roots_to_monicPolynomial(roots, poly);
        constValuesMat.row(i)<<poly.head(N);
      }
    }
    
    VectorXi constIndices(N*bc.size());
    VectorXcd constValues(N*b.size());
    
    for (int n=0;n<N;n++){
      constIndices.segment(bc.rows()*n,bc.rows())=bc.array()+n*B1.rows();
      constValues.segment(b.rows()*n,b.rows())=constValuesMat.col(n);
    }
    
    VectorXcd torhs(N*B1.rows(),1);
    torhs.setZero();
    for (int i=0;i<constIndices.size();i++)
      torhs(constIndices(i))=constValues(i);
    
    VectorXcd rhs=-AVar.adjoint()*Afull*torhs;
    VectorXcd varFieldVector=solver.solve(rhs);
    assert(solver.info() == Success);
    
    VectorXcd polyVectorFieldVector(N*B1.rows());
    VectorXi varMask=VectorXi::Constant(N*B1.rows(),1);
    for (int i=0;i<constIndices.size();i++)
      varMask(constIndices(i))=0;
    
    VectorXi full2var=VectorXi::Constant(N*B1.rows(),-1);
    int varCounter=0;
    for (int i=0;i<N*B1.rows();i++)
      if (varMask(i))
        full2var(i)=varCounter++;
    
    assert(varCounter==N*(B1.rows()-bc.size()));
    
    for (int i=0;i<constIndices.size();i++)
      polyVectorFieldVector(constIndices(i))=constValues(i);
    
    for (int i=0;i<N*B1.rows();i++)
      if (full2var(i)!=-1)
        polyVectorFieldVector(i)=varFieldVector(full2var(i));
    
    //converting to matrix form
    polyVectorField.conservativeResize(B1.rows(),N);
    for (int n=0;n<N;n++)
      polyVectorField.col(n)=polyVectorFieldVector.segment(n*B1.rows(),B1.rows());
    
  }
  
  
  // minimal version without auxiliary data
  IGL_INLINE void polyvector_field(const Eigen::MatrixXd& V,
                                   const Eigen::MatrixXi& F,
                                   const Eigen::VectorXi& bc,
                                   const Eigen::MatrixXd& b,
                                   const int N,
                                   Eigen::MatrixXcd& polyVectorField)
  {
    Eigen::MatrixXi EV, xi, EF;
    igl::edge_topology(V, F, EV, xi, EF);
    Eigen::MatrixXd B1, B2, xd;
    igl::local_basis(V, F, B1, B2, xd);
    Eigen::SparseMatrix<std::complex<double>> Afull, AVar;
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<std::complex<double>>> solver;
    polyvector_precompute(V,F,EV,EF,B1,B2,bc,N, solver,Afull,AVar);
    polyvector_field(B1, B2, bc, b, solver, Afull, AVar, N, polyVectorField);
  }
}
#endif
