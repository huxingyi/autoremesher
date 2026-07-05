#include "ICFS.h"
#include <cmath>
#include <algorithm>

#ifdef USE_OPENMP
#include <omp.h>
#endif

#include "ICFS.h"

/*****************************************************************

COPYRIGHT NOTIFICATION

This program discloses material protectable under copyright laws of
the United States. Permission to copy and modify this software and its
documentation for internal research use is hereby granted, provided
that this notice is retained thereon and on all copies or modifications.
The University of Chicago makes no representations as to the suitability
and operability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Use of this software for commercial purposes is expressly prohibited
without contacting

Jorge J. More'
Mathematics and Computer Science Division
Argonne National Laboratory
9700 S. Cass Ave.
Argonne, Illinois 60439-4844
e-mail: more@mcs.anl.gov

Argonne National Laboratory with facilities in the states of
Illinois and Idaho, is owned by The United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

*****************************************************************/

/* Subroutine */int dicfs_(int *n, int *nnz, double *a, double *adiag,
                                                   int *acol_ptr__, int *arow_ind__, double *l, double *ldiag,
                                                   int *lcol_ptr__, int * lrow_ind__, int *p, double *alpha, int *iwa,
                                                   double * wa1, double *wa2)
{
        /* System generated locals */
        int i__1, i__2;
        double d__1/*, d__2, d__3*/;

        /* Local variables */
        static int i__, j, k, nb;
        static int info;
        static double alphas;

        /*     ********* */

        /*     Subroutine dicfs */

        /*     Given a symmetric matrix A in compressed column storage, this */
        /*     subroutine computes an incomplete Cholesky factor of A + alpha*D, */
        /*     where alpha is a shift and D is the diagonal matrix with entries */
        /*     set to the l2 norms of the columns of A. */

        /*     The subroutine statement is */

        /*       subroutine dicfs(n,nnz,a,adiag,acol_ptr,arow_ind, */
        /*                        l,ldiag,lcol_ptr,lrow_ind, */
        /*                        p,alpha,iwa,wa1,wa2) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the order of A. */
        /*         On exit n is unchanged. */

        /*       nnz is an int variable. */
        /*         On entry nnz is the number of nonzeros in the strict lower */
        /*            triangular part of A. */
        /*         On exit nnz is unchanged. */

        /*       a is a double precision array of dimension nnz. */
        /*         On entry a must contain the strict lower triangular part */
        /*            of A in compressed column storage. */
        /*         On exit a is unchanged. */

        /*       adiag is a double precision array of dimension n. */
        /*         On entry adiag must contain the diagonal elements of A. */
        /*         On exit adiag is unchanged. */

        /*       acol_ptr is an int array of dimension n + 1. */
        /*         On entry acol_ptr must contain pointers to the columns of A. */
        /*            The nonzeros in column j of A must be in positions */
        /*            acol_ptr(j), ... , acol_ptr(j+1) - 1. */
        /*         On exit acol_ptr is unchanged. */

        /*       arow_ind is an int array of dimension nnz. */
        /*         On entry arow_ind must contain row indices for the strict */
        /*            lower triangular part of A in compressed column storage. */
        /*         On exit arow_ind is unchanged. */

        /*       l is a double precision array of dimension nnz+n*p. */
        /*         On entry l need not be specified. */
        /*         On exit l contains the strict lower triangular part */
        /*            of L in compressed column storage. */

        /*       ldiag is a double precision array of dimension n. */
        /*         On entry ldiag need not be specified. */
        /*         On exit ldiag contains the diagonal elements of L. */

        /*       lcol_ptr is an int array of dimension n + 1. */
        /*         On entry lcol_ptr need not be specified. */
        /*         On exit lcol_ptr contains pointers to the columns of L. */
        /*            The nonzeros in column j of L are in the */
        /*            lcol_ptr(j), ... , lcol_ptr(j+1) - 1 positions of l. */

        /*       lrow_ind is an int array of dimension nnz+n*p. */
        /*         On entry lrow_ind need not be specified. */
        /*         On exit lrow_ind contains row indices for the strict lower */
        /*            triangular part of L in compressed column storage. */

        /*       p is an int variable. */
        /*         On entry p specifes the amount of memory available for the */
        /*            incomplete Cholesky factorization. */
        /*         On exit p is unchanged. */

        /*       alpha is a double precision variable. */
        /*         On entry alpha is the initial guess of the shift. */
        /*         On exit alpha is final shift */

        /*       iwa is an int work array of dimesnion 3*n. */

        /*       wa1 is a double precision work array of dimension n. */

        /*       wa2 is a double precision work array of dimension n. */

        /*     Subprograms called */

        /*         MINPACK-2  ......  dicf */

        /*     MINPACK-2 Project. October 1998. */
        /*     Argonne National Laboratory. */
        /*     Chih-Jen Lin and Jorge J. More'. */

        /*     ********** */
        /*     Compute the l2 norms of the columns of A. */
        /* Parameter adjustments */
        --wa2;
        --wa1;
        --iwa;
        --lcol_ptr__;
        --ldiag;
        --acol_ptr__;
        --adiag;
        --arow_ind__;
        --a;
        --lrow_ind__;
        --l;

        /* Function Body */
        i__1 = *n;
#ifdef USE_OPENMP
#pragma omp parallel for private(i__)
#endif
        for (i__ = 1; i__ <= i__1; ++i__)
        {
                /* Computing 2nd power */
                wa1[i__] = adiag[i__] * adiag[i__];
        }
        i__1 = *n;
        for (j = 1; j <= i__1; ++j)
        {
                i__2 = acol_ptr__[j + 1] - 1;
                for (i__ = acol_ptr__[j]; i__ <= i__2; ++i__)
                {
                        k = arow_ind__[i__];
                        /* Computing 2nd power */
                        d__1 = a[i__] * a[i__];
                        wa1[j] += d__1;
                        wa1[k] += d__1;
                }
        }
        i__1 = *n;
#ifdef USE_OPENMP
#pragma omp parallel for private(j)
#endif
        for (j = 1; j <= i__1; ++j)
        {
                wa1[j] = std::sqrt(wa1[j]);
        }
        /*     Compute the scaling matrix D. */
        i__1 = *n;
        std::fill(&wa2[1], &wa2[i__1 + 1], 1.0);
#ifdef USE_OPENMP
#pragma omp parallel for private(i__)
#endif
        for (i__ = 1; i__ <= i__1; ++i__)
        {
                if (wa1[i__] > 0.)
                {
                        wa2[i__] = 1. / std::sqrt(wa1[i__]);
                }
        }
        /*     Determine a lower bound for the step. */
        if (*alpha <= 0.)
        {
                alphas = .001;
        }
        else
        {
                alphas = *alpha;
        }
        /*     Compute the initial shift. */
        *alpha = 0.;
        i__1 = *n;
        for (i__ = 1; i__ <= i__1; ++i__)
        {
                if (adiag[i__] == 0.)
                {
                        *alpha = alphas;
                }
                else
                {
                        *alpha = std::max<double>(*alpha, -adiag[i__] * (wa2[i__]
                        * wa2[i__]));
                }
        }
        if (*alpha > 0.)
        {
                *alpha = std::max<double>(*alpha, alphas);
        }
        /*     Search for an acceptable shift. During the search we decrease */
        /*     the lower bound alphas until we Determine a lower bound that */
        /*     is not acceptable. We then increase the shift. */
        /*     The lower bound is decreased by nbfactor at most nbmax times. */
        nb = 1;
        for(;;)
        {
                /*        Copy the sparsity structure of A into L. */
                i__1 = *n + 1;
                std::copy(&acol_ptr__[1], &acol_ptr__[i__1 + 1], &lcol_ptr__[1]);
                i__1 = *nnz;
                std::copy(&arow_ind__[1], &arow_ind__[i__1 + 1], &lrow_ind__[1]);
                /*        Scale A and store in the lower triangular matrix L. */
                i__1 = *n;
#ifdef USE_OPENMP
#pragma omp parallel for private(j)
#endif
                for (j = 1; j <= i__1; ++j)
                {
                        /* Computing 2nd power */
                        ldiag[j] = adiag[j] * (wa2[j] * wa2[j]) + *alpha;
                }
                i__1 = *n;
#ifdef USE_OPENMP
#pragma omp parallel for private(j)
#endif
                for (j = 1; j <= i__1; ++j)
                {
#ifdef USE_OPENMP
#pragma omp parallel for private(i__)
#endif
                        for (i__ = acol_ptr__[j]; i__ <= acol_ptr__[j + 1] - 1; ++i__)
                        {
                                l[i__] = a[i__] * wa2[j] * wa2[arow_ind__[i__]];
                        }
                }
                /*        Attempt an incomplete factorization. */
                dicf_(n, nnz, &l[1], &ldiag[1], &lcol_ptr__[1], &lrow_ind__[1], p,
                        &info, &iwa[1], &iwa[*n + 1], &iwa[(*n << 1) + 1], &wa1[1]);
                /*        If the factorization exists, then test for termination. */
                /*        Otherwise increment the shift. */
                if (info >= 0)
                {
                        /*           If the shift is at the lower bound, reduce the shift. */
                        /*           Otherwise undo the scaling of L and exit. */
                        if (*alpha == alphas && nb < 3)
                        {
                                alphas /= 512.;
                                *alpha = alphas;
                                ++nb;
                        }
                        else
                        {
                                i__1 = *n;
#ifdef USE_OPENMP
#pragma omp parallel for private(i__)
#endif
                                for (i__ = 1; i__ <= i__1; ++i__)
                                {
                                        ldiag[i__] /= wa2[i__];
                                }
                                i__1 = lcol_ptr__[*n + 1] - 1;
#ifdef USE_OPENMP
#pragma omp parallel for private(j)
#endif
                                for (j = 1; j <= i__1; ++j)
                                {
                                        l[j] /= wa2[lrow_ind__[j]];
                                }
                                return 0;
                        }
                }
                else
                {
                        /* Computing std::max<double> */
                        d__1 = *alpha * 2.;
                        *alpha = std::max<double>(d__1, alphas);
                }
        }
        return 0;
} /* dicfs_ */

