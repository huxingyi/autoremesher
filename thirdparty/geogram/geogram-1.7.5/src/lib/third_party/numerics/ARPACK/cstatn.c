/* ../FORTRAN/ARPACK/SRC/cstatn.f -- translated by f2c (version 20100827).
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
    integer nopx, nbx, nrorth, nitref, nrstrt;
    real tsaupd, tsaup2, tsaitr, tseigt, tsgets, tsapps, tsconv, tnaupd, 
	    tnaup2, tnaitr, tneigh, tngets, tnapps, tnconv, tcaupd, tcaup2, 
	    tcaitr, tceigh, tcgets, tcapps, tcconv, tmvopx, tmvbx, tgetv0, 
	    titref, trvec;
} timing_;

#define timing_1 timing_


/* \SCCS Information: @(#) */
/* FILE: statn.F   SID: 2.2   DATE OF SID: 4/20/96   RELEASE: 2 */

/*     %---------------------------------------------% */
/*     | Initialize statistic and timing information | */
/*     | for complex nonsymmetric Arnoldi code.      | */
/*     %---------------------------------------------% */
/* Subroutine */ int cstatn_(void)
{

/*     %--------------------------------% */
/*     | See stat.doc for documentation | */
/*     %--------------------------------% */

/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */
/*     %--------------------------------% */
/*     | See stat.doc for documentation | */
/*     %--------------------------------% */

/* \SCCS Information: @(#) */
/* FILE: stat.h   SID: 2.2   DATE OF SID: 11/16/95   RELEASE: 2 */


    timing_1.nopx = 0;
    timing_1.nbx = 0;
    timing_1.nrorth = 0;
    timing_1.nitref = 0;
    timing_1.nrstrt = 0;
    timing_1.tcaupd = 0.f;
    timing_1.tcaup2 = 0.f;
    timing_1.tcaitr = 0.f;
    timing_1.tceigh = 0.f;
    timing_1.tcgets = 0.f;
    timing_1.tcapps = 0.f;
    timing_1.tcconv = 0.f;
    timing_1.titref = 0.f;
    timing_1.tgetv0 = 0.f;
    timing_1.trvec = 0.f;
/*     %----------------------------------------------------% */
/*     | User time including reverse communication overhead | */
/*     %----------------------------------------------------% */
    timing_1.tmvopx = 0.f;
    timing_1.tmvbx = 0.f;
    return 0;

/*     %---------------% */
/*     | End of cstatn | */
/*     %---------------% */

} /* cstatn_ */

