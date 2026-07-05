/* ../FORTRAN/ARPACK/SRC/snapps.f -- translated by f2c (version 20100827).
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

static real c_b5 = 0.f;
static real c_b6 = 1.f;
static integer c__1 = 1;
static real c_b43 = -1.f;

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: snapps */

/* \Description: */
/*  Given the Arnoldi factorization */

/*     A*V_{k} - V_{k}*H_{k} = r_{k+p}*e_{k+p}^T, */

/*  apply NP implicit shifts resulting in */

/*     A*(V_{k}*Q) - (V_{k}*Q)*(Q^T* H_{k}*Q) = r_{k+p}*e_{k+p}^T * Q */

/*  where Q is an orthogonal matrix which is the product of rotations */
/*  and reflections resulting from the NP bulge chage sweeps. */
/*  The updated Arnoldi factorization becomes: */

/*     A*VNEW_{k} - VNEW_{k}*HNEW_{k} = rnew_{k}*e_{k}^T. */

/* \Usage: */
/*  call snapps */
/*     ( N, KEV, NP, SHIFTR, SHIFTI, V, LDV, H, LDH, RESID, Q, LDQ, */
/*       WORKL, WORKD ) */

/* \Arguments */
/*  N       Integer.  (INPUT) */
/*          Problem size, i.e. size of matrix A. */

/*  KEV     Integer.  (INPUT/OUTPUT) */
/*          KEV+NP is the size of the input matrix H. */
/*          KEV is the size of the updated matrix HNEW.  KEV is only */
/*          updated on ouput when fewer than NP shifts are applied in */
/*          order to keep the conjugate pair together. */

/*  NP      Integer.  (INPUT) */
/*          Number of implicit shifts to be applied. */

/*  SHIFTR, Real array of length NP.  (INPUT) */
/*  SHIFTI  Real and imaginary part of the shifts to be applied. */
/*          Upon, entry to snapps, the shifts must be sorted so that the */
/*          conjugate pairs are in consecutive locations. */

/*  V       Real N by (KEV+NP) array.  (INPUT/OUTPUT) */
/*          On INPUT, V contains the current KEV+NP Arnoldi vectors. */
/*          On OUTPUT, V contains the updated KEV Arnoldi vectors */
/*          in the first KEV columns of V. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling */
/*          program. */

/*  H       Real (KEV+NP) by (KEV+NP) array.  (INPUT/OUTPUT) */
/*          On INPUT, H contains the current KEV+NP by KEV+NP upper */
/*          Hessenber matrix of the Arnoldi factorization. */
/*          On OUTPUT, H contains the updated KEV by KEV upper Hessenberg */
/*          matrix in the KEV leading submatrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  RESID   Real array of length N.  (INPUT/OUTPUT) */
/*          On INPUT, RESID contains the the residual vector r_{k+p}. */
/*          On OUTPUT, RESID is the update residual vector rnew_{k} */
/*          in the first KEV locations. */

/*  Q       Real KEV+NP by KEV+NP work array.  (WORKSPACE) */
/*          Work array used to accumulate the rotations and reflections */
/*          during the bulge chase sweep. */

/*  LDQ     Integer.  (INPUT) */
/*          Leading dimension of Q exactly as declared in the calling */
/*          program. */

/*  WORKL   Real work array of length (KEV+NP).  (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end. */

/*  WORKD   Real work array of length 2*N.  (WORKSPACE) */
/*          Distributed array used in the application of the accumulated */
/*          orthogonal matrix Q. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  real */

/* \References: */
/*  1. D.C. Sorensen, "Implicit Application of Polynomial Filters in */
/*     a k-Step Arnoldi Method", SIAM J. Matr. Anal. Apps., 13 (1992), */
/*     pp 357-385. */

