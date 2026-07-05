#ifndef LINE_SEARCH_H
#define LINE_SEARCH_H

#include "HLBFGS_BLAS.h"

//The following functions are from LBFGS,VA35,TNPACK,CG+.
//for details, see lbfgs.f and va35.f

/*
The license of LBFGS:

This software is freely available for educational or commercial purposes.
This software is released under the GNU Public License (GPL)
*/

/*
MCSRCH is modified a little for Preconditioned CG.
*/

//!LINE SEARCH ROUTINE
int MCSRCH(int *n, double *x, double *f, double *g, double *s, double *stp,
		   double *ftol, double *gtol, double *xtol, double *stpmin,
		   double * stpmax, int *maxfev, int *info, int *nfev, double *wa,
		   int *keep, double *rkeep, double *cg_dginit = 0);

//!   MCSTEP ROUTINE
/*!
*   COMPUTE A SAFEGUARDED STEP FOR A LINESEARCH AND TO
*   UPDATE AN INTERVAL OF UNCERTAINTY FOR  A MINIMIZER OF THE FUNCTION
*/
int MCSTEP(double *stx, double *fx, double *dx, double *sty, double *fy,
		   double *dy, double *stp, double *fp, double *dp, bool *brackt,
		   double *stpmin, double *stpmax, int *info);

#endif
