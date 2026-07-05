/* ../FORTRAN/ARPACK/SRC/znaitr.f -- translated by f2c (version 20100827).
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
static logical c_false = FALSE_;
static doublereal c_b27 = 1.;
static integer c__2 = 2;

/* \BeginDoc */

/* \Name: znaitr */

/* \Description: */
/*  Reverse communication interface for applying NP additional steps to */
/*  a K step nonsymmetric Arnoldi factorization. */

/*  Input:  OP*V_{k}  -  V_{k}*H = r_{k}*e_{k}^T */

/*          with (V_{k}^T)*B*V_{k} = I, (V_{k}^T)*B*r_{k} = 0. */

/*  Output: OP*V_{k+p}  -  V_{k+p}*H = r_{k+p}*e_{k+p}^T */

/*          with (V_{k+p}^T)*B*V_{k+p} = I, (V_{k+p}^T)*B*r_{k+p} = 0. */

/*  where OP and B are as in znaupd.  The B-norm of r_{k+p} is also */
/*  computed and returned. */

/* \Usage: */
/*  call znaitr */
/*     ( IDO, BMAT, N, K, NP, NB, RESID, RNORM, V, LDV, H, LDH, */
/*       IPNTR, WORKD, INFO ) */

/* \Arguments */
/*  IDO     Integer.  (INPUT/OUTPUT) */
/*          Reverse communication flag. */
/*          ------------------------------------------------------------- */
/*          IDO =  0: first call to the reverse communication interface */
/*          IDO = -1: compute  Y = OP * X  where */
/*                    IPNTR(1) is the pointer into WORK for X, */
/*                    IPNTR(2) is the pointer into WORK for Y. */
/*                    This is for the restart phase to force the new */
/*                    starting vector into the range of OP. */
/*          IDO =  1: compute  Y = OP * X  where */
/*                    IPNTR(1) is the pointer into WORK for X, */
/*                    IPNTR(2) is the pointer into WORK for Y, */
/*                    IPNTR(3) is the pointer into WORK for B * X. */
/*          IDO =  2: compute  Y = B * X  where */
/*                    IPNTR(1) is the pointer into WORK for X, */
/*                    IPNTR(2) is the pointer into WORK for Y. */
/*          IDO = 99: done */
/*          ------------------------------------------------------------- */
/*          When the routine is used in the "shift-and-invert" mode, the */
/*          vector B * Q is already available and do not need to be */
/*          recomputed in forming OP * Q. */

/*  BMAT    Character*1.  (INPUT) */
/*          BMAT specifies the type of the matrix B that defines the */
/*          semi-inner product for the operator OP.  See znaupd. */
/*          B = 'I' -> standard eigenvalue problem A*x = lambda*x */
/*          B = 'G' -> generalized eigenvalue problem A*x = lambda*M**x */

/*  N       Integer.  (INPUT) */
/*          Dimension of the eigenproblem. */

/*  K       Integer.  (INPUT) */
/*          Current size of V and H. */

/*  NP      Integer.  (INPUT) */
/*          Number of additional Arnoldi steps to take. */

/*  NB      Integer.  (INPUT) */
/*          Blocksize to be used in the recurrence. */
/*          Only work for NB = 1 right now.  The goal is to have a */
/*          program that implement both the block and non-block method. */

/*  RESID   Complex*16 array of length N.  (INPUT/OUTPUT) */
/*          On INPUT:  RESID contains the residual vector r_{k}. */
/*          On OUTPUT: RESID contains the residual vector r_{k+p}. */

/*  RNORM   Double precision scalar.  (INPUT/OUTPUT) */
/*          B-norm of the starting residual on input. */
/*          B-norm of the updated residual r_{k+p} on output. */

/*  V       Complex*16 N by K+NP array.  (INPUT/OUTPUT) */
/*          On INPUT:  V contains the Arnoldi vectors in the first K */
/*          columns. */
/*          On OUTPUT: V contains the new NP Arnoldi vectors in the next */
/*          NP columns.  The first K columns are unchanged. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling */
/*          program. */

/*  H       Complex*16 (K+NP) by (K+NP) array.  (INPUT/OUTPUT) */
/*          H is used to store the generated upper Hessenberg matrix. */

/*  LDH     Integer.  (INPUT) */
/*          Leading dimension of H exactly as declared in the calling */
/*          program. */

