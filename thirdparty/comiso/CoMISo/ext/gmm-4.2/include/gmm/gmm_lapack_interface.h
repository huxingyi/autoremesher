/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard
 
 This file is a part of GETFEM++
 
 Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
 under  the  terms  of the  GNU  Lesser General Public License as published
 by  the  Free Software Foundation;  either version 3 of the License,  or
 (at your option) any later version along with the GCC Runtime Library
 Exception either version 3.1 or (at your option) any later version.
 This program  is  distributed  in  the  hope  that it will be useful,  but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License and GCC Runtime Library Exception for more details.
 You  should  have received a copy of the GNU Lesser General Public License
 along  with  this program;  if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 
 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.
 
===========================================================================*/

/**@file gmm_lapack_interface.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 7, 2003.
   @brief gmm interface for LAPACK
*/

#if defined(GMM_USES_LAPACK)

#ifndef GMM_LAPACK_INTERFACE_H
#define GMM_LAPACK_INTERFACE_H

#include "gmm_blas_interface.h"
#include "gmm_dense_lu.h"
#include "gmm_dense_qr.h"


namespace gmm {

  /* ********************************************************************* */
  /* Operations interfaced for T = float, double, std::complex<float>      */
  /*    or std::complex<double> :                                          */
  /*                                                                       */
  /* lu_factor(dense_matrix<T>, std::vector<int>)                          */
  /* lu_solve(dense_matrix<T>, std::vector<T>, std::vector<T>)             */
  /* lu_solve(dense_matrix<T>, std::vector<int>, std::vector<T>,           */
  /*          std::vector<T>)                                              */
  /* lu_solve_transposed(dense_matrix<T>, std::vector<int>, std::vector<T>,*/
  /*          std::vector<T>)                                              */
  /* lu_inverse(dense_matrix<T>)                                           */
  /* lu_inverse(dense_matrix<T>, std::vector<int>, dense_matrix<T>)        */
  /*                                                                       */
  /* qr_factor(dense_matrix<T>, dense_matrix<T>, dense_matrix<T>)          */
  /*                                                                       */
  /* implicit_qr_algorithm(dense_matrix<T>, std::vector<T>)                */
  /* implicit_qr_algorithm(dense_matrix<T>, std::vector<T>,                */
  /*                       dense_matrix<T>)                                */
  /* implicit_qr_algorithm(dense_matrix<T>, std::vector<std::complex<T> >) */
  /* implicit_qr_algorithm(dense_matrix<T>, std::vector<std::complex<T> >, */
  /*                       dense_matrix<T>)                                */
  /*                                                                       */
  /* ********************************************************************* */

  /* ********************************************************************* */
  /* LAPACK functions used.                                                */
  /* ********************************************************************* */

  extern "C" {
    void sgetrf_(...); void dgetrf_(...); void cgetrf_(...); void zgetrf_(...);
    void sgetrs_(...); void dgetrs_(...); void cgetrs_(...); void zgetrs_(...);
    void sgetri_(...); void dgetri_(...); void cgetri_(...); void zgetri_(...);
    void sgeqrf_(...); void dgeqrf_(...); void cgeqrf_(...); void zgeqrf_(...);
    void sorgqr_(...); void dorgqr_(...); void cungqr_(...); void zungqr_(...);
    void sormqr_(...); void dormqr_(...); void cunmqr_(...); void zunmqr_(...);
    void sgees_ (...); void dgees_ (...); void cgees_ (...); void zgees_ (...);
    void sgeev_ (...); void dgeev_ (...); void cgeev_ (...); void zgeev_ (...);
    void sgesvd_(...); void dgesvd_(...); void cgesvd_(...); void zgesvd_(...);
  }

  /* ********************************************************************* */
  /* LU decomposition.                                                     */
  /* ********************************************************************* */

# define getrf_interface(lapack_name, base_type) inline                    \
  size_type lu_factor(dense_matrix<base_type > &A, std::vector<int> &ipvt){\
    GMMLAPACK_TRACE("getrf_interface");                                    \
    int m = int(mat_nrows(A)), n = int(mat_ncols(A)), lda(m), info(0);     \
    if (m && n) lapack_name(&m, &n, &A(0,0), &lda, &ipvt[0], &info);       \
    return size_type(info);                                                \
  }

