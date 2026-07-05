/* ../FORTRAN/ARPACK/SRC/snaitr.f -- translated by f2c (version 20100827).
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

static integer c__1 = 1;
static logical c_false = FALSE_;
static real c_b25 = 1.f;
static real c_b47 = 0.f;
static real c_b50 = -1.f;
static integer c__2 = 2;

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: snaitr */

/* \Description: */
/*  Reverse communication interface for applying NP additional steps to */
/*  a K step nonsymmetric Arnoldi factorization. */

/*  Input:  OP*V_{k}  -  V_{k}*H = r_{k}*e_{k}^T */

/*          with (V_{k}^T)*B*V_{k} = I, (V_{k}^T)*B*r_{k} = 0. */

/*  Output: OP*V_{k+p}  -  V_{k+p}*H = r_{k+p}*e_{k+p}^T */

/*          with (V_{k+p}^T)*B*V_{k+p} = I, (V_{k+p}^T)*B*r_{k+p} = 0. */

/*  where OP and B are as in snaupd.  The B-norm of r_{k+p} is also */
/*  computed and returned. */

/* \Usage: */
/*  call snaitr */
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
/*          recompute in forming OP * Q. */

/*  BMAT    Character*1.  (INPUT) */
/*          BMAT specifies the type of the matrix B that defines the */
/*          semi-inner product for the operator OP.  See snaupd. */
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

/*  RESID   Real array of length N.  (INPUT/OUTPUT) */
/*          On INPUT:  RESID contains the residual vector r_{k}. */
/*          On OUTPUT: RESID contains the residual vector r_{k+p}. */

/*  RNORM   Real scalar.  (INPUT/OUTPUT) */
/*          B-norm of the starting residual on input. */
/*          B-norm of the updated residual r_{k+p} on output. */

/*  V       Real N by K+NP array.  (INPUT/OUTPUT) */
/*          On INPUT:  V contains the Arnoldi vectors in the first K */
/*          columns. */
/*          On OUTPUT: V contains the new NP Arnoldi vectors in the next */
/*          NP columns.  The first K columns are unchanged. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling */
/*          program. */

/*  H       Real (K+NP) by (K+NP) array.  (INPUT/OUTPUT) */
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

/*  WORKD   Real work array of length 3*N.  (REVERSE COMMUNICATION) */
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
/*     xxxxxx  real */

/* \References: */
/*  1. D.C. Sorensen, "Implicit Application of Polynomial Filters in */
/*     a k-Step Arnoldi Method", SIAM J. Matr. Anal. Apps., 13 (1992), */
/*     pp 357-385. */
/*  2. R.B. Lehoucq, "Analysis and Implementation of an Implicitly */
/*     Restarted Arnoldi Iteration", Rice University Technical Report */
/*     TR95-13, Department of Computational and Applied Mathematics. */

/* \Routines called: */
/*     sgetv0  ARPACK routine to generate the initial vector. */
/*     ivout   ARPACK utility routine that prints integers. */
/*     second  ARPACK utility routine for timing. */
/*     smout   ARPACK utility routine that prints matrices */
/*     svout   ARPACK utility routine that prints vectors. */
/*     slabad  LAPACK routine that computes machine constants. */
/*     slamch  LAPACK routine that determines machine constants. */
/*     slascl  LAPACK routine for careful scaling of a matrix. */
/*     slanhs  LAPACK routine that computes various norms of a matrix. */
/*     sgemv   Level 2 BLAS routine for matrix vector multiplication. */
/*     saxpy   Level 1 BLAS that computes a vector triad. */
/*     sscal   Level 1 BLAS that scales a vector. */
/*     scopy   Level 1 BLAS that copies one vector to another . */
/*     sdot    Level 1 BLAS that computes the scalar product of two vectors. */
/*     snrm2   Level 1 BLAS that computes the norm of a vector. */

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
/* FILE: naitr.F   SID: 2.4   DATE OF SID: 8/27/96   RELEASE: 2 */

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
/*     3) r_{j} = OP*v_{j} where OP is defined as in snaupd */
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

