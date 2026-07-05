/* ../FORTRAN/ARPACK/SRC/dngets.f -- translated by f2c (version 20100827).
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

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: dngets */

/* \Description: */
/*  Given the eigenvalues of the upper Hessenberg matrix H, */
/*  computes the NP shifts AMU that are zeros of the polynomial of */
/*  degree NP which filters out components of the unwanted eigenvectors */
/*  corresponding to the AMU's based on some given criteria. */

/*  NOTE: call this even in the case of user specified shifts in order */
/*  to sort the eigenvalues, and error bounds of H for later use. */

/* \Usage: */
/*  call dngets */
/*     ( ISHIFT, WHICH, KEV, NP, RITZR, RITZI, BOUNDS, SHIFTR, SHIFTI ) */

/* \Arguments */
/*  ISHIFT  Integer.  (INPUT) */
/*          Method for selecting the implicit shifts at each iteration. */
/*          ISHIFT = 0: user specified shifts */
/*          ISHIFT = 1: exact shift with respect to the matrix H. */

/*  WHICH   Character*2.  (INPUT) */
/*          Shift selection criteria. */
/*          'LM' -> want the KEV eigenvalues of largest magnitude. */
/*          'SM' -> want the KEV eigenvalues of smallest magnitude. */
/*          'LR' -> want the KEV eigenvalues of largest real part. */
/*          'SR' -> want the KEV eigenvalues of smallest real part. */
/*          'LI' -> want the KEV eigenvalues of largest imaginary part. */
/*          'SI' -> want the KEV eigenvalues of smallest imaginary part. */

/*  KEV      Integer.  (INPUT/OUTPUT) */
/*           INPUT: KEV+NP is the size of the matrix H. */
/*           OUTPUT: Possibly increases KEV by one to keep complex conjugate */
/*           pairs together. */

/*  NP       Integer.  (INPUT/OUTPUT) */
/*           Number of implicit shifts to be computed. */
/*           OUTPUT: Possibly decreases NP by one to keep complex conjugate */
/*           pairs together. */

/*  RITZR,  Double precision array of length KEV+NP.  (INPUT/OUTPUT) */
/*  RITZI   On INPUT, RITZR and RITZI contain the real and imaginary */
/*          parts of the eigenvalues of H. */
/*          On OUTPUT, RITZR and RITZI are sorted so that the unwanted */
/*          eigenvalues are in the first NP locations and the wanted */
/*          portion is in the last KEV locations.  When exact shifts are */
/*          selected, the unwanted part corresponds to the shifts to */
/*          be applied. Also, if ISHIFT .eq. 1, the unwanted eigenvalues */
/*          are further sorted so that the ones with largest Ritz values */
/*          are first. */

/*  BOUNDS  Double precision array of length KEV+NP.  (INPUT/OUTPUT) */
/*          Error bounds corresponding to the ordering in RITZ. */

/*  SHIFTR, SHIFTI  *** USE deprecated as of version 2.1. *** */


/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  real */

/* \Routines called: */
/*     dsortc  ARPACK sorting routine. */
/*     dcopy   Level 1 BLAS that copies one vector to another . */

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
/* FILE: ngets.F   SID: 2.3   DATE OF SID: 4/20/96   RELEASE: 2 */

/* \Remarks */
/*     1. xxxx */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int dngets_(integer *ishift, char *which, integer *kev, 
	integer *np, doublereal *ritzr, doublereal *ritzi, doublereal *bounds,
	 doublereal *shiftr, doublereal *shifti, ftnlen which_len)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    integer s_cmp(char *, char *, ftnlen, ftnlen);

    /* Local variables */
    static real t0, t1;
    extern /* Subroutine */ int dvout_(integer *, integer *, doublereal *, 
	    integer *, char *, ftnlen), ivout_(integer *, integer *, integer *
	    , integer *, char *, ftnlen), second_(real *);
    static integer msglvl;
    extern /* Subroutine */ int dsortc_(char *, logical *, integer *, 
	    doublereal *, doublereal *, doublereal *, ftnlen);


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


