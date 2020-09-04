/* ../FORTRAN/ARPACK/UTIL/dmout.f -- translated by f2c (version 20100827).
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
static integer c__3 = 3;

/* ----------------------------------------------------------------------- */
/*  Routine:    DMOUT */

/*  Purpose:    Real matrix output routine. */

/*  Usage:      CALL DMOUT (LOUT, M, N, A, LDA, IDIGIT, IFMT) */

/*  Arguments */
/*     M      - Number of rows of A.  (Input) */
/*     N      - Number of columns of A.  (Input) */
/*     A      - Real M by N matrix to be printed.  (Input) */
/*     LDA    - Leading dimension of A exactly as specified in the */
/*              dimension statement of the calling program.  (Input) */
/*     IFMT   - Format to be used in printing matrix A.  (Input) */
/*     IDIGIT - Print up to IABS(IDIGIT) decimal digits per number.  (In) */
/*              If IDIGIT .LT. 0, printing is done with 72 columns. */
/*              If IDIGIT .GT. 0, printing is done with 132 columns. */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int dmout_(integer *lout, integer *m, integer *n, doublereal 
	*a, integer *lda, integer *idigit, char *ifmt, ftnlen ifmt_len)
{
    /* Initialized data */

    static char icol[1*3] = "C" "o" "l";

    /* Format strings */
    static char fmt_9999[] = "(/1x,a,/1x,a)";
    static char fmt_9998[] = "(10x,10(4x,3a1,i4,1x))";
    static char fmt_9994[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,10d12.3)";
    static char fmt_9997[] = "(10x,8(5x,3a1,i4,2x))";
    static char fmt_9993[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,8d14.5)";
    static char fmt_9996[] = "(10x,6(7x,3a1,i4,4x))";
    static char fmt_9992[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,6d18.9)";
    static char fmt_9995[] = "(10x,5(9x,3a1,i4,6x))";
    static char fmt_9991[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,5d22.13)";
    static char fmt_9990[] = "(1x,\002 \002)";

    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;

    /* Builtin functions */
    integer i_len(char *, ftnlen), s_wsfe(cilist *), do_fio(integer *, char *,
	     ftnlen), e_wsfe(void);

    /* Local variables */
    static integer i__, j, k1, k2, lll;
    static char line[80];
    static integer ndigit;

    /* Fortran I/O blocks */
    static cilist io___5 = { 0, 0, 0, fmt_9999, 0 };
    static cilist io___9 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___10 = { 0, 0, 0, fmt_9994, 0 };
    static cilist io___12 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___13 = { 0, 0, 0, fmt_9993, 0 };
    static cilist io___14 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___15 = { 0, 0, 0, fmt_9992, 0 };
    static cilist io___16 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___17 = { 0, 0, 0, fmt_9991, 0 };
    static cilist io___18 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___19 = { 0, 0, 0, fmt_9994, 0 };
    static cilist io___20 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___21 = { 0, 0, 0, fmt_9993, 0 };
    static cilist io___22 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___23 = { 0, 0, 0, fmt_9992, 0 };
    static cilist io___24 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___25 = { 0, 0, 0, fmt_9991, 0 };
    static cilist io___26 = { 0, 0, 0, fmt_9990, 0 };


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
/*     .. Local Arrays .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Data statements .. */
    /* Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    /* Function Body */
/*     .. */
/*     .. Executable Statements .. */
/*     ... */
/*     ... FIRST EXECUTABLE STATEMENT */

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

    io___5.ciunit = *lout;
    s_wsfe(&io___5);
    do_fio(&c__1, ifmt, ifmt_len);
    do_fio(&c__1, line, lll);
    e_wsfe();

    if (*m <= 0 || *n <= 0 || *lda <= 0) {
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
		io___9.ciunit = *lout;
		s_wsfe(&io___9);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___10.ciunit = *lout;
		    s_wsfe(&io___10);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L30: */
		}
/* L40: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 4) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 3;
		k2 = min(i__2,i__3);
		io___12.ciunit = *lout;
		s_wsfe(&io___12);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___13.ciunit = *lout;
		    s_wsfe(&io___13);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L50: */
		}
/* L60: */
	    }

	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		io___14.ciunit = *lout;
		s_wsfe(&io___14);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___15.ciunit = *lout;
		    s_wsfe(&io___15);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L70: */
		}
/* L80: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		io___16.ciunit = *lout;
		s_wsfe(&io___16);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___17.ciunit = *lout;
		    s_wsfe(&io___17);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L90: */
		}
/* L100: */
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
		io___18.ciunit = *lout;
		s_wsfe(&io___18);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___19.ciunit = *lout;
		    s_wsfe(&io___19);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L110: */
		}
/* L120: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 8) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 7;
		k2 = min(i__2,i__3);
		io___20.ciunit = *lout;
		s_wsfe(&io___20);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___21.ciunit = *lout;
		    s_wsfe(&io___21);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L130: */
		}
/* L140: */
	    }

	} else if (ndigit <= 10) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 6) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 5;
		k2 = min(i__2,i__3);
		io___22.ciunit = *lout;
		s_wsfe(&io___22);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___23.ciunit = *lout;
		    s_wsfe(&io___23);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L150: */
		}
/* L160: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 5) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 4;
		k2 = min(i__2,i__3);
		io___24.ciunit = *lout;
		s_wsfe(&io___24);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___25.ciunit = *lout;
		    s_wsfe(&io___25);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    i__3 = k2;
		    for (j = k1; j <= i__3; ++j) {
			do_fio(&c__1, (char *)&a[i__ + j * a_dim1], (ftnlen)
				sizeof(doublereal));
		    }
		    e_wsfe();
/* L170: */
		}
/* L180: */
	    }
	}
    }
    io___26.ciunit = *lout;
    s_wsfe(&io___26);
    e_wsfe();


    return 0;
} /* dmout_ */