/*  IPNTR   Integer array of length 3.  (OUTPUT) */
/*          Pointer to mark the starting locations in the WORK for */
/*          vectors used by the Arnoldi iteration. */
/*          ------------------------------------------------------------- */
/*          IPNTR(1): pointer to the current operand vector X. */
/*          IPNTR(2): pointer to the current result vector Y. */
/*          IPNTR(3): pointer to the vector B * X when used in the */
/*                    shift-and-invert mode.  X is the current operand. */
/*          ------------------------------------------------------------- */

/*  WORKD   Complex*16 work array of length 3*N.  (REVERSE COMMUNICATION) */
/*          Distributed array to be used in the basic Arnoldi iteration */
/*          for reverse communication.  The calling program should not */
/*          use WORKD as temporary workspace during the iteration !!!!!! */
/*          On input, WORKD(1:N) = B*RESID and is used to save some */
/*          computation at the first step. */

/*  INFO    Integer.  (OUTPUT) */
/*          = 0: Normal exit. */
/*          > 0: Size of the spanning invariant subspace of OP found. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  Complex*16 */

/* \References: */
/*  1. D.C. Sorensen, "Implicit Application of Polynomial Filters in */
/*     a k-Step Arnoldi Method", SIAM J. Matr. Anal. Apps., 13 (1992), */
/*     pp 357-385. */
/*  2. R.B. Lehoucq, "Analysis and Implementation of an Implicitly */
/*     Restarted Arnoldi Iteration", Rice University Technical Report */
/*     TR95-13, Department of Computational and Applied Mathematics. */

/* \Routines called: */
/*     zgetv0  ARPACK routine to generate the initial vector. */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     zmout   ARPACK utility routine that prints matrices */
/*     zvout   ARPACK utility routine that prints vectors. */
/*     zlanhs  LAPACK routine that computes various norms of a matrix. */
/*     zlascl  LAPACK routine for careful scaling of a matrix. */
/*     dlabad  LAPACK routine for defining the underflow and overflow */
/*             limits. */
/*     dlamch  LAPACK routine that determines machine constants. */
/*     dlapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully. */
/*     zgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     zaxpy   Level 1 BLAS that computes a vector triad. */
/*     zcopy   Level 1 BLAS that copies one vector to another . */
/*     zdotc   Level 1 BLAS that computes the scalar product of two vectors. */
/*     zscal   Level 1 BLAS that scales a vector. */
/*     zdscal  Level 1 BLAS that scales a complex vector by a real number. */
/*     dznrm2  Level 1 BLAS that computes the norm of a vector. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \SCCS Information: @(#) */
/* FILE: naitr.F   SID: 2.3   DATE OF SID: 8/27/96   RELEASE: 2 */

/* \Remarks */
/*  The algorithm implemented is: */

/*  restart = .false. */
/*  Given V_{k} = [v_{1}, ..., v_{k}], r_{k}; */
/*  r_{k} contains the initial residual vector even for k = 0; */
/*  Also assume that rnorm = || B*r_{k} || and B*r_{k} are already */
/*  computed by the calling program. */