/* Subroutine */ int snaitr_(integer *ido, char *bmat, integer *n, integer *k,
	 integer *np, integer *nb, real *resid, real *rnorm, real *v, integer 
	*ldv, real *h__, integer *ldh, integer *ipntr, real *workd, integer *
	info, ftnlen bmat_len)
{
    /* Initialized data */

    static logical first = TRUE_;

    /* System generated locals */
    integer h_dim1, h_offset, v_dim1, v_offset, i__1, i__2;
    real r__1, r__2;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static integer i__, j;
    static real t0, t1, t2, t3, t4, t5;
    static integer jj, ipj, irj, ivj;
    static real ulp, tst1;
    static integer ierr, iter;
    static real unfl, ovfl;
    extern doublereal sdot_(integer *, real *, integer *, real *, integer *);
    static integer itry;
    static real temp1;
    static logical orth1, orth2, step3, step4;
    extern doublereal snrm2_(integer *, real *, integer *);
    static real betaj;
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *);
    static integer infol;
    extern /* Subroutine */ int sgemv_(char *, integer *, integer *, real *, 
	    real *, integer *, real *, integer *, real *, real *, integer *, 
	    ftnlen);
    static real xtemp[2];
    extern /* Subroutine */ int scopy_(integer *, real *, integer *, real *, 
	    integer *);
    static real wnorm;
    extern /* Subroutine */ int saxpy_(integer *, real *, real *, integer *, 
	    real *, integer *), ivout_(integer *, integer *, integer *, 
	    integer *, char *, ftnlen), smout_(integer *, integer *, integer *
	    , real *, integer *, integer *, char *, ftnlen), svout_(integer *,
	     integer *, real *, integer *, char *, ftnlen), sgetv0_(integer *,
	     char *, integer *, logical *, integer *, integer *, real *, 
	    integer *, real *, real *, integer *, real *, integer *, ftnlen);
    static real rnorm1;
    extern /* Subroutine */ int slabad_(real *, real *);
    extern doublereal slamch_(char *, ftnlen);
    extern /* Subroutine */ int second_(real *), slascl_(char *, integer *, 
	    integer *, real *, real *, integer *, integer *, real *, integer *
	    , integer *, ftnlen);
    static logical rstart;
    static integer msglvl;
    static real smlnum;
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


/*     %---------------% */
/*     | Local Scalars | */
/*     %---------------% */


/*     %-----------------------% */
/*     | Local Array Arguments | */
/*     %-----------------------% */


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
/*        | REFERENCE: LAPACK subroutine slahqr     | */
/*        %-----------------------------------------% */

	unfl = slamch_("safe minimum", (ftnlen)12);
	ovfl = 1.f / unfl;
	slabad_(&unfl, &ovfl);
	ulp = slamch_("precision", (ftnlen)9);
	smlnum = unfl * (*n / ulp);
	first = FALSE_;
    }

    if (*ido == 0) {

/*        %-------------------------------% */
/*        | Initialize timing statistics  | */
/*        | & message level for debugging | */
/*        %-------------------------------% */

	second_(&t0);
	msglvl = debug_1.mnaitr;

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
/*     |         sgetv0.                                 | */
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
	svout_(&debug_1.logfil, &c__1, rnorm, &debug_1.ndigit, "_naitr: B-no"
		"rm of the current residual is", (ftnlen)41);
    }

