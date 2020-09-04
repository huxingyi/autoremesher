/* ../FORTRAN/ARPACK/SRC/znaupd.f -- translated by f2c (version 20100827).
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

/* \BeginDoc */

/* \Name: znaupd */

/* \Description: */
/*  Reverse communication interface for the Implicitly Restarted Arnoldi */
/*  iteration. This is intended to be used to find a few eigenpairs of a */
/*  complex linear operator OP with respect to a semi-inner product defined */
/*  by a hermitian positive semi-definite real matrix B. B may be the identity */
/*  matrix.  NOTE: if both OP and B are real, then dsaupd or dnaupd should */
/*  be used. */


/*  The computed approximate eigenvalues are called Ritz values and */
/*  the corresponding approximate eigenvectors are called Ritz vectors. */

/*  znaupd is usually called iteratively to solve one of the */
/*  following problems: */

/*  Mode 1:  A*x = lambda*x. */
/*           ===> OP = A  and  B = I. */

/*  Mode 2:  A*x = lambda*M*x, M hermitian positive definite */
/*           ===> OP = inv[M]*A  and  B = M. */
/*           ===> (If M can be factored see remark 3 below) */

/*  Mode 3:  A*x = lambda*M*x, M hermitian semi-definite */
/*           ===> OP =  inv[A - sigma*M]*M   and  B = M. */
/*           ===> shift-and-invert mode */
/*           If OP*x = amu*x, then lambda = sigma + 1/amu. */


/*  NOTE: The action of w <- inv[A - sigma*M]*v or w <- inv[M]*v */
/*        should be accomplished either by a direct method */
/*        using a sparse matrix factorization and solving */

/*           [A - sigma*M]*w = v  or M*w = v, */

/*        or through an iterative method for solving these */
/*        systems.  If an iterative method is used, the */
/*        convergence test must be more stringent than */
/*        the accuracy requirements for the eigenvalue */
/*        approximations. */

/* \Usage: */
/*  call znaupd */
/*     ( IDO, BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, */
/*       IPNTR, WORKD, WORKL, LWORKL, RWORK, INFO ) */

/* \Arguments */
/*  IDO     Integer.  (INPUT/OUTPUT) */
/*          Reverse communication flag.  IDO must be zero on the first */
/*          call to znaupd.  IDO will be set internally to */
/*          indicate the type of operation to be performed.  Control is */
/*          then given back to the calling routine which has the */
/*          responsibility to carry out the requested operation and call */
/*          znaupd with the result.  The operand is given in */
/*          WORKD(IPNTR(1)), the result must be put in WORKD(IPNTR(2)). */
/*          ------------------------------------------------------------- */
/*          IDO =  0: first call to the reverse communication interface */
/*          IDO = -1: compute  Y = OP * X  where */
/*                    IPNTR(1) is the pointer into WORKD for X, */
/*                    IPNTR(2) is the pointer into WORKD for Y. */
/*                    This is for the initialization phase to force the */
/*                    starting vector into the range of OP. */
/*          IDO =  1: compute  Y = OP * X  where */
/*                    IPNTR(1) is the pointer into WORKD for X, */
/*                    IPNTR(2) is the pointer into WORKD for Y. */
/*                    In mode 3, the vector B * X is already */
/*                    available in WORKD(ipntr(3)).  It does not */
/*                    need to be recomputed in forming OP * X. */
/*          IDO =  2: compute  Y = M * X  where */
/*                    IPNTR(1) is the pointer into WORKD for X, */
/*                    IPNTR(2) is the pointer into WORKD for Y. */
/*          IDO =  3: compute and return the shifts in the first */
/*                    NP locations of WORKL. */
/*          IDO = 99: done */
/*          ------------------------------------------------------------- */
/*          After the initialization phase, when the routine is used in */
/*          the "shift-and-invert" mode, the vector M * X is already */
/*          available and does not need to be recomputed in forming OP*X. */

/*  BMAT    Character*1.  (INPUT) */
/*          BMAT specifies the type of the matrix B that defines the */
/*          semi-inner product for the operator OP. */
/*          BMAT = 'I' -> standard eigenvalue problem A*x = lambda*x */
/*          BMAT = 'G' -> generalized eigenvalue problem A*x = lambda*M*x */

/*  N       Integer.  (INPUT) */
/*          Dimension of the eigenproblem. */