/* \Routines called: */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     smout   ARPACK utility routine that prints matrices. */
/*     svout   ARPACK utility routine that prints vectors. */
/*     slabad  LAPACK routine that computes machine constants. */
/*     slacpy  LAPACK matrix copy routine. */
/*     slamch  LAPACK routine that determines machine constants. */
/*     slanhs  LAPACK routine that computes various norms of a matrix. */
/*     slapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully. */
/*     slarf   LAPACK routine that applies Householder reflection to */
/*             a matrix. */
/*     slarfg  LAPACK Householder reflection construction routine. */
/*     slartg  LAPACK Givens rotation construction routine. */
/*     slaset  LAPACK matrix initialization routine. */
/*     sgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     saxpy   Level 1 BLAS that computes a vector triad. */
/*     scopy   Level 1 BLAS that copies one vector to another . */
/*     sscal   Level 1 BLAS that scales a vector. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     xx/xx/92: Version ' 2.4' */

/* \SCCS Information: @(#) */
/* FILE: napps.F   SID: 2.4   DATE OF SID: 3/28/97   RELEASE: 2 */

/* \Remarks */
/*  1. In this version, each shift is applied to all the sublocks of */
/*     the Hessenberg matrix H and not just to the submatrix that it */
/*     comes from. Deflation as in LAPACK routine slahqr (QR algorithm */
/*     for upper Hessenberg matrices ) is used. */
/*     The subdiagonals of H are enforced to be non-negative. */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int snapps_(integer *n, integer *kev, integer *np, real *
	shiftr, real *shifti, real *v, integer *ldv, real *h__, integer *ldh, 
	real *resid, real *q, integer *ldq, real *workl, real *workd)
{
    /* Initialized data */

    static logical first = TRUE_;

    /* System generated locals */
    integer h_dim1, h_offset, v_dim1, v_offset, q_dim1, q_offset, i__1, i__2, 
	    i__3, i__4;
    real r__1, r__2;

    /* Local variables */
    static real c__, f, g;
    static integer i__, j;
    static real r__, s, t, u[3], t0, t1, h11, h12, h21, h22, h32;
    static integer jj, ir, nr;
    static real tau, ulp, tst1;
    static integer iend;
    static real unfl, ovfl;
    static logical cconj;
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *), 
	    slarf_(char *, integer *, integer *, real *, integer *, real *, 
	    real *, integer *, real *, ftnlen), sgemv_(char *, integer *, 
	    integer *, real *, real *, integer *, real *, integer *, real *, 
	    real *, integer *, ftnlen), scopy_(integer *, real *, integer *, 
	    real *, integer *), saxpy_(integer *, real *, real *, integer *, 
	    real *, integer *), ivout_(integer *, integer *, integer *, 
	    integer *, char *, ftnlen), smout_(integer *, integer *, integer *
	    , real *, integer *, integer *, char *, ftnlen), svout_(integer *,
	     integer *, real *, integer *, char *, ftnlen);
    extern doublereal slapy2_(real *, real *);
    extern /* Subroutine */ int slabad_(real *, real *);
    extern doublereal slamch_(char *, ftnlen);
    static real sigmai;
    extern /* Subroutine */ int second_(real *);
    static real sigmar;
    static integer istart, kplusp, msglvl;
    static real smlnum;
    extern /* Subroutine */ int slacpy_(char *, integer *, integer *, real *, 
	    integer *, real *, integer *, ftnlen), slarfg_(integer *, real *, 
	    real *, integer *, real *), slaset_(char *, integer *, integer *, 
	    real *, real *, real *, integer *, ftnlen), slartg_(real *, real *
	    , real *, real *, real *);
    extern doublereal slanhs_(char *, integer *, real *, integer *, real *, 
	    ftnlen);


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


/*     %----------------------% */
/*     | Intrinsics Functions | */
/*     %----------------------% */


/*     %----------------% */
/*     | Data statments | */
/*     %----------------% */

    /* Parameter adjustments */
    --workd;
    --resid;
    --workl;
    --shifti;
    --shiftr;
    v_dim1 = *ldv;
    v_offset = 1 + v_dim1;
    v -= v_offset;
    h_dim1 = *ldh;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    q_dim1 = *ldq;
    q_offset = 1 + q_dim1;
    q -= q_offset;

    /* Function Body */