/* Subroutine */
int dicf_(int *n, int *nnz, double *a, double *diag, int *col_ptr__,
                  int *row_ind__, int *p, int *info, int *indr, int *indf, int *list,
                  double *w)
{
        /* System generated locals */
        int i__1, i__2;
        //double d__1;

        /* Local variables */
        static int i__, j, k, ip, np, iej, iek, mlj, nlj, isj, kth, isk;
        static double lval;
        static int newk;
        static int newiej, newisj;

        /*     ********* */

        /*     Subroutine dicf */

        /*     Given a sparse symmetric matrix A in compressed row storage, */
        /*     this subroutine computes an incomplete Cholesky factorization. */

        /*     Implementation of dicf is based on the Jones-Plassmann code. */
        /*     Arrays indf and list define the data structure. */
        /*     At the beginning of the computation of the j-th column, */

        /*       For k < j, indf(k) is the index of A for the first */
        /*       nonzero l(i,k) in the k-th column with i >= j. */

        /*       For k < j, list(i) is a pointer to a linked list of column */
        /*       indices k with i = row_ind(indf(k)). */

        /*     For the computation of the j-th column, the array indr records */
        /*     the row indices. Hence, if nlj is the number of nonzeros in the */
        /*     j-th column, then indr(1),...,indr(nlj) are the row indices. */
        /*     Also, for i > j, indf(i) marks the row indices in the j-th */
        /*     column so that indf(i) = 1 if l(i,j) is not zero. */

        /*     The subroutine statement is */

        /*       subroutine dicf(n,nnz,a,diag,col_ptr,row_ind,p,info, */
        /*                       indr,indf,list,w) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the order of A. */
        /*         On exit n is unchanged. */

        /*       nnz is an int variable. */
        /*         On entry nnz is the number of nonzeros in the strict lower */
        /*            triangular part of A. */
        /*         On exit nnz is unchanged. */

        /*       a is a double precision array of dimension nnz+n*p. */
        /*         On entry the first nnz entries of a must contain the strict */
        /*            lower triangular part of A in compressed column storage. */
        /*         On exit a contains the strict lower triangular part */
        /*            of L in compressed column storage. */

        /*       diag is a double precision array of dimension n. */
        /*         On entry diag must contain the diagonal elements of A. */
        /*         On exit diag contains the diagonal elements of L. */

        /*       col_ptr is an int array of dimension n + 1. */
        /*         On entry col_ptr must contain pointers to the columns of A. */
        /*            The nonzeros in column j of A must be in positions */
        /*            col_ptr(j), ... , col_ptr(j+1) - 1. */
        /*         On exit col_ptr contains pointers to the columns of L. */
        /*            The nonzeros in column j of L are in the */
        /*            col_ptr(j), ... , col_ptr(j+1) - 1 positions of l. */

        /*       row_ind is an int array of dimension nnz+n*p. */
        /*         On entry row_ind must contain row indices for the strict */
        /*            lower triangular part of A in compressed column storage. */
        /*         On exit row_ind contains row indices for the strict lower */
        /*            triangular part of L in compressed column storage. */

        /*       p is an int variable. */
        /*         On entry p specifes the amount of memory available for the */
        /*            incomplete Cholesky factorization. */
        /*         On exit p is unchanged. */

        /*       info is an int variable. */
        /*         On entry info need not be specified. */
        /*         On exit info = 0 if the factorization succeeds, and */
        /*            info < 0 if the -info pivot is not positive. */

        /*       indr is an int work array of dimension n. */

        /*       indf is an int work array of dimension n. */

        /*       list is an int work array of dimension n. */

        /*       w is a double precision work array of dimension n. */

        /*     Subprograms called */

        /*       MINPACK-2  ......  dsel2, ihsort, insort */

        /*       Level 1 BLAS  ...  daxpy, dcopy, ddot, dnrm2 */

        /*     MINPACK-2 Project. May 1998. */
        /*     Argonne National Laboratory. */
        /*     Chih-Jen Lin and Jorge J. More'. */

        /*     ********** */
        /* Parameter adjustments */
        --w;
        --list;
        --indf;
        --indr;
        --col_ptr__;
        --diag;
        --a;
        --row_ind__;

        /* Function Body */
        *info = 0;
        i__1 = *n;
        std::fill(&indf[1], &indf[i__1 + 1], 0);
        std::fill(&list[1], &list[i__1 + 1], 0);
        /*     Make room for L by moving A to the last n*p positions in a. */
        np = *n * *p;
        i__1 = *n + 1;
#ifdef USE_OPENMP
#pragma omp parallel for private(j)
#endif
        for (j = 1; j <= i__1; ++j)
        {
                col_ptr__[j] += np;
        }
        for (j = *nnz; j >= 1; --j)
        {
                row_ind__[np + j] = row_ind__[j];
                a[np + j] = a[j];
        }
        /*     Compute the incomplete Cholesky factorization. */
        isj = col_ptr__[1];
        col_ptr__[1] = 1;
        i__1 = *n;
        for (j = 1; j <= i__1; ++j)
        {
                /*        Load column j into the array w. The first and last elements */
                /*        of the j-th column of A are a(isj) and a(iej). */
                nlj = 0;
                iej = col_ptr__[j + 1] - 1;
                i__2 = iej;
                for (ip = isj; ip <= i__2; ++ip)
                {
                        i__ = row_ind__[ip];
                        w[i__] = a[ip];
                        ++nlj;
                        indr[nlj] = i__;
                        indf[i__] = 1;
                }
                /*        Exit if the current pivot is not positive. */
                if (diag[j] <= 0.)
                {
                        *info = -j;
                        return 0;
                }
                diag[j] = std::sqrt(diag[j]);
                /*        Update column j using the previous columns. */
                k = list[j];
                while (k != 0)
                {
                        isk = indf[k];
                        iek = col_ptr__[k + 1] - 1;
                        /*           Set lval to l(j,k). */
                        lval = a[isk];
                        /*           Update indf and list. */
                        newk = list[k];
                        ++isk;
                        if (isk < iek)
                        {
                                indf[k] = isk;
                                list[k] = list[row_ind__[isk]];
                                list[row_ind__[isk]] = k;
                        }
                        k = newk;
                        /*           Compute the update a(i,i) <- a(i,j) - l(i,k)*l(j,k). */
                        /*           In this loop we pick up l(i,k) for k < j and i > j. */
                        i__2 = iek;
                        for (ip = isk; ip <= i__2; ++ip)
                        {
                                i__ = row_ind__[ip];
                                if (indf[i__] != 0)
                                {
                                        w[i__] -= lval * a[ip];
                                }
                                else
                                {
                                        indf[i__] = 1;
                                        ++nlj;
                                        indr[nlj] = i__;
                                        w[i__] = -lval * a[ip];
                                }
                        }
                }
                /*        Compute the j-th column of L. */
                i__2 = nlj;
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                for (k = 1; k <= i__2; ++k)
                {
                        w[indr[k]] /= diag[j];
                }
                /*        Set mlj to the number of nonzeros to be retained. */
                /* Computing min */
                i__2 = iej - isj + 1 + *p;
                mlj = std::min<int>(i__2, nlj);
                kth = nlj - mlj + 1;
                if (nlj >= 1)
                {
                        /*           Determine the kth smallest elements in the current */
                        /*           column, and hence, the largest mlj elements. */
                        dsel2_(&nlj, &w[1], &indr[1], &kth);
                        /*           Sort the row indices of the selected elements. Insertion */
                        /*           sort is used for small arrays, and heap sort for larger */
                        /*           arrays. The sorting of the row indices is required so that */
                        /*           we can retrieve l(i,k) with i > k from indf(k). */
                        if (mlj <= 20)
                        {
                                insort_(&mlj, &indr[kth]);
                        }
                        else
                        {
                                ihsort_(&mlj, &indr[kth]);
                        }
                }
                /*        Store the largest elements in L. The first and last elements */
                /*        of the j-th column of L are a(newisj) and a(newiej). */
                newisj = col_ptr__[j];
                newiej = newisj + mlj - 1;
                i__2 = newiej;
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                for (k = newisj; k <= i__2; ++k)
                {
                        a[k] = w[indr[k - newisj + kth]];
                        row_ind__[k] = indr[k - newisj + kth];
                }
                /*        Update the diagonal elements. */
                i__2 = nlj;
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                for (k = kth; k <= i__2; ++k)
                {
                        /* Computing 2nd power */
                        diag[indr[k]] -= w[indr[k]] * w[indr[k]];
                }
                /*        Update indf and list for the j-th column. */
                if (newisj < newiej)
                {
                        indf[j] = newisj;
                        list[j] = list[row_ind__[newisj]];
                        list[row_ind__[newisj]] = j;
                }
                /*        Clear out elements j+1,...,n of the array indf. */
                i__2 = nlj;
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                for (k = 1; k <= i__2; ++k)
                {
                        indf[indr[k]] = 0;
                }
                /*        Update isj and col_ptr. */
                isj = col_ptr__[j + 1];
                col_ptr__[j + 1] = newiej + 1;
        }
        return 0;
} /* dicf_ */

