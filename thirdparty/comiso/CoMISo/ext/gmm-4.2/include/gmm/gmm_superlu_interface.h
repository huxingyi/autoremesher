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

/**@file gmm_superlu_interface.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 17, 2003.
   @brief Interface with SuperLU (LU direct solver for sparse matrices).
*/
#if defined(GMM_USES_SUPERLU) && !defined(GETFEM_VERSION)

#ifndef GMM_SUPERLU_INTERFACE_H
#define GMM_SUPERLU_INTERFACE_H

#include "gmm_kernel.h"

typedef int int_t;

/* because SRC/util.h defines TRUE and FALSE ... */
#ifdef TRUE
# undef TRUE
#endif
#ifdef FALSE
# undef FALSE
#endif

#include "superlu/slu_Cnames.h"
#include "superlu/supermatrix.h"
#include "superlu/slu_util.h"

namespace SuperLU_S {
#include "superlu/slu_sdefs.h"
}
namespace SuperLU_D {
#include "superlu/slu_ddefs.h"
}
namespace SuperLU_C {
#include "superlu/slu_cdefs.h"
}
namespace SuperLU_Z {
#include "superlu/slu_zdefs.h" 
}



namespace gmm {

  /*  interface for Create_CompCol_Matrix */

  inline void Create_CompCol_Matrix(SuperMatrix *A, int m, int n, int nnz,
			     float *a, int *ir, int *jc) {
    SuperLU_S::sCreate_CompCol_Matrix(A, m, n, nnz, a, ir, jc,
				      SLU_NC, SLU_S, SLU_GE);
  }
  
  inline void Create_CompCol_Matrix(SuperMatrix *A, int m, int n, int nnz,
			     double *a, int *ir, int *jc) {
    SuperLU_D::dCreate_CompCol_Matrix(A, m, n, nnz, a, ir, jc,
				      SLU_NC, SLU_D, SLU_GE);
  }
  
  inline void Create_CompCol_Matrix(SuperMatrix *A, int m, int n, int nnz,
			     std::complex<float> *a, int *ir, int *jc) {
    SuperLU_C::cCreate_CompCol_Matrix(A, m, n, nnz, (SuperLU_C::complex *)(a),
				      ir, jc, SLU_NC, SLU_C, SLU_GE);
  }
  
  inline void Create_CompCol_Matrix(SuperMatrix *A, int m, int n, int nnz,
			     std::complex<double> *a, int *ir, int *jc) {
    SuperLU_Z::zCreate_CompCol_Matrix(A, m, n, nnz,
				      (SuperLU_Z::doublecomplex *)(a), ir, jc,
				      SLU_NC, SLU_Z, SLU_GE);
  }

  /*  interface for Create_Dense_Matrix */

  inline void Create_Dense_Matrix(SuperMatrix *A, int m, int n, float *a, int k)
  { SuperLU_S::sCreate_Dense_Matrix(A, m, n, a, k, SLU_DN, SLU_S, SLU_GE); }
  inline void Create_Dense_Matrix(SuperMatrix *A, int m, int n, double *a, int k)
  { SuperLU_D::dCreate_Dense_Matrix(A, m, n, a, k, SLU_DN, SLU_D, SLU_GE); }
  inline void Create_Dense_Matrix(SuperMatrix *A, int m, int n,
			   std::complex<float> *a, int k) {
    SuperLU_C::cCreate_Dense_Matrix(A, m, n, (SuperLU_C::complex *)(a),
				    k, SLU_DN, SLU_C, SLU_GE);
  }
  inline void Create_Dense_Matrix(SuperMatrix *A, int m, int n, 
			   std::complex<double> *a, int k) {
    SuperLU_Z::zCreate_Dense_Matrix(A, m, n, (SuperLU_Z::doublecomplex *)(a),
				    k, SLU_DN, SLU_Z, SLU_GE);
  }

  /*  interface for gssv */

#define DECL_GSSV(NAMESPACE,FNAME,FLOATTYPE,KEYTYPE) \
  inline void SuperLU_gssv(superlu_options_t *options, SuperMatrix *A, int *p, \
  int *q, SuperMatrix *L, SuperMatrix *U, SuperMatrix *B,               \
  SuperLUStat_t *stats, int *info, KEYTYPE) {                           \
  NAMESPACE::FNAME(options, A, p, q, L, U, B, stats, info);             \
  }

