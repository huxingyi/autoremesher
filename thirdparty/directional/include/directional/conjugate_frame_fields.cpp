// This file is part of Directional, a library for directional field processing.
//
// Copyright (C) 2014 Olga Diamanti <olga.diam@gmail.com>, 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include <directional/conjugate_frame_fields.h>
#include <igl/speye.h>
#include <igl/slice.h>
#include <directional/polyroots.h>
#include <directional/polyvector_to_raw.h>
#include <Eigen/Sparse>

#include <iostream>

namespace directional {
  class ConjugateFFSolver
  {
  public:
    IGL_INLINE ConjugateFFSolver(const ConjugateFFSolverData &_data,
                                 int _maxIter = 20,
                                 const double _lambdaOrtho = .1,
                                 const double _lambdaInit = 100,
                                 const double _lambdaMultFactor = 1.01,
                                 bool _doHardConstraints = true);
    IGL_INLINE double solve(const Eigen::VectorXi &isConstrained,
                            const Eigen::MatrixXd &initialSolution,
                            Eigen::MatrixXd &output);
    
  private:
    
    const ConjugateFFSolverData &data;
    
    //polyVF data
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> Acoeff, Bcoeff;
    Eigen::Matrix<double, Eigen::Dynamic, 2> pvU, pvV;
    double lambda;
    
    //parameters
    double lambdaOrtho;
    double lambdaInit,lambdaMultFactor;
    int maxIter;
    bool doHardConstraints;
    
    IGL_INLINE void localStep();
    IGL_INLINE void getPolyCoeffsForLocalSolve(const Eigen::Matrix<double, 4, 1> &s,
                                               const Eigen::Matrix<double, 4, 1> &z,
                                               Eigen::Matrix<double, Eigen::Dynamic, 1> &polyCoeff);
    
    IGL_INLINE void globalStep(const Eigen::Matrix<int, Eigen::Dynamic, 1>  &isConstrained,
                               const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1>  &Ak,
                               const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1>  &Bk);
    IGL_INLINE void minQuadWithKnownMini(const Eigen::SparseMatrix<std::complex<double> > &Q,
                                         const Eigen::SparseMatrix<std::complex<double> > &f,
                                         const Eigen::VectorXi isConstrained,
                                         const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> &xknown,
                                         Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> &x);
    IGL_INLINE void setFieldFromCoefficients();
    IGL_INLINE void setCoefficientsFromField();
    
  };
}

//Implementation
/***************************** Solver ***********************************/
IGL_INLINE directional::ConjugateFFSolver::ConjugateFFSolver(const ConjugateFFSolverData &_data,
                                                             int _maxIter,
                                                             const double _lambdaOrtho,
                                                             const double _lambdaInit,
                                                             const double _lambdaMultFactor,
                                                             bool _doHardConstraints):
data(_data),
lambdaOrtho(_lambdaOrtho),
lambdaInit(_lambdaInit),
maxIter(_maxIter),
lambdaMultFactor(_lambdaMultFactor),
doHardConstraints(_doHardConstraints)
{
  Acoeff.resize(data.numF,1);
  Bcoeff.resize(data.numF,1);
  pvU.setZero(data.numF, 2);
  pvV.setZero(data.numF, 2);
};



