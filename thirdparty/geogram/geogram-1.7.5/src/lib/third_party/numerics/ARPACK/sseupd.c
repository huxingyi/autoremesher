/* ../FORTRAN/ARPACK/SRC/sseupd.f -- translated by f2c (version 20100827).
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

static doublereal c_b21 = .66666666666666663;
static integer c__1 = 1;
static logical c_true = TRUE_;
static real c_b110 = 1.f;

/* \BeginDoc */

/* \Name: sseupd */

/* \Description: */

/*  This subroutine returns the converged approximations to eigenvalues */
/*  of A*z = lambda*B*z and (optionally): */

/*      (1) the corresponding approximate eigenvectors, */

/*      (2) an orthonormal (Lanczos) basis for the associated approximate */
/*          invariant subspace, */

/*      (3) Both. */

/*  There is negligible additional cost to obtain eigenvectors.  An orthonormal */
/*  (Lanczos) basis is always computed.  There is an additional storage cost */
/*  of n*nev if both are requested (in this case a separate array Z must be */
/*  supplied). */

/*  These quantities are obtained from the Lanczos factorization computed */
/*  by SSAUPD for the linear operator OP prescribed by the MODE selection */
/*  (see IPARAM(7) in SSAUPD documentation.)  SSAUPD must be called before */
/*  this routine is called. These approximate eigenvalues and vectors are */
/*  commonly called Ritz values and Ritz vectors respectively.  They are */
/*  referred to as such in the comments that follow.   The computed orthonormal */
/*  basis for the invariant subspace corresponding to these Ritz values is */
/*  referred to as a Lanczos basis. */

/*  See documentation in the header of the subroutine SSAUPD for a definition */
/*  of OP as well as other terms and the relation of computed Ritz values */
/*  and vectors of OP with respect to the given problem  A*z = lambda*B*z. */

/*  The approximate eigenvalues of the original problem are returned in */
/*  ascending algebraic order.  The user may elect to call this routine */
/*  once for each desired Ritz vector and store it peripherally if desired. */
/*  There is also the option of computing a selected set of these vectors */
/*  with a single call. */

/* \Usage: */
/*  call sseupd */
/*     ( RVEC, HOWMNY, SELECT, D, Z, LDZ, SIGMA, BMAT, N, WHICH, NEV, TOL, */
/*       RESID, NCV, V, LDV, IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO ) */

/*  RVEC    LOGICAL  (INPUT) */
/*          Specifies whether Ritz vectors corresponding to the Ritz value */
/*          approximations to the eigenproblem A*z = lambda*B*z are computed. */

/*             RVEC = .FALSE.     Compute Ritz values only. */

/*             RVEC = .TRUE.      Compute Ritz vectors. */

/*  HOWMNY  Character*1  (INPUT) */
/*          Specifies how many Ritz vectors are wanted and the form of Z */
/*          the matrix of Ritz vectors. See remark 1 below. */
/*          = 'A': compute NEV Ritz vectors; */
/*          = 'S': compute some of the Ritz vectors, specified */
/*                 by the logical array SELECT. */

/*  SELECT  Logical array of dimension NCV.  (INPUT/WORKSPACE) */
/*          If HOWMNY = 'S', SELECT specifies the Ritz vectors to be */
/*          computed. To select the Ritz vector corresponding to a */
/*          Ritz value D(j), SELECT(j) must be set to .TRUE.. */
/*          If HOWMNY = 'A' , SELECT is used as a workspace for */
/*          reordering the Ritz values. */

/*  D       Real  array of dimension NEV.  (OUTPUT) */
/*          On exit, D contains the Ritz value approximations to the */
/*          eigenvalues of A*z = lambda*B*z. The values are returned */
/*          in ascending order. If IPARAM(7) = 3,4,5 then D represents */
/*          the Ritz values of OP computed by ssaupd transformed to */
/*          those of the original eigensystem A*z = lambda*B*z. If */
/*          IPARAM(7) = 1,2 then the Ritz values of OP are the same */
/*          as the those of A*z = lambda*B*z. */

/*  Z       Real  N by NEV array if HOWMNY = 'A'.  (OUTPUT) */
/*          On exit, Z contains the B-orthonormal Ritz vectors of the */
/*          eigensystem A*z = lambda*B*z corresponding to the Ritz */
/*          value approximations. */
/*          If  RVEC = .FALSE. then Z is not referenced. */
/*          NOTE: The array Z may be set equal to first NEV columns of the */
/*          Arnoldi/Lanczos basis array V computed by SSAUPD. */

/*  LDZ     Integer.  (INPUT) */
/*          The leading dimension of the array Z.  If Ritz vectors are */
/*          desired, then  LDZ .ge.  max( 1, N ).  In any case,  LDZ .ge. 1. */

/*  SIGMA   Real   (INPUT) */
/*          If IPARAM(7) = 3,4,5 represents the shift. Not referenced if */
/*          IPARAM(7) = 1 or 2. */


/*  **** The remaining arguments MUST be the same as for the   **** */
/*  **** call to SSAUPD that was just completed.               **** */