  DECL_GSSV(SuperLU_S,sgssv,float,float)
  DECL_GSSV(SuperLU_C,cgssv,float,std::complex<float>)
  DECL_GSSV(SuperLU_D,dgssv,double,double)
  DECL_GSSV(SuperLU_Z,zgssv,double,std::complex<double>)

  /*  interface for gssvx */

#define DECL_GSSVX(NAMESPACE,FNAME,FLOATTYPE,KEYTYPE) \
    inline float SuperLU_gssvx(superlu_options_t *options, SuperMatrix *A,	\
		     int *perm_c, int *perm_r, int *etree, char *equed,  \
		     FLOATTYPE *R, FLOATTYPE *C, SuperMatrix *L,         \
		     SuperMatrix *U, void *work, int lwork,              \
		     SuperMatrix *B, SuperMatrix *X,                     \
		     FLOATTYPE *recip_pivot_growth,                      \
		     FLOATTYPE *rcond, FLOATTYPE *ferr, FLOATTYPE *berr, \
		     SuperLUStat_t *stats, int *info, KEYTYPE) {         \
    NAMESPACE::mem_usage_t mem_usage;                                    \
    NAMESPACE::FNAME(options, A, perm_c, perm_r, etree, equed, R, C, L,  \
		     U, work, lwork, B, X, recip_pivot_growth, rcond,    \
		     ferr, berr, &mem_usage, stats, info);               \
    return mem_usage.for_lu; /* bytes used by the factor storage */     \
  }

  DECL_GSSVX(SuperLU_S,sgssvx,float,float)
  DECL_GSSVX(SuperLU_C,cgssvx,float,std::complex<float>)
  DECL_GSSVX(SuperLU_D,dgssvx,double,double)
  DECL_GSSVX(SuperLU_Z,zgssvx,double,std::complex<double>)

  /* ********************************************************************* */
  /*   SuperLU solve interface                                             */
  /* ********************************************************************* */

  template <typename MAT, typename VECTX, typename VECTB>
  int SuperLU_solve(const MAT &A, const VECTX &X_, const VECTB &B,
		     double& rcond_, int permc_spec = 3) {
    VECTX &X = const_cast<VECTX &>(X_);
    /*
     * Get column permutation vector perm_c[], according to permc_spec:
     *   permc_spec = 0: use the natural ordering 
     *   permc_spec = 1: use minimum degree ordering on structure of A'*A
     *   permc_spec = 2: use minimum degree ordering on structure of A'+A
     *   permc_spec = 3: use approximate minimum degree column ordering
     */
    typedef typename linalg_traits<MAT>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    int m = mat_nrows(A), n = mat_ncols(A), nrhs = 1, info = 0;

    csc_matrix<T> csc_A(m, n); gmm::copy(A, csc_A);
    std::vector<T> rhs(m), sol(m);
    gmm::copy(B, rhs);

    int nz = nnz(csc_A);
    if ((2 * nz / n) >= m)
      GMM_WARNING2("CAUTION : it seems that SuperLU has a problem"
		  " for nearly dense sparse matrices");

    superlu_options_t options;
    set_default_options(&options);
    options.ColPerm = NATURAL;
    options.PrintStat = NO;
    options.ConditionNumber = YES;
    switch (permc_spec) {
    case 1 : options.ColPerm = MMD_ATA; break;
    case 2 : options.ColPerm = MMD_AT_PLUS_A; break;
    case 3 : options.ColPerm = COLAMD; break;
    }
    SuperLUStat_t stat;
    StatInit(&stat);

    SuperMatrix SA, SL, SU, SB, SX; // SuperLU format.
    Create_CompCol_Matrix(&SA, m, n, nz, (double *)(&(csc_A.pr[0])),
			  (int *)(&(csc_A.ir[0])), (int *)(&(csc_A.jc[0])));
    Create_Dense_Matrix(&SB, m, nrhs, &rhs[0], m);
    Create_Dense_Matrix(&SX, m, nrhs, &sol[0], m);
    memset(&SL,0,sizeof SL);
    memset(&SU,0,sizeof SU);

    std::vector<int> etree(n);
    char equed[] = "B";
    std::vector<R> Rscale(m),Cscale(n); // row scale factors
    std::vector<R> ferr(nrhs), berr(nrhs);
    R recip_pivot_gross, rcond;
    std::vector<int> perm_r(m), perm_c(n);

    SuperLU_gssvx(&options, &SA, &perm_c[0], &perm_r[0], 
		  &etree[0] /* output */, equed /* output         */, 
		  &Rscale[0] /* row scale factors (output)        */, 
		  &Cscale[0] /* col scale factors (output)        */,
		  &SL /* fact L (output)*/, &SU /* fact U (output)*/, 
		  NULL /* work                                    */, 
		  0 /* lwork: superlu auto allocates (input)      */, 
		  &SB /* rhs */, &SX /* solution                  */,
		  &recip_pivot_gross /* reciprocal pivot growth   */
		  /* factor max_j( norm(A_j)/norm(U_j) ).         */,  
		  &rcond /*estimate of the reciprocal condition   */
		  /* number of the matrix A after equilibration   */,
		  &ferr[0] /* estimated forward error             */,
		  &berr[0] /* relative backward error             */,
		  &stat, &info, T());
    rcond_ = rcond;
    Destroy_SuperMatrix_Store(&SB);
    Destroy_SuperMatrix_Store(&SX);
    Destroy_SuperMatrix_Store(&SA);
    Destroy_SuperNode_Matrix(&SL);
    Destroy_CompCol_Matrix(&SU);
    StatFree(&stat);
    GMM_ASSERT1(info >= 0, "SuperLU solve failed: info =" << info);
    if (info > 0) GMM_WARNING1("SuperLU solve failed: info =" << info);
    gmm::copy(sol, X);
    return info;
  }