  getrf_interface(sgetrf_, BLAS_S)
  getrf_interface(dgetrf_, BLAS_D)
  getrf_interface(cgetrf_, BLAS_C)
  getrf_interface(zgetrf_, BLAS_Z)

  /* ********************************************************************* */
  /* LU solve.                                                             */
  /* ********************************************************************* */

# define getrs_interface(f_name, trans1, lapack_name, base_type) inline    \
  void f_name(const dense_matrix<base_type > &A,                           \
	      const std::vector<int> &ipvt, std::vector<base_type > &x,    \
	      const std::vector<base_type > &b) {                          \
    GMMLAPACK_TRACE("getrs_interface");                                    \
    int n = int(mat_nrows(A)), info, nrhs(1);                              \
    gmm::copy(b, x); trans1;                                               \
    if (n)                                                                 \
      lapack_name(&t, &n, &nrhs, &(A(0,0)),&n,&ipvt[0], &x[0], &n, &info); \
  }
  
# define getrs_trans_n const char t = 'N'
# define getrs_trans_t const char t = 'T'

  getrs_interface(lu_solve, getrs_trans_n, sgetrs_, BLAS_S)
  getrs_interface(lu_solve, getrs_trans_n, dgetrs_, BLAS_D)
  getrs_interface(lu_solve, getrs_trans_n, cgetrs_, BLAS_C)
  getrs_interface(lu_solve, getrs_trans_n, zgetrs_, BLAS_Z)
  getrs_interface(lu_solve_transposed, getrs_trans_t, sgetrs_, BLAS_S)
  getrs_interface(lu_solve_transposed, getrs_trans_t, dgetrs_, BLAS_D)
  getrs_interface(lu_solve_transposed, getrs_trans_t, cgetrs_, BLAS_C)
  getrs_interface(lu_solve_transposed, getrs_trans_t, zgetrs_, BLAS_Z)

  /* ********************************************************************* */
  /* LU inverse.                                                           */
  /* ********************************************************************* */

# define getri_interface(lapack_name, base_type) inline                    \
  void lu_inverse(const dense_matrix<base_type > &LU,                      \
       std::vector<int> &ipvt, const dense_matrix<base_type > &A_) {       \
    GMMLAPACK_TRACE("getri_interface");                                    \
    dense_matrix<base_type >&                                              \
    A = const_cast<dense_matrix<base_type > &>(A_);                        \
    int n = int(mat_nrows(A)), info, lwork(-1); base_type work1;           \
    if (n) {                                                               \
      gmm::copy(LU, A);                                                    \
      lapack_name(&n, &A(0,0), &n, &ipvt[0], &work1, &lwork, &info);       \
      lwork = int(gmm::real(work1));                                       \
      std::vector<base_type > work(lwork);                                 \
      lapack_name(&n, &A(0,0), &n, &ipvt[0], &work[0], &lwork, &info);     \
    }                                                                      \
  }

  getri_interface(sgetri_, BLAS_S)
  getri_interface(dgetri_, BLAS_D)
  getri_interface(cgetri_, BLAS_C)
  getri_interface(zgetri_, BLAS_Z)


  /* ********************************************************************* */
  /* QR factorization.                                                     */
  /* ********************************************************************* */

# define geqrf_interface(lapack_name1, base_type) inline		   \
  void qr_factor(dense_matrix<base_type > &A){			           \
    GMMLAPACK_TRACE("geqrf_interface");				           \
    int m = int(mat_nrows(A)), n = int(mat_ncols(A)), info, lwork(-1);     \
    base_type work1;                                                       \
    if (m && n) {							   \
      std::vector<base_type > tau(n);					   \
      lapack_name1(&m, &n, &A(0,0), &m, &tau[0], &work1  , &lwork, &info); \
      lwork = int(gmm::real(work1));					   \
      std::vector<base_type > work(lwork);				   \
      lapack_name1(&m, &n, &A(0,0), &m, &tau[0], &work[0], &lwork, &info); \
      GMM_ASSERT1(!info, "QR factorization failed");			   \
    }									   \
  }
    