/*  WHICH   Character*2.  (INPUT) */
/*          'LM' -> want the NEV eigenvalues of largest magnitude. */
/*          'SM' -> want the NEV eigenvalues of smallest magnitude. */
/*          'LR' -> want the NEV eigenvalues of largest real part. */
/*          'SR' -> want the NEV eigenvalues of smallest real part. */
/*          'LI' -> want the NEV eigenvalues of largest imaginary part. */
/*          'SI' -> want the NEV eigenvalues of smallest imaginary part. */

/*  NEV     Integer.  (INPUT) */
/*          Number of eigenvalues of OP to be computed. 0 < NEV < N-1. */

/*  TOL     Double precision  scalar.  (INPUT) */
/*          Stopping criteria: the relative accuracy of the Ritz value */
/*          is considered acceptable if BOUNDS(I) .LE. TOL*ABS(RITZ(I)) */
/*          where ABS(RITZ(I)) is the magnitude when RITZ(I) is complex. */
/*          DEFAULT = dlamch('EPS')  (machine precision as computed */
/*                    by the LAPACK auxiliary subroutine dlamch). */

/*  RESID   Complex*16 array of length N.  (INPUT/OUTPUT) */
/*          On INPUT: */
/*          If INFO .EQ. 0, a random initial residual vector is used. */
/*          If INFO .NE. 0, RESID contains the initial residual vector, */
/*                          possibly from a previous run. */
/*          On OUTPUT: */
/*          RESID contains the final residual vector. */

/*  NCV     Integer.  (INPUT) */
/*          Number of columns of the matrix V. NCV must satisfy the two */
/*          inequalities 1 <= NCV-NEV and NCV <= N. */
/*          This will indicate how many Arnoldi vectors are generated */
/*          at each iteration.  After the startup phase in which NEV */
/*          Arnoldi vectors are generated, the algorithm generates */
/*          approximately NCV-NEV Arnoldi vectors at each subsequent update */
/*          iteration. Most of the cost in generating each Arnoldi vector is */
/*          in the matrix-vector operation OP*x. (See remark 4 below.) */

/*  V       Complex*16 array N by NCV.  (OUTPUT) */
/*          Contains the final set of Arnoldi basis vectors. */

/*  LDV     Integer.  (INPUT) */
/*          Leading dimension of V exactly as declared in the calling program. */

/*  IPARAM  Integer array of length 11.  (INPUT/OUTPUT) */
/*          IPARAM(1) = ISHIFT: method for selecting the implicit shifts. */
/*          The shifts selected at each iteration are used to filter out */
/*          the components of the unwanted eigenvector. */
/*          ------------------------------------------------------------- */
/*          ISHIFT = 0: the shifts are to be provided by the user via */
/*                      reverse communication.  The NCV eigenvalues of */
/*                      the Hessenberg matrix H are returned in the part */
/*                      of WORKL array corresponding to RITZ. */
/*          ISHIFT = 1: exact shifts with respect to the current */
/*                      Hessenberg matrix H.  This is equivalent to */
/*                      restarting the iteration from the beginning */
/*                      after updating the starting vector with a linear */
/*                      combination of Ritz vectors associated with the */
/*                      "wanted" eigenvalues. */
/*          ISHIFT = 2: other choice of internal shift to be defined. */
/*          ------------------------------------------------------------- */

/*          IPARAM(2) = No longer referenced */

/*          IPARAM(3) = MXITER */
/*          On INPUT:  maximum number of Arnoldi update iterations allowed. */
/*          On OUTPUT: actual number of Arnoldi update iterations taken. */

/*          IPARAM(4) = NB: blocksize to be used in the recurrence. */
/*          The code currently works only for NB = 1. */

/*          IPARAM(5) = NCONV: number of "converged" Ritz values. */
/*          This represents the number of Ritz values that satisfy */
/*          the convergence criterion. */

/*          IPARAM(6) = IUPD */
/*          No longer referenced. Implicit restarting is ALWAYS used. */

/*          IPARAM(7) = MODE */
/*          On INPUT determines what type of eigenproblem is being solved. */
/*          Must be 1,2,3; See under \Description of znaupd for the */
/*          four modes available. */