/*        %---------------------------------------------------% */
/*        | STEP 1: Check if the B norm of j-th residual      | */
/*        | vector is zero. Equivalent to determing whether   | */
/*        | an exact j-step Arnoldi factorization is present. | */
/*        %---------------------------------------------------% */

    betaj = *rnorm;
    if (*rnorm > 0.f) {
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

    betaj = 0.f;
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

    sgetv0_(ido, bmat, &itry, &c_false, n, &j, &v[v_offset], ldv, &resid[1], 
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
	timing_1.tnaitr += t1 - t0;
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

    scopy_(n, &resid[1], &c__1, &v[j * v_dim1 + 1], &c__1);
    if (*rnorm >= unfl) {
	temp1 = 1.f / *rnorm;
	sscal_(n, &temp1, &v[j * v_dim1 + 1], &c__1);
	sscal_(n, &temp1, &workd[ipj], &c__1);
    } else {

/*            %-----------------------------------------% */
/*            | To scale both v_{j} and p_{j} carefully | */
/*            | use LAPACK routine SLASCL               | */
/*            %-----------------------------------------% */

	slascl_("General", &i__, &i__, rnorm, &c_b25, n, &c__1, &v[j * v_dim1 
		+ 1], n, &infol, (ftnlen)7);
	slascl_("General", &i__, &i__, rnorm, &c_b25, n, &c__1, &workd[ipj], 
		n, &infol, (ftnlen)7);
    }

/*        %------------------------------------------------------% */
/*        | STEP 3:  r_{j} = OP*v_{j}; Note that p_{j} = B*v_{j} | */
/*        | Note that this is not quite yet r_{j}. See STEP 4    | */
/*        %------------------------------------------------------% */

    step3 = TRUE_;
    ++timing_1.nopx;
    second_(&t2);
    scopy_(n, &v[j * v_dim1 + 1], &c__1, &workd[ivj], &c__1);
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

    scopy_(n, &workd[irj], &c__1, &resid[1], &c__1);

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
	scopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
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
	wnorm = sdot_(n, &resid[1], &c__1, &workd[ipj], &c__1);
	wnorm = sqrt((dabs(wnorm)));
    } else if (*(unsigned char *)bmat == 'I') {
	wnorm = snrm2_(n, &resid[1], &c__1);
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

    sgemv_("T", n, &j, &c_b25, &v[v_offset], ldv, &workd[ipj], &c__1, &c_b47, 
	    &h__[j * h_dim1 + 1], &c__1, (ftnlen)1);

/*        %--------------------------------------% */
/*        | Orthogonalize r_{j} against V_{j}.   | */
/*        | RESID contains OP*v_{j}. See STEP 3. | */
/*        %--------------------------------------% */

    sgemv_("N", n, &j, &c_b50, &v[v_offset], ldv, &h__[j * h_dim1 + 1], &c__1,
	     &c_b25, &resid[1], &c__1, (ftnlen)1);

    if (j > 1) {
	h__[j + (j - 1) * h_dim1] = betaj;
    }

    second_(&t4);

    orth1 = TRUE_;

    second_(&t2);
    if (*(unsigned char *)bmat == 'G') {
	++timing_1.nbx;
	scopy_(n, &resid[1], &c__1, &workd[irj], &c__1);
	ipntr[1] = irj;
	ipntr[2] = ipj;
	*ido = 2;

/*           %----------------------------------% */
/*           | Exit in order to compute B*r_{j} | */
/*           %----------------------------------% */

	goto L9000;
    } else if (*(unsigned char *)bmat == 'I') {
	scopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
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
	*rnorm = sdot_(n, &resid[1], &c__1, &workd[ipj], &c__1);
	*rnorm = sqrt((dabs(*rnorm)));
    } else if (*(unsigned char *)bmat == 'I') {
	*rnorm = snrm2_(n, &resid[1], &c__1);
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
	xtemp[0] = wnorm;
	xtemp[1] = *rnorm;
	svout_(&debug_1.logfil, &c__2, xtemp, &debug_1.ndigit, "_naitr: re-o"
		"rthonalization; wnorm and rnorm are", (ftnlen)47);
	svout_(&debug_1.logfil, &j, &h__[j * h_dim1 + 1], &debug_1.ndigit, 
		"_naitr: j-th column of H", (ftnlen)24);
    }

/*        %----------------------------------------------------% */
/*        | Compute V_{j}^T * B * r_{j}.                       | */
/*        | WORKD(IRJ:IRJ+J-1) = v(:,1:J)'*WORKD(IPJ:IPJ+N-1). | */
/*        %----------------------------------------------------% */

    sgemv_("T", n, &j, &c_b25, &v[v_offset], ldv, &workd[ipj], &c__1, &c_b47, 
	    &workd[irj], &c__1, (ftnlen)1);

/*        %---------------------------------------------% */
/*        | Compute the correction to the residual:     | */
/*        | r_{j} = r_{j} - V_{j} * WORKD(IRJ:IRJ+J-1). | */
/*        | The correction to H is v(:,1:J)*H(1:J,1:J)  | */
/*        | + v(:,1:J)*WORKD(IRJ:IRJ+J-1)*e'_j.         | */
/*        %---------------------------------------------% */

    sgemv_("N", n, &j, &c_b50, &v[v_offset], ldv, &workd[irj], &c__1, &c_b25, 
	    &resid[1], &c__1, (ftnlen)1);
    saxpy_(&j, &c_b25, &workd[irj], &c__1, &h__[j * h_dim1 + 1], &c__1);

    orth2 = TRUE_;
    second_(&t2);
    if (*(unsigned char *)bmat == 'G') {
	++timing_1.nbx;
	scopy_(n, &resid[1], &c__1, &workd[irj], &c__1);
	ipntr[1] = irj;
	ipntr[2] = ipj;
	*ido = 2;

/*           %-----------------------------------% */
/*           | Exit in order to compute B*r_{j}. | */
/*           | r_{j} is the corrected residual.  | */
/*           %-----------------------------------% */

	goto L9000;
    } else if (*(unsigned char *)bmat == 'I') {
	scopy_(n, &resid[1], &c__1, &workd[ipj], &c__1);
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
	rnorm1 = sdot_(n, &resid[1], &c__1, &workd[ipj], &c__1);
	rnorm1 = sqrt((dabs(rnorm1)));
    } else if (*(unsigned char *)bmat == 'I') {
	rnorm1 = snrm2_(n, &resid[1], &c__1);
    }

    if (msglvl > 0 && iter > 0) {
	ivout_(&debug_1.logfil, &c__1, &j, &debug_1.ndigit, "_naitr: Iterati"
		"ve refinement for Arnoldi residual", (ftnlen)49);
	if (msglvl > 2) {
	    xtemp[0] = *rnorm;
	    xtemp[1] = rnorm1;
	    svout_(&debug_1.logfil, &c__2, xtemp, &debug_1.ndigit, "_naitr: "
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
	    resid[jj] = 0.f;
/* L95: */
	}
	*rnorm = 0.f;
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
	timing_1.tnaitr += t1 - t0;
	*ido = 99;
	i__1 = *k + *np - 1;
	for (i__ = max(1,*k); i__ <= i__1; ++i__) {

/*              %--------------------------------------------% */
/*              | Check for splitting and deflation.         | */
/*              | Use a standard test as in the QR algorithm | */
/*              | REFERENCE: LAPACK subroutine slahqr        | */
/*              %--------------------------------------------% */

	    tst1 = (r__1 = h__[i__ + i__ * h_dim1], dabs(r__1)) + (r__2 = h__[
		    i__ + 1 + (i__ + 1) * h_dim1], dabs(r__2));
	    if (tst1 == 0.f) {
		i__2 = *k + *np;
		tst1 = slanhs_("1", &i__2, &h__[h_offset], ldh, &workd[*n + 1]
			, (ftnlen)1);
	    }
/* Computing MAX */
	    r__2 = ulp * tst1;
	    if ((r__1 = h__[i__ + 1 + i__ * h_dim1], dabs(r__1)) <= dmax(r__2,
		    smlnum)) {
		h__[i__ + 1 + i__ * h_dim1] = 0.f;
	    }
/* L110: */
	}

	if (msglvl > 2) {
	    i__1 = *k + *np;
	    i__2 = *k + *np;
	    smout_(&debug_1.logfil, &i__1, &i__2, &h__[h_offset], ldh, &
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
/*     | End of snaitr | */
/*     %---------------% */

} /* snaitr_ */