/*  NOTE: The remaining arguments */

/*           BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, LDV, IPARAM, IPNTR, */
/*           WORKD, WORKL, LWORKL, INFO */

/*         must be passed directly to SSEUPD following the last call */
/*         to SSAUPD.  These arguments MUST NOT BE MODIFIED between */
/*         the the last call to SSAUPD and the call to SSEUPD. */

/*  Two of these parameters (WORKL, INFO) are also output parameters: */

/*  WORKL   Real  work array of length LWORKL.  (OUTPUT/WORKSPACE) */
/*          WORKL(1:4*ncv) contains information obtained in */
/*          ssaupd.  They are not changed by sseupd. */
/*          WORKL(4*ncv+1:ncv*ncv+8*ncv) holds the */
/*          untransformed Ritz values, the computed error estimates, */
/*          and the associated eigenvector matrix of H. */

/*          Note: IPNTR(8:10) contains the pointer into WORKL for addresses */
/*          of the above information computed by sseupd. */
/*          ------------------------------------------------------------- */
/*          IPNTR(8): pointer to the NCV RITZ values of the original system. */
/*          IPNTR(9): pointer to the NCV corresponding error bounds. */
/*          IPNTR(10): pointer to the NCV by NCV matrix of eigenvectors */
/*                     of the tridiagonal matrix T. Only referenced by */
/*                     sseupd if RVEC = .TRUE. See Remarks. */
/*          ------------------------------------------------------------- */

/*  INFO    Integer.  (OUTPUT) */
/*          Error flag on output. */
/*          =  0: Normal exit. */
/*          = -1: N must be positive. */
/*          = -2: NEV must be positive. */
/*          = -3: NCV must be greater than NEV and less than or equal to N. */
/*          = -5: WHICH must be one of 'LM', 'SM', 'LA', 'SA' or 'BE'. */
/*          = -6: BMAT must be one of 'I' or 'G'. */
/*          = -7: Length of private work WORKL array is not sufficient. */
/*          = -8: Error return from trid. eigenvalue calculation; */
/*                Information error from LAPACK routine ssteqr. */
/*          = -9: Starting vector is zero. */
/*          = -10: IPARAM(7) must be 1,2,3,4,5. */
/*          = -11: IPARAM(7) = 1 and BMAT = 'G' are incompatible. */
/*          = -12: NEV and WHICH = 'BE' are incompatible. */
/*          = -14: SSAUPD did not find any eigenvalues to sufficient */
/*                 accuracy. */
/*          = -15: HOWMNY must be one of 'A' or 'S' if RVEC = .true. */
/*          = -16: HOWMNY = 'S' not yet implemented */
/*          = -17: SSEUPD got a different count of the number of converged */
/*                 Ritz values than SSAUPD got.  This indicates the user */
/*                 probably made an error in passing data from SSAUPD to */
/*                 SSEUPD or that the data was modified before entering */
/*                 SSEUPD. */

/* \BeginLib */

/* \References: */
/*  1. D.C. Sorensen, "Implicit Application of Polynomial Filters in */
/*     a k-Step Arnoldi Method", SIAM J. Matr. Anal. Apps., 13 (1992), */
/*     pp 357-385. */
/*  2. R.B. Lehoucq, "Analysis and Implementation of an Implicitly */
/*     Restarted Arnoldi Iteration", Rice University Technical Report */
/*     TR95-13, Department of Computational and Applied Mathematics. */
/*  3. B.N. Parlett, "The Symmetric Eigenvalue Problem". Prentice-Hall, */
/*     1980. */
/*  4. B.N. Parlett, B. Nour-Omid, "Towards a Black Box Lanczos Program", */
/*     Computer Physics Communications, 53 (1989), pp 169-179. */
/*  5. B. Nour-Omid, B.N. Parlett, T. Ericson, P.S. Jensen, "How to */
/*     Implement the Spectral Transformation", Math. Comp., 48 (1987), */
/*     pp 663-673. */
/*  6. R.G. Grimes, J.G. Lewis and H.D. Simon, "A Shifted Block Lanczos */
/*     Algorithm for Solving Sparse Symmetric Generalized Eigenproblems", */
/*     SIAM J. Matr. Anal. Apps.,  January (1993). */
/*  7. L. Reichel, W.B. Gragg, "Algorithm 686: FORTRAN Subroutines */
/*     for Updating the QR decomposition", ACM TOMS, December 1990, */
/*     Volume 16 Number 4, pp 369-377. */

/* \Remarks */
/*  1. The converged Ritz values are always returned in increasing */
/*     (algebraic) order. */

/*  2. Currently only HOWMNY = 'A' is implemented. It is included at this */
/*     stage for the user who wants to incorporate it. */