/*  betaj = rnorm ; p_{k+1} = B*r_{k} ; */
/*  For  j = k+1, ..., k+np  Do */
/*     1) if ( betaj < tol ) stop or restart depending on j. */
/*        ( At present tol is zero ) */
/*        if ( restart ) generate a new starting vector. */
/*     2) v_{j} = r(j-1)/betaj;  V_{j} = [V_{j-1}, v_{j}]; */
/*        p_{j} = p_{j}/betaj */
/*     3) r_{j} = OP*v_{j} where OP is defined as in znaupd */
/*        For shift-invert mode p_{j} = B*v_{j} is already available. */
/*        wnorm = || OP*v_{j} || */
/*     4) Compute the j-th step residual vector. */
/*        w_{j} =  V_{j}^T * B * OP * v_{j} */
/*        r_{j} =  OP*v_{j} - V_{j} * w_{j} */
/*        H(:,j) = w_{j}; */
/*        H(j,j-1) = rnorm */
/*        rnorm = || r_(j) || */
/*        If (rnorm > 0.717*wnorm) accept step and go back to 1) */
/*     5) Re-orthogonalization step: */
/*        s = V_{j}'*B*r_{j} */
/*        r_{j} = r_{j} - V_{j}*s;  rnorm1 = || r_{j} || */
/*        alphaj = alphaj + s_{j}; */
/*     6) Iterative refinement step: */
/*        If (rnorm1 > 0.717*rnorm) then */
/*           rnorm = rnorm1 */
/*           accept step and go back to 1) */
/*        Else */
/*           rnorm = rnorm1 */
/*           If this is the first time in step 6), go to 5) */
/*           Else r_{j} lies in the span of V_{j} numerically. */
/*              Set r_{j} = 0 and rnorm = 0; go to 1) */
/*        EndIf */
/*  End Do */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int znaitr_(integer *ido, char *bmat, integer *n, integer *k,
	 integer *np, integer *nb, doublecomplex *resid, doublereal *rnorm, 
	doublecomplex *v, integer *ldv, doublecomplex *h__, integer *ldh, 
	integer *ipntr, doublecomplex *workd, integer *info, ftnlen bmat_len)
{
    /* Initialized data */

    static logical first = TRUE_;

    /* System generated locals */
    integer h_dim1, h_offset, v_dim1, v_offset, i__1, i__2, i__3;
    doublereal d__1, d__2, d__3, d__4;
    doublecomplex z__1;

    /* Builtin functions */
    double d_imag(doublecomplex *), sqrt(doublereal);

    /* Local variables */
    static integer i__, j;
    static real t0, t1, t2, t3, t4, t5;
    static integer jj, ipj, irj, ivj;
    static doublereal ulp, tst1;
    static integer ierr, iter;
    static doublereal unfl, ovfl;
    static integer itry;
    static doublereal temp1;
    static logical orth1, orth2, step3, step4;
    static doublereal betaj;
    static integer infol;
    static doublecomplex cnorm;
    extern /* Double Complex */ VOID zdotc_(doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    static doublereal rtemp[2];
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *, ftnlen);
    static doublereal wnorm;
    extern /* Subroutine */ int dvout_(integer *, integer *, doublereal *, 
	    integer *, char *, ftnlen), zcopy_(integer *, doublecomplex *, 
	    integer *, doublecomplex *, integer *), ivout_(integer *, integer 
	    *, integer *, integer *, char *, ftnlen), zaxpy_(integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *), zmout_(integer *, integer *, integer *, doublecomplex 
	    *, integer *, integer *, char *, ftnlen), zvout_(integer *, 
	    integer *, doublecomplex *, integer *, char *, ftnlen);
    extern doublereal dlapy2_(doublereal *, doublereal *);
    extern /* Subroutine */ int dlabad_(doublereal *, doublereal *);
    extern doublereal dznrm2_(integer *, doublecomplex *, integer *);
    static doublereal rnorm1;
    extern /* Subroutine */ int zgetv0_(integer *, char *, integer *, logical 
	    *, integer *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, doublereal *, integer *, doublecomplex *, 
	    integer *, ftnlen);
    extern doublereal dlamch_(char *, ftnlen);
    extern /* Subroutine */ int second_(real *), zdscal_(integer *, 
	    doublereal *, doublecomplex *, integer *);
    static logical rstart;
    static integer msglvl;
    static doublereal smlnum;
    extern doublereal zlanhs_(char *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, ftnlen);
    extern /* Subroutine */ int zlascl_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, integer *, doublecomplex *,
	     integer *, integer *, ftnlen);


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


/*     %--------------% */
/*     | Local Arrays | */
/*     %--------------% */


/*     %---------------% */
/*     | Local Scalars | */
/*     %---------------% */



/*     %----------------------% */
/*     | External Subroutines | */
/*     %----------------------% */


/*     %--------------------% */
/*     | External Functions | */
/*     %--------------------% */


/*     %---------------------% */
/*     | Intrinsic Functions | */
/*     %---------------------% */


/*     %-----------------% */
/*     | Data statements | */
/*     %-----------------% */

    /* Parameter adjustments */
    --workd;
    --resid;
    v_dim1 = *ldv;
    v_offset = 1 + v_dim1;
    v -= v_offset;
    h_dim1 = *ldh;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    --ipntr;

    /* Function Body */