/* Subroutine */
int dsel2_(int *n, double *x, int *keys, int *k)
{
        /* System generated locals */
        int i__1;
        //double d__1, d__2;

        /* Local variables */
        static int i__, l, m, p, u, p1, p2, p3, lc, lp;

        /*     ********** */

        /*     Subroutine dsel2 */

        /*     Given an array x, this subroutine permutes the elements of the */
        /*     array keys so that */

        /*       abs(x(keys(i))) <= abs(x(keys(k))),  1 <= i <= k, */
        /*       abs(x(keys(k))) <= abs(x(keys(i))),  k <= i <= n. */

        /*     In other words, the smallest k elements of x in absolute value are */
        /*     x(keys(i)), i = 1,...,k, and x(keys(k)) is the kth smallest element. */

        /*     The subroutine statement is */

        /*       subroutine dsel2(n,x,keys,k) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the number of keys. */
        /*         On exit n is unchanged. */

        /*       x is a double precision array of length *. */
        /*         On entry x is the array to be sorted. */
        /*         On exit x is unchanged. */

        /*       keys is an int array of length n. */
        /*         On entry keys is the array of indices for x. */
        /*         On exit keys is permuted so that the smallest k elements */
        /*            of x in absolute value are x(keys(i)), i = 1,...,k, and */
        /*            x(keys(k)) is the kth smallest element. */

        /*       k is an int. */
        /*         On entry k specifes the kth largest element. */
        /*         On exit k is unchanged. */

        /*     MINPACK-2 Project. March 1998. */
        /*     Argonne National Laboratory. */
        /*     William D. Kastak, Chih-Jen Lin, and Jorge J. More'. */

        /*     Revised October 1999. Length of x was incorrectly set to n. */

        /*     ********** */
        /* Parameter adjustments */
        --keys;
        --x;

        /* Function Body */
        if (*n <= 1 || *k <= 0 || *k > *n)
        {
                return 0;
        }
        u = *n;
        l = 1;
        lc = *n;
        lp = *n << 1;
        /*     Start of iteration loop. */
        while (l < u)
        {
                /*        Choose the partition as the median of the elements in */
                /*        positions l+s*(u-l) for s = 0, 0.25, 0.5, 0.75, 1. */
                /*        Move the partition element into position l. */
                p1 = (u + l * 3) / 4;
                p2 = (u + l) / 2;
                p3 = (u * 3 + l) / 4;
                /*        Order the elements in positions l and p1. */
                if (std::fabs(x[keys[l]]) > std::fabs(x[keys[p1]]))
                {
                        std::swap(keys[l], keys[p1]);
                }
                /*        Order the elements in positions p2 and p3. */
                if (std::fabs(x[keys[p2]]) > std::fabs(x[keys[p3]]))
                {
                        std::swap(keys[p2], keys[p3]);
                }
                /*        Swap the larger of the elements in positions p1 */
                /*        and p3, with the element in position u, and reorder */
                /*        the first two pairs of elements as necessary. */
                if (std::fabs(x[keys[p3]]) > std::fabs(x[keys[p1]]))
                {
                        std::swap(keys[p3], keys[u]);
                        if (std::fabs(x[keys[p2]]) > std::fabs(x[keys[p3]]))
                        {
                                std::swap(keys[p2], keys[p3]);
                        }
                }
                else
                {
                        std::swap(keys[p1], keys[u]);
                        if (std::fabs(x[keys[l]]) > std::fabs(x[keys[p1]]))
                        {
                                std::swap(keys[l], keys[p1]);
                        }
                }
                /*        If we define a(i) = abs(x(keys(i)) for i = 1,..., n, we have */
                /*        permuted keys so that */

                /*          a(l) <= a(p1), a(p2) <= a(p3), std::max<double>(a(p1),a(p3)) <= a(u). */

                /*        Find the third largest element of the four remaining */
                /*        elements (the median), and place in position l. */
                if (std::fabs(x[keys[p1]]) > std::fabs(x[keys[p3]]))
                {
                        if (std::fabs(x[keys[l]]) <= std::fabs(x[keys[p3]]))
                        {
                                std::swap(keys[l], keys[p3]);
                        }
                }
                else
                {
                        if (std::fabs(x[keys[p2]]) <= std::fabs(x[keys[p1]]))
                        {
                                std::swap(keys[l], keys[p1]);
                        }
                        else
                        {
                                std::swap(keys[l], keys[p2]);
                        }
                }
                /*        Partition the array about the element in position l. */
                m = l;
                i__1 = u;
                for (i__ = l + 1; i__ <= i__1; ++i__)
                {
                        if (std::fabs(x[keys[i__]]) < std::fabs(x[keys[l]]))
                        {
                                ++m;
                                std::swap(keys[m], keys[i__]);
                        }
                }
                /*        Move the partition element into position m. */
                std::swap(keys[l], keys[m]);
                /*        Adjust the values of l and u. */
                if (*k >= m)
                {
                        l = m + 1;
                }
                if (*k <= m)
                {
                        u = m - 1;
                }
                /*        Check for multiple medians if the length of the subarray */
                /*        has not decreased by 1/3 after two consecutive iterations. */
                if ((u - l) * 3 > lp << 1 && *k > m)
                {
                        /*           Partition the remaining elements into those elements */
                        /*           equal to x(m), and those greater than x(m). Adjust */
                        /*           the values of l and u. */
                        p = m;
                        i__1 = u;
                        for (i__ = m + 1; i__ <= i__1; ++i__)
                        {
                                if (std::fabs(x[keys[i__]]) == std::fabs(x[keys[m]]))
                                {
                                        ++p;
                                        std::swap(keys[p], keys[i__]);
                                }
                        }
                        l = p + 1;
                        if (*k <= p)
                        {
                                u = p - 1;
                        }
                }
                /*        Update the length indicators for the subarray. */
                lp = lc;
                lc = u - l;
        }
        return 0;
} /* dsel2_ */