IGL_INLINE void directional::ConjugateFFSolver::
getPolyCoeffsForLocalSolve(const Eigen::Matrix<double, 4, 1> &s,
                           const Eigen::Matrix<double, 4, 1> &z,
                           Eigen::Matrix<double, Eigen::Dynamic, 1> &polyCoeff)
{
  double s0 = s(0);
  double s1 = s(1);
  double s2 = s(2);
  double s3 = s(3);
  double z0 = z(0);
  double z1 = z(1);
  double z2 = z(2);
  double z3 = z(3);
  
  polyCoeff.resize(7,1);
  polyCoeff(0) =  s0*s0* s1*s1* s2*s2* s3* z3*z3 +  s0*s0* s1*s1* s2* s3*s3* z2*z2 +  s0*s0* s1* s2*s2* s3*s3* z1*z1 +  s0* s1*s1* s2*s2* s3*s3* z0*z0 ;
  polyCoeff(1) = 2* s0*s0* s1*s1* s2* s3* z2*z2 + 2* s0*s0* s1*s1* s2* s3* z3*z3 + 2* s0*s0* s1* s2*s2* s3* z1*z1 + 2* s0*s0* s1* s2*s2* s3* z3*z3 + 2* s0*s0* s1* s2* s3*s3* z1*z1 + 2* s0*s0* s1* s2* s3*s3* z2*z2 + 2* s0* s1*s1* s2*s2* s3* z0*z0 + 2* s0* s1*s1* s2*s2* s3* z3*z3 + 2* s0* s1*s1* s2* s3*s3* z0*z0 + 2* s0* s1*s1* s2* s3*s3* z2*z2 + 2* s0* s1* s2*s2* s3*s3* z0*z0 + 2* s0* s1* s2*s2* s3*s3* z1*z1 ;
  polyCoeff(2) =  s0*s0* s1*s1* s2* z2*z2 +  s0*s0* s1*s1* s3* z3*z3 +  s0*s0* s1* s2*s2* z1*z1 + 4* s0*s0* s1* s2* s3* z1*z1 + 4* s0*s0* s1* s2* s3* z2*z2 + 4* s0*s0* s1* s2* s3* z3*z3 +  s0*s0* s1* s3*s3* z1*z1 +  s0*s0* s2*s2* s3* z3*z3 +  s0*s0* s2* s3*s3* z2*z2 +  s0* s1*s1* s2*s2* z0*z0 + 4* s0* s1*s1* s2* s3* z0*z0 + 4* s0* s1*s1* s2* s3* z2*z2 + 4* s0* s1*s1* s2* s3* z3*z3 +  s0* s1*s1* s3*s3* z0*z0 + 4* s0* s1* s2*s2* s3* z0*z0 + 4* s0* s1* s2*s2* s3* z1*z1 + 4* s0* s1* s2*s2* s3* z3*z3 + 4* s0* s1* s2* s3*s3* z0*z0 + 4* s0* s1* s2* s3*s3* z1*z1 + 4* s0* s1* s2* s3*s3* z2*z2 +  s0* s2*s2* s3*s3* z0*z0 +  s1*s1* s2*s2* s3* z3*z3 +  s1*s1* s2* s3*s3* z2*z2 +  s1* s2*s2* s3*s3* z1*z1;
  polyCoeff(3) = 2* s0*s0* s1* s2* z1*z1 + 2* s0*s0* s1* s2* z2*z2 + 2* s0*s0* s1* s3* z1*z1 + 2* s0*s0* s1* s3* z3*z3 + 2* s0*s0* s2* s3* z2*z2 + 2* s0*s0* s2* s3* z3*z3 + 2* s0* s1*s1* s2* z0*z0 + 2* s0* s1*s1* s2* z2*z2 + 2* s0* s1*s1* s3* z0*z0 + 2* s0* s1*s1* s3* z3*z3 + 2* s0* s1* s2*s2* z0*z0 + 2* s0* s1* s2*s2* z1*z1 + 8* s0* s1* s2* s3* z0*z0 + 8* s0* s1* s2* s3* z1*z1 + 8* s0* s1* s2* s3* z2*z2 + 8* s0* s1* s2* s3* z3*z3 + 2* s0* s1* s3*s3* z0*z0 + 2* s0* s1* s3*s3* z1*z1 + 2* s0* s2*s2* s3* z0*z0 + 2* s0* s2*s2* s3* z3*z3 + 2* s0* s2* s3*s3* z0*z0 + 2* s0* s2* s3*s3* z2*z2 + 2* s1*s1* s2* s3* z2*z2 + 2* s1*s1* s2* s3* z3*z3 + 2* s1* s2*s2* s3* z1*z1 + 2* s1* s2*s2* s3* z3*z3 + 2* s1* s2* s3*s3* z1*z1 + 2* s1* s2* s3*s3* z2*z2 ;
  polyCoeff(4) =  s0*s0* s1* z1*z1 +  s0*s0* s2* z2*z2 +  s0*s0* s3* z3*z3 +  s0* s1*s1* z0*z0 + 4* s0* s1* s2* z0*z0 + 4* s0* s1* s2* z1*z1 + 4* s0* s1* s2* z2*z2 + 4* s0* s1* s3* z0*z0 + 4* s0* s1* s3* z1*z1 + 4* s0* s1* s3* z3*z3 +  s0* s2*s2* z0*z0 + 4* s0* s2* s3* z0*z0 + 4* s0* s2* s3* z2*z2 + 4* s0* s2* s3* z3*z3 +  s0* s3*s3* z0*z0 +  s1*s1* s2* z2*z2 +  s1*s1* s3* z3*z3 +  s1* s2*s2* z1*z1 + 4* s1* s2* s3* z1*z1 + 4* s1* s2* s3* z2*z2 + 4* s1* s2* s3* z3*z3 +  s1* s3*s3* z1*z1 +  s2*s2* s3* z3*z3 +  s2* s3*s3* z2*z2;
  polyCoeff(5) = 2* s0* s1* z0*z0 + 2* s0* s1* z1*z1 + 2* s0* s2* z0*z0 + 2* s0* s2* z2*z2 + 2* s0* s3* z0*z0 + 2* s0* s3* z3*z3 + 2* s1* s2* z1*z1 + 2* s1* s2* z2*z2 + 2* s1* s3* z1*z1 + 2* s1* s3* z3*z3 + 2* s2* s3* z2*z2 + 2* s2* s3* z3*z3 ;
  polyCoeff(6) =  s0* z0*z0 +  s1* z1*z1 +  s2* z2*z2 +  s3* z3*z3;
  
}