/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    if (first) {

/*        %-----------------------------------------% */
/*        | Set machine-dependent constants for the | */
/*        | the splitting and deflation criterion.  | */
/*        | If norm(H) <= sqrt(OVFL),               | */
/*        | overflow should not occur.              | */
/*        | REFERENCE: LAPACK subroutine zlahqr     | */
/*        %-----------------------------------------% */

	unfl = dlamch_("safe minimum", (ftnlen)12);
	z__1.r = 1. / unfl, z__1.i = 0. / unfl;
	ovfl = z__1.r;
	dlabad_(&unfl, &ovfl);
	ulp = dlamch_("precision", (ftnlen)9);
	smlnum = unfl * (*n / ulp);
	first = FALSE_;
    }

    if (*ido == 0) {

/*        %-------------------------------% */
/*        | Initialize timing statistics  | */
/*        | & message level for debugging | */
/*        %-------------------------------% */

	second_(&t0);
	msglvl = debug_1.mcaitr;

/*        %------------------------------% */
/*        | Initial call to this routine | */
/*        %------------------------------% */

	*info = 0;
	step3 = FALSE_;
	step4 = FALSE_;
	rstart = FALSE_;
	orth1 = FALSE_;
	orth2 = FALSE_;
	j = *k + 1;
	ipj = 1;
	irj = ipj + *n;
	ivj = irj + *n;
    }

/*     %-------------------------------------------------% */
/*     | When in reverse communication mode one of:      | */
/*     | STEP3, STEP4, ORTH1, ORTH2, RSTART              | */
/*     | will be .true. when ....                        | */
/*     | STEP3: return from computing OP*v_{j}.          | */
/*     | STEP4: return from computing B-norm of OP*v_{j} | */
/*     | ORTH1: return from computing B-norm of r_{j+1}  | */
/*     | ORTH2: return from computing B-norm of          | */
/*     |        correction to the residual vector.       | */
/*     | RSTART: return from OP computations needed by   | */
/*     |         zgetv0.                                 | */
/*     %-------------------------------------------------% */

    if (step3) {
	goto L50;
    }
    if (step4) {
	goto L60;
    }
    if (orth1) {
	goto L70;
    }
    if (orth2) {
	goto L90;
    }
    if (rstart) {
	goto L30;
    }

/*     %-----------------------------% */
/*     | Else this is the first step | */
/*     %-----------------------------% */

/*     %--------------------------------------------------------------% */
/*     |                                                              | */
/*     |        A R N O L D I     I T E R A T I O N     L O O P       | */
/*     |                                                              | */
/*     | Note:  B*r_{j-1} is already in WORKD(1:N)=WORKD(IPJ:IPJ+N-1) | */
/*     %--------------------------------------------------------------% */
L1000:

    if (msglvl > 1) {
	ivout_(&debug_1.logfil, &c__1, &j, &debug_1.ndigit, "_naitr: generat"
		"ing Arnoldi vector number", (ftnlen)40);
	dvout_(&debug_1.logfil, &c__1, rnorm, &debug_1.ndigit, "_naitr: B-no"
		"rm of the current residual is", (ftnlen)41);
    }

/*        %---------------------------------------------------% */
/*        | STEP 1: Check if the B norm of j-th residual      | */
/*        | vector is zero. Equivalent to determine whether   | */
/*        | an exact j-step Arnoldi factorization is present. | */
/*        %---------------------------------------------------% */

    betaj = *rnorm;
    if (*rnorm > 0.) {
	goto L40;
    }

/*           %---------------------------------------------------% */
/*           | Invariant subspace found, generate a new starting | */
/*           | vector which is orthogonal to the current Arnoldi | */
/*           | basis and continue the iteration.                 | */
/*           %---------------------------------------------------% */

    if (msglvl > 0) {
	ivout_(&debug_1.logfil, &c__1, &j, &debug_1.ndigit, "_naitr: ****** "
		"RESTART AT STEP ******", (ftnlen)37);
    }

/*           %---------------------------------------------% */
/*           | ITRY is the loop variable that controls the | */
/*           | maximum amount of times that a restart is   | */
/*           | attempted. NRSTRT is used by stat.h         | */
/*           %---------------------------------------------% */

    betaj = 0.;
    ++timing_1.nrstrt;
    itry = 1;
L20:
    rstart = TRUE_;
    *ido = 0;
L30:

/*           %--------------------------------------% */
/*           | If in reverse communication mode and | */
/*           | RSTART = .true. flow returns here.   | */
/*           %--------------------------------------% */

    zgetv0_(ido, bmat, &itry, &c_false, n, &j, &v[v_offset], ldv, &resid[1], 
	    rnorm, &ipntr[1], &workd[1], &ierr, (ftnlen)1);
    if (*ido != 99) {
	goto L9000;
    }
    if (ierr < 0) {
	++itry;
	if (itry <= 3) {
	    goto L20;
	}

/*              %------------------------------------------------% */
/*              | Give up after several restart attempts.        | */
/*              | Set INFO to the size of the invariant subspace | */
/*              | which spans OP and exit.                       | */
/*              %------------------------------------------------% */

	*info = j - 1;
	second_(&t1);
	timing_1.tcaitr += t1 - t0;
	*ido = 99;
	goto L9000;
    }