  geqrf_interface(sgeqrf_, BLAS_S)
  geqrf_interface(dgeqrf_, BLAS_D)
    // For complex values, housholder vectors are not the same as in
    // gmm::lu_factor. Impossible to interface for the moment.
    //  geqrf_interface(cgeqrf_, BLAS_C)
    //  geqrf_interface(zgeqrf_, BLAS_Z)

# define geqrf_interface2(lapack_name1, lapack_name2, base_type) inline    \
  void qr_factor(const dense_matrix<base_type > &A,                        \
       dense_matrix<base_type > &Q, dense_matrix<base_type > &R) {         \
    GMMLAPACK_TRACE("geqrf_interface2");                                   \
    int m = int(mat_nrows(A)), n = int(mat_ncols(A)), info, lwork(-1);     \
    base_type work1;		 					   \
    if (m && n) {                                                          \
      gmm::copy(A, Q);                                                     \
      std::vector<base_type > tau(n);                                      \
      lapack_name1(&m, &n, &Q(0,0), &m, &tau[0], &work1  , &lwork, &info); \
      lwork = int(gmm::real(work1));                                       \
      std::vector<base_type > work(lwork);                                 \
      lapack_name1(&m, &n, &Q(0,0), &m, &tau[0], &work[0], &lwork, &info); \
      GMM_ASSERT1(!info, "QR factorization failed");                       \
      base_type *p = &R(0,0), *q = &Q(0,0);                                \
      for (int j = 0; j < n; ++j, q += m-n)                                \
        for (int i = 0; i < n; ++i, ++p, ++q)                              \
          *p = (j < i) ? base_type(0) : *q;                                \
      lapack_name2(&m, &n, &n, &Q(0,0), &m,&tau[0],&work[0],&lwork,&info); \
    }                                                                      \
    else gmm::clear(Q);                                                    \
  }

  geqrf_interface2(sgeqrf_, sorgqr_, BLAS_S)
  geqrf_interface2(dgeqrf_, dorgqr_, BLAS_D)
  geqrf_interface2(cgeqrf_, cungqr_, BLAS_C)
  geqrf_interface2(zgeqrf_, zungqr_, BLAS_Z)
  
  /* ********************************************************************* */
  /* QR algorithm for eigenvalues search.                                  */
  /* ********************************************************************* */

# define gees_interface(lapack_name, base_type)                            \
  template <typename VECT> inline void implicit_qr_algorithm(              \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q,                                      \
         double tol=gmm::default_tol(base_type()), bool compvect = true) { \
    GMMLAPACK_TRACE("gees_interface");                                     \
    typedef bool (*L_fp)(...);  L_fp p = 0;                                \
    int n = int(mat_nrows(A)), info, lwork(-1), sdim; base_type work1;     \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvs = (compvect ? 'V' : 'N'), sort = 'N';                       \
    std::vector<double> rwork(n), eigv1(n), eigv2(n);                      \
    lapack_name(&jobvs, &sort, p, &n, &H(0,0), &n, &sdim, &eigv1[0],       \
                &eigv2[0], &Q(0,0), &n, &work1, &lwork, &rwork[0], &info); \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvs, &sort, p, &n, &H(0,0), &n, &sdim, &eigv1[0],       \
		&eigv2[0], &Q(0,0), &n, &work[0], &lwork, &rwork[0],&info);\
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    extract_eig(H, const_cast<VECT &>(eigval_), tol);                      \
  }

