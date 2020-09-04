/* ../FORTRAN/ARPACK/SRC/ssapps.f -- translated by f2c (version 20100827).
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

static real c_b4 = 0.f;
static real c_b5 = 1.f;
static integer c__1 = 1;
static real c_b20 = -1.f;

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: ssapps */

/* \Description: */
/*  Given the Arnoldi factorization */

/*     A*V_{k} - V_{k}*H_{k} = r_{k+p}*e_{k+p}^T, */

/*  apply NP shifts implicitly resulting in */

/*     A*(V_{k}*Q) - (V_{k}*Q)*(Q^T* H_{k}*Q) = r_{k+p}*e_{k+p}^T * Q */

/*  where Q is an orthogonal matrix of order KEV+NP. Q is the product of */
/*  rotations resulting from the NP bulge chasing sweeps.  The updated Arnoldi */
/*  factorization becomes: */

/*     A*VNEW_{k} - VNEW_{k}*HNEW_{k} = rnew_{k}*e_{k}^T. */

/* \Usage: */
/*  call ssapps */
/*     ( N, KEV, NP, SHIFT, V, LDV, H, LDH, RESID, Q, LDQ, WORKD ) */

/* \Arguments */
/*  N       Integer.  (INPUT) */
/*          Problem size, i.e. dimension of matrix A. */

/*  KEV     Integer.  (INPUT) */
/*          INPUT: KEV+NP is the size of the input matrix H. */
/*          OUTPUT: KEV is the size of the updated matrix HNEW. */

/*  NP      Integer.  (INPUT) */
/*          Number of implicit shifts to be applied. */

/*  SHIFT   Real array of length NP.  (INPUT) */
/*          The shifts to be applied. */

/*  V       Real N by (KEV+NP) array.  (INPUT/OUTPUT) */
/*          INPUT: V contains the current KEV+NP Arnoldi vectors. */
/*          OUTPUT: VNEW = V(1:n,1:KEV); the updated Arnoldi vectors */
/*          are in the first KEV columns of V. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling */
/*          program. */

/*  H       Real (KEV+NP) by 2 array.  (INPUT/OUTPUT) */
/*          INPUT: H contains the symmetric tridiagonal matrix of the */
/*          Arnoldi factorization with the subdiagonal in the 1st column */
/*          starting at H(2,1) and the main diagonal in the 2nd column. */
/*          OUTPUT: H contains the updated tridiagonal matrix in the */
/*          KEV leading submatrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  RESID   Real array of length (N).  (INPUT/OUTPUT) */
/*          INPUT: RESID contains the the residual vector r_{k+p}. */
/*          OUTPUT: RESID is the updated residual vector rnew_{k}. */

/*  Q       Real KEV+NP by KEV+NP work array.  (WORKSPACE) */
/*          Work array used to accumulate the rotations during the bulge */
/*          chase sweep. */

/*  LDQ     Integer.  (INPUT) */
/*          Leading dimension of Q exactly as declared in the calling */
/*          program. */

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
/*  2. R.B. Lehoucq, "Analysis and Implementation of an Implicitly */
/*     Restarted Arnoldi Iteration", Rice University Technical Report */
/*     TR95-13, Department of Computational and Applied Mathematics. */

/* \Routines called: */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     svout   ARPACK utility routine that prints vectors. */
/*     slamch  LAPACK routine that determines machine constants. */
/*     slartg  LAPACK Givens rotation construction routine. */
/*     slacpy  LAPACK matrix copy routine. */
/*     slaset  LAPACK matrix initialization routine. */
/*     sgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     saxpy   Level 1 BLAS that computes a vector triad. */
/*     scopy   Level 1 BLAS that copies one vector to another. */
/*     sscal   Level 1 BLAS that scales a vector. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     12/16/93: Version ' 2.4' */

/* \SCCS Information: @(#) */
/* FILE: sapps.F   SID: 2.6   DATE OF SID: 3/28/97   RELEASE: 2 */

