/* ../FORTRAN/ARPACK/SRC/slaqrb.f -- translated by f2c (version 20100827).
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
/* \BeginDoc */

/* \Name: slaqrb */

/* \Description: */
/*  Compute the eigenvalues and the Schur decomposition of an upper */
/*  Hessenberg submatrix in rows and columns ILO to IHI.  Only the */
/*  last component of the Schur vectors are computed. */

/*  This is mostly a modification of the LAPACK routine slahqr. */

/* \Usage: */
/*  call slaqrb */
/*     ( WANTT, N, ILO, IHI, H, LDH, WR, WI,  Z, INFO ) */

/* \Arguments */
/*  WANTT   Logical variable.  (INPUT) */
/*          = .TRUE. : the full Schur form T is required; */
/*          = .FALSE.: only eigenvalues are required. */

/*  N       Integer.  (INPUT) */
/*          The order of the matrix H.  N >= 0. */

/*  ILO     Integer.  (INPUT) */
/*  IHI     Integer.  (INPUT) */
/*          It is assumed that H is already upper quasi-triangular in */
/*          rows and columns IHI+1:N, and that H(ILO,ILO-1) = 0 (unless */
/*          ILO = 1). SLAQRB works primarily with the Hessenberg */
/*          submatrix in rows and columns ILO to IHI, but applies */
/*          transformations to all of H if WANTT is .TRUE.. */
/*          1 <= ILO <= max(1,IHI); IHI <= N. */

/*  H       Real array, dimension (LDH,N).  (INPUT/OUTPUT) */
/*          On entry, the upper Hessenberg matrix H. */
/*          On exit, if WANTT is .TRUE., H is upper quasi-triangular in */
/*          rows and columns ILO:IHI, with any 2-by-2 diagonal blocks in */
/*          standard form. If WANTT is .FALSE., the contents of H are */
/*          unspecified on exit. */

/*  LDH     Integer.  (INPUT) */
/*          The leading dimension of the array H. LDH >= max(1,N). */

/*  WR      Real array, dimension (N).  (OUTPUT) */
/*  WI      Real array, dimension (N).  (OUTPUT) */
/*          The real and imaginary parts, respectively, of the computed */
/*          eigenvalues ILO to IHI are stored in the corresponding */
/*          elements of WR and WI. If two eigenvalues are computed as a */
/*          complex conjugate pair, they are stored in consecutive */
/*          elements of WR and WI, say the i-th and (i+1)th, with */
/*          WI(i) > 0 and WI(i+1) < 0. If WANTT is .TRUE., the */
/*          eigenvalues are stored in the same order as on the diagonal */
/*          of the Schur form returned in H, with WR(i) = H(i,i), and, if */
/*          H(i:i+1,i:i+1) is a 2-by-2 diagonal block, */
/*          WI(i) = sqrt(H(i+1,i)*H(i,i+1)) and WI(i+1) = -WI(i). */

/*  Z       Real array, dimension (N).  (OUTPUT) */
/*          On exit Z contains the last components of the Schur vectors. */

/*  INFO    Integer.  (OUPUT) */
/*          = 0: successful exit */
/*          > 0: SLAQRB failed to compute all the eigenvalues ILO to IHI */
/*               in a total of 30*(IHI-ILO+1) iterations; if INFO = i, */
/*               elements i+1:ihi of WR and WI contain those eigenvalues */
/*               which have been successfully computed. */

/* \Remarks */
/*  1. None. */

/* ----------------------------------------------------------------------- */

/* \BeginLib */

/* \Local variables: */
/*     xxxxxx  real */

/* \Routines called: */
/*     slabad  LAPACK routine that computes machine constants. */
/*     slamch  LAPACK routine that determines machine constants. */
/*     slanhs  LAPACK routine that computes various norms of a matrix. */
/*     slanv2  LAPACK routine that computes the Schur factorization of */
/*             2 by 2 nonsymmetric matrix in standard form. */
/*     slarfg  LAPACK Householder reflection construction routine. */
/*     scopy   Level 1 BLAS that copies one vector to another. */
/*     srot    Level 1 BLAS that applies a rotation to a 2 by 2 matrix. */

/* \Author */
/*     Danny Sorensen               Phuong Vu */
/*     Richard Lehoucq              CRPC / Rice University */
/*     Dept. of Computational &     Houston, Texas */
/*     Applied Mathematics */
/*     Rice University */
/*     Houston, Texas */