/*          IPARAM(8) = NP */
/*          When ido = 3 and the user provides shifts through reverse */
/*          communication (IPARAM(1)=0), _naupd returns NP, the number */
/*          of shifts the user is to provide. 0 < NP < NCV-NEV. */

/*          IPARAM(9) = NUMOP, IPARAM(10) = NUMOPB, IPARAM(11) = NUMREO, */
/*          OUTPUT: NUMOP  = total number of OP*x operations, */
/*                  NUMOPB = total number of B*x operations if BMAT='G', */
/*                  NUMREO = total number of steps of re-orthogonalization. */

/*  IPNTR   Integer array of length 14.  (OUTPUT) */
/*          Pointer to mark the starting locations in the WORKD and WORKL */
/*          arrays for matrices/vectors used by the Arnoldi iteration. */
/*          ------------------------------------------------------------- */
/*          IPNTR(1): pointer to the current operand vector X in WORKD. */
/*          IPNTR(2): pointer to the current result vector Y in WORKD. */
/*          IPNTR(3): pointer to the vector B * X in WORKD when used in */
/*                    the shift-and-invert mode. */
/*          IPNTR(4): pointer to the next available location in WORKL */
/*                    that is untouched by the program. */
/*          IPNTR(5): pointer to the NCV by NCV upper Hessenberg */
/*                    matrix H in WORKL. */
/*          IPNTR(6): pointer to the  ritz value array  RITZ */
/*          IPNTR(7): pointer to the (projected) ritz vector array Q */
/*          IPNTR(8): pointer to the error BOUNDS array in WORKL. */
/*          IPNTR(14): pointer to the NP shifts in WORKL. See Remark 5 below. */

/*          Note: IPNTR(9:13) is only referenced by zneupd. See Remark 2 below. */

/*          IPNTR(9): pointer to the NCV RITZ values of the */
/*                    original system. */
/*          IPNTR(10): Not Used */
/*          IPNTR(11): pointer to the NCV corresponding error bounds. */
/*          IPNTR(12): pointer to the NCV by NCV upper triangular */
/*                     Schur matrix for H. */
/*          IPNTR(13): pointer to the NCV by NCV matrix of eigenvectors */
/*                     of the upper Hessenberg matrix H. Only referenced by */
/*                     zneupd if RVEC = .TRUE. See Remark 2 below. */

/*          ------------------------------------------------------------- */

/*  WORKD   Complex*16 work array of length 3*N.  (REVERSE COMMUNICATION) */
/*          Distributed array to be used in the basic Arnoldi iteration */
/*          for reverse communication.  The user should not use WORKD */
/*          as temporary workspace during the iteration !!!!!!!!!! */
/*          See Data Distribution Note below. */

/*  WORKL   Complex*16 work array of length LWORKL.  (OUTPUT/WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end.  See Data Distribution Note below. */

/*  LWORKL  Integer.  (INPUT) */
/*          LWORKL must be at least 3*NCV**2 + 5*NCV. */

/*  RWORK   Double precision  work array of length NCV (WORKSPACE) */
/*          Private (replicated) array on each PE or array allocated on */
/*          the front end. */


/*  INFO    Integer.  (INPUT/OUTPUT) */
/*          If INFO .EQ. 0, a randomly initial residual vector is used. */
/*          If INFO .NE. 0, RESID contains the initial residual vector, */
/*                          possibly from a previous run. */
/*          Error flag on output. */
/*          =  0: Normal exit. */
/*          =  1: Maximum number of iterations taken. */
/*                All possible eigenvalues of OP has been found. IPARAM(5) */
/*                returns the number of wanted converged Ritz values. */
/*          =  2: No longer an informational error. Deprecated starting */
/*                with release 2 of ARPACK. */
/*          =  3: No shifts could be applied during a cycle of the */
/*                Implicitly restarted Arnoldi iteration. One possibility */
/*                is to increase the size of NCV relative to NEV. */
/*                See remark 4 below. */
/*          = -1: N must be positive. */
/*          = -2: NEV must be positive. */
/*          = -3: NCV-NEV >= 1 and less than or equal to N. */
/*          = -4: The maximum number of Arnoldi update iteration */
/*                must be greater than zero. */
/*          = -5: WHICH must be one of 'LM', 'SM', 'LR', 'SR', 'LI', 'SI' */
/*          = -6: BMAT must be one of 'I' or 'G'. */
/*          = -7: Length of private work array is not sufficient. */
/*          = -8: Error return from LAPACK eigenvalue calculation; */
/*          = -9: Starting vector is zero. */
/*          = -10: IPARAM(7) must be 1,2,3. */
/*          = -11: IPARAM(7) = 1 and BMAT = 'G' are incompatible. */
/*          = -12: IPARAM(1) must be equal to 0 or 1. */
/*          = -9999: Could not build an Arnoldi factorization. */
/*                   User input error highly likely.  Please */
/*                   check actual array dimensions and layout. */
/*                   IPARAM(5) returns the size of the current Arnoldi */
/*                   factorization. */