/* \Remarks */
/*  1. In this version, each shift is applied to all the subblocks of */
/*     the tridiagonal matrix H and not just to the submatrix that it */
/*     comes from. This routine assumes that the subdiagonal elements */
/*     of H that are stored in h(1:kev+np,1) are nonegative upon input */
/*     and enforce this condition upon output. This version incorporates */
/*     deflation. See code for documentation. */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int ssapps_(integer *n, integer *kev, integer *np, real *
	shift, real *v, integer *ldv, real *h__, integer *ldh, real *resid, 
	real *q, integer *ldq, real *workd)
{
    /* Initialized data */

    static logical first = TRUE_;

    /* System generated locals */
    integer h_dim1, h_offset, q_dim1, q_offset, v_dim1, v_offset, i__1, i__2, 
	    i__3, i__4;
    real r__1, r__2;

    /* Local variables */
    static real c__, f, g;
    static integer i__, j;
    static real r__, s, a1, a2, a3, a4, t0, t1;
    static integer jj;
    static real big;
    static integer iend, itop;
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *), 
	    sgemv_(char *, integer *, integer *, real *, real *, integer *, 
	    real *, integer *, real *, real *, integer *, ftnlen), scopy_(
	    integer *, real *, integer *, real *, integer *), saxpy_(integer *
	    , real *, real *, integer *, real *, integer *), ivout_(integer *,
	     integer *, integer *, integer *, char *, ftnlen), svout_(integer 
	    *, integer *, real *, integer *, char *, ftnlen);
    extern doublereal slamch_(char *, ftnlen);
    extern /* Subroutine */ int second_(real *);
    static real epsmch;
    static integer istart, kplusp, msglvl;
    extern /* Subroutine */ int slacpy_(char *, integer *, integer *, real *, 
	    integer *, real *, integer *, ftnlen), slartg_(real *, real *, 
	    real *, real *, real *), slaset_(char *, integer *, integer *, 
	    real *, real *, real *, integer *, ftnlen);


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


/*     %---------------% */
/*     | Local Scalars | */
/*     %---------------% */



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
    --shift;
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
	epsmch = slamch_("Epsilon-Machine", (ftnlen)15);
	first = FALSE_;
    }
    itop = 1;

/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    second_(&t0);
    msglvl = debug_1.msapps;

    kplusp = *kev + *np;

/*     %----------------------------------------------% */
/*     | Initialize Q to the identity matrix of order | */
/*     | kplusp used to accumulate the rotations.     | */
/*     %----------------------------------------------% */

    slaset_("All", &kplusp, &kplusp, &c_b4, &c_b5, &q[q_offset], ldq, (ftnlen)
	    3);

/*     %----------------------------------------------% */
/*     | Quick return if there are no shifts to apply | */
/*     %----------------------------------------------% */

    if (*np == 0) {
	goto L9000;
    }