/* \Routines called: */
/*     ssesrt  ARPACK routine that sorts an array X, and applies the */
/*             corresponding permutation to a matrix A. */
/*     ssortr  ssortr  ARPACK sorting routine. */
/*     ivout   ARPACK utility routine that prints integers. */
/*     svout   ARPACK utility routine that prints vectors. */
/*     sgeqr2  LAPACK routine that computes the QR factorization of */
/*             a matrix. */
/*     slacpy  LAPACK matrix copy routine. */
/*     slamch  LAPACK routine that determines machine constants. */
/*     sorm2r  LAPACK routine that applies an orthogonal matrix in */
/*             factored form. */
/*     ssteqr  LAPACK routine that computes eigenvalues and eigenvectors */
/*             of a tridiagonal matrix. */
/*     sger    Level 2 BLAS rank one update to a matrix. */
/*     scopy   Level 1 BLAS that copies one vector to another . */
/*     snrm2   Level 1 BLAS that computes the norm of a vector. */
/*     sscal   Level 1 BLAS that scales a vector. */
/*     sswap   Level 1 BLAS that swaps the contents of two vectors. */
/* \Authors */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Chao Yang                    Houston, Texas */
/*     Dept. of Computational & */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     12/15/93: Version ' 2.1' */

/* \SCCS Information: @(#) */
/* FILE: seupd.F   SID: 2.11   DATE OF SID: 04/10/01   RELEASE: 2 */

/* \EndLib */

/* ----------------------------------------------------------------------- */
/* Subroutine */ int sseupd_(logical *rvec, char *howmny, logical *select, 
	real *d__, real *z__, integer *ldz, real *sigma, char *bmat, integer *
	n, char *which, integer *nev, real *tol, real *resid, integer *ncv, 
	real *v, integer *ldv, integer *iparam, integer *ipntr, real *workd, 
	real *workl, integer *lworkl, integer *info, ftnlen howmny_len, 
	ftnlen bmat_len, ftnlen which_len)
{
    /* System generated locals */
    integer v_dim1, v_offset, z_dim1, z_offset, i__1;
    real r__1, r__2, r__3;
    doublereal d__1;

    /* Builtin functions */
    integer s_cmp(char *, char *, ftnlen, ftnlen);
    /* Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen);
    double pow_dd(doublereal *, doublereal *);

    /* Local variables */
    static integer j, k, ih, jj, iq, np, iw, ibd, ihb, ihd, ldh, ldq, irz, 
	    mode;
    static real eps23;
    extern /* Subroutine */ int sger_(integer *, integer *, real *, real *, 
	    integer *, real *, integer *, real *, integer *);
    static integer ierr;
    static real temp;
    static integer next;
    static char type__[6];
    static integer ritz;
    static real temp1;
    extern doublereal snrm2_(integer *, real *, integer *);
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *);
    static logical reord;
    static integer nconv;
    static real rnorm;
    extern /* Subroutine */ int scopy_(integer *, real *, integer *, real *, 
	    integer *), ivout_(integer *, integer *, integer *, integer *, 
	    char *, ftnlen), svout_(integer *, integer *, real *, integer *, 
	    char *, ftnlen);
    static real bnorm2;
    extern /* Subroutine */ int sgeqr2_(integer *, integer *, real *, integer 
	    *, real *, real *, integer *), sorm2r_(char *, char *, integer *, 
	    integer *, integer *, real *, integer *, real *, real *, integer *
	    , real *, integer *, ftnlen, ftnlen);
    extern doublereal slamch_(char *, ftnlen);
    static integer bounds, msglvl, ishift, numcnv;
    extern /* Subroutine */ int slacpy_(char *, integer *, integer *, real *, 
	    integer *, real *, integer *, ftnlen), ssesrt_(char *, logical *, 
	    integer *, real *, integer *, real *, integer *, ftnlen), ssteqr_(
	    char *, integer *, real *, real *, real *, integer *, real *, 
	    integer *, ftnlen), ssortr_(char *, logical *, integer *, real *, 
	    real *, ftnlen), ssgets_(integer *, char *, integer *, integer *, 
	    real *, real *, real *, ftnlen);
    static integer leftptr, rghtptr;


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


/*     %---------------------% */
/*     | Intrinsic Functions | */
/*     %---------------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

/*     %------------------------% */
/*     | Set default parameters | */
/*     %------------------------% */

    /* Parameter adjustments */
    --workd;
    --resid;
    z_dim1 = *ldz;
    z_offset = 1 + z_dim1;
    z__ -= z_offset;
    --d__;
    --select;
    v_dim1 = *ldv;
    v_offset = 1 + v_dim1;
    v -= v_offset;
    --iparam;
    --ipntr;
    --workl;

    /* Function Body */
    msglvl = debug_1.mseupd;
    mode = iparam[7];
    nconv = iparam[5];
    *info = 0;