/* \Remarks */
/*  1. The computed Ritz values are approximate eigenvalues of OP. The */
/*     selection of WHICH should be made with this in mind when using */
/*     Mode = 3.  When operating in Mode = 3 setting WHICH = 'LM' will */
/*     compute the NEV eigenvalues of the original problem that are */
/*     closest to the shift SIGMA . After convergence, approximate eigenvalues */
/*     of the original problem may be obtained with the ARPACK subroutine zneupd. */

/*  2. If a basis for the invariant subspace corresponding to the converged Ritz */
/*     values is needed, the user must call zneupd immediately following */
/*     completion of znaupd. This is new starting with release 2 of ARPACK. */

/*  3. If M can be factored into a Cholesky factorization M = LL` */
/*     then Mode = 2 should not be selected.  Instead one should use */
/*     Mode = 1 with  OP = inv(L)*A*inv(L`).  Appropriate triangular */
/*     linear systems should be solved with L and L` rather */
/*     than computing inverses.  After convergence, an approximate */
/*     eigenvector z of the original problem is recovered by solving */
/*     L`z = x  where x is a Ritz vector of OP. */

/*  4. At present there is no a-priori analysis to guide the selection */
/*     of NCV relative to NEV.  The only formal requirement is that NCV > NEV + 1. */
/*     However, it is recommended that NCV .ge. 2*NEV.  If many problems of */
/*     the same type are to be solved, one should experiment with increasing */
/*     NCV while keeping NEV fixed for a given test problem.  This will */
/*     usually decrease the required number of OP*x operations but it */
/*     also increases the work and storage required to maintain the orthogonal */
/*     basis vectors.  The optimal "cross-over" with respect to CPU time */
/*     is problem dependent and must be determined empirically. */
/*     See Chapter 8 of Reference 2 for further information. */

/*  5. When IPARAM(1) = 0, and IDO = 3, the user needs to provide the */
/*     NP = IPARAM(8) complex shifts in locations */
/*     WORKL(IPNTR(14)), WORKL(IPNTR(14)+1), ... , WORKL(IPNTR(14)+NP). */
/*     Eigenvalues of the current upper Hessenberg matrix are located in */
/*     WORKL(IPNTR(6)) through WORKL(IPNTR(6)+NCV-1). They are ordered */
/*     according to the order defined by WHICH.  The associated Ritz estimates */
/*     are located in WORKL(IPNTR(8)), WORKL(IPNTR(8)+1), ... , */
/*     WORKL(IPNTR(8)+NCV-1). */

/* ----------------------------------------------------------------------- */

/* \Data Distribution Note: */

/*  Fortran-D syntax: */
/*  ================ */
/*  Complex*16 resid(n), v(ldv,ncv), workd(3*n), workl(lworkl) */
/*  decompose  d1(n), d2(n,ncv) */
/*  align      resid(i) with d1(i) */
/*  align      v(i,j)   with d2(i,j) */
/*  align      workd(i) with d1(i)     range (1:n) */
/*  align      workd(i) with d1(i-n)   range (n+1:2*n) */
/*  align      workd(i) with d1(i-2*n) range (2*n+1:3*n) */
/*  distribute d1(block), d2(block,:) */
/*  replicated workl(lworkl) */

/*  Cray MPP syntax: */
/*  =============== */
/*  Complex*16 resid(n), v(ldv,ncv), workd(n,3), workl(lworkl) */
/*  shared     resid(block), v(block,:), workd(block,:) */
/*  replicated workl(lworkl) */

/*  CM2/CM5 syntax: */
/*  ============== */

/* ----------------------------------------------------------------------- */