/* \Revision history: */
/*     xx/xx/92: Version ' 2.4' */
/*               Modified from the LAPACK routine slahqr so that only the */
/*               last component of the Schur vectors are computed. */

/* \SCCS Information: @(#) */
/* FILE: laqrb.F   SID: 2.2   DATE OF SID: 8/27/96   RELEASE: 2 */

/* \Remarks */
/*     1. None */

/* \EndLib */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int slaqrb_(logical *wantt, integer *n, integer *ilo, 
	integer *ihi, real *h__, integer *ldh, real *wr, real *wi, real *z__, 
	integer *info)
{
    /* System generated locals */
    integer h_dim1, h_offset, i__1, i__2, i__3, i__4;
    real r__1, r__2;

    /* Local variables */
    static integer i__, j, k, l, m;
    static real s, v[3];
    static integer i1, i2;
    static real t1, t2, t3, v1, v2, v3, h00, h10, h11, h12, h21, h22, h33, 
	    h44;
    static integer nh;
    static real cs;
    static integer nr;
    static real sn, h33s, h44s;
    static integer itn, its;
    static real ulp, sum, tst1, h43h34, unfl, ovfl, work[1];
    extern /* Subroutine */ int srot_(integer *, real *, integer *, real *, 
	    integer *, real *, real *), scopy_(integer *, real *, integer *, 
	    real *, integer *), slanv2_(real *, real *, real *, real *, real *
	    , real *, real *, real *, real *, real *), slabad_(real *, real *)
	    ;
    extern doublereal slamch_(char *, ftnlen);
    extern /* Subroutine */ int slarfg_(integer *, real *, real *, integer *, 
	    real *);
    extern doublereal slanhs_(char *, integer *, real *, integer *, real *, 
	    ftnlen);
    static real smlnum;


/*     %------------------% */
/*     | Scalar Arguments | */
/*     %------------------% */


/*     %-----------------% */
/*     | Array Arguments | */
/*     %-----------------% */


/*     %------------% */
/*     | Parameters | */
/*     %------------% */


/*     %------------------------% */
/*     | Local Scalars & Arrays | */
/*     %------------------------% */


/*     %--------------------% */
/*     | External Functions | */
/*     %--------------------% */


/*     %----------------------% */
/*     | External Subroutines | */
/*     %----------------------% */


/*     %-----------------------% */
/*     | Executable Statements | */
/*     %-----------------------% */

    /* Parameter adjustments */
    h_dim1 = *ldh;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    --wr;
    --wi;
    --z__;

    /* Function Body */
    *info = 0;

/*     %--------------------------% */
/*     | Quick return if possible | */
/*     %--------------------------% */

    if (*n == 0) {
	return 0;
    }
    if (*ilo == *ihi) {
	wr[*ilo] = h__[*ilo + *ilo * h_dim1];
	wi[*ilo] = 0.f;
	return 0;
    }

/*     %---------------------------------------------% */
/*     | Initialize the vector of last components of | */
/*     | the Schur vectors for accumulation.         | */
/*     %---------------------------------------------% */

    i__1 = *n - 1;
    for (j = 1; j <= i__1; ++j) {
	z__[j] = 0.f;
/* L5: */
    }
    z__[*n] = 1.f;

    nh = *ihi - *ilo + 1;

/*     %-------------------------------------------------------------% */
/*     | Set machine-dependent constants for the stopping criterion. | */
/*     | If norm(H) <= sqrt(OVFL), overflow should not occur.        | */
/*     %-------------------------------------------------------------% */

    unfl = slamch_("safe minimum", (ftnlen)12);
    ovfl = 1.f / unfl;
    slabad_(&unfl, &ovfl);
    ulp = slamch_("precision", (ftnlen)9);
    smlnum = unfl * (nh / ulp);

/*     %---------------------------------------------------------------% */
/*     | I1 and I2 are the indices of the first row and last column    | */
/*     | of H to which transformations must be applied. If eigenvalues | */
/*     | only are computed, I1 and I2 are set inside the main loop.    | */
/*     | Zero out H(J+2,J) = ZERO for J=1:N if WANTT = .TRUE.          | */
/*     | else H(J+2,J) for J=ILO:IHI-ILO-1 if WANTT = .FALSE.          | */
/*     %---------------------------------------------------------------% */

    if (*wantt) {
	i1 = 1;
	i2 = *n;
	i__1 = i2 - 2;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    h__[i1 + i__ + 1 + i__ * h_dim1] = 0.f;
/* L8: */
	}
    } else {
	i__1 = *ihi - *ilo - 1;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    h__[*ilo + i__ + 1 + (*ilo + i__ - 1) * h_dim1] = 0.f;
/* L9: */
	}
    }

