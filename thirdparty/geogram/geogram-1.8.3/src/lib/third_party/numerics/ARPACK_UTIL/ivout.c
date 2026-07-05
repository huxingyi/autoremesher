/* ../FORTRAN/ARPACK/UTIL/ivout.f -- translated by f2c (version 20100827).
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

/* ----------------------------------------------------------------------- */
/*  Routine:    IVOUT */

/*  Purpose:    Integer vector output routine. */

/*  Usage:      CALL IVOUT (LOUT, N, IX, IDIGIT, IFMT) */

/*  Arguments */
/*     N      - Length of array IX. (Input) */
/*     IX     - Integer array to be printed. (Input) */
/*     IFMT   - Format to be used in printing array IX. (Input) */
/*     IDIGIT - Print up to ABS(IDIGIT) decimal digits / number. (Input) */
/*              If IDIGIT .LT. 0, printing is done with 72 columns. */
/*              If IDIGIT .GT. 0, printing is done with 132 columns. */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int ivout_(integer *lout, integer *n, integer *ix, integer *
	idigit, char *ifmt, ftnlen ifmt_len)
{
    /* Format strings */
    static char fmt_2000[] = "(/1x,a/1x,a)";
    static char fmt_1000[] = "(1x,i4,\002 - \002,i4,\002:\002,20(1x,i5))";
    static char fmt_1001[] = "(1x,i4,\002 - \002,i4,\002:\002,15(1x,i7))";
    static char fmt_1002[] = "(1x,i4,\002 - \002,i4,\002:\002,10(1x,i11))";
    static char fmt_1003[] = "(1x,i4,\002 - \002,i4,\002:\002,7(1x,i15))";
    static char fmt_1004[] = "(1x,\002 \002)";

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
    static cilist io___4 = { 0, 0, 0, fmt_2000, 0 };
    static cilist io___8 = { 0, 0, 0, fmt_1000, 0 };
    static cilist io___9 = { 0, 0, 0, fmt_1001, 0 };
    static cilist io___10 = { 0, 0, 0, fmt_1002, 0 };
    static cilist io___11 = { 0, 0, 0, fmt_1003, 0 };
    static cilist io___12 = { 0, 0, 0, fmt_1000, 0 };
    static cilist io___13 = { 0, 0, 0, fmt_1001, 0 };
    static cilist io___14 = { 0, 0, 0, fmt_1002, 0 };
    static cilist io___15 = { 0, 0, 0, fmt_1003, 0 };
    static cilist io___16 = { 0, 0, 0, fmt_1004, 0 };


/*     ... */
/*     ... SPECIFICATIONS FOR ARGUMENTS */
/*     ... */
/*     ... SPECIFICATIONS FOR LOCAL VARIABLES */
/*     ... */
/*     ... SPECIFICATIONS INTRINSICS */


    /* Parameter adjustments */
    --ix;

    /* Function Body */
/* Computing MIN */
    i__1 = i_len(ifmt, ifmt_len);
    lll = min(i__1,80);
    i__1 = lll;
    for (i__ = 1; i__ <= i__1; ++i__) {
	*(unsigned char *)&line[i__ - 1] = '-';
/* L1: */
    }

    for (i__ = lll + 1; i__ <= 80; ++i__) {
	*(unsigned char *)&line[i__ - 1] = ' ';
/* L2: */
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
	    for (k1 = 1; k1 <= i__1; k1 += 10) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 9;
		k2 = min(i__2,i__3);
		io___8.ciunit = *lout;
		s_wsfe(&io___8);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L10: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 7) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 6;
		k2 = min(i__2,i__3);
		io___9.ciunit = *lout;
		s_wsfe(&io___9);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L30: */
	    }

	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 5) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 4;
		k2 = min(i__2,i__3);
		io___10.ciunit = *lout;
		s_wsfe(&io___10);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L50: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		io___11.ciunit = *lout;
		s_wsfe(&io___11);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L70: */
	    }
	}

/* ======================================================================= */
/*             CODE FOR OUTPUT USING 132 COLUMNS FORMAT */
/* ======================================================================= */

    } else {

	if (ndigit <= 4) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 20) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 19;
		k2 = min(i__2,i__3);
		io___12.ciunit = *lout;
		s_wsfe(&io___12);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L90: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 15) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 14;
		k2 = min(i__2,i__3);
		io___13.ciunit = *lout;
		s_wsfe(&io___13);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L110: */
	    }

	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 10) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 9;
		k2 = min(i__2,i__3);
		io___14.ciunit = *lout;
		s_wsfe(&io___14);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L130: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 7) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 6;
		k2 = min(i__2,i__3);
		io___15.ciunit = *lout;
		s_wsfe(&io___15);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&ix[i__], (ftnlen)sizeof(integer));
		}
		e_wsfe();
/* L150: */
	    }
	}
    }
    io___16.ciunit = *lout;
    s_wsfe(&io___16);
    e_wsfe();


    return 0;
} /* ivout_ */