/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    if (first) {

/*        %-----------------------------------------------% */
/*        | Set machine-dependent constants for the       | */
/*        | stopping criterion. If norm(H) <= sqrt(OVFL), | */
/*        | overflow should not occur.                    | */
/*        | REFERENCE: LAPACK subroutine slahqr           | */
/*        %-----------------------------------------------% */

	unfl = slamch_("safe minimum", (ftnlen)12);
	ovfl = 1.f / unfl;
	slabad_(&unfl, &ovfl);
	ulp = slamch_("precision", (ftnlen)9);
	smlnum = unfl * (*n / ulp);
	first = FALSE_;
    }

/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    second_(&t0);
    msglvl = debug_1.mnapps;
    kplusp = *kev + *np;

/*     %--------------------------------------------% */
/*     | Initialize Q to the identity to accumulate | */
/*     | the rotations and reflections              | */
/*     %--------------------------------------------% */

    slaset_("All", &kplusp, &kplusp, &c_b5, &c_b6, &q[q_offset], ldq, (ftnlen)
	    3);

/*     %----------------------------------------------% */
/*     | Quick return if there are no shifts to apply | */
/*     %----------------------------------------------% */

    if (*np == 0) {
	goto L9000;
    }

/*     %----------------------------------------------% */
/*     | Chase the bulge with the application of each | */
/*     | implicit shift. Each shift is applied to the | */
/*     | whole matrix including each block.           | */
/*     %----------------------------------------------% */

    cconj = FALSE_;
    i__1 = *np;
    for (jj = 1; jj <= i__1; ++jj) {
	sigmar = shiftr[jj];
	sigmai = shifti[jj];

	if (msglvl > 2) {
	    ivout_(&debug_1.logfil, &c__1, &jj, &debug_1.ndigit, "_napps: sh"
		    "ift number.", (ftnlen)21);
	    svout_(&debug_1.logfil, &c__1, &sigmar, &debug_1.ndigit, "_napps"
		    ": The real part of the shift ", (ftnlen)35);
	    svout_(&debug_1.logfil, &c__1, &sigmai, &debug_1.ndigit, "_napps"
		    ": The imaginary part of the shift ", (ftnlen)40);
	}

/*        %-------------------------------------------------% */
/*        | The following set of conditionals is necessary  | */
/*        | in order that complex conjugate pairs of shifts | */
/*        | are applied together or not at all.             | */
/*        %-------------------------------------------------% */

	if (cconj) {

/*           %-----------------------------------------% */
/*           | cconj = .true. means the previous shift | */
/*           | had non-zero imaginary part.            | */
/*           %-----------------------------------------% */

	    cconj = FALSE_;
	    goto L110;
	} else if (jj < *np && dabs(sigmai) > 0.f) {

/*           %------------------------------------% */
/*           | Start of a complex conjugate pair. | */
/*           %------------------------------------% */

	    cconj = TRUE_;
	} else if (jj == *np && dabs(sigmai) > 0.f) {

/*           %----------------------------------------------% */
/*           | The last shift has a nonzero imaginary part. | */
/*           | Don't apply it; thus the order of the        | */
/*           | compressed H is order KEV+1 since only np-1  | */
/*           | were applied.                                | */
/*           %----------------------------------------------% */

	    ++(*kev);
	    goto L110;
	}
	istart = 1;
L20:

/*        %--------------------------------------------------% */
/*        | if sigmai = 0 then                               | */
/*        |    Apply the jj-th shift ...                     | */
/*        | else                                             | */
/*        |    Apply the jj-th and (jj+1)-th together ...    | */
/*        |    (Note that jj < np at this point in the code) | */
/*        | end                                              | */
/*        | to the current block of H. The next do loop      | */
/*        | determines the current block ;                   | */
/*        %--------------------------------------------------% */

	i__2 = kplusp - 1;
	for (i__ = istart; i__ <= i__2; ++i__) {

/*           %----------------------------------------% */
/*           | Check for splitting and deflation. Use | */
/*           | a standard test as in the QR algorithm | */
/*           | REFERENCE: LAPACK subroutine slahqr    | */
/*           %----------------------------------------% */

	    tst1 = (r__1 = h__[i__ + i__ * h_dim1], dabs(r__1)) + (r__2 = h__[
		    i__ + 1 + (i__ + 1) * h_dim1], dabs(r__2));
	    if (tst1 == 0.f) {
		i__3 = kplusp - jj + 1;
		tst1 = slanhs_("1", &i__3, &h__[h_offset], ldh, &workl[1], (
			ftnlen)1);
	    }
/* Computing MAX */
	    r__2 = ulp * tst1;
	    if ((r__1 = h__[i__ + 1 + i__ * h_dim1], dabs(r__1)) <= dmax(r__2,
		    smlnum)) {
		if (msglvl > 0) {
		    ivout_(&debug_1.logfil, &c__1, &i__, &debug_1.ndigit, 
			    "_napps: matrix splitting at row/column no.", (
			    ftnlen)42);
		    ivout_(&debug_1.logfil, &c__1, &jj, &debug_1.ndigit, 
			    "_napps: matrix splitting with shift number.", (
			    ftnlen)43);
		    svout_(&debug_1.logfil, &c__1, &h__[i__ + 1 + i__ * 
			    h_dim1], &debug_1.ndigit, "_napps: off diagonal "
			    "element.", (ftnlen)29);
		}
		iend = i__;
		h__[i__ + 1 + i__ * h_dim1] = 0.f;
		goto L40;
	    }
/* L30: */
	}
	iend = kplusp;
L40:

	if (msglvl > 2) {
	    ivout_(&debug_1.logfil, &c__1, &istart, &debug_1.ndigit, "_napps"
		    ": Start of current block ", (ftnlen)31);
	    ivout_(&debug_1.logfil, &c__1, &iend, &debug_1.ndigit, "_napps: "
		    "End of current block ", (ftnlen)29);
	}

/*        %------------------------------------------------% */
/*        | No reason to apply a shift to block of order 1 | */
/*        %------------------------------------------------% */

	if (istart == iend) {
	    goto L100;
	}

/*        %------------------------------------------------------% */
/*        | If istart + 1 = iend then no reason to apply a       | */
/*        | complex conjugate pair of shifts on a 2 by 2 matrix. | */
/*        %------------------------------------------------------% */

	if (istart + 1 == iend && dabs(sigmai) > 0.f) {
	    goto L100;
	}

	h11 = h__[istart + istart * h_dim1];
	h21 = h__[istart + 1 + istart * h_dim1];
	if (dabs(sigmai) <= 0.f) {

/*           %---------------------------------------------% */
/*           | Real-valued shift ==> apply single shift QR | */
/*           %---------------------------------------------% */

	    f = h11 - sigmar;
	    g = h21;

	    i__2 = iend - 1;
	    for (i__ = istart; i__ <= i__2; ++i__) {

/*              %-----------------------------------------------------% */
/*              | Contruct the plane rotation G to zero out the bulge | */
/*              %-----------------------------------------------------% */

		slartg_(&f, &g, &c__, &s, &r__);
		if (i__ > istart) {

/*                 %-------------------------------------------% */
/*                 | The following ensures that h(1:iend-1,1), | */
/*                 | the first iend-2 off diagonal of elements | */
/*                 | H, remain non negative.                   | */
/*                 %-------------------------------------------% */

		    if (r__ < 0.f) {
			r__ = -r__;
			c__ = -c__;
			s = -s;
		    }
		    h__[i__ + (i__ - 1) * h_dim1] = r__;
		    h__[i__ + 1 + (i__ - 1) * h_dim1] = 0.f;
		}

/*              %---------------------------------------------% */
/*              | Apply rotation to the left of H;  H <- G'*H | */
/*              %---------------------------------------------% */

		i__3 = kplusp;
		for (j = i__; j <= i__3; ++j) {
		    t = c__ * h__[i__ + j * h_dim1] + s * h__[i__ + 1 + j * 
			    h_dim1];
		    h__[i__ + 1 + j * h_dim1] = -s * h__[i__ + j * h_dim1] + 
			    c__ * h__[i__ + 1 + j * h_dim1];
		    h__[i__ + j * h_dim1] = t;
/* L50: */
		}

/*              %---------------------------------------------% */
/*              | Apply rotation to the right of H;  H <- H*G | */
/*              %---------------------------------------------% */

/* Computing MIN */
		i__4 = i__ + 2;
		i__3 = min(i__4,iend);
		for (j = 1; j <= i__3; ++j) {
		    t = c__ * h__[j + i__ * h_dim1] + s * h__[j + (i__ + 1) * 
			    h_dim1];
		    h__[j + (i__ + 1) * h_dim1] = -s * h__[j + i__ * h_dim1] 
			    + c__ * h__[j + (i__ + 1) * h_dim1];
		    h__[j + i__ * h_dim1] = t;
/* L60: */
		}

/*              %----------------------------------------------------% */
/*              | Accumulate the rotation in the matrix Q;  Q <- Q*G | */
/*              %----------------------------------------------------% */

/* Computing MIN */
		i__4 = i__ + jj;
		i__3 = min(i__4,kplusp);
		for (j = 1; j <= i__3; ++j) {
		    t = c__ * q[j + i__ * q_dim1] + s * q[j + (i__ + 1) * 
			    q_dim1];
		    q[j + (i__ + 1) * q_dim1] = -s * q[j + i__ * q_dim1] + 
			    c__ * q[j + (i__ + 1) * q_dim1];
		    q[j + i__ * q_dim1] = t;
/* L70: */
		}

/*              %---------------------------% */
/*              | Prepare for next rotation | */
/*              %---------------------------% */

		if (i__ < iend - 1) {
		    f = h__[i__ + 1 + i__ * h_dim1];
		    g = h__[i__ + 2 + i__ * h_dim1];
		}
/* L80: */
	    }

/*           %-----------------------------------% */
/*           | Finished applying the real shift. | */
/*           %-----------------------------------% */

	} else {

/*           %----------------------------------------------------% */
/*           | Complex conjugate shifts ==> apply double shift QR | */
/*           %----------------------------------------------------% */

	    h12 = h__[istart + (istart + 1) * h_dim1];
	    h22 = h__[istart + 1 + (istart + 1) * h_dim1];
	    h32 = h__[istart + 2 + (istart + 1) * h_dim1];

/*           %---------------------------------------------------------% */
/*           | Compute 1st column of (H - shift*I)*(H - conj(shift)*I) | */
/*           %---------------------------------------------------------% */

	    s = sigmar * 2.f;
	    t = slapy2_(&sigmar, &sigmai);
	    u[0] = (h11 * (h11 - s) + t * t) / h21 + h12;
	    u[1] = h11 + h22 - s;
	    u[2] = h32;

	    i__2 = iend - 1;
	    for (i__ = istart; i__ <= i__2; ++i__) {

/* Computing MIN */
		i__3 = 3, i__4 = iend - i__ + 1;
		nr = min(i__3,i__4);

/*              %-----------------------------------------------------% */
/*              | Construct Householder reflector G to zero out u(1). | */
/*              | G is of the form I - tau*( 1 u )' * ( 1 u' ).       | */
/*              %-----------------------------------------------------% */

		slarfg_(&nr, u, &u[1], &c__1, &tau);

		if (i__ > istart) {
		    h__[i__ + (i__ - 1) * h_dim1] = u[0];
		    h__[i__ + 1 + (i__ - 1) * h_dim1] = 0.f;
		    if (i__ < iend - 1) {
			h__[i__ + 2 + (i__ - 1) * h_dim1] = 0.f;
		    }
		}
		u[0] = 1.f;

/*              %--------------------------------------% */
/*              | Apply the reflector to the left of H | */
/*              %--------------------------------------% */

		i__3 = kplusp - i__ + 1;
		slarf_("Left", &nr, &i__3, u, &c__1, &tau, &h__[i__ + i__ * 
			h_dim1], ldh, &workl[1], (ftnlen)4);

/*              %---------------------------------------% */
/*              | Apply the reflector to the right of H | */
/*              %---------------------------------------% */

/* Computing MIN */
		i__3 = i__ + 3;
		ir = min(i__3,iend);
		slarf_("Right", &ir, &nr, u, &c__1, &tau, &h__[i__ * h_dim1 + 
			1], ldh, &workl[1], (ftnlen)5);

/*              %-----------------------------------------------------% */
/*              | Accumulate the reflector in the matrix Q;  Q <- Q*G | */
/*              %-----------------------------------------------------% */

		slarf_("Right", &kplusp, &nr, u, &c__1, &tau, &q[i__ * q_dim1 
			+ 1], ldq, &workl[1], (ftnlen)5);

/*              %----------------------------% */
/*              | Prepare for next reflector | */
/*              %----------------------------% */

		if (i__ < iend - 1) {
		    u[0] = h__[i__ + 1 + i__ * h_dim1];
		    u[1] = h__[i__ + 2 + i__ * h_dim1];
		    if (i__ < iend - 2) {
			u[2] = h__[i__ + 3 + i__ * h_dim1];
		    }
		}

/* L90: */
	    }

/*           %--------------------------------------------% */
/*           | Finished applying a complex pair of shifts | */
/*           | to the current block                       | */
/*           %--------------------------------------------% */

	}

L100:

/*        %---------------------------------------------------------% */
/*        | Apply the same shift to the next block if there is any. | */
/*        %---------------------------------------------------------% */

	istart = iend + 1;
	if (iend < kplusp) {
	    goto L20;
	}

/*        %---------------------------------------------% */
/*        | Loop back to the top to get the next shift. | */
/*        %---------------------------------------------% */

L110:
	;
    }

