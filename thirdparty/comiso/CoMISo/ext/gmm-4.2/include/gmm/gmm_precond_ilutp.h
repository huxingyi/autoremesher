/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2004-2012 Yves Renard
 
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

/**@file gmm_precond_ilutp.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 14, 2004.
   @brief ILUTP: Incomplete LU with threshold and K fill-in Preconditioner and
   column pivoting.

   
*/
#ifndef GMM_PRECOND_ILUTP_H
#define GMM_PRECOND_ILUTP_H

#include "gmm_precond_ilut.h"

namespace gmm {

  /**
     ILUTP: Incomplete LU with threshold and K fill-in Preconditioner and
     column pivoting.
   
     See Yousef Saad, Iterative Methods for
     sparse linear systems, PWS Publishing Company, section 10.4.4

      TODO : store the permutation by cycles to avoid the temporary vector
  */
  template <typename Matrix>
  class ilutp_precond  {
  public :
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef wsvector<value_type> _wsvector;
    typedef rsvector<value_type> _rsvector;
    typedef row_matrix<_rsvector> LU_Matrix;
    typedef col_matrix<_wsvector> CLU_Matrix;

    bool invert;
    LU_Matrix L, U;
    gmm::unsorted_sub_index indperm;
    gmm::unsorted_sub_index indperminv;
    mutable std::vector<value_type> temporary;

  protected:
    size_type K;
    double eps;

    template<typename M> void do_ilutp(const M&, row_major);
    void do_ilutp(const Matrix&, col_major);

  public:
    void build_with(const Matrix& A, int k_ = -1, double eps_ = double(-1)) {
      if (k_ >= 0) K = k_;
      if (eps_ >= double(0)) eps = eps_;
      invert = false;
      gmm::resize(L, mat_nrows(A), mat_ncols(A));
      gmm::resize(U, mat_nrows(A), mat_ncols(A));
      do_ilutp(A, typename principal_orientation_type<typename
	      linalg_traits<Matrix>::sub_orientation>::potype());
    }
    ilutp_precond(const Matrix& A, size_type k_, double eps_) 
      : L(mat_nrows(A), mat_ncols(A)), U(mat_nrows(A), mat_ncols(A)),
	K(k_), eps(eps_) { build_with(A); }
    ilutp_precond(int k_, double eps_) :  K(k_), eps(eps_) {}
    ilutp_precond(void) { K = 10; eps = 1E-7; }
    size_type memsize() const { 
      return sizeof(*this) + (nnz(U)+nnz(L))*sizeof(value_type);
    }
  };