# define gees_interface2(lapack_name, base_type)                           \
  template <typename VECT> inline void implicit_qr_algorithm(              \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q,                                      \
         double tol=gmm::default_tol(base_type()), bool compvect = true) { \
    GMMLAPACK_TRACE("gees_interface2");                                    \
    typedef bool (*L_fp)(...);  L_fp p = 0;                                \
    int n = int(mat_nrows(A)), info, lwork(-1), sdim; base_type work1;     \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvs = (compvect ? 'V' : 'N'), sort = 'N';                       \
    std::vector<double> rwork(n), eigvv(n*2);                              \
    lapack_name(&jobvs, &sort, p, &n, &H(0,0), &n, &sdim, &eigvv[0],       \
                &Q(0,0), &n, &work1, &lwork, &rwork[0], &rwork[0], &info); \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvs, &sort, p, &n, &H(0,0), &n, &sdim, &eigvv[0],       \
                &Q(0,0), &n, &work[0], &lwork, &rwork[0], &rwork[0],&info);\
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    extract_eig(H, const_cast<VECT &>(eigval_), tol);                      \
  }

  gees_interface(sgees_, BLAS_S)
  gees_interface(dgees_, BLAS_D)
  gees_interface2(cgees_, BLAS_C)
  gees_interface2(zgees_, BLAS_Z)

# define geev_int_right(lapack_name, base_type)			           \
  template <typename VECT> inline void geev_interface_right(               \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q) {		                           \
    GMMLAPACK_TRACE("geev_interface");                                     \
    int n = int(mat_nrows(A)), info, lwork(-1); base_type work1;           \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvl = 'N', jobvr = 'V';                                         \
    std::vector<base_type > eigvr(n), eigvi(n);                            \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigvr[0], &eigvi[0],     \
		&Q(0,0), &n, &Q(0,0), &n, &work1, &lwork, &info);   	   \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigvr[0], &eigvi[0],     \
		&Q(0,0), &n, &Q(0,0), &n, &work[0], &lwork, &info);    	   \
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    gmm::copy(eigvr, gmm::real_part(const_cast<VECT &>(eigval_)));	   \
    gmm::copy(eigvi, gmm::imag_part(const_cast<VECT &>(eigval_)));	   \
  }

# define geev_int_rightc(lapack_name, base_type)			   \
  template <typename VECT> inline void geev_interface_right(               \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q) {		                           \
    GMMLAPACK_TRACE("geev_interface");                                     \
    int n = int(mat_nrows(A)), info, lwork(-1); base_type work1;           \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvl = 'N', jobvr = 'V';                                         \
    std::vector<double> rwork(2*n);                                        \
    std::vector<base_type> eigv(n);                                        \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigv[0], &Q(0,0), &n,    \
		&Q(0,0), &n, &work1, &lwork, &rwork[0], &info);	   	   \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigv[0], &Q(0,0), &n,    \
		&Q(0,0), &n, &work[0], &lwork,  &rwork[0],  &info);	   \
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    gmm::copy(eigv, const_cast<VECT &>(eigval_));			   \
  }

  geev_int_right(sgeev_, BLAS_S)
  geev_int_right(dgeev_, BLAS_D)
  geev_int_rightc(cgeev_, BLAS_C)
  geev_int_rightc(zgeev_, BLAS_Z)

# define geev_int_left(lapack_name, base_type)                             \
  template <typename VECT> inline void geev_interface_left(                \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q) {	 	                           \
    GMMLAPACK_TRACE("geev_interface");                                     \
    int n = int(mat_nrows(A)), info, lwork(-1); base_type work1;           \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvl = 'V', jobvr = 'N';                                         \
    std::vector<base_type > eigvr(n), eigvi(n);                            \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigvr[0], &eigvi[0],     \
		&Q(0,0), &n, &Q(0,0), &n, &work1, &lwork, &info);   	   \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigvr[0], &eigvi[0],     \
		&Q(0,0), &n, &Q(0,0), &n, &work[0], &lwork, &info);    	   \
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    gmm::copy(eigvr, gmm::real_part(const_cast<VECT &>(eigval_)));	   \
    gmm::copy(eigvi, gmm::imag_part(const_cast<VECT &>(eigval_)));	   \
  }