  template <class T> class SuperLU_factor {
    typedef typename number_traits<T>::magnitude_type R;

    csc_matrix<T> csc_A;
    mutable SuperMatrix SA, SL, SB, SU, SX;
    mutable SuperLUStat_t stat;
    mutable superlu_options_t options;
    float memory_used;
    mutable std::vector<int> etree, perm_r, perm_c;
    mutable std::vector<R> Rscale, Cscale;
    mutable std::vector<R> ferr, berr;
    mutable std::vector<T> rhs;
    mutable std::vector<T> sol;
    mutable bool is_init;
    mutable char equed;

  public :
    enum { LU_NOTRANSP, LU_TRANSP, LU_CONJUGATED };
    void free_supermatrix(void);
    template <class MAT> void build_with(const MAT &A,  int permc_spec = 3);
    template <typename VECTX, typename VECTB> 
    /* transp = LU_NOTRANSP   -> solves Ax = B
       transp = LU_TRANSP     -> solves A'x = B
       transp = LU_CONJUGATED -> solves conj(A)X = B */
    void solve(const VECTX &X_, const VECTB &B, int transp=LU_NOTRANSP) const;
    SuperLU_factor(void) { is_init = false; }
    SuperLU_factor(const SuperLU_factor& other) {
      GMM_ASSERT2(!(other.is_init),
		 "copy of initialized SuperLU_factor is forbidden");
      is_init = false;
    }
    SuperLU_factor& operator=(const SuperLU_factor& other) {
      GMM_ASSERT2(!(other.is_init) && !is_init,
		  "assignment of initialized SuperLU_factor is forbidden");
      return *this;
    }
    ~SuperLU_factor() { free_supermatrix(); }
    float memsize() { return memory_used; }
  };