IGL_INLINE void directional::ConjugateFFSolver::localStep()
{
  for (int j =0; j<data.numF; ++j)
  {
    Eigen::Matrix<double, 4, 1> xproj; xproj << pvU.row(j).transpose(),pvV.row(j).transpose();
    Eigen::Matrix<double, 4, 1> z = data.UH[j].transpose()*xproj;
    Eigen::Matrix<double, 4, 1> x;
    
    Eigen::Matrix<double, Eigen::Dynamic, 1> polyCoeff;
    getPolyCoeffsForLocalSolve(data.s[j], z, polyCoeff);
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> roots;
    
    //directional::polyvector_to_raw(data.B1(j),data.B2(j), polyCoeff,4,roots);
    
    igl::polyRoots<double, double> (polyCoeff, roots);
    
    //  find closest real root to xproj
    double minDist = 1e10;
    for (int i =0; i< 6; ++i)
    {
      if (fabs(imag(roots[i]))>1e-10)
        continue;
      Eigen::Matrix<double, 4, 4> D = ((Eigen::Matrix<double, 4, 1>::Ones()+real(roots(i))*data.s[j]).array().inverse()).matrix().asDiagonal();
      Eigen::Matrix<double, 4, 1> candidate = data.UH[j]*D*z;
      double dist = (candidate-xproj).norm();
      if (dist<minDist)
      {
        minDist = dist;
        x = candidate;
      }
      
    }
    
    pvU.row(j) << x(0),x(1);
    pvV.row(j) << x(2),x(3);
  }
}


IGL_INLINE void directional::ConjugateFFSolver::setCoefficientsFromField()
{
  for (int i = 0; i <data.numF; ++i)
  {
    std::complex<double> u(pvU(i,0),pvU(i,1));
    std::complex<double> v(pvV(i,0),pvV(i,1));
    Acoeff(i) = u*u+v*v;
    Bcoeff(i) = u*u*v*v;
  }
}