L40:

/*        %---------------------------------------------------------% */
/*        | STEP 2:  v_{j} = r_{j-1}/rnorm and p_{j} = p_{j}/rnorm  | */
/*        | Note that p_{j} = B*r_{j-1}. In order to avoid overflow | */
/*        | when reciprocating a small RNORM, test against lower    | */
/*        | machine bound.                                          | */
/*        %---------------------------------------------------------% */

    zcopy_(n, &resid[1], &c__1, &v[j * v_dim1 + 1], &c__1);
    if (*rnorm >= unfl) {
	temp1 = 1. / *rnorm;
	zdscal_(n, &temp1, &v[j * v_dim1 + 1], &c__1);
	zdscal_(n, &temp1, &workd[ipj], &c__1);
    } else {

/*            %-----------------------------------------% */
/*            | To scale both v_{j} and p_{j} carefully | */
/*            | use LAPACK routine zlascl               | */
/*            %-----------------------------------------% */

	zlascl_("General", &i__, &i__, rnorm, &c_b27, n, &c__1, &v[j * v_dim1 
		+ 1], n, &infol, (ftnlen)7);
	zlascl_("General", &i__, &i__, rnorm, &c_b27, n, &c__1, &workd[ipj], 
		n, &infol, (ftnlen)7);
    }

/*        %------------------------------------------------------% */
/*        | STEP 3:  r_{j} = OP*v_{j}; Note that p_{j} = B*v_{j} | */
/*        | Note that this is not quite yet r_{j}. See STEP 4    | */
/*        %------------------------------------------------------% */

    step3 = TRUE_;
    ++timing_1.nopx;
    second_(&t2);
    zcopy_(n, &v[j * v_dim1 + 1], &c__1, &workd[ivj], &c__1);
    ipntr[1] = ivj;
    ipntr[2] = irj;
    ipntr[3] = ipj;
    *ido = 1;

/*        %-----------------------------------% */
/*        | Exit in order to compute OP*v_{j} | */
/*        %-----------------------------------% */

    goto L9000;
L50:

/*        %----------------------------------% */
/*        | Back from reverse communication; | */
/*        | WORKD(IRJ:IRJ+N-1) := OP*v_{j}   | */
/*        | if step3 = .true.                | */
/*        %----------------------------------% */

    second_(&t3);
    timing_1.tmvopx += t3 - t2;
    step3 = FALSE_;

/*        %------------------------------------------% */
/*        | Put another copy of OP*v_{j} into RESID. | */
/*        %------------------------------------------% */

    zcopy_(n, &workd[irj], &c__1, &resid[1], &c__1);

/*        %---------------------------------------% */
/*        | STEP 4:  Finish extending the Arnoldi | */
/*        |          factorization to length j.   | */
/*        %---------------------------------------% */

    second_(&t2);
    if (*(unsigned char *)bmat == 'G') {
	++timing_1.nbx;
	step4 = TRUE_;
	ipntr[1] = irj;
	ipntr[2] = ipj;
	*ido = 2;

/*           %-------------------------------------% */
/*           | Exit in order to compute B*OP*v_{j} | */
/*           %-------------------------------------% */

	goto L9000;
    } else if (*(unsigned char *)bmat == 'I') {
	zcopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
    }
L60:

/*        %----------------------------------% */
/*        | Back from reverse communication; | */
/*        | WORKD(IPJ:IPJ+N-1) := B*OP*v_{j} | */
/*        | if step4 = .true.                | */
/*        %----------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	second_(&t3);
	timing_1.tmvbx += t3 - t2;
    }

    step4 = FALSE_;

/*        %-------------------------------------% */
/*        | The following is needed for STEP 5. | */
/*        | Compute the B-norm of OP*v_{j}.     | */
/*        %-------------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	zdotc_(&z__1, n, &resid[1], &c__1, &workd[ipj], &c__1);
	cnorm.r = z__1.r, cnorm.i = z__1.i;
	d__1 = cnorm.r;
	d__2 = d_imag(&cnorm);
	wnorm = sqrt(dlapy2_(&d__1, &d__2));
    } else if (*(unsigned char *)bmat == 'I') {
	wnorm = dznrm2_(n, &resid[1], &c__1);
    }

/*        %-----------------------------------------% */
/*        | Compute the j-th residual corresponding | */
/*        | to the j step factorization.            | */
/*        | Use Classical Gram Schmidt and compute: | */
/*        | w_{j} <-  V_{j}^T * B * OP * v_{j}      | */
/*        | r_{j} <-  OP*v_{j} - V_{j} * w_{j}      | */
/*        %-----------------------------------------% */