/*     %--------------% */
/*     | Quick return | */
/*     %--------------% */

    if (nconv == 0) {
	goto L9000;
    }
    ierr = 0;

    if (nconv <= 0) {
	ierr = -14;
    }
    if (*n <= 0) {
	ierr = -1;
    }
    if (*nev <= 0) {
	ierr = -2;
    }
    if (*ncv <= *nev || *ncv > *n) {
	ierr = -3;
    }
    if (s_cmp(which, "LM", (ftnlen)2, (ftnlen)2) != 0 && s_cmp(which, "SM", (
	    ftnlen)2, (ftnlen)2) != 0 && s_cmp(which, "LA", (ftnlen)2, (
	    ftnlen)2) != 0 && s_cmp(which, "SA", (ftnlen)2, (ftnlen)2) != 0 &&
	     s_cmp(which, "BE", (ftnlen)2, (ftnlen)2) != 0) {
	ierr = -5;
    }
    if (*(unsigned char *)bmat != 'I' && *(unsigned char *)bmat != 'G') {
	ierr = -6;
    }
    if (*(unsigned char *)howmny != 'A' && *(unsigned char *)howmny != 'P' && 
	    *(unsigned char *)howmny != 'S' && *rvec) {
	ierr = -15;
    }
    if (*rvec && *(unsigned char *)howmny == 'S') {
	ierr = -16;
    }

/* Computing 2nd power */
    i__1 = *ncv;
    if (*rvec && *lworkl < i__1 * i__1 + (*ncv << 3)) {
	ierr = -7;
    }

    if (mode == 1 || mode == 2) {
	s_copy(type__, "REGULR", (ftnlen)6, (ftnlen)6);
    } else if (mode == 3) {
	s_copy(type__, "SHIFTI", (ftnlen)6, (ftnlen)6);
    } else if (mode == 4) {
	s_copy(type__, "BUCKLE", (ftnlen)6, (ftnlen)6);
    } else if (mode == 5) {
	s_copy(type__, "CAYLEY", (ftnlen)6, (ftnlen)6);
    } else {
	ierr = -10;
    }
    if (mode == 1 && *(unsigned char *)bmat == 'G') {
	ierr = -11;
    }
    if (*nev == 1 && s_cmp(which, "BE", (ftnlen)2, (ftnlen)2) == 0) {
	ierr = -12;
    }

/*     %------------% */
/*     | Error Exit | */
/*     %------------% */

    if (ierr != 0) {
	*info = ierr;
	goto L9000;
    }

/*     %-------------------------------------------------------% */
/*     | Pointer into WORKL for address of H, RITZ, BOUNDS, Q  | */
/*     | etc... and the remaining workspace.                   | */
/*     | Also update pointer to be used on output.             | */
/*     | Memory is laid out as follows:                        | */
/*     | workl(1:2*ncv) := generated tridiagonal matrix H      | */
/*     |       The subdiagonal is stored in workl(2:ncv).      | */
/*     |       The dead spot is workl(1) but upon exiting      | */
/*     |       ssaupd stores the B-norm of the last residual   | */
/*     |       vector in workl(1). We use this !!!             | */
/*     | workl(2*ncv+1:2*ncv+ncv) := ritz values               | */
/*     |       The wanted values are in the first NCONV spots. | */
/*     | workl(3*ncv+1:3*ncv+ncv) := computed Ritz estimates   | */
/*     |       The wanted values are in the first NCONV spots. | */
/*     | NOTE: workl(1:4*ncv) is set by ssaupd and is not      | */
/*     |       modified by sseupd.                             | */
/*     %-------------------------------------------------------% */

/*     %-------------------------------------------------------% */
/*     | The following is used and set by sseupd.              | */
/*     | workl(4*ncv+1:4*ncv+ncv) := used as workspace during  | */
/*     |       computation of the eigenvectors of H. Stores    | */
/*     |       the diagonal of H. Upon EXIT contains the NCV   | */
/*     |       Ritz values of the original system. The first   | */
/*     |       NCONV spots have the wanted values. If MODE =   | */
/*     |       1 or 2 then will equal workl(2*ncv+1:3*ncv).    | */
/*     | workl(5*ncv+1:5*ncv+ncv) := used as workspace during  | */
/*     |       computation of the eigenvectors of H. Stores    | */
/*     |       the subdiagonal of H. Upon EXIT contains the    | */
/*     |       NCV corresponding Ritz estimates of the         | */
/*     |       original system. The first NCONV spots have the | */
/*     |       wanted values. If MODE = 1,2 then will equal    | */
/*     |       workl(3*ncv+1:4*ncv).                           | */
/*     | workl(6*ncv+1:6*ncv+ncv*ncv) := orthogonal Q that is  | */
/*     |       the eigenvector matrix for H as returned by     | */
/*     |       ssteqr. Not referenced if RVEC = .False.        | */
/*     |       Ordering follows that of workl(4*ncv+1:5*ncv)   | */
/*     | workl(6*ncv+ncv*ncv+1:6*ncv+ncv*ncv+2*ncv) :=         | */
/*     |       Workspace. Needed by ssteqr and by sseupd.      | */
/*     | GRAND total of NCV*(NCV+8) locations.                 | */
/*     %-------------------------------------------------------% */


    ih = ipntr[5];
    ritz = ipntr[6];
    bounds = ipntr[7];
    ldh = *ncv;
    ldq = *ncv;
    ihd = bounds + ldh;
    ihb = ihd + ldh;
    iq = ihb + ldh;
    iw = iq + ldh * *ncv;
    next = iw + (*ncv << 1);
    ipntr[4] = next;
    ipntr[8] = ihd;
    ipntr[9] = ihb;
    ipntr[10] = iq;