IGL_INLINE void directional::ConjugateFFSolver::globalStep(const Eigen::Matrix<int, Eigen::Dynamic, 1>  &isConstrained,
                                                           const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1>  &Ak,
                                                           const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1>  &Bk)
{
  setCoefficientsFromField();
  
  Eigen::SparseMatrix<std::complex<double> > I;
  igl::speye(data.numF, data.numF, I);
  Eigen::SparseMatrix<std::complex<double> > QA = data.DDA+lambda*data.planarityWeight+lambdaOrtho*I;
  Eigen::SparseMatrix<std::complex<double> > fA = (-2*lambda*data.planarityWeight*Acoeff).sparseView();
  
  Eigen::SparseMatrix<std::complex<double> > QB = data.DDB+lambda*data.planarityWeight;
  Eigen::SparseMatrix<std::complex<double> > fB = (-2*lambda*data.planarityWeight*Bcoeff).sparseView();
  
  if(doHardConstraints)
  {
    minQuadWithKnownMini(QA, fA, isConstrained, Ak, Acoeff);
    minQuadWithKnownMini(QB, fB, isConstrained, Bk, Bcoeff);
  }
  else
  {
    Eigen::Matrix<int, Eigen::Dynamic, 1>isknown_; isknown_.setZero(data.numF,1);
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> xknown_; xknown_.setZero(0,1);
    minQuadWithKnownMini(QA, fA, isknown_, xknown_, Acoeff);
    minQuadWithKnownMini(QB, fB, isknown_, xknown_, Bcoeff);
  }
  setFieldFromCoefficients();
  
}


IGL_INLINE void directional::ConjugateFFSolver::setFieldFromCoefficients()
{
  for (int i = 0; i <data.numF; ++i)
  {
    //    poly coefficients: 1, 0, -Acoeff, 0, Bcoeff
    //    matlab code from roots (given there are no trailing zeros in the polynomial coefficients)
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> polyCoeff(5,1);
    polyCoeff<<1., 0., -Acoeff(i), 0., Bcoeff(i);
    
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> roots;
    igl::polyRoots<std::complex<double> >(polyCoeff,roots);
    
    std::complex<double> u = roots[0];
    int maxi = -1;
    float maxd = -1;
    for (int k =1; k<4; ++k)
    {
      float dist = abs(roots[k]+u);
      if (dist>maxd)
      {
        maxd = dist;
        maxi = k;
      }
    }
    std::complex<double> v = roots[maxi];
    pvU(i,0) = real(u); pvU(i,1) = imag(u);
    pvV(i,0) = real(v); pvV(i,1) = imag(v);
  }
  
}

IGL_INLINE void directional::ConjugateFFSolver::minQuadWithKnownMini(const Eigen::SparseMatrix<std::complex<double> > &Q,
                                                                     const Eigen::SparseMatrix<std::complex<double> > &f,
                                                                     const Eigen::VectorXi isConstrained,
                                                                     const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> &xknown,
                                                                     Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> &x)
{
  int N = Q.rows();
  
  int nc = xknown.rows();
  Eigen::VectorXi known; known.setZero(nc,1);
  Eigen::VectorXi unknown; unknown.setZero(N-nc,1);
  
  int indk = 0, indu = 0;
  for (int i = 0; i<N; ++i)
    if (isConstrained[i])
    {
      known[indk] = i;
      indk++;
    }
    else
    {
      unknown[indu] = i;
      indu++;
    }
  
  Eigen::SparseMatrix<std::complex<double>> Quu, Quk;
  
  igl::slice(Q,unknown, unknown, Quu);
  igl::slice(Q,unknown, known, Quk);
  
  
  std::vector<typename Eigen::Triplet<std::complex<double> > > tripletList;
  
  Eigen::SparseMatrix<std::complex<double> > fu(N-nc,1);
  
  igl::slice(f,unknown, Eigen::VectorXi::Zero(1,1), fu);
  
  Eigen::SparseMatrix<std::complex<double> > rhs = (Quk*xknown).sparseView()+.5*fu;
  
  Eigen::SparseLU< Eigen::SparseMatrix<std::complex<double>>> solver;
  solver.compute(-Quu);
  if(solver.info()!=Eigen::Success)
  {
    std::cerr<<"Decomposition failed!"<<std::endl;
    return;
  }
  Eigen::SparseMatrix<std::complex<double>>  b  = solver.solve(rhs);
  if(solver.info()!=Eigen::Success)
  {
    std::cerr<<"Solving failed!"<<std::endl;
    return;
  }
  
  indk = 0, indu = 0;
  x.setZero(N,1);
  for (int i = 0; i<N; ++i)
    if (isConstrained[i])
      x[i] = xknown[indk++];
    else
      x[i] = b.coeff(indu++,0);
  
}