# define geev_int_leftc(lapack_name, base_type)	  		           \
  template <typename VECT> inline void geev_interface_left(                \
         const dense_matrix<base_type > &A,  const VECT &eigval_,          \
         dense_matrix<base_type > &Q) {		                           \
    GMMLAPACK_TRACE("geev_interface");                                     \
    int n = int(mat_nrows(A)), info, lwork(-1); base_type work1;           \
    if (!n) return;                                                        \
    dense_matrix<base_type > H(n,n); gmm::copy(A, H);                      \
    char jobvl = 'V', jobvr = 'N';                                         \
    std::vector<double> rwork(2*n);                                        \
    std::vector<base_type> eigv(n);                                        \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigv[0], &Q(0,0), &n,    \
		&Q(0,0), &n, &work1, &lwork, &rwork[0], &info);	   	   \
    lwork = int(gmm::real(work1));                                         \
    std::vector<base_type > work(lwork);                                   \
    lapack_name(&jobvl, &jobvr, &n, &H(0,0), &n, &eigv[0], &Q(0,0), &n,    \
		&Q(0,0), &n, &work[0], &lwork,  &rwork[0],  &info);	   \
    GMM_ASSERT1(!info, "QR algorithm failed");                             \
    gmm::copy(eigv, const_cast<VECT &>(eigval_));			   \
  }

  geev_int_left(sgeev_, BLAS_S)
  geev_int_left(dgeev_, BLAS_D)
  geev_int_leftc(cgeev_, BLAS_C)
  geev_int_leftc(zgeev_, BLAS_Z) 
    


  /* ********************************************************************* */
  /* Interface to SVD. Does not correspond to a Gmm++ functionnality.      */
  /* Author : Sebastian Nowozin <sebastian.nowozin@tuebingen.mpg.de>       */
  /* ********************************************************************* */
    
# define gesvd_interface(lapack_name, base_type) inline                 \
  void svd(dense_matrix<base_type> &X,					\
	   dense_matrix<base_type> &U,                                  \
           dense_matrix<base_type> &Vtransposed,			\
	   std::vector<base_type> &sigma) {				\
    GMMLAPACK_TRACE("gesvd_interface");					\
    int m = int(mat_nrows(X)), n = int(mat_ncols(X));	       		\
    int mn_min = m < n ? m : n;						\
    sigma.resize(mn_min);						\
    std::vector<base_type> work(15 * mn_min);				\
    int lwork = int(work.size());	       				\
    resize(U, m, n);							\
    resize(Vtransposed, n, n);						\
    char job = 'A';							\
    int info = -1;							\
    lapack_name(&job, &job, &m, &n, &X(0,0), &m, &sigma[0], &U(0,0),	\
		&m, &Vtransposed(0,0), &n, &work[0], &lwork, &info);	\
  }

# define cgesvd_interface(lapack_name, base_type, base_type2) inline    \
  void svd(dense_matrix<base_type> &X,					\
	   dense_matrix<base_type> &U,                                  \
           dense_matrix<base_type> &Vtransposed,			\
	   std::vector<base_type2> &sigma) {				\
    GMMLAPACK_TRACE("gesvd_interface");					\
    int m = int(mat_nrows(X)), n = int(mat_ncols(X));			\
    int mn_min = m < n ? m : n;						\
    sigma.resize(mn_min);						\
    std::vector<base_type> work(15 * mn_min);				\
    std::vector<base_type2> rwork(5 * mn_min);				\
    int lwork = int(work.size());			       		\
    resize(U, m, n);							\
    resize(Vtransposed, n, n);						\
    char job = 'A';							\
    int info = -1;							\
    lapack_name(&job, &job, &m, &n, &X(0,0), &m, &sigma[0], &U(0,0),	\
		&m, &Vtransposed(0,0), &n, &work[0], &lwork,            \
		&rwork[0], &info);					\
  }
  
  gesvd_interface(sgesvd_, BLAS_S)
  gesvd_interface(dgesvd_, BLAS_D)
  cgesvd_interface(cgesvd_, BLAS_C, BLAS_S)
  cgesvd_interface(zgesvd_, BLAS_Z, BLAS_D)
    
    



}

#endif // GMM_LAPACK_INTERFACE_H

#endif // GMM_USES_LAPACK
