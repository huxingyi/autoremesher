/* ../FORTRAN/ARPACK/SRC/znapps.f -- translated by f2c (version 20100827).
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

static doublecomplex c_b1 = {1.,0.};
static doublecomplex c_b2 = {0.,0.};
static integer c__1 = 1;

/* \BeginDoc */

/* \Name: znapps */

/* \Description: */
/*  Given the Arnoldi factorization */

/*     A*V_{k} - V_{k}*H_{k} = r_{k+p}*e_{k+p}^T, */

/*  apply NP implicit shifts resulting in */

/*     A*(V_{k}*Q) - (V_{k}*Q)*(Q^T* H_{k}*Q) = r_{k+p}*e_{k+p}^T * Q */

/*  where Q is an orthogonal matrix which is the product of rotations */
/*  and reflections resulting from the NP bulge change sweeps. */
/*  The updated Arnoldi factorization becomes: */

/*     A*VNEW_{k} - VNEW_{k}*HNEW_{k} = rnew_{k}*e_{k}^T. */

/* \Usage: */
/*  call znapps */
/*     ( N, KEV, NP, SHIFT, V, LDV, H, LDH, RESID, Q, LDQ, */
/*       WORKL, WORKD ) */

/* \Arguments */
/*  N       Integer.  (INPUT) */
/*          Problem size, i.e. size of matrix A. */

/*  KEV     Integer.  (INPUT/OUTPUT) */
/*          KEV+NP is the size of the input matrix H. */
/*          KEV is the size of the updated matrix HNEW. */

/*  NP      Integer.  (INPUT) */
/*          Number of implicit shifts to be applied. */

/*  SHIFT   Complex*16 array of length NP.  (INPUT) */
/*          The shifts to be applied. */

/*  V       Complex*16 N by (KEV+NP) array.  (INPUT/OUTPUT) */
/*          On INPUT, V contains the current KEV+NP Arnoldi vectors. */
/*          On OUTPUT, V contains the updated KEV Arnoldi vectors */
/*          in the first KEV columns of V. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling */
/*          program. */

/*  H       Complex*16 (KEV+NP) by (KEV+NP) array.  (INPUT/OUTPUT) */
/*          On INPUT, H contains the current KEV+NP by KEV+NP upper */
/*          Hessenberg matrix of the Arnoldi factorization. */
/*          On OUTPUT, H contains the updated KEV by KEV upper Hessenberg */
/*          matrix in the KEV leading submatrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  RESID   Complex*16 array of length N.  (INPUT/OUTPUT) */
/*          On INPUT, RESID contains the the residual vector r_{k+p}. */
/*          On OUTPUT, RESID is the update residual vector rnew_{k} */
/*          in the first KEV locations. */

/*  Q       Complex*16 KEV+NP by KEV+NP work array.  (WORKSPACE) */
/*          Work array used to accumulate the rotations and reflections */
/*          during the bulge chase sweep. */

/*  LDQ     Integer.  (INPUT) */
/*          Leading dimension of Q exactly as declared in the calling */
/*          program. */

/*  WORKL   Complex*16 work array of length (KEV+NP).  (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end. */

/*  WORKD   Complex*16 work array of length 2*N.  (WORKSPACE) */
/*          Distributed array used in the application of the accumulated */
/*          orthogonal matrix Q. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  Complex*16 */

/* \References: */
/*  1. D.C. Sorensen, "Implicit Application of Polynomial Filters in */
/*     a k-Step Arnoldi Method", SIAM J. Matr. Anal. Apps., 13 (1992), */
/*     pp 357-385. */

/* \Routines called: */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     zmout   ARPACK utility routine that prints matrices */
/*     zvout   ARPACK utility routine that prints vectors. */
/*     zlacpy  LAPACK matrix copy routine. */
/*     zlanhs  LAPACK routine that computes various norms of a matrix. */
/*     zlartg  LAPACK Givens rotation construction routine. */
/*     zlaset  LAPACK matrix initialization routine. */
/*     dlabad  LAPACK routine for defining the underflow and overflow */
/*             limits. */
/*     dlamch  LAPACK routine that determines machine constants. */
/*     dlapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully. */
/*     zgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     zaxpy   Level 1 BLAS that computes a vector triad. */
/*     zcopy   Level 1 BLAS that copies one vector to another. */
/*     zscal   Level 1 BLAS that scales a vector. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \SCCS Information: @(#) */
/* FILE: napps.F   SID: 2.3   DATE OF SID: 3/28/97   RELEASE: 2 */

