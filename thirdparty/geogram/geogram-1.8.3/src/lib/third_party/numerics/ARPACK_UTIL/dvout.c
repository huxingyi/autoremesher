/* ../FORTRAN/ARPACK/UTIL/dvout.f -- translated by f2c (version 20100827).
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
/*  Routine:    DVOUT */

/*  Purpose:    Real vector output routine. */

/*  Usage:      CALL DVOUT (LOUT, N, SX, IDIGIT, IFMT) */

/*  Arguments */
/*     N      - Length of array SX.  (Input) */
/*     SX     - Real array to be printed.  (Input) */
/*     IFMT   - Format to be used in printing array SX.  (Input) */
/*     IDIGIT - Print up to IABS(IDIGIT) decimal digits per number.  (In) */
/*              If IDIGIT .LT. 0, printing is done with 72 columns. */
/*              If IDIGIT .GT. 0, printing is done with 132 columns. */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int dvout_(integer *lout, integer *n, doublereal *sx, 
	integer *idigit, char *ifmt, ftnlen ifmt_len)
{
    /* Format strings */
    static char fmt_9999[] = "(/1x,a,/1x,a)";
    static char fmt_9998[] = "(1x,i4,\002 - \002,i4,\002:\002,1p,10d12.3)";
    static char fmt_9997[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,8d14.5)";
    static char fmt_9996[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,6d18.9)";
    static char fmt_9995[] = "(1x,i4,\002 - \002,i4,\002:\002,1x,1p,5d24.13)";
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
    static cilist io___10 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___11 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___12 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___13 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___14 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___15 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___16 = { 0, 0, 0, fmt_9994, 0 };


/*     ... */
/*     ... SPECIFICATIONS FOR ARGUMENTS */
/*     ... */
/*     ... SPECIFICATIONS FOR LOCAL VARIABLES */
/*     .. Scalar Arguments .. */
/*     .. */
/*     .. Array Arguments .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */
/*     ... */
/*     ... FIRST EXECUTABLE STATEMENT */


    /* Parameter adjustments */
    --sx;

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
	    for (k1 = 1; k1 <= i__1; k1 += 5) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 4;
		k2 = min(i__2,i__3);
		io___8.ciunit = *lout;
		s_wsfe(&io___8);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L30: */
	    }
	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 4) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 3;
		k2 = min(i__2,i__3);
		io___9.ciunit = *lout;
		s_wsfe(&io___9);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L40: */
	    }
	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		io___10.ciunit = *lout;
		s_wsfe(&io___10);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L50: */
	    }
	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		io___11.ciunit = *lout;
		s_wsfe(&io___11);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
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
	    for (k1 = 1; k1 <= i__1; k1 += 10) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 9;
		k2 = min(i__2,i__3);
		io___12.ciunit = *lout;
		s_wsfe(&io___12);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L70: */
	    }
	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 8) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 7;
		k2 = min(i__2,i__3);
		io___13.ciunit = *lout;
		s_wsfe(&io___13);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L80: */
	    }
	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 6) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 5;
		k2 = min(i__2,i__3);
		io___14.ciunit = *lout;
		s_wsfe(&io___14);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L90: */
	    }
	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 5) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 4;
		k2 = min(i__2,i__3);
		io___15.ciunit = *lout;
		s_wsfe(&io___15);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		do_fio(&c__1, (char *)&k2, (ftnlen)sizeof(integer));
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__1, (char *)&sx[i__], (ftnlen)sizeof(doublereal)
			    );
		}
		e_wsfe();
/* L100: */
	    }
	}
    }
    io___16.ciunit = *lout;
    s_wsfe(&io___16);
    e_wsfe();
    return 0;
} /* dvout_ */