/*     %----------------------------------------% */
/*     | irz points to the Ritz values computed | */
/*     |     by _seigt before exiting _saup2.   | */
/*     | ibd points to the Ritz estimates       | */
/*     |     computed by _seigt before exiting  | */
/*     |     _saup2.                            | */
/*     %----------------------------------------% */

    irz = ipntr[11] + *ncv;
    ibd = irz + *ncv;


/*     %---------------------------------% */
/*     | Set machine dependent constant. | */
/*     %---------------------------------% */

    eps23 = slamch_("Epsilon-Machine", (ftnlen)15);
    d__1 = (doublereal) eps23;
    eps23 = pow_dd(&d__1, &c_b21);

/*     %---------------------------------------% */
/*     | RNORM is B-norm of the RESID(1:N).    | */
/*     | BNORM2 is the 2 norm of B*RESID(1:N). | */
/*     | Upon exit of ssaupd WORKD(1:N) has    | */
/*     | B*RESID(1:N).                         | */
/*     %---------------------------------------% */

    rnorm = workl[ih];
    if (*(unsigned char *)bmat == 'I') {
	bnorm2 = rnorm;
    } else if (*(unsigned char *)bmat == 'G') {
	bnorm2 = snrm2_(n, &workd[1], &c__1);
    }

    if (msglvl > 2) {
	svout_(&debug_1.logfil, ncv, &workl[irz], &debug_1.ndigit, "_seupd: "
		"Ritz values passed in from _SAUPD.", (ftnlen)42);
	svout_(&debug_1.logfil, ncv, &workl[ibd], &debug_1.ndigit, "_seupd: "
		"Ritz estimates passed in from _SAUPD.", (ftnlen)45);
    }

    if (*rvec) {

	reord = FALSE_;

/*        %---------------------------------------------------% */
/*        | Use the temporary bounds array to store indices   | */
/*        | These will be used to mark the select array later | */
/*        %---------------------------------------------------% */

	i__1 = *ncv;
	for (j = 1; j <= i__1; ++j) {
	    workl[bounds + j - 1] = (real) j;
	    select[j] = FALSE_;
/* L10: */
	}

/*        %-------------------------------------% */
/*        | Select the wanted Ritz values.      | */
/*        | Sort the Ritz values so that the    | */
/*        | wanted ones appear at the tailing   | */
/*        | NEV positions of workl(irr) and     | */
/*        | workl(iri).  Move the corresponding | */
/*        | error estimates in workl(bound)     | */
/*        | accordingly.                        | */
/*        %-------------------------------------% */

	np = *ncv - *nev;
	ishift = 0;
	ssgets_(&ishift, which, nev, &np, &workl[irz], &workl[bounds], &workl[
		1], (ftnlen)2);

	if (msglvl > 2) {
	    svout_(&debug_1.logfil, ncv, &workl[irz], &debug_1.ndigit, "_seu"
		    "pd: Ritz values after calling _SGETS.", (ftnlen)41);
	    svout_(&debug_1.logfil, ncv, &workl[bounds], &debug_1.ndigit, 
		    "_seupd: Ritz value indices after calling _SGETS.", (
		    ftnlen)48);
	}

/*        %-----------------------------------------------------% */
/*        | Record indices of the converged wanted Ritz values  | */
/*        | Mark the select array for possible reordering       | */
/*        %-----------------------------------------------------% */

	numcnv = 0;
	i__1 = *ncv;
	for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
	    r__2 = eps23, r__3 = (r__1 = workl[irz + *ncv - j], dabs(r__1));
	    temp1 = dmax(r__2,r__3);
	    jj = workl[bounds + *ncv - j];
	    if (numcnv < nconv && workl[ibd + jj - 1] <= *tol * temp1) {
		select[jj] = TRUE_;
		++numcnv;
		if (jj > *nev) {
		    reord = TRUE_;
		}
	    }
/* L11: */
	}

/*        %-----------------------------------------------------------% */
/*        | Check the count (numcnv) of converged Ritz values with    | */
/*        | the number (nconv) reported by _saupd.  If these two      | */
/*        | are different then there has probably been an error       | */
/*        | caused by incorrect passing of the _saupd data.           | */
/*        %-----------------------------------------------------------% */

	if (msglvl > 2) {
	    ivout_(&debug_1.logfil, &c__1, &numcnv, &debug_1.ndigit, "_seupd"
		    ": Number of specified eigenvalues", (ftnlen)39);
	    ivout_(&debug_1.logfil, &c__1, &nconv, &debug_1.ndigit, "_seupd:"
		    " Number of \"converged\" eigenvalues", (ftnlen)41);
	}

	if (numcnv != nconv) {
	    *info = -17;
	    goto L9000;
	}