/*        %------------------------------------------% */
/*        | Compute the j Fourier coefficients w_{j} | */
/*        | WORKD(IPJ:IPJ+N-1) contains B*OP*v_{j}.  | */
/*        %------------------------------------------% */

    zgemv_("C", n, &j, &c_b1, &v[v_offset], ldv, &workd[ipj], &c__1, &c_b2, &
	    h__[j * h_dim1 + 1], &c__1, (ftnlen)1);

/*        %--------------------------------------% */
/*        | Orthogonalize r_{j} against V_{j}.   | */
/*        | RESID contains OP*v_{j}. See STEP 3. | */
/*        %--------------------------------------% */

    z__1.r = -1., z__1.i = -0.;
    zgemv_("N", n, &j, &z__1, &v[v_offset], ldv, &h__[j * h_dim1 + 1], &c__1, 
	    &c_b1, &resid[1], &c__1, (ftnlen)1);

    if (j > 1) {
	i__1 = j + (j - 1) * h_dim1;
	z__1.r = betaj, z__1.i = 0.;
	h__[i__1].r = z__1.r, h__[i__1].i = z__1.i;
    }

    second_(&t4);

    orth1 = TRUE_;

    second_(&t2);
    if (*(unsigned char *)bmat == 'G') {
	++timing_1.nbx;
	zcopy_(n, &resid[1], &c__1, &workd[irj], &c__1);
	ipntr[1] = irj;
	ipntr[2] = ipj;
	*ido = 2;

/*           %----------------------------------% */
/*           | Exit in order to compute B*r_{j} | */
/*           %----------------------------------% */

	goto L9000;
    } else if (*(unsigned char *)bmat == 'I') {
	zcopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
    }
L70:

/*        %---------------------------------------------------% */
/*        | Back from reverse communication if ORTH1 = .true. | */
/*        | WORKD(IPJ:IPJ+N-1) := B*r_{j}.                    | */
/*        %---------------------------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	second_(&t3);
	timing_1.tmvbx += t3 - t2;
    }

    orth1 = FALSE_;

/*        %------------------------------% */
/*        | Compute the B-norm of r_{j}. | */
/*        %------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	zdotc_(&z__1, n, &resid[1], &c__1, &workd[ipj], &c__1);
	cnorm.r = z__1.r, cnorm.i = z__1.i;
	d__1 = cnorm.r;
	d__2 = d_imag(&cnorm);
	*rnorm = sqrt(dlapy2_(&d__1, &d__2));
    } else if (*(unsigned char *)bmat == 'I') {
	*rnorm = dznrm2_(n, &resid[1], &c__1);
    }

/*        %-----------------------------------------------------------% */
/*        | STEP 5: Re-orthogonalization / Iterative refinement phase | */
/*        | Maximum NITER_ITREF tries.                                | */
/*        |                                                           | */
/*        |          s      = V_{j}^T * B * r_{j}                     | */
/*        |          r_{j}  = r_{j} - V_{j}*s                         | */
/*        |          alphaj = alphaj + s_{j}                          | */
/*        |                                                           | */
/*        | The stopping criteria used for iterative refinement is    | */
/*        | discussed in Parlett's book SEP, page 107 and in Gragg &  | */
/*        | Reichel ACM TOMS paper; Algorithm 686, Dec. 1990.         | */
/*        | Determine if we need to correct the residual. The goal is | */
/*        | to enforce ||v(:,1:j)^T * r_{j}|| .le. eps * || r_{j} ||  | */
/*        | The following test determines whether the sine of the     | */
/*        | angle between  OP*x and the computed residual is less     | */
/*        | than or equal to 0.717.                                   | */
/*        %-----------------------------------------------------------% */

    if (*rnorm > wnorm * .717f) {
	goto L100;
    }

    iter = 0;
    ++timing_1.nrorth;