/*     include   'ex-nonsym.doc' */

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
/*  3. B.N. Parlett & Y. Saad, "_Complex_ Shift and Invert Strategies for */
/*     Double precision Matrices", Linear Algebra and its Applications, vol 88/89, */
/*     pp 575-595, (1987). */

/* \Routines called: */
/*     znaup2  ARPACK routine that implements the Implicitly Restarted */
/*             Arnoldi Iteration. */
/*     zstatn  ARPACK routine that initializes the timing variables. */
/*     ivout   ARPACK utility routine that prints integers. */
/*     zvout   ARPACK utility routine that prints vectors. */
/*     second  ARPACK utility routine for timing. */
/*     dlamch  LAPACK routine that determines machine constants. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \SCCS Information: @(#) */
/* FILE: naupd.F   SID: 2.9   DATE OF SID: 07/21/02   RELEASE: 2 */

/* \Remarks */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int znaupd_(integer *ido, char *bmat, integer *n, char *
	which, integer *nev, doublereal *tol, doublecomplex *resid, integer *
	ncv, doublecomplex *v, integer *ldv, integer *iparam, integer *ipntr, 
	doublecomplex *workd, doublecomplex *workl, integer *lworkl, 
	doublereal *rwork, integer *info, ftnlen bmat_len, ftnlen which_len)
{
    /* Format strings */
    static char fmt_1000[] = "(//,5x,\002==================================="
	    "==========\002,/5x,\002= Complex implicit Arnoldi update code   "
	    "   =\002,/5x,\002= Version Number: \002,\002 2.3\002,21x,\002 "
	    "=\002,/5x,\002= Version Date:   \002,\002 07/31/96\002,16x,\002 ="
	    "\002,/5x,\002=============================================\002,/"
	    "5x,\002= Summary of timing statistics              =\002,/5x,"
	    "\002=============================================\002,//)";
    static char fmt_1100[] = "(5x,\002Total number update iterations        "
	    "     = \002,i5,/5x,\002Total number of OP*x operations          "
	    "  = \002,i5,/5x,\002Total number of B*x operations             = "
	    "\002,i5,/5x,\002Total number of reorthogonalization steps  = "
	    "\002,i5,/5x,\002Total number of iterative refinement steps = "
	    "\002,i5,/5x,\002Total number of restart steps              = "
	    "\002,i5,/5x,\002Total time in user OP*x operation          = "
	    "\002,f12.6,/5x,\002Total time in user B*x operation           ="
	    " \002,f12.6,/5x,\002Total time in Arnoldi update routine       = "
	    "\002,f12.6,/5x,\002Total time in naup2 routine                ="
	    " \002,f12.6,/5x,\002Total time in basic Arnoldi iteration loop = "
	    "\002,f12.6,/5x,\002Total time in reorthogonalization phase    ="
	    " \002,f12.6,/5x,\002Total time in (re)start vector generation  = "
	    "\002,f12.6,/5x,\002Total time in Hessenberg eig. subproblem   ="
	    " \002,f12.6,/5x,\002Total time in getting the shifts           = "
	    "\002,f12.6,/5x,\002Total time in applying the shifts          ="
	    " \002,f12.6,/5x,\002Total time in convergence testing          = "
	    "\002,f12.6,/5x,\002Total time in computing final Ritz vectors ="
	    " \002,f12.6/)";

    /* System generated locals */
    integer v_dim1, v_offset, i__1, i__2;

    /* Builtin functions */
    integer s_cmp(char *, char *, ftnlen, ftnlen), s_wsfe(cilist *), e_wsfe(
	    void), do_fio(integer *, char *, ftnlen);

    /* Local variables */
    static integer j;
    static real t0, t1;
    static integer nb, ih, iq, np, iw, ldh, ldq, nev0, mode, ierr, iupd, next,
	     ritz;
    extern /* Subroutine */ int ivout_(integer *, integer *, integer *, 
	    integer *, char *, ftnlen), zvout_(integer *, integer *, 
	    doublecomplex *, integer *, char *, ftnlen), znaup2_(integer *, 
	    char *, integer *, char *, integer *, integer *, doublereal *, 
	    doublecomplex *, integer *, integer *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, doublecomplex *, doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublereal *, 
	    integer *, ftnlen, ftnlen);
    extern doublereal dlamch_(char *, ftnlen);
    extern /* Subroutine */ int second_(real *);
    static integer bounds, ishift, msglvl, mxiter;
    extern /* Subroutine */ int zstatn_(void);

    /* Fortran I/O blocks */
    static cilist io___21 = { 0, 6, 0, fmt_1000, 0 };
    static cilist io___22 = { 0, 6, 0, fmt_1100, 0 };



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


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    /* Parameter adjustments */
    --workd;
    --resid;
    --rwork;
    v_dim1 = *ldv;
    v_offset = 1 + v_dim1;
    v -= v_offset;
    --iparam;
    --ipntr;
    --workl;

    /* Function Body */
    if (*ido == 0) {

/*        %-------------------------------% */
/*        | Initialize timing statistics  | */
/*        | & message level for debugging | */
/*        %-------------------------------% */

	zstatn_();
	second_(&t0);
	msglvl = debug_1.mcaupd;

/*        %----------------% */
/*        | Error checking | */
/*        %----------------% */

	ierr = 0;
	ishift = iparam[1];
/*         levec  = iparam(2) */
	mxiter = iparam[3];
/*         nb     = iparam(4) */
	nb = 1;

/*        %--------------------------------------------% */
/*        | Revision 2 performs only implicit restart. | */
/*        %--------------------------------------------% */

	iupd = 1;
	mode = iparam[7];

	if (*n <= 0) {
	    ierr = -1;
	} else if (*nev <= 0) {
	    ierr = -2;
	} else if (*ncv <= *nev || *ncv > *n) {
	    ierr = -3;
	} else if (mxiter <= 0) {
	    ierr = -4;
	} else if (s_cmp(which, "LM", (ftnlen)2, (ftnlen)2) != 0 && s_cmp(
		which, "SM", (ftnlen)2, (ftnlen)2) != 0 && s_cmp(which, "LR", 
		(ftnlen)2, (ftnlen)2) != 0 && s_cmp(which, "SR", (ftnlen)2, (
		ftnlen)2) != 0 && s_cmp(which, "LI", (ftnlen)2, (ftnlen)2) != 
		0 && s_cmp(which, "SI", (ftnlen)2, (ftnlen)2) != 0) {
	    ierr = -5;
	} else if (*(unsigned char *)bmat != 'I' && *(unsigned char *)bmat != 
		'G') {
	    ierr = -6;
	} else /* if(complicated condition) */ {
/* Computing 2nd power */
	    i__1 = *ncv;
	    if (*lworkl < i__1 * i__1 * 3 + *ncv * 5) {
		ierr = -7;
	    } else if (mode < 1 || mode > 3) {
		ierr = -10;
	    } else if (mode == 1 && *(unsigned char *)bmat == 'G') {
		ierr = -11;
	    }
	}

/*        %------------% */
/*        | Error Exit | */
/*        %------------% */

	if (ierr != 0) {
	    *info = ierr;
	    *ido = 99;
	    goto L9000;
	}

/*        %------------------------% */
/*        | Set default parameters | */
/*        %------------------------% */

	if (nb <= 0) {
	    nb = 1;
	}
	if (*tol <= 0.) {
	    *tol = dlamch_("EpsMach", (ftnlen)7);
	}
	if (ishift != 0 && ishift != 1 && ishift != 2) {
	    ishift = 1;
	}

/*        %----------------------------------------------% */
/*        | NP is the number of additional steps to      | */
/*        | extend the length NEV Lanczos factorization. | */
/*        | NEV0 is the local variable designating the   | */
/*        | size of the invariant subspace desired.      | */
/*        %----------------------------------------------% */

	np = *ncv - *nev;
	nev0 = *nev;

/*        %-----------------------------% */
/*        | Zero out internal workspace | */
/*        %-----------------------------% */

/* Computing 2nd power */
	i__2 = *ncv;
	i__1 = i__2 * i__2 * 3 + *ncv * 5;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = j;
	    workl[i__2].r = 0., workl[i__2].i = 0.;
/* L10: */
	}

/*        %-------------------------------------------------------------% */
/*        | Pointer into WORKL for address of H, RITZ, BOUNDS, Q        | */
/*        | etc... and the remaining workspace.                         | */
/*        | Also update pointer to be used on output.                   | */
/*        | Memory is laid out as follows:                              | */
/*        | workl(1:ncv*ncv) := generated Hessenberg matrix             | */
/*        | workl(ncv*ncv+1:ncv*ncv+ncv) := the ritz values             | */
/*        | workl(ncv*ncv+ncv+1:ncv*ncv+2*ncv)   := error bounds        | */
/*        | workl(ncv*ncv+2*ncv+1:2*ncv*ncv+2*ncv) := rotation matrix Q | */
/*        | workl(2*ncv*ncv+2*ncv+1:3*ncv*ncv+5*ncv) := workspace       | */
/*        | The final workspace is needed by subroutine zneigh called   | */
/*        | by znaup2. Subroutine zneigh calls LAPACK routines for      | */
/*        | calculating eigenvalues and the last row of the eigenvector | */
/*        | matrix.                                                     | */
/*        %-------------------------------------------------------------% */

	ldh = *ncv;
	ldq = *ncv;
	ih = 1;
	ritz = ih + ldh * *ncv;
	bounds = ritz + *ncv;
	iq = bounds + *ncv;
	iw = iq + ldq * *ncv;
/* Computing 2nd power */
	i__1 = *ncv;
	next = iw + i__1 * i__1 + *ncv * 3;

	ipntr[4] = next;
	ipntr[5] = ih;
	ipntr[6] = ritz;
	ipntr[7] = iq;
	ipntr[8] = bounds;
	ipntr[14] = iw;
    }