/*        %-----------------------------------------------------------% */
/*        | Call LAPACK routine _steqr to compute the eigenvalues and | */
/*        | eigenvectors of the final symmetric tridiagonal matrix H. | */
/*        | Initialize the eigenvector matrix Q to the identity.      | */
/*        %-----------------------------------------------------------% */

	i__1 = *ncv - 1;
	scopy_(&i__1, &workl[ih + 1], &c__1, &workl[ihb], &c__1);
	scopy_(ncv, &workl[ih + ldh], &c__1, &workl[ihd], &c__1);

	ssteqr_("Identity", ncv, &workl[ihd], &workl[ihb], &workl[iq], &ldq, &
		workl[iw], &ierr, (ftnlen)8);

	if (ierr != 0) {
	    *info = -8;
	    goto L9000;
	}

	if (msglvl > 1) {
	    scopy_(ncv, &workl[iq + *ncv - 1], &ldq, &workl[iw], &c__1);
	    svout_(&debug_1.logfil, ncv, &workl[ihd], &debug_1.ndigit, "_seu"
		    "pd: NCV Ritz values of the final H matrix", (ftnlen)45);
	    svout_(&debug_1.logfil, ncv, &workl[iw], &debug_1.ndigit, "_seup"
		    "d: last row of the eigenvector matrix for H", (ftnlen)48);
	}

	if (reord) {

/*           %---------------------------------------------% */
/*           | Reordered the eigenvalues and eigenvectors  | */
/*           | computed by _steqr so that the "converged"  | */
/*           | eigenvalues appear in the first NCONV       | */
/*           | positions of workl(ihd), and the associated | */
/*           | eigenvectors appear in the first NCONV      | */
/*           | columns.                                    | */
/*           %---------------------------------------------% */

	    leftptr = 1;
	    rghtptr = *ncv;

	    if (*ncv == 1) {
		goto L30;
	    }

L20:
	    if (select[leftptr]) {

/*              %-------------------------------------------% */
/*              | Search, from the left, for the first Ritz | */
/*              | value that has not converged.             | */
/*              %-------------------------------------------% */

		++leftptr;

	    } else if (! select[rghtptr]) {

/*              %----------------------------------------------% */
/*              | Search, from the right, the first Ritz value | */
/*              | that has converged.                          | */
/*              %----------------------------------------------% */

		--rghtptr;

	    } else {

/*              %----------------------------------------------% */
/*              | Swap the Ritz value on the left that has not | */
/*              | converged with the Ritz value on the right   | */
/*              | that has converged.  Swap the associated     | */
/*              | eigenvector of the tridiagonal matrix H as   | */
/*              | well.                                        | */
/*              %----------------------------------------------% */

		temp = workl[ihd + leftptr - 1];
		workl[ihd + leftptr - 1] = workl[ihd + rghtptr - 1];
		workl[ihd + rghtptr - 1] = temp;
		scopy_(ncv, &workl[iq + *ncv * (leftptr - 1)], &c__1, &workl[
			iw], &c__1);
		scopy_(ncv, &workl[iq + *ncv * (rghtptr - 1)], &c__1, &workl[
			iq + *ncv * (leftptr - 1)], &c__1);
		scopy_(ncv, &workl[iw], &c__1, &workl[iq + *ncv * (rghtptr - 
			1)], &c__1);
		++leftptr;
		--rghtptr;

	    }

	    if (leftptr < rghtptr) {
		goto L20;
	    }

L30:
	    ;
	}

	if (msglvl > 2) {
	    svout_(&debug_1.logfil, ncv, &workl[ihd], &debug_1.ndigit, "_seu"
		    "pd: The eigenvalues of H--reordered", (ftnlen)39);
	}

/*        %----------------------------------------% */
/*        | Load the converged Ritz values into D. | */
/*        %----------------------------------------% */

	scopy_(&nconv, &workl[ihd], &c__1, &d__[1], &c__1);

    } else {

/*        %-----------------------------------------------------% */
/*        | Ritz vectors not required. Load Ritz values into D. | */
/*        %-----------------------------------------------------% */

	scopy_(&nconv, &workl[ritz], &c__1, &d__[1], &c__1);
	scopy_(ncv, &workl[ritz], &c__1, &workl[ihd], &c__1);

    }