/*        %---------------------------------------------------% */
/*        | Enter the Iterative refinement phase. If further  | */
/*        | refinement is necessary, loop back here. The loop | */
/*        | variable is ITER. Perform a step of Classical     | */
/*        | Gram-Schmidt using all the Arnoldi vectors V_{j}  | */
/*        %---------------------------------------------------% */

L80:

    if (msglvl > 2) {
	rtemp[0] = wnorm;
	rtemp[1] = *rnorm;
	dvout_(&debug_1.logfil, &c__2, rtemp, &debug_1.ndigit, "_naitr: re-o"
		"rthogonalization; wnorm and rnorm are", (ftnlen)49);
	zvout_(&debug_1.logfil, &j, &h__[j * h_dim1 + 1], &debug_1.ndigit, 
		"_naitr: j-th column of H", (ftnlen)24);
    }

/*        %----------------------------------------------------% */
/*        | Compute V_{j}^T * B * r_{j}.                       | */
/*        | WORKD(IRJ:IRJ+J-1) = v(:,1:J)'*WORKD(IPJ:IPJ+N-1). | */
/*        %----------------------------------------------------% */

    zgemv_("C", n, &j, &c_b1, &v[v_offset], ldv, &workd[ipj], &c__1, &c_b2, &
	    workd[irj], &c__1, (ftnlen)1);

/*        %---------------------------------------------% */
/*        | Compute the correction to the residual:     | */
/*        | r_{j} = r_{j} - V_{j} * WORKD(IRJ:IRJ+J-1). | */
/*        | The correction to H is v(:,1:J)*H(1:J,1:J)  | */
/*        | + v(:,1:J)*WORKD(IRJ:IRJ+J-1)*e'_j.         | */
/*        %---------------------------------------------% */

    z__1.r = -1., z__1.i = -0.;
    zgemv_("N", n, &j, &z__1, &v[v_offset], ldv, &workd[irj], &c__1, &c_b1, &
	    resid[1], &c__1, (ftnlen)1);
    zaxpy_(&j, &c_b1, &workd[irj], &c__1, &h__[j * h_dim1 + 1], &c__1);

    orth2 = TRUE_;
    second_(&t2);
    if (*(unsigned char *)bmat == 'G') {
	++timing_1.nbx;
	zcopy_(n, &resid[1], &c__1, &workd[irj], &c__1);
	ipntr[1] = irj;
	ipntr[2] = ipj;
	*ido = 2;

/*           %-----------------------------------% */
/*           | Exit in order to compute B*r_{j}. | */
/*           | r_{j} is the corrected residual.  | */
/*           %-----------------------------------% */

	goto L9000;
    } else if (*(unsigned char *)bmat == 'I') {
	zcopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
    }
L90:

/*        %---------------------------------------------------% */
/*        | Back from reverse communication if ORTH2 = .true. | */
/*        %---------------------------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	second_(&t3);
	timing_1.tmvbx += t3 - t2;
    }

/*        %-----------------------------------------------------% */
/*        | Compute the B-norm of the corrected residual r_{j}. | */
/*        %-----------------------------------------------------% */

    if (*(unsigned char *)bmat == 'G') {
	zdotc_(&z__1, n, &resid[1], &c__1, &workd[ipj], &c__1);
	cnorm.r = z__1.r, cnorm.i = z__1.i;
	d__1 = cnorm.r;
	d__2 = d_imag(&cnorm);
	rnorm1 = sqrt(dlapy2_(&d__1, &d__2));
    } else if (*(unsigned char *)bmat == 'I') {
	rnorm1 = dznrm2_(n, &resid[1], &c__1);
    }

    if (msglvl > 0 && iter > 0) {
	ivout_(&debug_1.logfil, &c__1, &j, &debug_1.ndigit, "_naitr: Iterati"
		"ve refinement for Arnoldi residual", (ftnlen)49);
	if (msglvl > 2) {
	    rtemp[0] = *rnorm;
	    rtemp[1] = rnorm1;
	    dvout_(&debug_1.logfil, &c__2, rtemp, &debug_1.ndigit, "_naitr: "
		    "iterative refinement ; rnorm and rnorm1 are", (ftnlen)51);
	}
    }

