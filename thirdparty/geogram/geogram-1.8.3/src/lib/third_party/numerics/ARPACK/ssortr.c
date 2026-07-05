/* ../FORTRAN/ARPACK/SRC/ssortr.f -- translated by f2c (version 20100827).
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

/* ----------------------------------------------------------------------- */
/* \BeginDoc */

/* \Name: ssortr */

/* \Description: */
/*  Sort the array X1 in the order specified by WHICH and optionally */
/*  applies the permutation to the array X2. */

/* \Usage: */
/*  call ssortr */
/*     ( WHICH, APPLY, N, X1, X2 ) */

/* \Arguments */
/*  WHICH   Character*2.  (Input) */
/*          'LM' -> X1 is sorted into increasing order of magnitude. */
/*          'SM' -> X1 is sorted into decreasing order of magnitude. */
/*          'LA' -> X1 is sorted into increasing order of algebraic. */
/*          'SA' -> X1 is sorted into decreasing order of algebraic. */

/*  APPLY   Logical.  (Input) */
/*          APPLY = .TRUE.  -> apply the sorted order to X2. */
/*          APPLY = .FALSE. -> do not apply the sorted order to X2. */

/*  N       Integer.  (INPUT) */
/*          Size of the arrays. */

/*  X1      Real array of length N.  (INPUT/OUTPUT) */
/*          The array to be sorted. */

/*  X2      Real array of length N.  (INPUT/OUTPUT) */
/*          Only referenced if APPLY = .TRUE. */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     12/16/93: Version ' 2.1'. */
/*               Adapted from the sort routine in LANSO. */

/* \SCCS Information: @(#) */
/* FILE: sortr.F   SID: 2.3   DATE OF SID: 4/19/96   RELEASE: 2 */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int ssortr_(char *which, logical *apply, integer *n, real *
	x1, real *x2, ftnlen which_len)
{
    /* System generated locals */
    integer i__1;
    real r__1, r__2;

    /* Builtin functions */
    integer s_cmp(char *, char *, ftnlen, ftnlen);

    /* Local variables */
    static integer i__, j, igap;
    static real temp;


/*     %------------------% */
/*     | Scalar Arguments | */
/*     %------------------% */


/*     %-----------------% */
/*     | Array Arguments | */
/*     %-----------------% */


/*     %---------------% */
/*     | Local Scalars | */
/*     %---------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    igap = *n / 2;

    if (s_cmp(which, "SA", (ftnlen)2, (ftnlen)2) == 0) {

/*        X1 is sorted into decreasing order of algebraic. */

L10:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L20:

	    if (j < 0) {
		goto L30;
	    }

	    if (x1[j] < x1[j + igap]) {
		temp = x1[j];
		x1[j] = x1[j + igap];
		x1[j + igap] = temp;
		if (*apply) {
		    temp = x2[j];
		    x2[j] = x2[j + igap];
		    x2[j + igap] = temp;
		}
	    } else {
		goto L30;
	    }
	    j -= igap;
	    goto L20;
L30:
	    ;
	}
	igap /= 2;
	goto L10;

    } else if (s_cmp(which, "SM", (ftnlen)2, (ftnlen)2) == 0) {

/*        X1 is sorted into decreasing order of magnitude. */

L40:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L50:

	    if (j < 0) {
		goto L60;
	    }

	    if ((r__1 = x1[j], dabs(r__1)) < (r__2 = x1[j + igap], dabs(r__2))
		    ) {
		temp = x1[j];
		x1[j] = x1[j + igap];
		x1[j + igap] = temp;
		if (*apply) {
		    temp = x2[j];
		    x2[j] = x2[j + igap];
		    x2[j + igap] = temp;
		}
	    } else {
		goto L60;
	    }
	    j -= igap;
	    goto L50;
L60:
	    ;
	}
	igap /= 2;
	goto L40;

    } else if (s_cmp(which, "LA", (ftnlen)2, (ftnlen)2) == 0) {

/*        X1 is sorted into increasing order of algebraic. */

L70:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L80:

	    if (j < 0) {
		goto L90;
	    }

	    if (x1[j] > x1[j + igap]) {
		temp = x1[j];
		x1[j] = x1[j + igap];
		x1[j + igap] = temp;
		if (*apply) {
		    temp = x2[j];
		    x2[j] = x2[j + igap];
		    x2[j + igap] = temp;
		}
	    } else {
		goto L90;
	    }
	    j -= igap;
	    goto L80;
L90:
	    ;
	}
	igap /= 2;
	goto L70;

    } else if (s_cmp(which, "LM", (ftnlen)2, (ftnlen)2) == 0) {

/*        X1 is sorted into increasing order of magnitude. */

L100:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L110:

	    if (j < 0) {
		goto L120;
	    }

	    if ((r__1 = x1[j], dabs(r__1)) > (r__2 = x1[j + igap], dabs(r__2))
		    ) {
		temp = x1[j];
		x1[j] = x1[j + igap];
		x1[j + igap] = temp;
		if (*apply) {
		    temp = x2[j];
		    x2[j] = x2[j + igap];
		    x2[j + igap] = temp;
		}
	    } else {
		goto L120;
	    }
	    j -= igap;
	    goto L110;
L120:
	    ;
	}
	igap /= 2;
	goto L100;
    }

L9000:
    return 0;

/*     %---------------% */
/*     | End of ssortr | */
/*     %---------------% */

} /* ssortr_ */