/*     %------------------------------------------------------------------% */
/*     | Transform the Ritz values and possibly vectors and corresponding | */
/*     | Ritz estimates of OP to those of A*x=lambda*B*x. The Ritz values | */
/*     | (and corresponding data) are returned in ascending order.        | */
/*     %------------------------------------------------------------------% */

    if (s_cmp(type__, "REGULR", (ftnlen)6, (ftnlen)6) == 0) {

/*        %---------------------------------------------------------% */
/*        | Ascending sort of wanted Ritz values, vectors and error | */
/*        | bounds. Not necessary if only Ritz values are desired.  | */
/*        %---------------------------------------------------------% */

	if (*rvec) {
	    ssesrt_("LA", rvec, &nconv, &d__[1], ncv, &workl[iq], &ldq, (
		    ftnlen)2);
	} else {
	    scopy_(ncv, &workl[bounds], &c__1, &workl[ihb], &c__1);
	}

    } else {

/*        %-------------------------------------------------------------% */
/*        | *  Make a copy of all the Ritz values.                      | */
/*        | *  Transform the Ritz values back to the original system.   | */
/*        |    For TYPE = 'SHIFTI' the transformation is                | */
/*        |             lambda = 1/theta + sigma                        | */
/*        |    For TYPE = 'BUCKLE' the transformation is                | */
/*        |             lambda = sigma * theta / ( theta - 1 )          | */
/*        |    For TYPE = 'CAYLEY' the transformation is                | */
/*        |             lambda = sigma * (theta + 1) / (theta - 1 )     | */
/*        |    where the theta are the Ritz values returned by ssaupd.  | */
/*        | NOTES:                                                      | */
/*        | *The Ritz vectors are not affected by the transformation.   | */
/*        |  They are only reordered.                                   | */
/*        %-------------------------------------------------------------% */

	scopy_(ncv, &workl[ihd], &c__1, &workl[iw], &c__1);
	if (s_cmp(type__, "SHIFTI", (ftnlen)6, (ftnlen)6) == 0) {
	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
		workl[ihd + k - 1] = 1.f / workl[ihd + k - 1] + *sigma;
/* L40: */
	    }
	} else if (s_cmp(type__, "BUCKLE", (ftnlen)6, (ftnlen)6) == 0) {
	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
		workl[ihd + k - 1] = *sigma * workl[ihd + k - 1] / (workl[ihd 
			+ k - 1] - 1.f);
/* L50: */
	    }
	} else if (s_cmp(type__, "CAYLEY", (ftnlen)6, (ftnlen)6) == 0) {
	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
		workl[ihd + k - 1] = *sigma * (workl[ihd + k - 1] + 1.f) / (
			workl[ihd + k - 1] - 1.f);
/* L60: */
	    }
	}

/*        %-------------------------------------------------------------% */
/*        | *  Store the wanted NCONV lambda values into D.             | */
/*        | *  Sort the NCONV wanted lambda in WORKL(IHD:IHD+NCONV-1)   | */
/*        |    into ascending order and apply sort to the NCONV theta   | */
/*        |    values in the transformed system. We will need this to   | */
/*        |    compute Ritz estimates in the original system.           | */
/*        | *  Finally sort the lambda`s into ascending order and apply | */
/*        |    to Ritz vectors if wanted. Else just sort lambda`s into  | */
/*        |    ascending order.                                         | */
/*        | NOTES:                                                      | */
/*        | *workl(iw:iw+ncv-1) contain the theta ordered so that they  | */
/*        |  match the ordering of the lambda. We`ll use them again for | */
/*        |  Ritz vector purification.                                  | */
/*        %-------------------------------------------------------------% */

	scopy_(&nconv, &workl[ihd], &c__1, &d__[1], &c__1);
	ssortr_("LA", &c_true, &nconv, &workl[ihd], &workl[iw], (ftnlen)2);
	if (*rvec) {
	    ssesrt_("LA", rvec, &nconv, &d__[1], ncv, &workl[iq], &ldq, (
		    ftnlen)2);
	} else {
	    scopy_(ncv, &workl[bounds], &c__1, &workl[ihb], &c__1);
	    r__1 = bnorm2 / rnorm;
	    sscal_(ncv, &r__1, &workl[ihb], &c__1);
	    ssortr_("LA", &c_true, &nconv, &d__[1], &workl[ihb], (ftnlen)2);
	}

    }

