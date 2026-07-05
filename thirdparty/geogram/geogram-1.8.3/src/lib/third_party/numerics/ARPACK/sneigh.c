/* ../FORTRAN/ARPACK/SRC/sneigh.f -- translated by f2c (version 20100827).
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

static logical c_true = TRUE_;
static integer c__1 = 1;
static real c_b18 = 1.f;
static real c_b20 = 0.f;

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: sneigh */

/* \Description: */
/*  Compute the eigenvalues of the current upper Hessenberg matrix */
/*  and the corresponding Ritz estimates given the current residual norm. */

/* \Usage: */
/*  call sneigh */
/*     ( RNORM, N, H, LDH, RITZR, RITZI, BOUNDS, Q, LDQ, WORKL, IERR ) */

/* \Arguments */
/*  RNORM   Real scalar.  (INPUT) */
/*          Residual norm corresponding to the current upper Hessenberg */
/*          matrix H. */

/*  N       Integer.  (INPUT) */
/*          Size of the matrix H. */

/*  H       Real N by N array.  (INPUT) */
/*          H contains the current upper Hessenberg matrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  RITZR,  Real arrays of length N.  (OUTPUT) */
/*  RITZI   On output, RITZR(1:N) (resp. RITZI(1:N)) contains the real */
/*          (respectively imaginary) parts of the eigenvalues of H. */

/*  BOUNDS  Real array of length N.  (OUTPUT) */
/*          On output, BOUNDS contains the Ritz estimates associated with */
/*          the eigenvalues RITZR and RITZI.  This is equal to RNORM */
/*          times the last components of the eigenvectors corresponding */
/*          to the eigenvalues in RITZR and RITZI. */

/*  Q       Real N by N array.  (WORKSPACE) */
/*          Workspace needed to store the eigenvectors of H. */

/*  LDQ     Integer.  (INPUT) */
/*          Leading dimension of Q exactly as declared in the calling */
/*          program. */

/*  WORKL   Real work array of length N**2 + 3*N.  (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end.  This is needed to keep the full Schur form */
/*          of H and also in the calculation of the eigenvectors of H. */

/*  IERR    Integer.  (OUTPUT) */
/*          Error exit flag from slaqrb or strevc. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  real */

/* \Routines called: */
/*     slaqrb  ARPACK routine to compute the real Schur form of an */
/*             upper Hessenberg matrix and last row of the Schur vectors. */
/*     second  ARPACK utility routine for timing. */
/*     smout   ARPACK utility routine that prints matrices */
/*     svout   ARPACK utility routine that prints vectors. */
/*     slacpy  LAPACK matrix copy routine. */
/*     slapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully. */
/*     strevc  LAPACK routine to compute the eigenvectors of a matrix */
/*             in upper quasi-triangular form */
/*     sgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     scopy   Level 1 BLAS that copies one vector to another . */
/*     snrm2   Level 1 BLAS that computes the norm of a vector. */
/*     sscal   Level 1 BLAS that scales a vector. */


/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     xx/xx/92: Version ' 2.1' */

/* \SCCS Information: @(#) */
/* FILE: neigh.F   SID: 2.3   DATE OF SID: 4/20/96   RELEASE: 2 */

/* \Remarks */
/*     None */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int sneigh_(real *rnorm, integer *n, real *h__, integer *ldh,
	 real *ritzr, real *ritzi, real *bounds, real *q, integer *ldq, real *
	workl, integer *ierr)
{
    /* System generated locals */
    integer h_dim1, h_offset, q_dim1, q_offset, i__1;
    real r__1, r__2;

    /* Local variables */
    static integer i__;
    static real t0, t1, vl[1], temp;
    extern doublereal snrm2_(integer *, real *, integer *);
    static integer iconj;
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *), 
	    sgemv_(char *, integer *, integer *, real *, real *, integer *, 
	    real *, integer *, real *, real *, integer *, ftnlen), smout_(
	    integer *, integer *, integer *, real *, integer *, integer *, 
	    char *, ftnlen), svout_(integer *, integer *, real *, integer *, 
	    char *, ftnlen);
    extern doublereal slapy2_(real *, real *);
    extern /* Subroutine */ int second_(real *);
    static logical select[1];
    static integer msglvl;
    extern /* Subroutine */ int slacpy_(char *, integer *, integer *, real *, 
	    integer *, real *, integer *, ftnlen), slaqrb_(logical *, integer 
	    *, integer *, integer *, real *, integer *, real *, real *, real *
	    , integer *), strevc_(char *, char *, logical *, integer *, real *
	    , integer *, real *, integer *, real *, integer *, integer *, 
	    integer *, real *, integer *, ftnlen, ftnlen);


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


/*     %---------------------% */
/*     | Intrinsic Functions | */
/*     %---------------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */


/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    /* Parameter adjustments */
    --workl;
    --bounds;
    --ritzi;
    --ritzr;
    h_dim1 = *ldh;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    q_dim1 = *ldq;
    q_offset = 1 + q_dim1;
    q -= q_offset;

    /* Function Body */
    second_(&t0);
    msglvl = debug_1.mneigh;

    if (msglvl > 2) {
	smout_(&debug_1.logfil, n, n, &h__[h_offset], ldh, &debug_1.ndigit, 
		"_neigh: Entering upper Hessenberg matrix H ", (ftnlen)43);
    }