/*     %-------------------------------------------------------% */
/*     | Carry out the Implicitly restarted Arnoldi Iteration. | */
/*     %-------------------------------------------------------% */

    znaup2_(ido, bmat, n, which, &nev0, &np, tol, &resid[1], &mode, &iupd, &
	    ishift, &mxiter, &v[v_offset], ldv, &workl[ih], &ldh, &workl[ritz]
	    , &workl[bounds], &workl[iq], &ldq, &workl[iw], &ipntr[1], &workd[
	    1], &rwork[1], info, (ftnlen)1, (ftnlen)2);

/*     %--------------------------------------------------% */
/*     | ido .ne. 99 implies use of reverse communication | */
/*     | to compute operations involving OP.              | */
/*     %--------------------------------------------------% */

    if (*ido == 3) {
	iparam[8] = np;
    }
    if (*ido != 99) {
	goto L9000;
    }

    iparam[3] = mxiter;
    iparam[5] = np;
    iparam[9] = timing_1.nopx;
    iparam[10] = timing_1.nbx;
    iparam[11] = timing_1.nrorth;

/*     %------------------------------------% */
/*     | Exit if there was an informational | */
/*     | error within znaup2.               | */
/*     %------------------------------------% */

    if (*info < 0) {
	goto L9000;
    }
    if (*info == 2) {
	*info = 3;
    }

    if (msglvl > 0) {
	ivout_(&debug_1.logfil, &c__1, &mxiter, &debug_1.ndigit, "_naupd: Nu"
		"mber of update iterations taken", (ftnlen)41);
	ivout_(&debug_1.logfil, &c__1, &np, &debug_1.ndigit, "_naupd: Number"
		" of wanted \"converged\" Ritz values", (ftnlen)48);
	zvout_(&debug_1.logfil, &np, &workl[ritz], &debug_1.ndigit, "_naupd:"
		" The final Ritz values", (ftnlen)29);
	zvout_(&debug_1.logfil, &np, &workl[bounds], &debug_1.ndigit, "_naup"
		"d: Associated Ritz estimates", (ftnlen)33);
    }

    second_(&t1);
    timing_1.tcaupd = t1 - t0;

    if (msglvl > 0) {

/*        %--------------------------------------------------------% */
/*        | Version Number & Version Date are defined in version.h | */
/*        %--------------------------------------------------------% */

	s_wsfe(&io___21);
	e_wsfe();
	s_wsfe(&io___22);
	do_fio(&c__1, (char *)&mxiter, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.nopx, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.nbx, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.nrorth, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.nitref, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.nrstrt, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&timing_1.tmvopx, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tmvbx, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcaupd, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcaup2, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcaitr, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.titref, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tgetv0, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tceigh, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcgets, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcapps, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.tcconv, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&timing_1.trvec, (ftnlen)sizeof(real));
	e_wsfe();
    }

L9000:

    return 0;

/*     %---------------% */
/*     | End of znaupd | */
/*     %---------------% */

} /* znaupd_ */