/*     %------------------------------------------------% */
/*     | Compute the Ritz vectors. Transform the wanted | */
/*     | eigenvectors of the symmetric tridiagonal H by | */
/*     | the Lanczos basis matrix V.                    | */
/*     %------------------------------------------------% */

    if (*rvec && *(unsigned char *)howmny == 'A') {

/*        %----------------------------------------------------------% */
/*        | Compute the QR factorization of the matrix representing  | */
/*        | the wanted invariant subspace located in the first NCONV | */
/*        | columns of workl(iq,ldq).                                | */
/*        %----------------------------------------------------------% */

	sgeqr2_(ncv, &nconv, &workl[iq], &ldq, &workl[iw + *ncv], &workl[ihb],
		 &ierr);

/*        %--------------------------------------------------------% */
/*        | * Postmultiply V by Q.                                 | */
/*        | * Copy the first NCONV columns of VQ into Z.           | */
/*        | The N by NCONV matrix Z is now a matrix representation | */
/*        | of the approximate invariant subspace associated with  | */
/*        | the Ritz values in workl(ihd).                         | */
/*        %--------------------------------------------------------% */

	sorm2r_("Right", "Notranspose", n, ncv, &nconv, &workl[iq], &ldq, &
		workl[iw + *ncv], &v[v_offset], ldv, &workd[*n + 1], &ierr, (
		ftnlen)5, (ftnlen)11);
	slacpy_("All", n, &nconv, &v[v_offset], ldv, &z__[z_offset], ldz, (
		ftnlen)3);

/*        %-----------------------------------------------------% */
/*        | In order to compute the Ritz estimates for the Ritz | */
/*        | values in both systems, need the last row of the    | */
/*        | eigenvector matrix. Remember, it`s in factored form | */
/*        %-----------------------------------------------------% */

	i__1 = *ncv - 1;
	for (j = 1; j <= i__1; ++j) {
	    workl[ihb + j - 1] = 0.f;
/* L65: */
	}
	workl[ihb + *ncv - 1] = 1.f;
	sorm2r_("Left", "Transpose", ncv, &c__1, &nconv, &workl[iq], &ldq, &
		workl[iw + *ncv], &workl[ihb], ncv, &temp, &ierr, (ftnlen)4, (
		ftnlen)9);

    } else if (*rvec && *(unsigned char *)howmny == 'S') {

/*     Not yet implemented. See remark 2 above. */

    }

    if (s_cmp(type__, "REGULR", (ftnlen)6, (ftnlen)6) == 0 && *rvec) {

	i__1 = *ncv;
	for (j = 1; j <= i__1; ++j) {
	    workl[ihb + j - 1] = rnorm * (r__1 = workl[ihb + j - 1], dabs(
		    r__1));
/* L70: */
	}

    } else if (s_cmp(type__, "REGULR", (ftnlen)6, (ftnlen)6) != 0 && *rvec) {

/*        %-------------------------------------------------% */
/*        | *  Determine Ritz estimates of the theta.       | */
/*        |    If RVEC = .true. then compute Ritz estimates | */
/*        |               of the theta.                     | */
/*        |    If RVEC = .false. then copy Ritz estimates   | */
/*        |              as computed by ssaupd.             | */
/*        | *  Determine Ritz estimates of the lambda.      | */
/*        %-------------------------------------------------% */

	sscal_(ncv, &bnorm2, &workl[ihb], &c__1);
	if (s_cmp(type__, "SHIFTI", (ftnlen)6, (ftnlen)6) == 0) {

	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
/* Computing 2nd power */
		r__2 = workl[iw + k - 1];
		workl[ihb + k - 1] = (r__1 = workl[ihb + k - 1], dabs(r__1)) /
			 (r__2 * r__2);
/* L80: */
	    }

	} else if (s_cmp(type__, "BUCKLE", (ftnlen)6, (ftnlen)6) == 0) {

	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
/* Computing 2nd power */
		r__2 = workl[iw + k - 1] - 1.f;
		workl[ihb + k - 1] = *sigma * (r__1 = workl[ihb + k - 1], 
			dabs(r__1)) / (r__2 * r__2);
/* L90: */
	    }

	} else if (s_cmp(type__, "CAYLEY", (ftnlen)6, (ftnlen)6) == 0) {

	    i__1 = *ncv;
	    for (k = 1; k <= i__1; ++k) {
		workl[ihb + k - 1] = (r__1 = workl[ihb + k - 1] / workl[iw + 
			k - 1] * (workl[iw + k - 1] - 1.f), dabs(r__1));
/* L100: */
	    }

	}

    }

    if (s_cmp(type__, "REGULR", (ftnlen)6, (ftnlen)6) != 0 && msglvl > 1) {
	svout_(&debug_1.logfil, &nconv, &d__[1], &debug_1.ndigit, "_seupd: U"
		"ntransformed converged Ritz values", (ftnlen)43);
	svout_(&debug_1.logfil, &nconv, &workl[ihb], &debug_1.ndigit, "_seup"
		"d: Ritz estimates of the untransformed Ritz values", (ftnlen)
		55);
    } else if (msglvl > 1) {
	svout_(&debug_1.logfil, &nconv, &d__[1], &debug_1.ndigit, "_seupd: C"
		"onverged Ritz values", (ftnlen)29);
	svout_(&debug_1.logfil, &nconv, &workl[ihb], &debug_1.ndigit, "_seup"
		"d: Associated Ritz estimates", (ftnlen)33);
    }

/*     %-------------------------------------------------% */
/*     | Ritz vector purification step. Formally perform | */
/*     | one of inverse subspace iteration. Only used    | */
/*     | for MODE = 3,4,5. See reference 7               | */
/*     %-------------------------------------------------% */

    if (*rvec && (s_cmp(type__, "SHIFTI", (ftnlen)6, (ftnlen)6) == 0 || s_cmp(
	    type__, "CAYLEY", (ftnlen)6, (ftnlen)6) == 0)) {

	i__1 = nconv - 1;
	for (k = 0; k <= i__1; ++k) {
	    workl[iw + k] = workl[iq + k * ldq + *ncv - 1] / workl[iw + k];
/* L110: */
	}

    } else if (*rvec && s_cmp(type__, "BUCKLE", (ftnlen)6, (ftnlen)6) == 0) {

	i__1 = nconv - 1;
	for (k = 0; k <= i__1; ++k) {
	    workl[iw + k] = workl[iq + k * ldq + *ncv - 1] / (workl[iw + k] - 
		    1.f);
/* L120: */
	}

    }

    if (s_cmp(type__, "REGULR", (ftnlen)6, (ftnlen)6) != 0) {
	sger_(n, &nconv, &c_b110, &resid[1], &c__1, &workl[iw], &c__1, &z__[
		z_offset], ldz);
    }

L9000:

    return 0;

/*     %---------------% */
/*     | End of sseupd| */
/*     %---------------% */

} /* sseupd_ */