/*     %---------------------------------------------------% */
/*     | ITN is the total number of QR iterations allowed. | */
/*     %---------------------------------------------------% */

    itn = nh * 30;

/*     ------------------------------------------------------------------ */
/*     The main loop begins here. I is the loop index and decreases from */
/*     IHI to ILO in steps of 1 or 2. Each iteration of the loop works */
/*     with the active submatrix in rows and columns L to I. */
/*     Eigenvalues I+1 to IHI have already converged. Either L = ILO or */
/*     H(L,L-1) is negligible so that the matrix splits. */
/*     ------------------------------------------------------------------ */

    i__ = *ihi;
L10:
    l = *ilo;
    if (i__ < *ilo) {
	goto L150;
    }
/*     %--------------------------------------------------------------% */
/*     | Perform QR iterations on rows and columns ILO to I until a   | */
/*     | submatrix of order 1 or 2 splits off at the bottom because a | */
/*     | subdiagonal element has become negligible.                   | */
/*     %--------------------------------------------------------------% */
    i__1 = itn;
    for (its = 0; its <= i__1; ++its) {

/*        %----------------------------------------------% */
/*        | Look for a single small subdiagonal element. | */
/*        %----------------------------------------------% */

	i__2 = l + 1;
	for (k = i__; k >= i__2; --k) {
	    tst1 = (r__1 = h__[k - 1 + (k - 1) * h_dim1], dabs(r__1)) + (r__2 
		    = h__[k + k * h_dim1], dabs(r__2));
	    if (tst1 == 0.f) {
		i__3 = i__ - l + 1;
		tst1 = slanhs_("1", &i__3, &h__[l + l * h_dim1], ldh, work, (
			ftnlen)1);
	    }
/* Computing MAX */
	    r__2 = ulp * tst1;
	    if ((r__1 = h__[k + (k - 1) * h_dim1], dabs(r__1)) <= dmax(r__2,
		    smlnum)) {
		goto L30;
	    }
/* L20: */
	}
L30:
	l = k;
	if (l > *ilo) {

/*           %------------------------% */
/*           | H(L,L-1) is negligible | */
/*           %------------------------% */

	    h__[l + (l - 1) * h_dim1] = 0.f;
	}

/*        %-------------------------------------------------------------% */
/*        | Exit from loop if a submatrix of order 1 or 2 has split off | */
/*        %-------------------------------------------------------------% */

	if (l >= i__ - 1) {
	    goto L140;
	}

/*        %---------------------------------------------------------% */
/*        | Now the active submatrix is in rows and columns L to I. | */
/*        | If eigenvalues only are being computed, only the active | */
/*        | submatrix need be transformed.                          | */
/*        %---------------------------------------------------------% */

	if (! (*wantt)) {
	    i1 = l;
	    i2 = i__;
	}

	if (its == 10 || its == 20) {

/*           %-------------------% */
/*           | Exceptional shift | */
/*           %-------------------% */

	    s = (r__1 = h__[i__ + (i__ - 1) * h_dim1], dabs(r__1)) + (r__2 = 
		    h__[i__ - 1 + (i__ - 2) * h_dim1], dabs(r__2));
	    h44 = s * .75f;
	    h33 = h44;
	    h43h34 = s * -.4375f * s;

	} else {

/*           %-----------------------------------------% */
/*           | Prepare to use Wilkinson's double shift | */
/*           %-----------------------------------------% */

	    h44 = h__[i__ + i__ * h_dim1];
	    h33 = h__[i__ - 1 + (i__ - 1) * h_dim1];
	    h43h34 = h__[i__ + (i__ - 1) * h_dim1] * h__[i__ - 1 + i__ * 
		    h_dim1];
	}

/*        %-----------------------------------------------------% */
/*        | Look for two consecutive small subdiagonal elements | */
/*        %-----------------------------------------------------% */

	i__2 = l;
	for (m = i__ - 2; m >= i__2; --m) {

/*           %---------------------------------------------------------% */
/*           | Determine the effect of starting the double-shift QR    | */
/*           | iteration at row M, and see if this would make H(M,M-1) | */
/*           | negligible.                                             | */
/*           %---------------------------------------------------------% */

	    h11 = h__[m + m * h_dim1];
	    h22 = h__[m + 1 + (m + 1) * h_dim1];
	    h21 = h__[m + 1 + m * h_dim1];
	    h12 = h__[m + (m + 1) * h_dim1];
	    h44s = h44 - h11;
	    h33s = h33 - h11;
	    v1 = (h33s * h44s - h43h34) / h21 + h12;
	    v2 = h22 - h11 - h33s - h44s;
	    v3 = h__[m + 2 + (m + 1) * h_dim1];
	    s = dabs(v1) + dabs(v2) + dabs(v3);
	    v1 /= s;
	    v2 /= s;
	    v3 /= s;
	    v[0] = v1;
	    v[1] = v2;
	    v[2] = v3;
	    if (m == l) {
		goto L50;
	    }
	    h00 = h__[m - 1 + (m - 1) * h_dim1];
	    h10 = h__[m + (m - 1) * h_dim1];
	    tst1 = dabs(v1) * (dabs(h00) + dabs(h11) + dabs(h22));
	    if (dabs(h10) * (dabs(v2) + dabs(v3)) <= ulp * tst1) {
		goto L50;
	    }
/* L40: */
	}
L50:

/*        %----------------------% */
/*        | Double-shift QR step | */
/*        %----------------------% */

	i__2 = i__ - 1;
	for (k = m; k <= i__2; ++k) {

/*           ------------------------------------------------------------ */
/*           The first iteration of this loop determines a reflection G */
/*           from the vector V and applies it from left and right to H, */
/*           thus creating a nonzero bulge below the subdiagonal. */

/*           Each subsequent iteration determines a reflection G to */
/*           restore the Hessenberg form in the (K-1)th column, and thus */
/*           chases the bulge one step toward the bottom of the active */
/*           submatrix. NR is the order of G. */
/*           ------------------------------------------------------------ */

/* Computing MIN */
	    i__3 = 3, i__4 = i__ - k + 1;
	    nr = min(i__3,i__4);
	    if (k > m) {
		scopy_(&nr, &h__[k + (k - 1) * h_dim1], &c__1, v, &c__1);
	    }
	    slarfg_(&nr, v, &v[1], &c__1, &t1);
	    if (k > m) {
		h__[k + (k - 1) * h_dim1] = v[0];
		h__[k + 1 + (k - 1) * h_dim1] = 0.f;
		if (k < i__ - 1) {
		    h__[k + 2 + (k - 1) * h_dim1] = 0.f;
		}
	    } else if (m > l) {
		h__[k + (k - 1) * h_dim1] = -h__[k + (k - 1) * h_dim1];
	    }
	    v2 = v[1];
	    t2 = t1 * v2;
	    if (nr == 3) {
		v3 = v[2];
		t3 = t1 * v3;

/*              %------------------------------------------------% */
/*              | Apply G from the left to transform the rows of | */
/*              | the matrix in columns K to I2.                 | */
/*              %------------------------------------------------% */

		i__3 = i2;
		for (j = k; j <= i__3; ++j) {
		    sum = h__[k + j * h_dim1] + v2 * h__[k + 1 + j * h_dim1] 
			    + v3 * h__[k + 2 + j * h_dim1];
		    h__[k + j * h_dim1] -= sum * t1;
		    h__[k + 1 + j * h_dim1] -= sum * t2;
		    h__[k + 2 + j * h_dim1] -= sum * t3;
/* L60: */
		}

/*              %----------------------------------------------------% */
/*              | Apply G from the right to transform the columns of | */
/*              | the matrix in rows I1 to min(K+3,I).               | */
/*              %----------------------------------------------------% */

/* Computing MIN */
		i__4 = k + 3;
		i__3 = min(i__4,i__);
		for (j = i1; j <= i__3; ++j) {
		    sum = h__[j + k * h_dim1] + v2 * h__[j + (k + 1) * h_dim1]
			     + v3 * h__[j + (k + 2) * h_dim1];
		    h__[j + k * h_dim1] -= sum * t1;
		    h__[j + (k + 1) * h_dim1] -= sum * t2;
		    h__[j + (k + 2) * h_dim1] -= sum * t3;
/* L70: */
		}

/*              %----------------------------------% */
/*              | Accumulate transformations for Z | */
/*              %----------------------------------% */

		sum = z__[k] + v2 * z__[k + 1] + v3 * z__[k + 2];
		z__[k] -= sum * t1;
		z__[k + 1] -= sum * t2;
		z__[k + 2] -= sum * t3;
	    } else if (nr == 2) {

/*              %------------------------------------------------% */
/*              | Apply G from the left to transform the rows of | */
/*              | the matrix in columns K to I2.                 | */
/*              %------------------------------------------------% */

		i__3 = i2;
		for (j = k; j <= i__3; ++j) {
		    sum = h__[k + j * h_dim1] + v2 * h__[k + 1 + j * h_dim1];
		    h__[k + j * h_dim1] -= sum * t1;
		    h__[k + 1 + j * h_dim1] -= sum * t2;
/* L90: */
		}

/*              %----------------------------------------------------% */
/*              | Apply G from the right to transform the columns of | */
/*              | the matrix in rows I1 to min(K+3,I).               | */
/*              %----------------------------------------------------% */

		i__3 = i__;
		for (j = i1; j <= i__3; ++j) {
		    sum = h__[j + k * h_dim1] + v2 * h__[j + (k + 1) * h_dim1]
			    ;
		    h__[j + k * h_dim1] -= sum * t1;
		    h__[j + (k + 1) * h_dim1] -= sum * t2;
/* L100: */
		}

/*              %----------------------------------% */
/*              | Accumulate transformations for Z | */
/*              %----------------------------------% */

		sum = z__[k] + v2 * z__[k + 1];
		z__[k] -= sum * t1;
		z__[k + 1] -= sum * t2;
	    }
/* L120: */
	}
/* L130: */
    }