/* \Remarks */
/*  1. In this version, each shift is applied to all the sublocks of */
/*     the Hessenberg matrix H and not just to the submatrix that it */
/*     comes from. Deflation as in LAPACK routine zlahqr (QR algorithm */
/*     for upper Hessenberg matrices ) is used. */
/*     Upon output, the subdiagonals of H are enforced to be non-negative */
/*     real numbers. */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int znapps_(integer *n, integer *kev, integer *np, 
	doublecomplex *shift, doublecomplex *v, integer *ldv, doublecomplex *
	h__, integer *ldh, doublecomplex *resid, doublecomplex *q, integer *
	ldq, doublecomplex *workl, doublecomplex *workd)
{
    /* Initialized data */

    static logical first = TRUE_;

    /* System generated locals */
    integer h_dim1, h_offset, v_dim1, v_offset, q_dim1, q_offset, i__1, i__2, 
	    i__3, i__4, i__5, i__6;
    doublereal d__1, d__2, d__3, d__4;
    doublecomplex z__1, z__2, z__3, z__4, z__5;

    /* Builtin functions */
    double d_imag(doublecomplex *);
    void d_cnjg(doublecomplex *, doublecomplex *);

    /* Local variables */
    static doublereal c__;
    static doublecomplex f, g;
    static integer i__, j;
    static doublecomplex r__, s, t;
    static real t0, t1;
    static doublecomplex h11, h21;
    static integer jj;
    static doublereal ulp, tst1;
    static integer iend;
    static doublereal unfl, ovfl;
    static doublecomplex sigma;
    extern /* Subroutine */ int zscal_(integer *, doublecomplex *, 
	    doublecomplex *, integer *), zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *, ftnlen), 
	    zcopy_(integer *, doublecomplex *, integer *, doublecomplex *, 
	    integer *), ivout_(integer *, integer *, integer *, integer *, 
	    char *, ftnlen), zaxpy_(integer *, doublecomplex *, doublecomplex 
	    *, integer *, doublecomplex *, integer *), zmout_(integer *, 
	    integer *, integer *, doublecomplex *, integer *, integer *, char 
	    *, ftnlen), zvout_(integer *, integer *, doublecomplex *, integer 
	    *, char *, ftnlen);
    extern doublereal dlapy2_(doublereal *, doublereal *);
    extern /* Subroutine */ int dlabad_(doublereal *, doublereal *);
    extern doublereal dlamch_(char *, ftnlen);
    extern /* Subroutine */ int second_(real *);
    static integer istart, kplusp, msglvl;
    static doublereal smlnum;
    extern /* Subroutine */ int zlacpy_(char *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, ftnlen), 
	    zlartg_(doublecomplex *, doublecomplex *, doublereal *, 
	    doublecomplex *, doublecomplex *), zlaset_(char *, integer *, 
	    integer *, doublecomplex *, doublecomplex *, doublecomplex *, 
	    integer *, ftnlen);
    extern doublereal zlanhs_(char *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, ftnlen);


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


/*     %---------------------% */
/*     | Statement Functions | */
/*     %---------------------% */


/*     %----------------% */
/*     | Data statments | */
/*     %----------------% */

    /* Parameter adjustments */
    --workd;
    --resid;
    --workl;
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

/*        %-----------------------------------------------% */
/*        | Set machine-dependent constants for the       | */
/*        | stopping criterion. If norm(H) <= sqrt(OVFL), | */
/*        | overflow should not occur.                    | */
/*        | REFERENCE: LAPACK subroutine zlahqr           | */
/*        %-----------------------------------------------% */

	unfl = dlamch_("safe minimum", (ftnlen)12);
	z__1.r = 1. / unfl, z__1.i = 0. / unfl;
	ovfl = z__1.r;
	dlabad_(&unfl, &ovfl);
	ulp = dlamch_("precision", (ftnlen)9);
	smlnum = unfl * (*n / ulp);
	first = FALSE_;
    }

/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    second_(&t0);
    msglvl = debug_1.mcapps;

    kplusp = *kev + *np;