  template<typename Matrix> template<typename M> 
  void ilutp_precond<Matrix>::do_ilutp(const M& A, row_major) {
    typedef value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type n = mat_nrows(A);
    CLU_Matrix CU(n,n);
    if (n == 0) return;
    std::vector<T> indiag(n);
    temporary.resize(n);
    std::vector<size_type> ipvt(n), ipvtinv(n);
    for (size_type i = 0; i < n; ++i) ipvt[i] = ipvtinv[i] = i;
    indperm = unsorted_sub_index(ipvt);
    indperminv = unsorted_sub_index(ipvtinv);
    _wsvector w(mat_ncols(A));
    _rsvector ww(mat_ncols(A));
    
    T tmp = T(0);
    gmm::clear(L); gmm::clear(U);
    R prec = default_tol(R()); 
    R max_pivot = gmm::abs(A(0,0)) * prec;

    for (size_type i = 0; i < n; ++i) {

      copy(sub_vector(mat_const_row(A, i), indperm), w);
      double norm_row = gmm::vect_norm2(mat_const_row(A, i)); 

      typename _wsvector::iterator wkold = w.end();
      for (typename _wsvector::iterator wk = w.begin();
	   wk != w.end() && wk->first < i; )  {
	size_type k = wk->first;
	tmp = (wk->second) * indiag[k];
	if (gmm::abs(tmp) < eps * norm_row) w.erase(k); 
	else { wk->second += tmp; gmm::add(scaled(mat_row(U, k), -tmp), w); }
	if (wkold == w.end()) wk = w.begin(); else { wk = wkold; ++wk; }
	if (wk != w.end() && wk->first == k)
	  { if (wkold == w.end()) wkold = w.begin(); else ++wkold; ++wk; }
      }

      gmm::clean(w, eps * norm_row);
      gmm::copy(w, ww);

      std::sort(ww.begin(), ww.end(), elt_rsvector_value_less_<T>());
      typename _rsvector::const_iterator wit = ww.begin(), wite = ww.end();
      size_type ip = size_type(-1);

      for (; wit != wite; ++wit)
	if (wit->c >= i) { ip = wit->c; tmp = wit->e; break; }
      if (ip == size_type(-1) || gmm::abs(tmp) <= max_pivot)
	{ GMM_WARNING2("pivot " << i << " too small"); ip=i; ww[i]=tmp=T(1); }
      max_pivot = std::max(max_pivot, std::min(gmm::abs(tmp) * prec, R(1)));
      indiag[i] = T(1) / tmp;
      wit = ww.begin();

      size_type nnl = 0, nnu = 0;
      L[i].base_resize(K); U[i].base_resize(K+1);
      typename _rsvector::iterator witL = L[i].begin(), witU = U[i].begin();
      for (; wit != wite; ++wit) {
	if (wit->c < i) { if (nnl < K) { *witL++ = *wit; ++nnl; } }
	else if (nnu < K || wit->c == i)
	  { CU(i, wit->c) = wit->e; *witU++ = *wit; ++nnu; }
      }
      L[i].base_resize(nnl); U[i].base_resize(nnu);
      std::sort(L[i].begin(), L[i].end());
      std::sort(U[i].begin(), U[i].end());

      if (ip != i) {
	typename _wsvector::const_iterator iti = CU.col(i).begin();
	typename _wsvector::const_iterator itie = CU.col(i).end();
	typename _wsvector::const_iterator itp = CU.col(ip).begin();
	typename _wsvector::const_iterator itpe = CU.col(ip).end();
	
	while (iti != itie && itp != itpe) {
	  if (iti->first < itp->first)
	    { U.row(iti->first).swap_indices(i, ip); ++iti; }
	  else if (iti->first > itp->first)
	    { U.row(itp->first).swap_indices(i,ip);++itp; }
	  else
	    { U.row(iti->first).swap_indices(i, ip); ++iti; ++itp; }
	}
	
	for( ; iti != itie; ++iti) U.row(iti->first).swap_indices(i, ip);
	for( ; itp != itpe; ++itp) U.row(itp->first).swap_indices(i, ip);

	CU.swap_col(i, ip);
	
	indperm.swap(i, ip);
	indperminv.swap(ipvt[i], ipvt[ip]);
	std::swap(ipvtinv[ipvt[i]], ipvtinv[ipvt[ip]]);
	std::swap(ipvt[i], ipvt[ip]);
      }
    }
  }

  template<typename Matrix> 
  void ilutp_precond<Matrix>::do_ilutp(const Matrix& A, col_major) {
    do_ilutp(gmm::transposed(A), row_major());
    invert = true;
  }

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const ilutp_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    if (P.invert) {
      gmm::copy(gmm::sub_vector(v1, P.indperm), v2);
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
    else {
      gmm::copy(v1, P.temporary);
      gmm::lower_tri_solve(P.L, P.temporary, true);
      gmm::upper_tri_solve(P.U, P.temporary, false);
      gmm::copy(gmm::sub_vector(P.temporary, P.indperminv), v2);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const ilutp_precond<Matrix>& P,const V1 &v1,V2 &v2) {
    if (P.invert) {
      gmm::copy(v1, P.temporary);
      gmm::lower_tri_solve(P.L, P.temporary, true);
      gmm::upper_tri_solve(P.U, P.temporary, false);
      gmm::copy(gmm::sub_vector(P.temporary, P.indperminv), v2);
    }
    else {
      gmm::copy(gmm::sub_vector(v1, P.indperm), v2);
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void left_mult(const ilutp_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    if (P.invert) {
      gmm::copy(gmm::sub_vector(v1, P.indperm), v2);
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
    }
    else {
      copy(v1, v2);
      gmm::lower_tri_solve(P.L, v2, true);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void right_mult(const ilutp_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    if (P.invert) {
      copy(v1, v2);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
    else {
      copy(v1, P.temporary);
      gmm::upper_tri_solve(P.U, P.temporary, false);
      gmm::copy(gmm::sub_vector(P.temporary, P.indperminv), v2);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_left_mult(const ilutp_precond<Matrix>& P, const V1 &v1,
			    V2 &v2) {
    if (P.invert) {
      copy(v1, P.temporary);
      gmm::upper_tri_solve(P.U, P.temporary, false);
      gmm::copy(gmm::sub_vector(P.temporary, P.indperminv), v2);
    }
    else {
      copy(v1, v2);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
  }
  
  template <typename Matrix, typename V1, typename V2> inline
  void transposed_right_mult(const ilutp_precond<Matrix>& P, const V1 &v1,
			     V2 &v2) {
    if (P.invert) {
      copy(v1, v2);
      gmm::lower_tri_solve(P.L, v2, true);
    }
    else {
      gmm::copy(gmm::sub_vector(v1, P.indperm), v2);
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
    }
  }

}

#endif 