/*     %-------------------------------------------------------% */
/*     | Failure to converge in remaining number of iterations | */
/*     %-------------------------------------------------------% */

    *info = i__;
    return 0;
L140:
    if (l == i__) {

/*        %------------------------------------------------------% */
/*        | H(I,I-1) is negligible: one eigenvalue has converged | */
/*        %------------------------------------------------------% */

	wr[i__] = h__[i__ + i__ * h_dim1];
	wi[i__] = 0.f;
    } else if (l == i__ - 1) {

/*        %--------------------------------------------------------% */
/*        | H(I-1,I-2) is negligible;                              | */
/*        | a pair of eigenvalues have converged.                  | */
/*        |                                                        | */
/*        | Transform the 2-by-2 submatrix to standard Schur form, | */
/*        | and compute and store the eigenvalues.                 | */
/*        %--------------------------------------------------------% */

	slanv2_(&h__[i__ - 1 + (i__ - 1) * h_dim1], &h__[i__ - 1 + i__ * 
		h_dim1], &h__[i__ + (i__ - 1) * h_dim1], &h__[i__ + i__ * 
		h_dim1], &wr[i__ - 1], &wi[i__ - 1], &wr[i__], &wi[i__], &cs, 
		&sn);
	if (*wantt) {

/*           %-----------------------------------------------------% */
/*           | Apply the transformation to the rest of H and to Z, | */
/*           | as required.                                        | */
/*           %-----------------------------------------------------% */

	    if (i2 > i__) {
		i__1 = i2 - i__;
		srot_(&i__1, &h__[i__ - 1 + (i__ + 1) * h_dim1], ldh, &h__[
			i__ + (i__ + 1) * h_dim1], ldh, &cs, &sn);
	    }
	    i__1 = i__ - i1 - 1;
	    srot_(&i__1, &h__[i1 + (i__ - 1) * h_dim1], &c__1, &h__[i1 + i__ *
		     h_dim1], &c__1, &cs, &sn);
	    sum = cs * z__[i__ - 1] + sn * z__[i__];
	    z__[i__] = cs * z__[i__] - sn * z__[i__ - 1];
	    z__[i__ - 1] = sum;
	}
    }

/*     %---------------------------------------------------------% */
/*     | Decrement number of remaining iterations, and return to | */
/*     | start of the main loop with new value of I.             | */
/*     %---------------------------------------------------------% */

    itn -= its;
    i__ = l - 1;
    goto L10;
L150:
    return 0;

/*     %---------------% */
/*     | End of slaqrb | */
/*     %---------------% */

} /* slaqrb_ */