/*     %-----------------------------------------------------------% */
/*     | 1. Compute the eigenvalues, the last components of the    | */
/*     |    corresponding Schur vectors and the full Schur form T  | */
/*     |    of the current upper Hessenberg matrix H.              | */
/*     | slaqrb returns the full Schur form of H in WORKL(1:N**2)  | */
/*     | and the last components of the Schur vectors in BOUNDS.   | */
/*     %-----------------------------------------------------------% */

    slacpy_("All", n, n, &h__[h_offset], ldh, &workl[1], n, (ftnlen)3);
    slaqrb_(&c_true, n, &c__1, n, &workl[1], n, &ritzr[1], &ritzi[1], &bounds[
	    1], ierr);
    if (*ierr != 0) {
	goto L9000;
    }

    if (msglvl > 1) {
	svout_(&debug_1.logfil, n, &bounds[1], &debug_1.ndigit, "_neigh: las"
		"t row of the Schur matrix for H", (ftnlen)42);
    }

/*     %-----------------------------------------------------------% */
/*     | 2. Compute the eigenvectors of the full Schur form T and  | */
/*     |    apply the last components of the Schur vectors to get  | */
/*     |    the last components of the corresponding eigenvectors. | */
/*     | Remember that if the i-th and (i+1)-st eigenvalues are    | */
/*     | complex conjugate pairs, then the real & imaginary part   | */
/*     | of the eigenvector components are split across adjacent   | */
/*     | columns of Q.                                             | */
/*     %-----------------------------------------------------------% */

    strevc_("R", "A", select, n, &workl[1], n, vl, n, &q[q_offset], ldq, n, n,
	     &workl[*n * *n + 1], ierr, (ftnlen)1, (ftnlen)1);

    if (*ierr != 0) {
	goto L9000;
    }

/*     %------------------------------------------------% */
/*     | Scale the returning eigenvectors so that their | */
/*     | euclidean norms are all one. LAPACK subroutine | */
/*     | strevc returns each eigenvector normalized so  | */
/*     | that the element of largest magnitude has      | */
/*     | magnitude 1; here the magnitude of a complex   | */
/*     | number (x,y) is taken to be |x| + |y|.         | */
/*     %------------------------------------------------% */

    iconj = 0;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if ((r__1 = ritzi[i__], dabs(r__1)) <= 0.f) {

/*           %----------------------% */
/*           | Real eigenvalue case | */
/*           %----------------------% */

	    temp = snrm2_(n, &q[i__ * q_dim1 + 1], &c__1);
	    r__1 = 1.f / temp;
	    sscal_(n, &r__1, &q[i__ * q_dim1 + 1], &c__1);
	} else {

/*           %-------------------------------------------% */
/*           | Complex conjugate pair case. Note that    | */
/*           | since the real and imaginary part of      | */
/*           | the eigenvector are stored in consecutive | */
/*           | columns, we further normalize by the      | */
/*           | square root of two.                       | */
/*           %-------------------------------------------% */

	    if (iconj == 0) {
		r__1 = snrm2_(n, &q[i__ * q_dim1 + 1], &c__1);
		r__2 = snrm2_(n, &q[(i__ + 1) * q_dim1 + 1], &c__1);
		temp = slapy2_(&r__1, &r__2);
		r__1 = 1.f / temp;
		sscal_(n, &r__1, &q[i__ * q_dim1 + 1], &c__1);
		r__1 = 1.f / temp;
		sscal_(n, &r__1, &q[(i__ + 1) * q_dim1 + 1], &c__1);
		iconj = 1;
	    } else {
		iconj = 0;
	    }
	}
/* L10: */
    }

    sgemv_("T", n, n, &c_b18, &q[q_offset], ldq, &bounds[1], &c__1, &c_b20, &
	    workl[1], &c__1, (ftnlen)1);

    if (msglvl > 1) {
	svout_(&debug_1.logfil, n, &workl[1], &debug_1.ndigit, "_neigh: Last"
		" row of the eigenvector matrix for H", (ftnlen)48);
    }

/*     %----------------------------% */
/*     | Compute the Ritz estimates | */
/*     %----------------------------% */

    iconj = 0;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if ((r__1 = ritzi[i__], dabs(r__1)) <= 0.f) {

/*           %----------------------% */
/*           | Real eigenvalue case | */
/*           %----------------------% */

	    bounds[i__] = *rnorm * (r__1 = workl[i__], dabs(r__1));
	} else {

/*           %-------------------------------------------% */
/*           | Complex conjugate pair case. Note that    | */
/*           | since the real and imaginary part of      | */
/*           | the eigenvector are stored in consecutive | */
/*           | columns, we need to take the magnitude    | */
/*           | of the last components of the two vectors | */
/*           %-------------------------------------------% */

	    if (iconj == 0) {
		bounds[i__] = *rnorm * slapy2_(&workl[i__], &workl[i__ + 1]);
		bounds[i__ + 1] = bounds[i__];
		iconj = 1;
	    } else {
		iconj = 0;
	    }
	}
/* L20: */
    }

    if (msglvl > 2) {
	svout_(&debug_1.logfil, n, &ritzr[1], &debug_1.ndigit, "_neigh: Real"
		" part of the eigenvalues of H", (ftnlen)41);
	svout_(&debug_1.logfil, n, &ritzi[1], &debug_1.ndigit, "_neigh: Imag"
		"inary part of the eigenvalues of H", (ftnlen)46);
	svout_(&debug_1.logfil, n, &bounds[1], &debug_1.ndigit, "_neigh: Rit"
		"z estimates for the eigenvalues of H", (ftnlen)47);
    }

    second_(&t1);
    timing_1.tneigh += t1 - t0;

L9000:
    return 0;

/*     %---------------% */
/*     | End of sneigh | */
/*     %---------------% */

} /* sneigh_ */

