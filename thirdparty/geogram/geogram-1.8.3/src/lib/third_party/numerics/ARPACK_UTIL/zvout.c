/* ../FORTRAN/ARPACK/UTIL/zvout.f -- translated by f2c (version 20100827).
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

/* Table of constant values */

static integer c__1 = 1;
static integer c__2 = 2;

/* ----------------------------------------------------------------------- */

/* \SCCS Information: @(#) */
/* FILE: zvout.f   SID: 2.1   DATE OF SID: 11/16/95   RELEASE: 2 */

/* ----------------------------------------------------------------------- */
/*  Routine:    ZVOUT */

/*  Purpose:    Complex*16 vector output routine. */

/*  Usage:      CALL ZVOUT (LOUT, N, CX, IDIGIT, IFMT) */

/*  Arguments */
/*     N      - Length of array CX.  (Input) */
/*     CX     - Complex*16 array to be printed.  (Input) */
/*     IFMT   - Format to be used in printing array CX.  (Input) */
/*     IDIGIT - Print up to IABS(IDIGIT) decimal digits per number.  (In) */
/*              If IDIGIT .LT. 0, printing is done with 72 columns. */
/*              If IDIGIT .GT. 0, printing is done with 132 columns. */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int zvout_(integer *lout, integer *n, doublecomplex *cx, 
	integer *idigit, char *ifmt, ftnlen ifmt_len)
{
    /* Format strings */
    static char fmt_9999[] = "(/1x,a/1x,a)";
    static char fmt_9998[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9997[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9988[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d12.5,\002,\002,d12.5,\002)  \002))";
    static char fmt_9987[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d12.5,\002,\002,d12.5,\002)  \002))";
    static char fmt_9978[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d14.7,\002,\002,d14.7,\002)  \002))";
    static char fmt_9977[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d14.7,\002,\002,d14.7,\002)  \002))";
    static char fmt_9968[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d20.13,\002,\002,d20.13,\002)  \002))";
    static char fmt_9958[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,4(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9957[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,3(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9956[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9955[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d10.3,\002,\002,d10.3,\002)  \002))";
    static char fmt_9948[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,3(\002"
	    "(\002,d12.5,\002,\002,d12.5,\002)  \002))";
    static char fmt_9947[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d12.5,\002,\002,d12.5,\002)  \002))";
    static char fmt_9946[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d12.5,\002,\002,d12.5,\002)  \002))";
    static char fmt_9938[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,3(\002"
	    "(\002,d14.7,\002,\002,d14.7,\002)  \002))";
    static char fmt_9937[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d14.7,\002,\002,d14.7,\002)  \002))";
    static char fmt_9936[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d14.7,\002,\002,d14.7,\002)  \002))";
    static char fmt_9928[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,2(\002"
	    "(\002,d20.13,\002,\002,d20.13,\002)  \002))";
    static char fmt_9927[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,1(\002"
	    "(\002,d20.13,\002,\002,d20.13,\002)  \002))";
    static char fmt_9994[] = "(1x,\002 \002)";

    /* System generated locals */
    integer i__1, i__2, i__3;

    /* Builtin functions */
    integer i_len(char *, ftnlen), s_wsfe(cilist *), do_fio(integer *, char *,
	     ftnlen), e_wsfe(void);

    /* Local variables */
    static integer i__, k1, k2, lll;
    static char line[80];
    static integer ndigit;

    /* Fortran I/O blocks */
    static cilist io___4 = { 0, 0, 0, fmt_9999, 0 };
    static cilist io___8 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___9 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___10 = { 0, 0, 0, fmt_9988, 0 };
    static cilist io___11 = { 0, 0, 0, fmt_9987, 0 };
    static cilist io___12 = { 0, 0, 0, fmt_9978, 0 };
    static cilist io___13 = { 0, 0, 0, fmt_9977, 0 };
    static cilist io___14 = { 0, 0, 0, fmt_9968, 0 };
    static cilist io___15 = { 0, 0, 0, fmt_9958, 0 };
    static cilist io___16 = { 0, 0, 0, fmt_9957, 0 };
    static cilist io___17 = { 0, 0, 0, fmt_9956, 0 };
    static cilist io___18 = { 0, 0, 0, fmt_9955, 0 };
    static cilist io___19 = { 0, 0, 0, fmt_9948, 0 };
    static cilist io___20 = { 0, 0, 0, fmt_9947, 0 };
    static cilist io___21 = { 0, 0, 0, fmt_9946, 0 };
    static cilist io___22 = { 0, 0, 0, fmt_9938, 0 };
    static cilist io___23 = { 0, 0, 0, fmt_9937, 0 };
    static cilist io___24 = { 0, 0, 0, fmt_9936, 0 };
    static cilist io___25 = { 0, 0, 0, fmt_9928, 0 };
    static cilist io___26 = { 0, 0, 0, fmt_9927, 0 };
    static cilist io___27 = { 0, 0, 0, fmt_9994, 0 };


/*     ... */
/*     ... SPECIFICATIONS FOR ARGUMENTS */
/*     ... */
/*     ... SPECIFICATIONS FOR LOCAL VARIABLES */
/*     ... */
/*     ... FIRST EXECUTABLE STATEMENT */


    /* Parameter adjustments */
    --cx;

    /* Function Body */
/* Computing MIN */
    i__1 = i_len(ifmt, ifmt_len);
    lll = min(i__1,80);
    i__1 = lll;
    for (i__ = 1; i__ <= i__1; ++i__) {
	*(unsigned char *)&line[i__ - 1] = '-';
/* L10: */
    }

    for (i__ = lll + 1; i__ <= 80; ++i__) {
	*(unsigned char *)&line[i__ - 1] = ' ';
/* L20: */
    }

    io___4.ciunit = *lout;
    s_wsfe(&io___4);
    do_fio(&c__1, ifmt, ifmt_len);
    do_fio(&c__1, line, lll);
    e_wsfe();

    if (*n <= 0) {
	return 0;
    }
    ndigit = *idigit;
    if (*idigit == 0) {
	ndigit = 4;
    }

/* ======================================================================= */
/*             CODE FOR OUTPUT USING 72 COLUMNS FORMAT */
/* ======================================================================= */

    if (*idigit < 0) {
	ndigit = -(*idigit);
	if (ndigit <= 4) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		if (k1 != *n) {
		    io___8.ciunit = *lout;
		    s_wsfe(&io___8);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else {
		    io___9.ciunit = *lout;
		    s_wsfe(&io___9);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L30: */
	    }
	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		if (k1 != *n) {
		    io___10.ciunit = *lout;
		    s_wsfe(&io___10);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else {
		    io___11.ciunit = *lout;
		    s_wsfe(&io___11);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L40: */
	    }
	} else if (ndigit <= 8) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		if (k1 != *n) {
		    io___12.ciunit = *lout;
		    s_wsfe(&io___12);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else {
		    io___13.ciunit = *lout;
		    s_wsfe(&io___13);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L50: */
	    }
	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; ++k1) {
		io___14.ciunit = *lout;
		s_wsfe(&io___14);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(doublereal));
		e_wsfe();
/* L60: */
	    }
	}

/* ======================================================================= */
/*             CODE FOR OUTPUT USING 132 COLUMNS FORMAT */
/* ======================================================================= */

    } else {
	if (ndigit <= 4) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 4) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 3;
		k2 = min(i__2,i__3);
		if (k1 + 3 <= *n) {
		    io___15.ciunit = *lout;
		    s_wsfe(&io___15);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 3 - *n == 1) {
		    io___16.ciunit = *lout;
		    s_wsfe(&io___16);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 3 - *n == 2) {
		    io___17.ciunit = *lout;
		    s_wsfe(&io___17);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 3 - *n == 1) {
		    io___18.ciunit = *lout;
		    s_wsfe(&io___18);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L70: */
	    }
	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		if (k1 + 2 <= *n) {
		    io___19.ciunit = *lout;
		    s_wsfe(&io___19);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 2 - *n == 1) {
		    io___20.ciunit = *lout;
		    s_wsfe(&io___20);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 2 - *n == 2) {
		    io___21.ciunit = *lout;
		    s_wsfe(&io___21);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L80: */
	    }
	} else if (ndigit <= 8) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		if (k1 + 2 <= *n) {
		    io___22.ciunit = *lout;
		    s_wsfe(&io___22);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 2 - *n == 1) {
		    io___23.ciunit = *lout;
		    s_wsfe(&io___23);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 2 - *n == 2) {
		    io___24.ciunit = *lout;
		    s_wsfe(&io___24);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L90: */
	    }
	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		if (k1 + 2 <= *n) {
		    io___25.ciunit = *lout;
		    s_wsfe(&io___25);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		} else if (k1 + 2 - *n == 1) {
		    io___26.ciunit = *lout;
		    s_wsfe(&io___26);
		    do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		    i__2 = k2;
		    for (i__ = k1; i__ <= i__2; ++i__) {
			do_fio(&c__2, (char *)&cx[i__], (ftnlen)sizeof(
				doublereal));
		    }
		    e_wsfe();
		}
/* L100: */
	    }
	}
    }
    io___27.ciunit = *lout;
    s_wsfe(&io___27);
    e_wsfe();
    return 0;

/* ======================================================================= */
/*                   FORMAT FOR 72 COLUMNS */
/* ======================================================================= */

/*                 DISPLAY 4 SIGNIFICANT DIGITS */


/*                 DISPLAY 6 SIGNIFICANT DIGITS */


/*                 DISPLAY 8 SIGNIFICANT DIGITS */


/*                 DISPLAY 13 SIGNIFICANT DIGITS */


/* ========================================================================= */
/*                   FORMAT FOR 132 COLUMNS */
/* ========================================================================= */

/*                 DISPLAY 4 SIGNIFICANT DIGITS */


/*                 DISPLAY 6 SIGNIFICANT DIGITS */


/*                 DISPLAY 8 SIGNIFICANT DIGITS */


/*                 DISPLAY 13 SIGNIFICANT DIGITS */




} /* zvout_ */