IGL_INLINE double directional::ConjugateFFSolver::solve(const Eigen::VectorXi &isConstrained,
                                                        const Eigen::MatrixXd &initialSolution,
                                                        Eigen::MatrixXd &output)
{
  int numConstrained = isConstrained.sum();
  // coefficient values
  Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> Ak, Bk;
  
  pvU.resize(data.numF,2);
  pvV.resize(data.numF,2);
  for (int fi = 0; fi <data.numF; ++fi)
  {
    const Eigen::Matrix<double, 1, 3> &b1 = data.B1.row(fi);
    const Eigen::Matrix<double, 1, 3> &b2 = data.B2.row(fi);
    const Eigen::Matrix<double, 1, 3> &u3 = initialSolution.block(fi,0,1,3);
    const Eigen::Matrix<double, 1, 3> &v3 = initialSolution.block(fi,3,1,3);
    pvU.row(fi)<< u3.dot(b1), u3.dot(b2);
    pvV.row(fi)<< v3.dot(b1), v3.dot(b2);
  }
  setCoefficientsFromField();
  Ak.resize(numConstrained,1);
  Bk.resize(numConstrained,1);
  int ind = 0;
  for (int i = 0; i <data.numF; ++i)
  {
    if(isConstrained[i])
    {
      Ak(ind) = Acoeff[i];
      Bk(ind) = Bcoeff[i];
      ind ++;
    }
  }
  
  
  
  double smoothnessValue;
  Eigen::Matrix<double, Eigen::Dynamic, 1> conjValues;
  double meanConj;
  double maxConj;
  
  data.evaluateConjugacy(pvU, pvV, conjValues);
  meanConj = conjValues.cwiseAbs().mean();
  maxConj = conjValues.cwiseAbs().maxCoeff();
  printf("Initial max non-conjugacy: %.5g\n",maxConj);
  
  smoothnessValue = (Acoeff.adjoint()*data.DDA*Acoeff + Bcoeff.adjoint()*data.DDB*Bcoeff).real()[0];
  printf("\n\nInitial smoothness: %.5g\n",smoothnessValue);
  
  lambda = lambdaInit;
  
  bool doit = false;
  for (int iter = 0; iter<maxIter; ++iter)
  {
    printf("\n\n--- Iteration %d ---\n",iter);
    
    double oldMeanConj = meanConj;
    
    localStep();
    globalStep(isConstrained, Ak, Bk);
    
    
    smoothnessValue = (Acoeff.adjoint()*data.DDA*Acoeff + Bcoeff.adjoint()*data.DDB*Bcoeff).real()[0];
    
    printf("Smoothness: %.5g\n",smoothnessValue);
    
    data.evaluateConjugacy(pvU, pvV, conjValues);
    meanConj = conjValues.cwiseAbs().mean();
    maxConj = conjValues.cwiseAbs().maxCoeff();
    printf("Mean/Max non-conjugacy: %.5g, %.5g\n",meanConj,maxConj);
    double diffMeanConj = fabs(oldMeanConj-meanConj);
    
    if (diffMeanConj<1e-4)
      doit = true;
    
    if (doit)
      lambda = lambda*lambdaMultFactor;
    printf(" %d %.5g %.5g\n",iter, smoothnessValue,maxConj);
    
  }
  
  output.setZero(data.numF,6);
  for (int fi=0; fi<data.numF; ++fi)
  {
    const Eigen::Matrix<double, 1, 3> &b1 = data.B1.row(fi);
    const Eigen::Matrix<double, 1, 3> &b2 = data.B2.row(fi);
    output.block(fi,0, 1, 3) = pvU(fi,0)*b1 + pvU(fi,1)*b2;
    output.block(fi,3, 1, 3) = pvV(fi,0)*b1 + pvV(fi,1)*b2;
  }
  
  return lambda;
}