/*     %--------------------------------------------------% */
/*     | Perform a similarity transformation that makes   | */
/*     | sure that H will have non negative sub diagonals | */
/*     %--------------------------------------------------% */

    i__1 = *kev;
    for (j = 1; j <= i__1; ++j) {
	if (h__[j + 1 + j * h_dim1] < 0.f) {
	    i__2 = kplusp - j + 1;
	    sscal_(&i__2, &c_b43, &h__[j + 1 + j * h_dim1], ldh);
/* Computing MIN */
	    i__3 = j + 2;
	    i__2 = min(i__3,kplusp);
	    sscal_(&i__2, &c_b43, &h__[(j + 1) * h_dim1 + 1], &c__1);
/* Computing MIN */
	    i__3 = j + *np + 1;
	    i__2 = min(i__3,kplusp);
	    sscal_(&i__2, &c_b43, &q[(j + 1) * q_dim1 + 1], &c__1);
	}
/* L120: */
    }

    i__1 = *kev;
    for (i__ = 1; i__ <= i__1; ++i__) {

/*        %--------------------------------------------% */
/*        | Final check for splitting and deflation.   | */
/*        | Use a standard test as in the QR algorithm | */
/*        | REFERENCE: LAPACK subroutine slahqr        | */
/*        %--------------------------------------------% */

	tst1 = (r__1 = h__[i__ + i__ * h_dim1], dabs(r__1)) + (r__2 = h__[i__ 
		+ 1 + (i__ + 1) * h_dim1], dabs(r__2));
	if (tst1 == 0.f) {
	    tst1 = slanhs_("1", kev, &h__[h_offset], ldh, &workl[1], (ftnlen)
		    1);
	}
/* Computing MAX */
	r__1 = ulp * tst1;
	if (h__[i__ + 1 + i__ * h_dim1] <= dmax(r__1,smlnum)) {
	    h__[i__ + 1 + i__ * h_dim1] = 0.f;
	}
/* L130: */
    }