/*     %----------------------------------------------------------% */
/*     | Apply the np shifts implicitly. Apply each shift to the  | */
/*     | whole matrix and not just to the submatrix from which it | */
/*     | comes.                                                   | */
/*     %----------------------------------------------------------% */

    i__1 = *np;
    for (jj = 1; jj <= i__1; ++jj) {

	istart = itop;

/*        %----------------------------------------------------------% */
/*        | Check for splitting and deflation. Currently we consider | */
/*        | an off-diagonal element h(i+1,1) negligible if           | */
/*        |         h(i+1,1) .le. epsmch*( |h(i,2)| + |h(i+1,2)| )   | */
/*        | for i=1:KEV+NP-1.                                        | */
/*        | If above condition tests true then we set h(i+1,1) = 0.  | */
/*        | Note that h(1:KEV+NP,1) are assumed to be non negative.  | */
/*        %----------------------------------------------------------% */

L20:

/*        %------------------------------------------------% */
/*        | The following loop exits early if we encounter | */
/*        | a negligible off diagonal element.             | */
/*        %------------------------------------------------% */

	i__2 = kplusp - 1;
	for (i__ = istart; i__ <= i__2; ++i__) {
	    big = (r__1 = h__[i__ + (h_dim1 << 1)], dabs(r__1)) + (r__2 = h__[
		    i__ + 1 + (h_dim1 << 1)], dabs(r__2));
	    if (h__[i__ + 1 + h_dim1] <= epsmch * big) {
		if (msglvl > 0) {
		    ivout_(&debug_1.logfil, &c__1, &i__, &debug_1.ndigit, 
			    "_sapps: deflation at row/column no.", (ftnlen)35)
			    ;
		    ivout_(&debug_1.logfil, &c__1, &jj, &debug_1.ndigit, 
			    "_sapps: occured before shift number.", (ftnlen)
			    36);
		    svout_(&debug_1.logfil, &c__1, &h__[i__ + 1 + h_dim1], &
			    debug_1.ndigit, "_sapps: the corresponding off d"
			    "iagonal element", (ftnlen)46);
		}
		h__[i__ + 1 + h_dim1] = 0.f;
		iend = i__;
		goto L40;
	    }
/* L30: */
	}
	iend = kplusp;
L40:

	if (istart < iend) {

/*           %--------------------------------------------------------% */
/*           | Construct the plane rotation G'(istart,istart+1,theta) | */
/*           | that attempts to drive h(istart+1,1) to zero.          | */
/*           %--------------------------------------------------------% */

	    f = h__[istart + (h_dim1 << 1)] - shift[jj];
	    g = h__[istart + 1 + h_dim1];
	    slartg_(&f, &g, &c__, &s, &r__);

/*            %-------------------------------------------------------% */
/*            | Apply rotation to the left and right of H;            | */
/*            | H <- G' * H * G,  where G = G(istart,istart+1,theta). | */
/*            | This will create a "bulge".                           | */
/*            %-------------------------------------------------------% */

	    a1 = c__ * h__[istart + (h_dim1 << 1)] + s * h__[istart + 1 + 
		    h_dim1];
	    a2 = c__ * h__[istart + 1 + h_dim1] + s * h__[istart + 1 + (
		    h_dim1 << 1)];
	    a4 = c__ * h__[istart + 1 + (h_dim1 << 1)] - s * h__[istart + 1 + 
		    h_dim1];
	    a3 = c__ * h__[istart + 1 + h_dim1] - s * h__[istart + (h_dim1 << 
		    1)];
	    h__[istart + (h_dim1 << 1)] = c__ * a1 + s * a2;
	    h__[istart + 1 + (h_dim1 << 1)] = c__ * a4 - s * a3;
	    h__[istart + 1 + h_dim1] = c__ * a3 + s * a4;

/*            %----------------------------------------------------% */
/*            | Accumulate the rotation in the matrix Q;  Q <- Q*G | */
/*            %----------------------------------------------------% */

/* Computing MIN */
	    i__3 = istart + jj;
	    i__2 = min(i__3,kplusp);
	    for (j = 1; j <= i__2; ++j) {
		a1 = c__ * q[j + istart * q_dim1] + s * q[j + (istart + 1) * 
			q_dim1];
		q[j + (istart + 1) * q_dim1] = -s * q[j + istart * q_dim1] + 
			c__ * q[j + (istart + 1) * q_dim1];
		q[j + istart * q_dim1] = a1;
/* L60: */
	    }


/*            %----------------------------------------------% */
/*            | The following loop chases the bulge created. | */
/*            | Note that the previous rotation may also be  | */
/*            | done within the following loop. But it is    | */
/*            | kept separate to make the distinction among  | */
/*            | the bulge chasing sweeps and the first plane | */
/*            | rotation designed to drive h(istart+1,1) to  | */
/*            | zero.                                        | */
/*            %----------------------------------------------% */

	    i__2 = iend - 1;
	    for (i__ = istart + 1; i__ <= i__2; ++i__) {

/*               %----------------------------------------------% */
/*               | Construct the plane rotation G'(i,i+1,theta) | */
/*               | that zeros the i-th bulge that was created   | */
/*               | by G(i-1,i,theta). g represents the bulge.   | */
/*               %----------------------------------------------% */

		f = h__[i__ + h_dim1];
		g = s * h__[i__ + 1 + h_dim1];

/*               %----------------------------------% */
/*               | Final update with G(i-1,i,theta) | */
/*               %----------------------------------% */

		h__[i__ + 1 + h_dim1] = c__ * h__[i__ + 1 + h_dim1];
		slartg_(&f, &g, &c__, &s, &r__);

/*               %-------------------------------------------% */
/*               | The following ensures that h(1:iend-1,1), | */
/*               | the first iend-2 off diagonal of elements | */
/*               | H, remain non negative.                   | */
/*               %-------------------------------------------% */

		if (r__ < 0.f) {
		    r__ = -r__;
		    c__ = -c__;
		    s = -s;
		}

/*               %--------------------------------------------% */
/*               | Apply rotation to the left and right of H; | */
/*               | H <- G * H * G',  where G = G(i,i+1,theta) | */
/*               %--------------------------------------------% */

		h__[i__ + h_dim1] = r__;

		a1 = c__ * h__[i__ + (h_dim1 << 1)] + s * h__[i__ + 1 + 
			h_dim1];
		a2 = c__ * h__[i__ + 1 + h_dim1] + s * h__[i__ + 1 + (h_dim1 
			<< 1)];
		a3 = c__ * h__[i__ + 1 + h_dim1] - s * h__[i__ + (h_dim1 << 1)
			];
		a4 = c__ * h__[i__ + 1 + (h_dim1 << 1)] - s * h__[i__ + 1 + 
			h_dim1];

		h__[i__ + (h_dim1 << 1)] = c__ * a1 + s * a2;
		h__[i__ + 1 + (h_dim1 << 1)] = c__ * a4 - s * a3;
		h__[i__ + 1 + h_dim1] = c__ * a3 + s * a4;

/*               %----------------------------------------------------% */
/*               | Accumulate the rotation in the matrix Q;  Q <- Q*G | */
/*               %----------------------------------------------------% */

/* Computing MIN */
		i__4 = i__ + jj;
		i__3 = min(i__4,kplusp);
		for (j = 1; j <= i__3; ++j) {
		    a1 = c__ * q[j + i__ * q_dim1] + s * q[j + (i__ + 1) * 
			    q_dim1];
		    q[j + (i__ + 1) * q_dim1] = -s * q[j + i__ * q_dim1] + 
			    c__ * q[j + (i__ + 1) * q_dim1];
		    q[j + i__ * q_dim1] = a1;
/* L50: */
		}

/* L70: */
	    }

	}

/*        %--------------------------% */
/*        | Update the block pointer | */
/*        %--------------------------% */

	istart = iend + 1;

/*        %------------------------------------------% */
/*        | Make sure that h(iend,1) is non-negative | */
/*        | If not then set h(iend,1) <-- -h(iend,1) | */
/*        | and negate the last column of Q.         | */
/*        | We have effectively carried out a        | */
/*        | similarity on transformation H           | */
/*        %------------------------------------------% */

	if (h__[iend + h_dim1] < 0.f) {
	    h__[iend + h_dim1] = -h__[iend + h_dim1];
	    sscal_(&kplusp, &c_b20, &q[iend * q_dim1 + 1], &c__1);
	}

/*        %--------------------------------------------------------% */
/*        | Apply the same shift to the next block if there is any | */
/*        %--------------------------------------------------------% */

	if (iend < kplusp) {
	    goto L20;
	}

/*        %-----------------------------------------------------% */
/*        | Check if we can increase the the start of the block | */
/*        %-----------------------------------------------------% */

	i__2 = kplusp - 1;
	for (i__ = itop; i__ <= i__2; ++i__) {
	    if (h__[i__ + 1 + h_dim1] > 0.f) {
		goto L90;
	    }
	    ++itop;
/* L80: */
	}

/*        %-----------------------------------% */
/*        | Finished applying the jj-th shift | */
/*        %-----------------------------------% */

L90:
	;
    }