  template <class T> void SuperLU_factor<T>::free_supermatrix(void) {
      if (is_init) {
	if (SB.Store) Destroy_SuperMatrix_Store(&SB);
	if (SX.Store) Destroy_SuperMatrix_Store(&SX);
	if (SA.Store) Destroy_SuperMatrix_Store(&SA);
	if (SL.Store) Destroy_SuperNode_Matrix(&SL);
	if (SU.Store) Destroy_CompCol_Matrix(&SU);
      }
    }

    
    template <class T> template <class MAT>
    void SuperLU_factor<T>::build_with(const MAT &A,  int permc_spec) {
    /*
     * Get column permutation vector perm_c[], according to permc_spec:
     *   permc_spec = 0: use the natural ordering 
     *   permc_spec = 1: use minimum degree ordering on structure of A'*A
     *   permc_spec = 2: use minimum degree ordering on structure of A'+A
     *   permc_spec = 3: use approximate minimum degree column ordering
     */
      free_supermatrix();
      int n = mat_nrows(A), m = mat_ncols(A), info = 0;
      csc_A.init_with(A);

      rhs.resize(m); sol.resize(m);
      gmm::clear(rhs);
      int nz = nnz(csc_A);

      set_default_options(&options);
      options.ColPerm = NATURAL;
      options.PrintStat = NO;
      options.ConditionNumber = NO;
      switch (permc_spec) {
      case 1 : options.ColPerm = MMD_ATA; break;
      case 2 : options.ColPerm = MMD_AT_PLUS_A; break;
      case 3 : options.ColPerm = COLAMD; break;
      }
      StatInit(&stat);

      Create_CompCol_Matrix(&SA, m, n, nz, (double *)(&(csc_A.pr[0])),
			    (int *)(&(csc_A.ir[0])), (int *)(&(csc_A.jc[0])));

      Create_Dense_Matrix(&SB, m, 0, &rhs[0], m);
      Create_Dense_Matrix(&SX, m, 0, &sol[0], m);
      memset(&SL,0,sizeof SL);
      memset(&SU,0,sizeof SU);
      equed = 'B';
      Rscale.resize(m); Cscale.resize(n); etree.resize(n);
      ferr.resize(1); berr.resize(1);
      R recip_pivot_gross, rcond;
      perm_r.resize(m); perm_c.resize(n);
      memory_used = SuperLU_gssvx(&options, &SA, &perm_c[0], &perm_r[0], 
		    &etree[0] /* output */, &equed /* output        */, 
		    &Rscale[0] /* row scale factors (output)        */, 
		    &Cscale[0] /* col scale factors (output)        */,
		    &SL /* fact L (output)*/, &SU /* fact U (output)*/, 
		    NULL /* work                                    */, 
		    0 /* lwork: superlu auto allocates (input)      */, 
		    &SB /* rhs */, &SX /* solution                  */,
		    &recip_pivot_gross /* reciprocal pivot growth   */
		    /* factor max_j( norm(A_j)/norm(U_j) ).         */,  
		    &rcond /*estimate of the reciprocal condition   */
		    /* number of the matrix A after equilibration   */,
		    &ferr[0] /* estimated forward error             */,
		    &berr[0] /* relative backward error             */,
		    &stat, &info, T());
      
      Destroy_SuperMatrix_Store(&SB);
      Destroy_SuperMatrix_Store(&SX);
      Create_Dense_Matrix(&SB, m, 1, &rhs[0], m);
      Create_Dense_Matrix(&SX, m, 1, &sol[0], m);
      StatFree(&stat);

      GMM_ASSERT1(info == 0, "SuperLU solve failed: info=" << info);
      is_init = true;
    }
    
    template <class T> template <typename VECTX, typename VECTB> 
    void SuperLU_factor<T>::solve(const VECTX &X_, const VECTB &B,
				  int transp) const {
      VECTX &X = const_cast<VECTX &>(X_);
      gmm::copy(B, rhs);
      options.Fact = FACTORED;
      options.IterRefine = NOREFINE;
      switch (transp) {
      case LU_NOTRANSP: options.Trans = NOTRANS; break;
      case LU_TRANSP: options.Trans = TRANS; break;
      case LU_CONJUGATED: options.Trans = CONJ; break;
      default: GMM_ASSERT1(false, "invalid value for transposition option");
      }
      StatInit(&stat);
      int info = 0;
      R recip_pivot_gross, rcond;
      SuperLU_gssvx(&options, &SA, &perm_c[0], &perm_r[0], 
		    &etree[0] /* output */, &equed /* output        */, 
		    &Rscale[0] /* row scale factors (output)        */, 
		    &Cscale[0] /* col scale factors (output)        */,
		    &SL /* fact L (output)*/, &SU /* fact U (output)*/, 
		    NULL /* work                                    */, 
		    0 /* lwork: superlu auto allocates (input)      */, 
		    &SB /* rhs */, &SX /* solution                  */,
		    &recip_pivot_gross /* reciprocal pivot growth   */
		    /* factor max_j( norm(A_j)/norm(U_j) ).         */,  
		    &rcond /*estimate of the reciprocal condition   */
		    /* number of the matrix A after equilibration   */,
		    &ferr[0] /* estimated forward error             */,
		    &berr[0] /* relative backward error             */,
		    &stat, &info, T());
     StatFree(&stat);
     GMM_ASSERT1(info == 0, "SuperLU solve failed: info=" << info);
     gmm::copy(sol, X);
    }

  template <typename T, typename V1, typename V2> inline
  void mult(const SuperLU_factor<T>& P, const V1 &v1, const V2 &v2) {
    P.solve(v2,v1);
  }

  template <typename T, typename V1, typename V2> inline
  void transposed_mult(const SuperLU_factor<T>& P,const V1 &v1,const V2 &v2) {
    P.solve(v2, v1, SuperLU_factor<T>::LU_TRANSP);
  }

}

  
#endif // GMM_SUPERLU_INTERFACE_H

#endif // GMM_USES_SUPERLU