/*     %--------------------------------------------% */
/*     | Initialize Q to the identity to accumulate | */
/*     | the rotations and reflections              | */
/*     %--------------------------------------------% */

    zlaset_("All", &kplusp, &kplusp, &c_b2, &c_b1, &q[q_offset], ldq, (ftnlen)
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

    i__1 = *np;
    for (jj = 1; jj <= i__1; ++jj) {
	i__2 = jj;
	sigma.r = shift[i__2].r, sigma.i = shift[i__2].i;

	if (msglvl > 2) {
	    ivout_(&debug_1.logfil, &c__1, &jj, &debug_1.ndigit, "_napps: sh"
		    "ift number.", (ftnlen)21);
	    zvout_(&debug_1.logfil, &c__1, &sigma, &debug_1.ndigit, "_napps:"
		    " Value of the shift ", (ftnlen)27);
	}

	istart = 1;
L20:

	i__2 = kplusp - 1;
	for (i__ = istart; i__ <= i__2; ++i__) {

/*           %----------------------------------------% */
/*           | Check for splitting and deflation. Use | */
/*           | a standard test as in the QR algorithm | */
/*           | REFERENCE: LAPACK subroutine zlahqr    | */
/*           %----------------------------------------% */

	    i__3 = i__ + i__ * h_dim1;
	    i__4 = i__ + 1 + (i__ + 1) * h_dim1;
	    tst1 = (d__1 = h__[i__3].r, abs(d__1)) + (d__2 = d_imag(&h__[i__ 
		    + i__ * h_dim1]), abs(d__2)) + ((d__3 = h__[i__4].r, abs(
		    d__3)) + (d__4 = d_imag(&h__[i__ + 1 + (i__ + 1) * h_dim1]
		    ), abs(d__4)));
	    if (tst1 == 0.) {
		i__3 = kplusp - jj + 1;
		tst1 = zlanhs_("1", &i__3, &h__[h_offset], ldh, &workl[1], (
			ftnlen)1);
	    }
	    i__3 = i__ + 1 + i__ * h_dim1;
/* Computing MAX */
	    d__2 = ulp * tst1;
	    if ((d__1 = h__[i__3].r, abs(d__1)) <= max(d__2,smlnum)) {
		if (msglvl > 0) {
		    ivout_(&debug_1.logfil, &c__1, &i__, &debug_1.ndigit, 
			    "_napps: matrix splitting at row/column no.", (
			    ftnlen)42);
		    ivout_(&debug_1.logfil, &c__1, &jj, &debug_1.ndigit, 
			    "_napps: matrix splitting with shift number.", (
			    ftnlen)43);
		    zvout_(&debug_1.logfil, &c__1, &h__[i__ + 1 + i__ * 
			    h_dim1], &debug_1.ndigit, "_napps: off diagonal "
			    "element.", (ftnlen)29);
		}
		iend = i__;
		i__3 = i__ + 1 + i__ * h_dim1;
		h__[i__3].r = 0., h__[i__3].i = 0.;
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
/*        | or if the current block starts after the point | */
/*        | of compression since we'll discard this stuff  | */
/*        %------------------------------------------------% */

	if (istart == iend || istart > *kev) {
	    goto L100;
	}

	i__2 = istart + istart * h_dim1;
	h11.r = h__[i__2].r, h11.i = h__[i__2].i;
	i__2 = istart + 1 + istart * h_dim1;
	h21.r = h__[i__2].r, h21.i = h__[i__2].i;
	z__1.r = h11.r - sigma.r, z__1.i = h11.i - sigma.i;
	f.r = z__1.r, f.i = z__1.i;
	g.r = h21.r, g.i = h21.i;

	i__2 = iend - 1;
	for (i__ = istart; i__ <= i__2; ++i__) {

/*           %------------------------------------------------------% */
/*           | Construct the plane rotation G to zero out the bulge | */
/*           %------------------------------------------------------% */

	    zlartg_(&f, &g, &c__, &s, &r__);
	    if (i__ > istart) {
		i__3 = i__ + (i__ - 1) * h_dim1;
		h__[i__3].r = r__.r, h__[i__3].i = r__.i;
		i__3 = i__ + 1 + (i__ - 1) * h_dim1;
		h__[i__3].r = 0., h__[i__3].i = 0.;
	    }

/*           %---------------------------------------------% */
/*           | Apply rotation to the left of H;  H <- G'*H | */
/*           %---------------------------------------------% */

	    i__3 = kplusp;
	    for (j = i__; j <= i__3; ++j) {
		i__4 = i__ + j * h_dim1;
		z__2.r = c__ * h__[i__4].r, z__2.i = c__ * h__[i__4].i;
		i__5 = i__ + 1 + j * h_dim1;
		z__3.r = s.r * h__[i__5].r - s.i * h__[i__5].i, z__3.i = s.r *
			 h__[i__5].i + s.i * h__[i__5].r;
		z__1.r = z__2.r + z__3.r, z__1.i = z__2.i + z__3.i;
		t.r = z__1.r, t.i = z__1.i;
		i__4 = i__ + 1 + j * h_dim1;
		d_cnjg(&z__4, &s);
		z__3.r = -z__4.r, z__3.i = -z__4.i;
		i__5 = i__ + j * h_dim1;
		z__2.r = z__3.r * h__[i__5].r - z__3.i * h__[i__5].i, z__2.i =
			 z__3.r * h__[i__5].i + z__3.i * h__[i__5].r;
		i__6 = i__ + 1 + j * h_dim1;
		z__5.r = c__ * h__[i__6].r, z__5.i = c__ * h__[i__6].i;
		z__1.r = z__2.r + z__5.r, z__1.i = z__2.i + z__5.i;
		h__[i__4].r = z__1.r, h__[i__4].i = z__1.i;
		i__4 = i__ + j * h_dim1;
		h__[i__4].r = t.r, h__[i__4].i = t.i;
/* L50: */
	    }

/*           %---------------------------------------------% */
/*           | Apply rotation to the right of H;  H <- H*G | */
/*           %---------------------------------------------% */

/* Computing MIN */
	    i__4 = i__ + 2;
	    i__3 = min(i__4,iend);
	    for (j = 1; j <= i__3; ++j) {
		i__4 = j + i__ * h_dim1;
		z__2.r = c__ * h__[i__4].r, z__2.i = c__ * h__[i__4].i;
		d_cnjg(&z__4, &s);
		i__5 = j + (i__ + 1) * h_dim1;
		z__3.r = z__4.r * h__[i__5].r - z__4.i * h__[i__5].i, z__3.i =
			 z__4.r * h__[i__5].i + z__4.i * h__[i__5].r;
		z__1.r = z__2.r + z__3.r, z__1.i = z__2.i + z__3.i;
		t.r = z__1.r, t.i = z__1.i;
		i__4 = j + (i__ + 1) * h_dim1;
		z__3.r = -s.r, z__3.i = -s.i;
		i__5 = j + i__ * h_dim1;
		z__2.r = z__3.r * h__[i__5].r - z__3.i * h__[i__5].i, z__2.i =
			 z__3.r * h__[i__5].i + z__3.i * h__[i__5].r;
		i__6 = j + (i__ + 1) * h_dim1;
		z__4.r = c__ * h__[i__6].r, z__4.i = c__ * h__[i__6].i;
		z__1.r = z__2.r + z__4.r, z__1.i = z__2.i + z__4.i;
		h__[i__4].r = z__1.r, h__[i__4].i = z__1.i;
		i__4 = j + i__ * h_dim1;
		h__[i__4].r = t.r, h__[i__4].i = t.i;
/* L60: */
	    }

/*           %-----------------------------------------------------% */
/*           | Accumulate the rotation in the matrix Q;  Q <- Q*G' | */
/*           %-----------------------------------------------------% */

/* Computing MIN */
	    i__4 = i__ + jj;
	    i__3 = min(i__4,kplusp);
	    for (j = 1; j <= i__3; ++j) {
		i__4 = j + i__ * q_dim1;
		z__2.r = c__ * q[i__4].r, z__2.i = c__ * q[i__4].i;
		d_cnjg(&z__4, &s);
		i__5 = j + (i__ + 1) * q_dim1;
		z__3.r = z__4.r * q[i__5].r - z__4.i * q[i__5].i, z__3.i = 
			z__4.r * q[i__5].i + z__4.i * q[i__5].r;
		z__1.r = z__2.r + z__3.r, z__1.i = z__2.i + z__3.i;
		t.r = z__1.r, t.i = z__1.i;
		i__4 = j + (i__ + 1) * q_dim1;
		z__3.r = -s.r, z__3.i = -s.i;
		i__5 = j + i__ * q_dim1;
		z__2.r = z__3.r * q[i__5].r - z__3.i * q[i__5].i, z__2.i = 
			z__3.r * q[i__5].i + z__3.i * q[i__5].r;
		i__6 = j + (i__ + 1) * q_dim1;
		z__4.r = c__ * q[i__6].r, z__4.i = c__ * q[i__6].i;
		z__1.r = z__2.r + z__4.r, z__1.i = z__2.i + z__4.i;
		q[i__4].r = z__1.r, q[i__4].i = z__1.i;
		i__4 = j + i__ * q_dim1;
		q[i__4].r = t.r, q[i__4].i = t.i;
/* L70: */
	    }

/*           %---------------------------% */
/*           | Prepare for next rotation | */
/*           %---------------------------% */

	    if (i__ < iend - 1) {
		i__3 = i__ + 1 + i__ * h_dim1;
		f.r = h__[i__3].r, f.i = h__[i__3].i;
		i__3 = i__ + 2 + i__ * h_dim1;
		g.r = h__[i__3].r, g.i = h__[i__3].i;
	    }
/* L80: */
	}

/*        %-------------------------------% */
/*        | Finished applying the shift.  | */
/*        %-------------------------------% */

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

/* L110: */
    }

/*     %---------------------------------------------------% */
/*     | Perform a similarity transformation that makes    | */
/*     | sure that the compressed H will have non-negative | */
/*     | real subdiagonal elements.                        | */
/*     %---------------------------------------------------% */

    i__1 = *kev;
    for (j = 1; j <= i__1; ++j) {
	i__2 = j + 1 + j * h_dim1;
	if (h__[i__2].r < 0. || d_imag(&h__[j + 1 + j * h_dim1]) != 0.) {
	    i__2 = j + 1 + j * h_dim1;
	    i__3 = j + 1 + j * h_dim1;
	    d__2 = h__[i__3].r;
	    d__3 = d_imag(&h__[j + 1 + j * h_dim1]);
	    d__1 = dlapy2_(&d__2, &d__3);
	    z__1.r = h__[i__2].r / d__1, z__1.i = h__[i__2].i / d__1;
	    t.r = z__1.r, t.i = z__1.i;
	    i__2 = kplusp - j + 1;
	    d_cnjg(&z__1, &t);
	    zscal_(&i__2, &z__1, &h__[j + 1 + j * h_dim1], ldh);
/* Computing MIN */
	    i__3 = j + 2;
	    i__2 = min(i__3,kplusp);
	    zscal_(&i__2, &t, &h__[(j + 1) * h_dim1 + 1], &c__1);
/* Computing MIN */
	    i__3 = j + *np + 1;
	    i__2 = min(i__3,kplusp);
	    zscal_(&i__2, &t, &q[(j + 1) * q_dim1 + 1], &c__1);
	    i__2 = j + 1 + j * h_dim1;
	    i__3 = j + 1 + j * h_dim1;
	    d__1 = h__[i__3].r;
	    z__1.r = d__1, z__1.i = 0.;
	    h__[i__2].r = z__1.r, h__[i__2].i = z__1.i;
	}
/* L120: */
    }

    i__1 = *kev;
    for (i__ = 1; i__ <= i__1; ++i__) {

/*        %--------------------------------------------% */
/*        | Final check for splitting and deflation.   | */
/*        | Use a standard test as in the QR algorithm | */
/*        | REFERENCE: LAPACK subroutine zlahqr.       | */
/*        | Note: Since the subdiagonals of the        | */
/*        | compressed H are nonnegative real numbers, | */
/*        | we take advantage of this.                 | */
/*        %--------------------------------------------% */

	i__2 = i__ + i__ * h_dim1;
	i__3 = i__ + 1 + (i__ + 1) * h_dim1;
	tst1 = (d__1 = h__[i__2].r, abs(d__1)) + (d__2 = d_imag(&h__[i__ + 
		i__ * h_dim1]), abs(d__2)) + ((d__3 = h__[i__3].r, abs(d__3)) 
		+ (d__4 = d_imag(&h__[i__ + 1 + (i__ + 1) * h_dim1]), abs(
		d__4)));
	if (tst1 == 0.) {
	    tst1 = zlanhs_("1", kev, &h__[h_offset], ldh, &workl[1], (ftnlen)
		    1);
	}
	i__2 = i__ + 1 + i__ * h_dim1;
/* Computing MAX */
	d__1 = ulp * tst1;
	if (h__[i__2].r <= max(d__1,smlnum)) {
	    i__3 = i__ + 1 + i__ * h_dim1;
	    h__[i__3].r = 0., h__[i__3].i = 0.;
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

    i__1 = *kev + 1 + *kev * h_dim1;
    if (h__[i__1].r > 0.) {
	zgemv_("N", n, &kplusp, &c_b1, &v[v_offset], ldv, &q[(*kev + 1) * 
		q_dim1 + 1], &c__1, &c_b2, &workd[*n + 1], &c__1, (ftnlen)1);
    }

/*     %----------------------------------------------------------% */
/*     | Compute column 1 to kev of (V*Q) in backward order       | */
/*     | taking advantage of the upper Hessenberg structure of Q. | */
/*     %----------------------------------------------------------% */

    i__1 = *kev;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = kplusp - i__ + 1;
	zgemv_("N", n, &i__2, &c_b1, &v[v_offset], ldv, &q[(*kev - i__ + 1) * 
		q_dim1 + 1], &c__1, &c_b2, &workd[1], &c__1, (ftnlen)1);
	zcopy_(n, &workd[1], &c__1, &v[(kplusp - i__ + 1) * v_dim1 + 1], &
		c__1);
/* L140: */
    }

/*     %-------------------------------------------------% */
/*     |  Move v(:,kplusp-kev+1:kplusp) into v(:,1:kev). | */
/*     %-------------------------------------------------% */

    zlacpy_("A", n, kev, &v[(kplusp - *kev + 1) * v_dim1 + 1], ldv, &v[
	    v_offset], ldv, (ftnlen)1);

/*     %--------------------------------------------------------------% */
/*     | Copy the (kev+1)-st column of (V*Q) in the appropriate place | */
/*     %--------------------------------------------------------------% */

    i__1 = *kev + 1 + *kev * h_dim1;
    if (h__[i__1].r > 0.) {
	zcopy_(n, &workd[*n + 1], &c__1, &v[(*kev + 1) * v_dim1 + 1], &c__1);
    }

/*     %-------------------------------------% */
/*     | Update the residual vector:         | */
/*     |    r <- sigmak*r + betak*v(:,kev+1) | */
/*     | where                               | */
/*     |    sigmak = (e_{kev+p}'*Q)*e_{kev}  | */
/*     |    betak = e_{kev+1}'*H*e_{kev}     | */
/*     %-------------------------------------% */

    zscal_(n, &q[kplusp + *kev * q_dim1], &resid[1], &c__1);
    i__1 = *kev + 1 + *kev * h_dim1;
    if (h__[i__1].r > 0.) {
	zaxpy_(n, &h__[*kev + 1 + *kev * h_dim1], &v[(*kev + 1) * v_dim1 + 1],
		 &c__1, &resid[1], &c__1);
    }

    if (msglvl > 1) {
	zvout_(&debug_1.logfil, &c__1, &q[kplusp + *kev * q_dim1], &
		debug_1.ndigit, "_napps: sigmak = (e_{kev+p}^T*Q)*e_{kev}", (
		ftnlen)40);
	zvout_(&debug_1.logfil, &c__1, &h__[*kev + 1 + *kev * h_dim1], &
		debug_1.ndigit, "_napps: betak = e_{kev+1}^T*H*e_{kev}", (
		ftnlen)37);
	ivout_(&debug_1.logfil, &c__1, kev, &debug_1.ndigit, "_napps: Order "
		"of the final Hessenberg matrix ", (ftnlen)45);
	if (msglvl > 2) {
	    zmout_(&debug_1.logfil, kev, kev, &h__[h_offset], ldh, &
		    debug_1.ndigit, "_napps: updated Hessenberg matrix H for"
		    " next iteration", (ftnlen)54);
	}

    }

L9000:
    second_(&t1);
    timing_1.tcapps += t1 - t0;

    return 0;

/*     %---------------% */
/*     | End of znapps | */
/*     %---------------% */

} /* znapps_ */