/*     %------------------------------------------% */
/*     | All shifts have been applied. Check for  | */
/*     | more possible deflation that might occur | */
/*     | after the last shift is applied.         | */
/*     %------------------------------------------% */

    i__1 = kplusp - 1;
    for (i__ = itop; i__ <= i__1; ++i__) {
	big = (r__1 = h__[i__ + (h_dim1 << 1)], dabs(r__1)) + (r__2 = h__[i__ 
		+ 1 + (h_dim1 << 1)], dabs(r__2));
	if (h__[i__ + 1 + h_dim1] <= epsmch * big) {
	    if (msglvl > 0) {
		ivout_(&debug_1.logfil, &c__1, &i__, &debug_1.ndigit, "_sapp"
			"s: deflation at row/column no.", (ftnlen)35);
		svout_(&debug_1.logfil, &c__1, &h__[i__ + 1 + h_dim1], &
			debug_1.ndigit, "_sapps: the corresponding off diago"
			"nal element", (ftnlen)46);
	    }
	    h__[i__ + 1 + h_dim1] = 0.f;
	}
/* L100: */
    }

/*     %-------------------------------------------------% */
/*     | Compute the (kev+1)-st column of (V*Q) and      | */
/*     | temporarily store the result in WORKD(N+1:2*N). | */
/*     | This is not necessary if h(kev+1,1) = 0.         | */
/*     %-------------------------------------------------% */

    if (h__[*kev + 1 + h_dim1] > 0.f) {
	sgemv_("N", n, &kplusp, &c_b5, &v[v_offset], ldv, &q[(*kev + 1) * 
		q_dim1 + 1], &c__1, &c_b4, &workd[*n + 1], &c__1, (ftnlen)1);
    }