/*     %-------------------------------------------------% */
/*     | Compute the (kev+1)-st column of (V*Q) and      | */
/*     | temporarily store the result in WORKD(N+1:2*N). | */
/*     | This is needed in the residual update since we  | */
/*     | cannot GUARANTEE that the corresponding entry   | */
/*     | of H would be zero as in exact arithmetic.      | */
/*     %-------------------------------------------------% */

    if (h__[*kev + 1 + *kev * h_dim1] > 0.f) {
	sgemv_("N", n, &kplusp, &c_b6, &v[v_offset], ldv, &q[(*kev + 1) * 
		q_dim1 + 1], &c__1, &c_b5, &workd[*n + 1], &c__1, (ftnlen)1);
    }

/*     %----------------------------------------------------------% */
/*     | Compute column 1 to kev of (V*Q) in backward order       | */
/*     | taking advantage of the upper Hessenberg structure of Q. | */
/*     %----------------------------------------------------------% */

    i__1 = *kev;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = kplusp - i__ + 1;
	sgemv_("N", n, &i__2, &c_b6, &v[v_offset], ldv, &q[(*kev - i__ + 1) * 
		q_dim1 + 1], &c__1, &c_b5, &workd[1], &c__1, (ftnlen)1);
	scopy_(n, &workd[1], &c__1, &v[(kplusp - i__ + 1) * v_dim1 + 1], &
		c__1);