/*     %----------------------% */
/*     | Intrinsics Functions | */
/*     %----------------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

/*     %-------------------------------% */
/*     | Initialize timing statistics  | */
/*     | & message level for debugging | */
/*     %-------------------------------% */

    /* Parameter adjustments */
    --bounds;
    --ritzi;
    --ritzr;
    --shiftr;
    --shifti;

    /* Function Body */
    second_(&t0);
    msglvl = debug_1.mngets;

/*     %----------------------------------------------------% */
/*     | LM, SM, LR, SR, LI, SI case.                       | */
/*     | Sort the eigenvalues of H into the desired order   | */
/*     | and apply the resulting order to BOUNDS.           | */
/*     | The eigenvalues are sorted so that the wanted part | */
/*     | are always in the last KEV locations.              | */
/*     | We first do a pre-processing sort in order to keep | */
/*     | complex conjugate pairs together                   | */
/*     %----------------------------------------------------% */

    if (s_cmp(which, "LM", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("LR", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    } else if (s_cmp(which, "SM", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("SR", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    } else if (s_cmp(which, "LR", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("LM", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    } else if (s_cmp(which, "SR", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("SM", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    } else if (s_cmp(which, "LI", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("LM", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    } else if (s_cmp(which, "SI", (ftnlen)2, (ftnlen)2) == 0) {
	i__1 = *kev + *np;
	dsortc_("SM", &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (
		ftnlen)2);
    }

    i__1 = *kev + *np;
    dsortc_(which, &c_true, &i__1, &ritzr[1], &ritzi[1], &bounds[1], (ftnlen)
	    2);

/*     %-------------------------------------------------------% */
/*     | Increase KEV by one if the ( ritzr(np),ritzi(np) )    | */
/*     | = ( ritzr(np+1),-ritzi(np+1) ) and ritz(np) .ne. zero | */
/*     | Accordingly decrease NP by one. In other words keep   | */
/*     | complex conjugate pairs together.                     | */
/*     %-------------------------------------------------------% */

    if (ritzr[*np + 1] - ritzr[*np] == 0. && ritzi[*np + 1] + ritzi[*np] == 
	    0.) {
	--(*np);
	++(*kev);
    }

    if (*ishift == 1) {

/*        %-------------------------------------------------------% */
/*        | Sort the unwanted Ritz values used as shifts so that  | */
/*        | the ones with largest Ritz estimates are first        | */
/*        | This will tend to minimize the effects of the         | */
/*        | forward instability of the iteration when they shifts | */
/*        | are applied in subroutine dnapps.                     | */
/*        | Be careful and use 'SR' since we want to sort BOUNDS! | */
/*        %-------------------------------------------------------% */

	dsortc_("SR", &c_true, np, &bounds[1], &ritzr[1], &ritzi[1], (ftnlen)
		2);
    }

    second_(&t1);
    timing_1.tngets += t1 - t0;

    if (msglvl > 0) {
	ivout_(&debug_1.logfil, &c__1, kev, &debug_1.ndigit, "_ngets: KEV is",
		 (ftnlen)14);
	ivout_(&debug_1.logfil, &c__1, np, &debug_1.ndigit, "_ngets: NP is", (
		ftnlen)13);
	i__1 = *kev + *np;
	dvout_(&debug_1.logfil, &i__1, &ritzr[1], &debug_1.ndigit, "_ngets: "
		"Eigenvalues of current H matrix -- real part", (ftnlen)52);
	i__1 = *kev + *np;
	dvout_(&debug_1.logfil, &i__1, &ritzi[1], &debug_1.ndigit, "_ngets: "
		"Eigenvalues of current H matrix -- imag part", (ftnlen)52);
	i__1 = *kev + *np;
	dvout_(&debug_1.logfil, &i__1, &bounds[1], &debug_1.ndigit, "_ngets:"
		" Ritz estimates of the current KEV+NP Ritz values", (ftnlen)
		56);
    }

    return 0;

/*     %---------------% */
/*     | End of dngets | */
/*     %---------------% */

} /* dngets_ */