IGL_INLINE void directional::conjugate_frame_fields(const Eigen::MatrixXd &V,
                                                    const Eigen::MatrixXi &F,
                                                    const Eigen::VectorXi &b,
                                                    const Eigen::MatrixXd &initialSolution,
                                                    Eigen::MatrixXd &output,
                                                    int maxIter,
                                                    const double lambdaOrtho,
                                                    const double lambdaInit,
                                                    const double lambdaMultFactor,
                                                    bool doHardConstraints)
{
  Eigen::VectorXi isConstrained = Eigen::VectorXi::Constant(initialSolution.rows(),0);
  for (unsigned i=0; i<b.size(); ++i)
    isConstrained(b(i)) = 1;
  Eigen::MatrixXd twoFieldMat =initialSolution.block(0,0,initialSolution.rows(),6);
  directional::ConjugateFFSolverData csdata(V, F);
  directional::ConjugateFFSolver cs(csdata, maxIter, lambdaOrtho, lambdaInit, lambdaMultFactor, doHardConstraints);
  cs.solve(isConstrained, twoFieldMat, output);
  output.conservativeResize(output.rows(), 2*output.cols());
  output.block(0,6,output.rows(),6) = -output.block(0,0,output.rows(),6);
}


IGL_INLINE double directional::conjugate_frame_fields(const directional::ConjugateFFSolverData &csdata,
                                                      const Eigen::VectorXi &b,
                                                      const Eigen::MatrixXd &initialSolution,
                                                      Eigen::MatrixXd &output,
                                                      int maxIter,
                                                      const double lambdaOrtho,
                                                      const double lambdaInit,
                                                      const double lambdaMultFactor,
                                                      bool doHardConstraints)
{
  Eigen::VectorXi isConstrained = Eigen::VectorXi::Constant(initialSolution.rows(),0);
  for (unsigned i=0; i<b.size(); ++i)
    isConstrained(b(i)) = 1;
  Eigen::MatrixXd twoFieldMat =initialSolution.block(0,0,initialSolution.rows(),6);
  directional::ConjugateFFSolver cs(csdata, maxIter, lambdaOrtho, lambdaInit, lambdaMultFactor, doHardConstraints);
  double lambdaOut = cs.solve(isConstrained, twoFieldMat, output);
  
  //hack - CCW order might have been lost: reorienting
  Eigen::MatrixXd U =output.block(0,0,output.rows(),3);
  Eigen::MatrixXd V =output.block(0,3,output.rows(),3);
  
  for (int i=0;i<csdata.FN.rows();i++){
    Eigen::RowVector3d vec1=U.row(i);
    Eigen::RowVector3d vec2=csdata.FN.row(i);
    double orientation =(((vec1.cross(vec2))* (V.row(i).transpose())).sum() > 0 ? 1.0 : -1.0);
    output.block(i,3,1,3) = output.block(i,3,1,3).array()*orientation;
    
  }
  
  output.conservativeResize(output.rows(), 2*output.cols());
  output.block(0,6,output.rows(),6) = -output.block(0,0,output.rows(),6);
  return lambdaOut;
}