/* Subroutine */
int insort_(int *n, int *keys)
{
        /* System generated locals */
        int i__1;

        /* Local variables */
        static int i__, j, ind;

        /*     ********** */

        /*     Subroutine insort */

        /*     Given an int array keys of length n, this subroutine uses */
        /*     an insertion sort to sort the keys in increasing order. */

        /*     The subroutine statement is */

        /*       subroutine insort(n,keys) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the number of keys. */
        /*         On exit n is unchanged. */

        /*       keys is an int array of length n. */
        /*         On entry keys is the array to be sorted. */
        /*         On exit keys is permuted to increasing order. */

        /*     MINPACK-2 Project. March 1998. */
        /*     Argonne National Laboratory. */
        /*     Chih-Jen Lin and Jorge J. More'. */

        /*     ********** */
        /* Parameter adjustments */
        --keys;

        /* Function Body */
        i__1 = *n;
        for (j = 2; j <= i__1; ++j)
        {
                ind = keys[j];
                i__ = j - 1;
                while (i__ > 0 && keys[i__] > ind)
                {
                        keys[i__ + 1] = keys[i__];
                        --i__;
                }
                keys[i__ + 1] = ind;
        }
        return 0;
} /* insort_ */

/* Subroutine */
int ihsort_(int *n, int *keys)
{
        static int k, m, x, mid, lheap, rheap;

        /*     ********** */

        /*     Subroutine ihsort */

        /*     Given an int array keys of length n, this subroutine uses */
        /*     a heap sort to sort the keys in increasing order. */

        /*     This subroutine is a minor modification of code written by */
        /*     Mark Jones and Paul Plassmann. */

        /*     The subroutine statement is */

        /*       subroutine ihsort(n,keys) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the number of keys. */
        /*         On exit n is unchanged. */

        /*       keys is an int array of length n. */
        /*         On entry keys is the array to be sorted. */
        /*         On exit keys is permuted to increasing order. */

        /*     MINPACK-2 Project. March 1998. */
        /*     Argonne National Laboratory. */
        /*     Chih-Jen Lin and Jorge J. More'. */

        /*     ********** */
        /* Parameter adjustments */
        --keys;

        /* Function Body */
        if (*n <= 1)
        {
                return 0;
        }
        /*     Build the heap. */
        mid = *n / 2;
        for (k = mid; k >= 1; --k)
        {
                x = keys[k];
                lheap = k;
                rheap = *n;
                m = lheap << 1;
                while (m <= rheap)
                {
                        if (m < rheap)
                        {
                                if (keys[m] < keys[m + 1])
                                {
                                        ++m;
                                }
                        }
                        if (x >= keys[m])
                        {
                                m = rheap + 1;
                        }
                        else
                        {
                                keys[lheap] = keys[m];
                                lheap = m;
                                m = lheap << 1;
                        }
                }
                keys[lheap] = x;
        }
        /*     Sort the heap. */
        for (k = *n; k >= 2; --k)
        {
                x = keys[k];
                keys[k] = keys[1];
                lheap = 1;
                rheap = k - 1;
                m = 2;
                while (m <= rheap)
                {
                        if (m < rheap)
                        {
                                if (keys[m] < keys[m + 1])
                                {
                                        ++m;
                                }
                        }
                        if (x >= keys[m])
                        {
                                m = rheap + 1;
                        }
                        else
                        {
                                keys[lheap] = keys[m];
                                lheap = m;
                                m = lheap << 1;
                        }
                }
                keys[lheap] = x;
        }
        return 0;
} /* ihsort_ */

