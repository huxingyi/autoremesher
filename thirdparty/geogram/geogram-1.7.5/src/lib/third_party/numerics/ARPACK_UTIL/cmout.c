/* ../FORTRAN/ARPACK/UTIL/cmout.f -- translated by f2c (version 20100827).
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
static integer c__2 = 2;


/*  Routine:    CMOUT */

/*  Purpose:    Complex matrix output routine. */

/*  Usage:      CALL CMOUT (LOUT, M, N, A, LDA, IDIGIT, IFMT) */

/*  Arguments */
/*     M      - Number of rows of A.  (Input) */
/*     N      - Number of columns of A.  (Input) */
/*     A      - Complex M by N matrix to be printed.  (Input) */
/*     LDA    - Leading dimension of A exactly as specified in the */
/*              dimension statement of the calling program.  (Input) */
/*     IFMT   - Format to be used in printing matrix A.  (Input) */
/*     IDIGIT - Print up to IABS(IDIGIT) decimal digits per number.  (In) */
/*              If IDIGIT .LT. 0, printing is done with 72 columns. */
/*              If IDIGIT .GT. 0, printing is done with 132 columns. */

/* \SCCS Information: @(#) */
/* FILE: cmout.f   SID: 2.1   DATE OF SID: 11/16/95   RELEASE: 2 */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int cmout_(integer *lout, integer *m, integer *n, complex *a,
	 integer *lda, integer *idigit, char *ifmt, ftnlen ifmt_len)
{
    /* Initialized data */

    static char icol[1*3] = "C" "o" "l";

    /* Format strings */
    static char fmt_9999[] = "(/1x,a/1x,a)";
    static char fmt_9998[] = "(11x,4(9x,3a1,i4,9x))";
    static char fmt_9994[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9984[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9997[] = "(10x,4(11x,3a1,i4,11x))";
    static char fmt_9993[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e12.5,\002,\002,e12.5,\002)  \002))";
    static char fmt_9983[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e12.5,\002,\002,e12.5,\002)  \002))";
    static char fmt_9996[] = "(10x,3(13x,3a1,i4,13x))";
    static char fmt_9992[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e14.7,\002,\002,e14.7,\002)  \002))";
    static char fmt_9982[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e14.7,\002,\002,e14.7,\002)  \002))";
    static char fmt_9995[] = "(12x,2(18x,3a1,i4,18x))";
    static char fmt_9991[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e20.13,\002,\002,e20.13,\002)\002))";
    static char fmt_9974[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,4(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9964[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,3(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9954[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9944[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e10.3,\002,\002,e10.3,\002)  \002))";
    static char fmt_9973[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,3(\002("
	    "\002,e12.5,\002,\002,e12.5,\002)  \002))";
    static char fmt_9963[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e12.5,\002,\002,e12.5,\002)  \002))";
    static char fmt_9953[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e12.5,\002,\002,e12.5,\002)  \002))";
    static char fmt_9972[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,3(\002("
	    "\002,e14.7,\002,\002,e14.7,\002)  \002))";
    static char fmt_9962[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e14.7,\002,\002,e14.7,\002)  \002))";
    static char fmt_9952[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e14.7,\002,\002,e14.7,\002)  \002))";
    static char fmt_9971[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,2(\002("
	    "\002,e20.13,\002,\002,e20.13,\002)  \002))";
    static char fmt_9961[] = "(1x,\002 Row\002,i4,\002:\002,1x,1p,1(\002("
	    "\002,e20.13,\002,\002,e20.13,\002)  \002))";
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
    static cilist io___12 = { 0, 0, 0, fmt_9984, 0 };
    static cilist io___13 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___14 = { 0, 0, 0, fmt_9993, 0 };
    static cilist io___15 = { 0, 0, 0, fmt_9983, 0 };
    static cilist io___16 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___17 = { 0, 0, 0, fmt_9992, 0 };
    static cilist io___18 = { 0, 0, 0, fmt_9982, 0 };
    static cilist io___19 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___20 = { 0, 0, 0, fmt_9991, 0 };
    static cilist io___21 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___22 = { 0, 0, 0, fmt_9974, 0 };
    static cilist io___23 = { 0, 0, 0, fmt_9964, 0 };
    static cilist io___24 = { 0, 0, 0, fmt_9954, 0 };
    static cilist io___25 = { 0, 0, 0, fmt_9944, 0 };
    static cilist io___26 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___27 = { 0, 0, 0, fmt_9973, 0 };
    static cilist io___28 = { 0, 0, 0, fmt_9963, 0 };
    static cilist io___29 = { 0, 0, 0, fmt_9953, 0 };
    static cilist io___30 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___31 = { 0, 0, 0, fmt_9972, 0 };
    static cilist io___32 = { 0, 0, 0, fmt_9962, 0 };
    static cilist io___33 = { 0, 0, 0, fmt_9952, 0 };
    static cilist io___34 = { 0, 0, 0, fmt_9995, 0 };
    static cilist io___35 = { 0, 0, 0, fmt_9971, 0 };
    static cilist io___36 = { 0, 0, 0, fmt_9961, 0 };
    static cilist io___37 = { 0, 0, 0, fmt_9990, 0 };


/*     ... */
/*     ... SPECIFICATIONS FOR ARGUMENTS */
/*     ... */
/*     ... SPECIFICATIONS FOR LOCAL VARIABLES */
/*     ... */
/*     ... SPECIFICATIONS INTRINSICS */

    /* Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    /* Function Body */
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
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
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
		    if (k1 != *n) {
			io___10.ciunit = *lout;
			s_wsfe(&io___10);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else {
			io___12.ciunit = *lout;
			s_wsfe(&io___12);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L30: */
		}
/* L40: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		io___13.ciunit = *lout;
		s_wsfe(&io___13);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    if (k1 != *n) {
			io___14.ciunit = *lout;
			s_wsfe(&io___14);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else {
			io___15.ciunit = *lout;
			s_wsfe(&io___15);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L50: */
		}
/* L60: */
	    }

	} else if (ndigit <= 8) {
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
		    if (k1 != *n) {
			io___17.ciunit = *lout;
			s_wsfe(&io___17);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else {
			io___18.ciunit = *lout;
			s_wsfe(&io___18);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L70: */
		}
/* L80: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; ++k1) {
		io___19.ciunit = *lout;
		s_wsfe(&io___19);
		do_fio(&c__3, icol, (ftnlen)1);
		do_fio(&c__1, (char *)&k1, (ftnlen)sizeof(integer));
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    io___20.ciunit = *lout;
		    s_wsfe(&io___20);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		    do_fio(&c__2, (char *)&a[i__ + k1 * a_dim1], (ftnlen)
			    sizeof(real));
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
	    for (k1 = 1; k1 <= i__1; k1 += 4) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 3;
		k2 = min(i__2,i__3);
		io___21.ciunit = *lout;
		s_wsfe(&io___21);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    if (k1 + 3 <= *n) {
			io___22.ciunit = *lout;
			s_wsfe(&io___22);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 3 - *n == 1) {
			io___23.ciunit = *lout;
			s_wsfe(&io___23);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 3 - *n == 2) {
			io___24.ciunit = *lout;
			s_wsfe(&io___24);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 3 - *n == 3) {
			io___25.ciunit = *lout;
			s_wsfe(&io___25);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L110: */
		}
/* L120: */
	    }

	} else if (ndigit <= 6) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		io___26.ciunit = *lout;
		s_wsfe(&io___26);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    if (k1 + 2 <= *n) {
			io___27.ciunit = *lout;
			s_wsfe(&io___27);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 2 - *n == 1) {
			io___28.ciunit = *lout;
			s_wsfe(&io___28);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 2 - *n == 2) {
			io___29.ciunit = *lout;
			s_wsfe(&io___29);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L130: */
		}
/* L140: */
	    }

	} else if (ndigit <= 8) {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 3) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 2;
		k2 = min(i__2,i__3);
		io___30.ciunit = *lout;
		s_wsfe(&io___30);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    if (k1 + 2 <= *n) {
			io___31.ciunit = *lout;
			s_wsfe(&io___31);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 2 - *n == 1) {
			io___32.ciunit = *lout;
			s_wsfe(&io___32);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else if (k1 + 2 - *n == 2) {
			io___33.ciunit = *lout;
			s_wsfe(&io___33);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L150: */
		}
/* L160: */
	    }

	} else {
	    i__1 = *n;
	    for (k1 = 1; k1 <= i__1; k1 += 2) {
/* Computing MIN */
		i__2 = *n, i__3 = k1 + 1;
		k2 = min(i__2,i__3);
		io___34.ciunit = *lout;
		s_wsfe(&io___34);
		i__2 = k2;
		for (i__ = k1; i__ <= i__2; ++i__) {
		    do_fio(&c__3, icol, (ftnlen)1);
		    do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
		}
		e_wsfe();
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    if (k1 + 1 <= *n) {
			io___35.ciunit = *lout;
			s_wsfe(&io___35);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    } else {
			io___36.ciunit = *lout;
			s_wsfe(&io___36);
			do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
			i__3 = k2;
			for (j = k1; j <= i__3; ++j) {
			    do_fio(&c__2, (char *)&a[i__ + j * a_dim1], (
				    ftnlen)sizeof(real));
			}
			e_wsfe();
		    }
/* L170: */
		}
/* L180: */
	    }
	}
    }
    io___37.ciunit = *lout;
    s_wsfe(&io___37);
    e_wsfe();


/* ======================================================== */
/*              FORMAT FOR 72 COLUMN */
/* ======================================================== */

/*            DISPLAY 4 SIGNIFICANT DIGITS */


/*            DISPLAY 6 SIGNIFICANT DIGITS */


/*            DISPLAY 8 SIGNIFICANT DIGITS */


/*            DISPLAY 13 SIGNIFICANT DIGITS */



/* ======================================================== */
/*              FORMAT FOR 132 COLUMN */
/* ======================================================== */

/*            DISPLAY 4 SIGNIFICANT DIGIT */


/*            DISPLAY 6 SIGNIFICANT DIGIT */


/*            DISPLAY 8 SIGNIFICANT DIGIT */


/*            DISPLAY 13 SIGNIFICANT DIGIT */





    return 0;
} /* cmout_ */

