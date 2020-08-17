#ifndef H_ARPACK_PROTOS_H
#define H_ARPACK_PROTOS_H

/*
 * [BL] This file for having the right linkage declaration for ARPACK functions
 *  so that they are visible from outside the DLL / shared object.
 */

#include <third_party/numerics/linkage.h>
#include "f2c.h"

int NUMERICS_API dnaupd_(
    integer *ido, char *bmat, integer *n, char *
    which, integer *nev, doublereal *tol, doublereal *resid, integer *ncv,
    doublereal *v, integer *ldv, integer *iparam, integer *ipntr, 
    doublereal *workd, doublereal *workl, integer *lworkl, integer *info, 
    ftnlen bmat_len, ftnlen which_len
);

int NUMERICS_API dneupd_(
    logical *rvec, char *howmny, logical *select, 
    doublereal *dr, doublereal *di, doublereal *z__, integer *ldz, 
    doublereal *sigmar, doublereal *sigmai, doublereal *workev, char *
    bmat, integer *n, char *which, integer *nev, doublereal *tol, 
    doublereal *resid, integer *ncv, doublereal *v, integer *ldv, integer 
    *iparam, integer *ipntr, doublereal *workd, doublereal *workl, 
    integer *lworkl, integer *info, ftnlen howmny_len, ftnlen bmat_len, 
    ftnlen which_len
);

int NUMERICS_API dsaupd_(
    integer *ido, char *bmat, integer *n, char *
    which, integer *nev, doublereal *tol, doublereal *resid, integer *ncv,
    doublereal *v, integer *ldv, integer *iparam, integer *ipntr, 
    doublereal *workd, doublereal *workl, integer *lworkl, integer *info, 
    ftnlen bmat_len, ftnlen which_len
);

int NUMERICS_API dseupd_(
    logical *rvec, char *howmny, logical *select, 
    doublereal *d__, doublereal *z__, integer *ldz, doublereal *sigma, 
    char *bmat, integer *n, char *which, integer *nev, doublereal *tol, 
    doublereal *resid, integer *ncv, doublereal *v, integer *ldv, integer 
    *iparam, integer *ipntr, doublereal *workd, doublereal *workl, 
    integer *lworkl, integer *info, ftnlen howmny_len, ftnlen bmat_len, 
    ftnlen which_len
);

#endif
