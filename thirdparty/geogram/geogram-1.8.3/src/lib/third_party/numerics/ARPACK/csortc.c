/* ../FORTRAN/ARPACK/SRC/csortc.f -- translated by f2c (version 20100827).
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

/* \BeginDoc */

/* \Name: csortc */

/* \Description: */
/*  Sorts the Complex array in X into the order */
/*  specified by WHICH and optionally applies the permutation to the */
/*  Real  array Y. */

/* \Usage: */
/*  call csortc */
/*     ( WHICH, APPLY, N, X, Y ) */

/* \Arguments */
/*  WHICH   Character*2.  (Input) */
/*          'LM' -> sort X into increasing order of magnitude. */
/*          'SM' -> sort X into decreasing order of magnitude. */
/*          'LR' -> sort X with real(X) in increasing algebraic order */
/*          'SR' -> sort X with real(X) in decreasing algebraic order */
/*          'LI' -> sort X with imag(X) in increasing algebraic order */
/*          'SI' -> sort X with imag(X) in decreasing algebraic order */

/*  APPLY   Logical.  (Input) */
/*          APPLY = .TRUE.  -> apply the sorted order to array Y. */
/*          APPLY = .FALSE. -> do not apply the sorted order to array Y. */

/*  N       Integer.  (INPUT) */
/*          Size of the arrays. */

/*  X       Complex array of length N.  (INPUT/OUTPUT) */
/*          This is the array to be sorted. */

/*  Y       Complex array of length N.  (INPUT/OUTPUT) */

/* \EndDoc */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Routines called: */
/*     slapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/*     Adapted from the sort routine in LANSO. */

/* \SCCS Information: @(#) */
/* FILE: sortc.F   SID: 2.2   DATE OF SID: 4/20/96   RELEASE: 2 */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int csortc_(char *which, logical *apply, integer *n, complex 
	*x, complex *y, ftnlen which_len)
{
    /* System generated locals */
    integer i__1, i__2, i__3;
    real r__1, r__2;

    /* Builtin functions */
    integer s_cmp(char *, char *, ftnlen, ftnlen);
    double r_imag(complex *);

    /* Local variables */
    static integer i__, j, igap;
    static complex temp;
    static real temp1, temp2;
    extern doublereal slapy2_(real *, real *);


/*     %------------------% */
/*     | Scalar Arguments | */
/*     %------------------% */


/*     %-----------------% */
/*     | Array Arguments | */
/*     %-----------------% */


/*     %---------------% */
/*     | Local Scalars | */
/*     %---------------% */


/*     %--------------------% */
/*     | External functions | */
/*     %--------------------% */


/*     %--------------------% */
/*     | Intrinsic Functions | */
/*     %--------------------% */

/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    igap = *n / 2;

    if (s_cmp(which, "LM", (ftnlen)2, (ftnlen)2) == 0) {

/*        %--------------------------------------------% */
/*        | Sort X into increasing order of magnitude. | */
/*        %--------------------------------------------% */

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

	    i__2 = j;
	    r__1 = x[i__2].r;
	    r__2 = r_imag(&x[j]);
	    temp1 = slapy2_(&r__1, &r__2);
	    i__2 = j + igap;
	    r__1 = x[i__2].r;
	    r__2 = r_imag(&x[j + igap]);
	    temp2 = slapy2_(&r__1, &r__2);

	    if (temp1 > temp2) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
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

/*        %--------------------------------------------% */
/*        | Sort X into decreasing order of magnitude. | */
/*        %--------------------------------------------% */

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

	    i__2 = j;
	    r__1 = x[i__2].r;
	    r__2 = r_imag(&x[j]);
	    temp1 = slapy2_(&r__1, &r__2);
	    i__2 = j + igap;
	    r__1 = x[i__2].r;
	    r__2 = r_imag(&x[j + igap]);
	    temp2 = slapy2_(&r__1, &r__2);

	    if (temp1 < temp2) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
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

    } else if (s_cmp(which, "LR", (ftnlen)2, (ftnlen)2) == 0) {

/*        %------------------------------------------------% */
/*        | Sort XREAL into increasing order of algebraic. | */
/*        %------------------------------------------------% */

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

	    i__2 = j;
	    i__3 = j + igap;
	    if (x[i__2].r > x[i__3].r) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
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

    } else if (s_cmp(which, "SR", (ftnlen)2, (ftnlen)2) == 0) {

/*        %------------------------------------------------% */
/*        | Sort XREAL into decreasing order of algebraic. | */
/*        %------------------------------------------------% */

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

	    i__2 = j;
	    i__3 = j + igap;
	    if (x[i__2].r < x[i__3].r) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
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

    } else if (s_cmp(which, "LI", (ftnlen)2, (ftnlen)2) == 0) {

/*        %--------------------------------------------% */
/*        | Sort XIMAG into increasing algebraic order | */
/*        %--------------------------------------------% */

L130:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L140:

	    if (j < 0) {
		goto L150;
	    }

	    if (r_imag(&x[j]) > r_imag(&x[j + igap])) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
		}
	    } else {
		goto L150;
	    }
	    j -= igap;
	    goto L140;
L150:
	    ;
	}
	igap /= 2;
	goto L130;

    } else if (s_cmp(which, "SI", (ftnlen)2, (ftnlen)2) == 0) {

/*        %---------------------------------------------% */
/*        | Sort XIMAG into decreasing algebraic order  | */
/*        %---------------------------------------------% */

L160:
	if (igap == 0) {
	    goto L9000;
	}
	i__1 = *n - 1;
	for (i__ = igap; i__ <= i__1; ++i__) {
	    j = i__ - igap;
L170:

	    if (j < 0) {
		goto L180;
	    }

	    if (r_imag(&x[j]) < r_imag(&x[j + igap])) {
		i__2 = j;
		temp.r = x[i__2].r, temp.i = x[i__2].i;
		i__2 = j;
		i__3 = j + igap;
		x[i__2].r = x[i__3].r, x[i__2].i = x[i__3].i;
		i__2 = j + igap;
		x[i__2].r = temp.r, x[i__2].i = temp.i;

		if (*apply) {
		    i__2 = j;
		    temp.r = y[i__2].r, temp.i = y[i__2].i;
		    i__2 = j;
		    i__3 = j + igap;
		    y[i__2].r = y[i__3].r, y[i__2].i = y[i__3].i;
		    i__2 = j + igap;
		    y[i__2].r = temp.r, y[i__2].i = temp.i;
		}
	    } else {
		goto L180;
	    }
	    j -= igap;
	    goto L170;
L180:
	    ;
	}
	igap /= 2;
	goto L160;
    }

L9000:
    return 0;

/*     %---------------% */
/*     | End of csortc | */
/*     %---------------% */

} /* csortc_ */

