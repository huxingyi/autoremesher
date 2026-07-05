/* ../FORTRAN/ARPACK/SRC/cneigh.f -- translated by f2c (version 20100827).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c.h"

/* Common Block Declarations */

struct {
    integer logfil, ndigit, mgetv0, msaupd, msaup2, msaitr, mseigt, msapps, 
	    msgets, mseupd, mnaupd, mnaup2, mnaitr, mneigh, mnapps, mngets, 
	    mneupd, mcaupd, mcaup2, mcaitr, mceigh, mcapps, mcgets, mceupd;
} debug_;

#define debug_1 debug_

struct {
    integer nopx, nbx, nrorth, nitref, nrstrt;
    real tsaupd, tsaup2, tsaitr, tseigt, tsgets, tsapps, tsconv, tnaupd, 
	    tnaup2, tnaitr, tneigh, tngets, tnapps, tnconv, tcaupd, tcaup2, 
	    tcaitr, tceigh, tcgets, tcapps, tcconv, tmvopx, tmvbx, tgetv0, 
	    titref, trvec;
} timing_;

#define timing_1 timing_

/* Table of constant values */

static complex c_b1 = {1.f,0.f};
static complex c_b2 = {0.f,0.f};
static logical c_true = TRUE_;
static integer c__1 = 1;

/* \BeginDoc */

/* \Name: cneigh */

/* \Description: */
/*  Compute the eigenvalues of the current upper Hessenberg matrix */
/*  and the corresponding Ritz estimates given the current residual norm. */

/* \Usage: */
/*  call cneigh */
/*     ( RNORM, N, H, LDH, RITZ, BOUNDS, Q, LDQ, WORKL, RWORK, IERR ) */

/* \Arguments */
/*  RNORM   Real scalar.  (INPUT) */
/*          Residual norm corresponding to the current upper Hessenberg */
/*          matrix H. */

/*  N       Integer.  (INPUT) */
/*          Size of the matrix H. */

/*  H       Complex N by N array.  (INPUT) */
/*          H contains the current upper Hessenberg matrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  RITZ    Complex array of length N.  (OUTPUT) */
/*          On output, RITZ(1:N) contains the eigenvalues of H. */

/*  BOUNDS  Complex array of length N.  (OUTPUT) */
/*          On output, BOUNDS contains the Ritz estimates associated with */
/*          the eigenvalues held in RITZ.  This is equal to RNORM */
/*          times the last components of the eigenvectors corresponding */
/*          to the eigenvalues in RITZ. */

/*  Q       Complex N by N array.  (WORKSPACE) */
/*          Workspace needed to store the eigenvectors of H. */

/*  LDQ     Integer.  (INPUT) */
/*          Leading dimension of Q exactly as declared in the calling */
/*          program. */

/*  WORKL   Complex work array of length N**2 + 3*N.  (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end.  This is needed to keep the full Schur form */
/*          of H and also in the calculation of the eigenvectors of H. */

/*  RWORK   Real  work array of length N (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end. */

/*  IERR    Integer.  (OUTPUT) */
/*          Error exit flag from clahqr or ctrevc. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  Complex */

/* \Routines called: */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     cmout   ARPACK utility routine that prints matrices */
/*     cvout   ARPACK utility routine that prints vectors. */
/*     svout   ARPACK utility routine that prints vectors. */
/*     clacpy  LAPACK matrix copy routine. */
/*     clahqr  LAPACK routine to compute the Schur form of an */
/*             upper Hessenberg matrix. */
/*     claset  LAPACK matrix initialization routine. */
/*     ctrevc  LAPACK routine to compute the eigenvectors of a matrix */
/*             in upper triangular form */
/*     ccopy   Level 1 BLAS that copies one vector to another. */
/*     csscal  Level 1 BLAS that scales a complex vector by a real number. */
/*     scnrm2  Level 1 BLAS that computes the norm of a vector. */