/*     %-------------------------------------------------------% */
/*     | Compute column 1 to kev of (V*Q) in backward order    | */
/*     | taking advantage that Q is an upper triangular matrix | */
/*     | with lower bandwidth np.                              | */
/*     | Place results in v(:,kplusp-kev:kplusp) temporarily.  | */
/*     %-------------------------------------------------------% */

    i__1 = *kev;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = kplusp - i__ + 1;
	sgemv_("N", n, &i__2, &c_b5, &v[v_offset], ldv, &q[(*kev - i__ + 1) * 
		q_dim1 + 1], &c__1, &c_b4, &workd[1], &c__1, (ftnlen)1);
	scopy_(n, &workd[1], &c__1, &v[(kplusp - i__ + 1) * v_dim1 + 1], &
		c__1);
/* L130: */
    }

/*     %-------------------------------------------------% */
/*     |  Move v(:,kplusp-kev+1:kplusp) into v(:,1:kev). | */
/*     %-------------------------------------------------% */

    slacpy_("All", n, kev, &v[(*np + 1) * v_dim1 + 1], ldv, &v[v_offset], ldv,
	     (ftnlen)3);

/*     %--------------------------------------------% */
/*     | Copy the (kev+1)-st column of (V*Q) in the | */
/*     | appropriate place if h(kev+1,1) .ne. zero. | */
/*     %--------------------------------------------% */

    if (h__[*kev + 1 + h_dim1] > 0.f) {
	scopy_(n, &workd[*n + 1], &c__1, &v[(*kev + 1) * v_dim1 + 1], &c__1);
    }

/*     %-------------------------------------% */
/*     | Update the residual vector:         | */
/*     |    r <- sigmak*r + betak*v(:,kev+1) | */
/*     | where                               | */
/*     |    sigmak = (e_{kev+p}'*Q)*e_{kev}  | */
/*     |    betak = e_{kev+1}'*H*e_{kev}     | */
/*     %-------------------------------------% */

    sscal_(n, &q[kplusp + *kev * q_dim1], &resid[1], &c__1);
    if (h__[*kev + 1 + h_dim1] > 0.f) {
	saxpy_(n, &h__[*kev + 1 + h_dim1], &v[(*kev + 1) * v_dim1 + 1], &c__1,
		 &resid[1], &c__1);
    }

    if (msglvl > 1) {
	svout_(&debug_1.logfil, &c__1, &q[kplusp + *kev * q_dim1], &
		debug_1.ndigit, "_sapps: sigmak of the updated residual vect"
		"or", (ftnlen)45);
	svout_(&debug_1.logfil, &c__1, &h__[*kev + 1 + h_dim1], &
		debug_1.ndigit, "_sapps: betak of the updated residual vector"
		, (ftnlen)44);
	svout_(&debug_1.logfil, kev, &h__[(h_dim1 << 1) + 1], &debug_1.ndigit,
		 "_sapps: updated main diagonal of H for next iteration", (
		ftnlen)53);
	if (*kev > 1) {
	    i__1 = *kev - 1;
	    svout_(&debug_1.logfil, &i__1, &h__[h_dim1 + 2], &debug_1.ndigit, 
		    "_sapps: updated sub diagonal of H for next iteration", (
		    ftnlen)52);
	}
    }

    second_(&t1);
    timing_1.tsapps += t1 - t0;

L9000:
    return 0;

/*     %---------------% */
/*     | End of ssapps | */
/*     %---------------% */

} /* ssapps_ */