/*        %-----------------------------------------% */
/*        | Determine if we need to perform another | */
/*        | step of re-orthogonalization.           | */
/*        %-----------------------------------------% */

    if (rnorm1 > *rnorm * .717f) {

/*           %---------------------------------------% */
/*           | No need for further refinement.       | */
/*           | The cosine of the angle between the   | */
/*           | corrected residual vector and the old | */
/*           | residual vector is greater than 0.717 | */
/*           | In other words the corrected residual | */
/*           | and the old residual vector share an  | */
/*           | angle of less than arcCOS(0.717)      | */
/*           %---------------------------------------% */

	*rnorm = rnorm1;

    } else {

/*           %-------------------------------------------% */
/*           | Another step of iterative refinement step | */
/*           | is required. NITREF is used by stat.h     | */
/*           %-------------------------------------------% */

	++timing_1.nitref;
	*rnorm = rnorm1;
	++iter;
	if (iter <= 1) {
	    goto L80;
	}

/*           %-------------------------------------------------% */
/*           | Otherwise RESID is numerically in the span of V | */
/*           %-------------------------------------------------% */

	i__1 = *n;
	for (jj = 1; jj <= i__1; ++jj) {
	    i__2 = jj;
	    resid[i__2].r = 0., resid[i__2].i = 0.;
/* L95: */
	}
	*rnorm = 0.;
    }

/*        %----------------------------------------------% */
/*        | Branch here directly if iterative refinement | */
/*        | wasn't necessary or after at most NITER_REF  | */
/*        | steps of iterative refinement.               | */
/*        %----------------------------------------------% */

L100:

    rstart = FALSE_;
    orth2 = FALSE_;

    second_(&t5);
    timing_1.titref += t5 - t4;

/*        %------------------------------------% */
/*        | STEP 6: Update  j = j+1;  Continue | */
/*        %------------------------------------% */

    ++j;
    if (j > *k + *np) {
	second_(&t1);
	timing_1.tcaitr += t1 - t0;
	*ido = 99;
	i__1 = *k + *np - 1;
	for (i__ = max(1,*k); i__ <= i__1; ++i__) {

/*              %--------------------------------------------% */
/*              | Check for splitting and deflation.         | */
/*              | Use a standard test as in the QR algorithm | */
/*              | REFERENCE: LAPACK subroutine zlahqr        | */
/*              %--------------------------------------------% */

	    i__2 = i__ + i__ * h_dim1;
	    d__1 = h__[i__2].r;
	    d__2 = d_imag(&h__[i__ + i__ * h_dim1]);
	    i__3 = i__ + 1 + (i__ + 1) * h_dim1;
	    d__3 = h__[i__3].r;
	    d__4 = d_imag(&h__[i__ + 1 + (i__ + 1) * h_dim1]);
	    tst1 = dlapy2_(&d__1, &d__2) + dlapy2_(&d__3, &d__4);
	    if (tst1 == 0.) {
		i__2 = *k + *np;
		tst1 = zlanhs_("1", &i__2, &h__[h_offset], ldh, &workd[*n + 1]
			, (ftnlen)1);
	    }
	    i__2 = i__ + 1 + i__ * h_dim1;
	    d__1 = h__[i__2].r;
	    d__2 = d_imag(&h__[i__ + 1 + i__ * h_dim1]);
/* Computing MAX */
	    d__3 = ulp * tst1;
	    if (dlapy2_(&d__1, &d__2) <= max(d__3,smlnum)) {
		i__3 = i__ + 1 + i__ * h_dim1;
		h__[i__3].r = 0., h__[i__3].i = 0.;
	    }
/* L110: */
	}

	if (msglvl > 2) {
	    i__1 = *k + *np;
	    i__2 = *k + *np;
	    zmout_(&debug_1.logfil, &i__1, &i__2, &h__[h_offset], ldh, &
		    debug_1.ndigit, "_naitr: Final upper Hessenberg matrix H"
		    " of order K+NP", (ftnlen)53);
	}

	goto L9000;
    }

/*        %--------------------------------------------------------% */
/*        | Loop back to extend the factorization by another step. | */
/*        %--------------------------------------------------------% */

    goto L1000;

/*     %---------------------------------------------------------------% */
/*     |                                                               | */
/*     |  E N D     O F     M A I N     I T E R A T I O N     L O O P  | */
/*     |                                                               | */
/*     %---------------------------------------------------------------% */

L9000:
    return 0;

/*     %---------------% */
/*     | End of znaitr | */
/*     %---------------% */

} /* znaitr_ */