/* L140: */
    }

/*     %-------------------------------------------------% */
/*     |  Move v(:,kplusp-kev+1:kplusp) into v(:,1:kev). | */
/*     %-------------------------------------------------% */

    slacpy_("A", n, kev, &v[(kplusp - *kev + 1) * v_dim1 + 1], ldv, &v[
	    v_offset], ldv, (ftnlen)1);

/*     %--------------------------------------------------------------% */
/*     | Copy the (kev+1)-st column of (V*Q) in the appropriate place | */
/*     %--------------------------------------------------------------% */

    if (h__[*kev + 1 + *kev * h_dim1] > 0.f) {
	scopy_(n, &workd[*n + 1], &c__1, &v[(*kev + 1) * v_dim1 + 1], &c__1);
    }

/*     %-------------------------------------% */
/*     | Update the residual vector:         | */
/*     |    r <- sigmak*r + betak*v(:,kev+1) | */
/*     | where                               | */
/*     |    sigmak = (e_{kplusp}'*Q)*e_{kev} | */
/*     |    betak = e_{kev+1}'*H*e_{kev}     | */
/*     %-------------------------------------% */

    sscal_(n, &q[kplusp + *kev * q_dim1], &resid[1], &c__1);
    if (h__[*kev + 1 + *kev * h_dim1] > 0.f) {
	saxpy_(n, &h__[*kev + 1 + *kev * h_dim1], &v[(*kev + 1) * v_dim1 + 1],
		 &c__1, &resid[1], &c__1);
    }

    if (msglvl > 1) {
	svout_(&debug_1.logfil, &c__1, &q[kplusp + *kev * q_dim1], &
		debug_1.ndigit, "_napps: sigmak = (e_{kev+p}^T*Q)*e_{kev}", (
		ftnlen)40);
	svout_(&debug_1.logfil, &c__1, &h__[*kev + 1 + *kev * h_dim1], &
		debug_1.ndigit, "_napps: betak = e_{kev+1}^T*H*e_{kev}", (
		ftnlen)37);
	ivout_(&debug_1.logfil, &c__1, kev, &debug_1.ndigit, "_napps: Order "
		"of the final Hessenberg matrix ", (ftnlen)45);
	if (msglvl > 2) {
	    smout_(&debug_1.logfil, kev, kev, &h__[h_offset], ldh, &
		    debug_1.ndigit, "_napps: updated Hessenberg matrix H for"
		    " next iteration", (ftnlen)54);
	}

    }

L9000:
    second_(&t1);
    timing_1.tnapps += t1 - t0;

    return 0;

/*     %---------------% */
/*     | End of snapps | */
/*     %---------------% */

} /* snapps_ */