/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \SCCS Information: @(#) */
/* FILE: neigh.F   SID: 2.2   DATE OF SID: 4/20/96   RELEASE: 2 */

/* \Remarks */
/*     None */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int cneigh_(real *rnorm, integer *n, complex *h__, integer *
	ldh, complex *ritz, complex *bounds, complex *q, integer *ldq, 
	complex *workl, real *rwork, integer *ierr)
{
    /* System generated locals */
    integer h_dim1, h_offset, q_dim1, q_offset, i__1;
    real r__1;

    /* Local variables */
    static integer j;
    static real t0, t1;
    static complex vl[1];
    static real temp;
    extern /* Subroutine */ int ccopy_(integer *, complex *, integer *, 
	    complex *, integer *), cmout_(integer *, integer *, integer *, 
	    complex *, integer *, integer *, char *, ftnlen), cvout_(integer *
	    , integer *, complex *, integer *, char *, ftnlen);
    extern doublereal scnrm2_(integer *, complex *, integer *);
    extern /* Subroutine */ int csscal_(integer *, real *, complex *, integer 
	    *), clahqr_(logical *, logical *, integer *, integer *, integer *,
	     complex *, integer *, complex *, integer *, integer *, complex *,
	     integer *, integer *), clacpy_(char *, integer *, integer *, 
	    complex *, integer *, complex *, integer *, ftnlen);
    static logical select[1];
    static integer msglvl;
    extern /* Subroutine */ int ctrevc_(char *, char *, logical *, integer *, 
	    complex *, integer *, complex *, integer *, complex *, integer *, 
	    integer *, integer *, complex *, real *, integer *, ftnlen, 
	    ftnlen), second_(real *), claset_(char *, integer *, integer *, 
	    complex *, complex *, complex *, integer *, ftnlen);


/*     %----------------------------------------------------% */
/*     | Include files for debugging and timing information | */
/*     %----------------------------------------------------% */


/* \SCCS Information: @(#) */
/* FILE: debug.h   SID: 2.3   DATE OF SID: 11/16/95   RELEASE: 2 */

/*     %---------------------------------% */
/*     | See debug.doc for documentation | */
/*     %---------------------------------% */

/*     %------------------% */
/*     | Scalar Arguments | */
/*     %------------------% */

/*     %--------------------------------% */
/*     | See stat.doc for documentation | */
/*     %--------------------------------% */

/* \SCCS Information: @(#) */
/* FILE: stat.h   SID: 2.2   DATE OF SID: 11/16/95   RELEASE: 2 */



/*     %-----------------% */
/*     | Array Arguments | */
/*     %-----------------% */


/*     %------------% */
/*     | Parameters | */
/*     %------------% */


/*     %------------------------% */
/*     | Local Scalars & Arrays | */
/*     %------------------------% */


/*     %----------------------% */
/*     | External Subroutines | */
/*     %----------------------% */


/*     %--------------------% */
/*     | External Functions | */
/*     %--------------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    /* Parameter adjustments */
    --rwork;
    --workl;
    --bounds;
    --ritz;
    h_dim1 = *ldh;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    q_dim1 = *ldq;
    q_offset = 1 + q_dim1;
    q -= q_offset;

    /* Function Body */
    second_(&t0);
    msglvl = debug_1.mceigh;

    if (msglvl > 2) {
	cmout_(&debug_1.logfil, n, n, &h__[h_offset], ldh, &debug_1.ndigit, 
		"_neigh: Entering upper Hessenberg matrix H ", (ftnlen)43);
    }

/*     %----------------------------------------------------------% */
/*     | 1. Compute the eigenvalues, the last components of the   | */
/*     |    corresponding Schur vectors and the full Schur form T | */
/*     |    of the current upper Hessenberg matrix H.             | */
/*     |    clahqr returns the full Schur form of H               | */
/*     |    in WORKL(1:N**2), and the Schur vectors in q.         | */
/*     %----------------------------------------------------------% */

    clacpy_("All", n, n, &h__[h_offset], ldh, &workl[1], n, (ftnlen)3);
    claset_("All", n, n, &c_b2, &c_b1, &q[q_offset], ldq, (ftnlen)3);
    clahqr_(&c_true, &c_true, n, &c__1, n, &workl[1], ldh, &ritz[1], &c__1, n,
	     &q[q_offset], ldq, ierr);
    if (*ierr != 0) {
	goto L9000;
    }

    ccopy_(n, &q[*n - 1 + q_dim1], ldq, &bounds[1], &c__1);
    if (msglvl > 1) {
	cvout_(&debug_1.logfil, n, &bounds[1], &debug_1.ndigit, "_neigh: las"
		"t row of the Schur matrix for H", (ftnlen)42);
    }

/*     %----------------------------------------------------------% */
/*     | 2. Compute the eigenvectors of the full Schur form T and | */
/*     |    apply the Schur vectors to get the corresponding      | */
/*     |    eigenvectors.                                         | */
/*     %----------------------------------------------------------% */

    ctrevc_("Right", "Back", select, n, &workl[1], n, vl, n, &q[q_offset], 
	    ldq, n, n, &workl[*n * *n + 1], &rwork[1], ierr, (ftnlen)5, (
	    ftnlen)4);

    if (*ierr != 0) {
	goto L9000;
    }

/*     %------------------------------------------------% */
/*     | Scale the returning eigenvectors so that their | */
/*     | Euclidean norms are all one. LAPACK subroutine | */
/*     | ctrevc returns each eigenvector normalized so  | */
/*     | that the element of largest magnitude has      | */
/*     | magnitude 1; here the magnitude of a complex   | */
/*     | number (x,y) is taken to be |x| + |y|.         | */
/*     %------------------------------------------------% */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	temp = scnrm2_(n, &q[j * q_dim1 + 1], &c__1);
	r__1 = 1.f / temp;
	csscal_(n, &r__1, &q[j * q_dim1 + 1], &c__1);
/* L10: */
    }

    if (msglvl > 1) {
	ccopy_(n, &q[*n + q_dim1], ldq, &workl[1], &c__1);
	cvout_(&debug_1.logfil, n, &workl[1], &debug_1.ndigit, "_neigh: Last"
		" row of the eigenvector matrix for H", (ftnlen)48);
    }

/*     %----------------------------% */
/*     | Compute the Ritz estimates | */
/*     %----------------------------% */

    ccopy_(n, &q[*n + q_dim1], n, &bounds[1], &c__1);
    csscal_(n, rnorm, &bounds[1], &c__1);

    if (msglvl > 2) {
	cvout_(&debug_1.logfil, n, &ritz[1], &debug_1.ndigit, "_neigh: The e"
		"igenvalues of H", (ftnlen)28);
	cvout_(&debug_1.logfil, n, &bounds[1], &debug_1.ndigit, "_neigh: Rit"
		"z estimates for the eigenvalues of H", (ftnlen)47);
    }

    second_(&t1);
    timing_1.tceigh += t1 - t0;

L9000:
    return 0;

/*     %---------------% */
/*     | End of cneigh | */
/*     %---------------% */

} /* cneigh_ */

