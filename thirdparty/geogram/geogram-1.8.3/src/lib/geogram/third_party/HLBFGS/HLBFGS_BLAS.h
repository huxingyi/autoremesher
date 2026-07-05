///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLBFGS                                                                    //
// http://www.loria.fr/~liuyang/software/HLBFGS/							 //
//                                                                           //
// HLBFGS is a hybrid L-BFGS optimization framework which unifies L-BFGS     //
// method, Preconditioned L-BFGS method and                                  //
// Preconditioned Conjugate Gradient method.                                 //
//                                                                           //
// Version 1.2                                                               //
// March 09, 2010                                                            //
//                                                                           //
// Copyright (C) 2009--2010                                                  //
// Yang Liu                                                                  //
//																			 //
// xueyuhanlang@gmail.com                                                    //
//                                                                           //
// HLBFGS is HLBFGS is freely available for non-commercial purposes.		 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef HLBFGS_BLAS_H
#define HLBFGS_BLAS_H

//! return \f$ \sum_{i=0}^{n-1} x_iy_i \f$
double HLBFGS_DDOT(const int n, const double *x, const double *y);
//!  \f$ y_i += \alpha x_i \f$
void HLBFGS_DAXPY(const int n, const double alpha, const double *x, double *y);
//! return \f$ \sqrt{\sum_{i=0}^{n-1} x_i^2} \f$
double HLBFGS_DNRM2(const int n, const double *x);
//!  \f$ x_i *= a \f$
void HLBFGS_DSCAL(const int n, const double a, double *x);

#endif /* HLBFGS_BLAS_H */