/* Subroutine */
int dstrsol_(int *n, double *l, double *ldiag, int *jptr, int *indr,
                         double *r__, char *task)
{
        /* System generated locals */
        int i__1, i__2;

        /* Local variables */
        static int j, k;
        static double temp;

        /*     ********** */

        /*     Subroutine dstrsol */

        /*     This subroutine solves the triangular systems L*x = r or L'*x = r. */

        /*     The subroutine statement is */

        /*       subroutine dstrsol(n,l,ldiag,jptr,indr,r,task) */

        /*     where */

        /*       n is an int variable. */
        /*         On entry n is the order of L. */
        /*         On exit n is unchanged. */

        /*       l is a double precision array of dimension *. */
        /*         On entry l must contain the nonzeros in the strict lower */
        /*            triangular part of L in compressed column storage. */
        /*         On exit l is unchanged. */

        /*       ldiag is a double precision array of dimension n. */
        /*         On entry ldiag must contain the diagonal elements of L. */
        /*         On exit ldiag is unchanged. */

        /*       jptr is an int array of dimension n + 1. */
        /*         On entry jptr must contain pointers to the columns of A. */
        /*            The nonzeros in column j of A must be in positions */
        /*            jptr(j), ... , jptr(j+1) - 1. */
        /*         On exit jptr is unchanged. */

        /*       indr is an int array of dimension *. */
        /*         On entry indr must contain row indices for the strict */
        /*            lower triangular part of L in compressed column storage. */
        /*         On exit indr is unchanged. */

        /*       r is a double precision array of dimension n. */
        /*         On entry r must contain the vector r. */
        /*         On exit r contains the solution vector x. */

        /*       task is a character variable of length 60. */
        /*         On entry */
        /*            task(1:1) = 'N' if we need to solve L*x = r */
        /*            task(1:1) = 'T' if we need to solve L'*x = r */
        /*         On exit task is unchanged. */

        /*     MINPACK-2 Project. May 1998. */
        /*     Argonne National Laboratory. */

        /*     ********** */
        /*     Solve L*x =r and store the result in r. */
        /* Parameter adjustments */
        --r__;
        --jptr;
        --ldiag;
        --l;
        --indr;

        /* Function Body */
        if (*(unsigned char *) task == 'N')
        {
                i__1 = *n;
                for (j = 1; j <= i__1; ++j)
                {
                        temp = r__[j] / ldiag[j];
                        i__2 = jptr[j + 1] - 1;
#ifdef USE_OPENMP
#pragma omp parallel for private(k)
#endif
                        for (k = jptr[j]; k <= i__2; ++k)
                        {
                                r__[indr[k]] -= l[k] * temp;
                        }
                        r__[j] = temp;
                }
                return 0;
        }
        /*     Solve L'*x =r and store the result in r. */
        if (*(unsigned char *) task == 'T')
        {
                r__[*n] /= ldiag[*n];
                for (j = *n - 1; j >= 1; --j)
                {
                        temp = 0.;
                        i__1 = jptr[j + 1] - 1;
#ifdef USE_OPENMP
#pragma omp parallel for private(k) reduction(+:temp)
#endif
                        for (k = jptr[j]; k <= i__1; ++k)
                        {
                                temp += l[k] * r__[indr[k]];
                        }
                        r__[j] = (r__[j] - temp) / ldiag[j];
                }
                return 0;
        }
        return 0;
} /* dstrsol_ */

