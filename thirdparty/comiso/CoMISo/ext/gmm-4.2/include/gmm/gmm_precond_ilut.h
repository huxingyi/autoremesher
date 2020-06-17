/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2002-2012 Yves Renard
 
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

// This file is a modified version of ilut.h from ITL.
// See http://osl.iu.edu/research/itl/
// Following the corresponding Copyright notice.
//===========================================================================
//
// Copyright (c) 1998-2001, University of Notre Dame. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Notre Dame nor the
//      names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE  IS  PROVIDED  BY  THE TRUSTEES  OF  INDIANA UNIVERSITY  AND
// CONTRIBUTORS  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS
// FOR  A PARTICULAR PURPOSE ARE DISCLAIMED. IN  NO  EVENT SHALL THE TRUSTEES
// OF INDIANA UNIVERSITY AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES (INCLUDING,  BUT
// NOT  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA,  OR PROFITS;  OR BUSINESS  INTERRUPTION)  HOWEVER  CAUSED AND ON ANY
// THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT  LIABILITY, OR TORT
// (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS  SOFTWARE,  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//===========================================================================

#ifndef GMM_PRECOND_ILUT_H
#define GMM_PRECOND_ILUT_H

/**@file gmm_precond_ilut.h
   @author  Andrew Lumsdaine <lums@osl.iu.edu>, Lie-Quan Lee <llee@osl.iu.edu>
   @date June 5, 2003.
   @brief ILUT:  Incomplete LU with threshold and K fill-in Preconditioner.
*/

/*
  Performane comparing for SSOR, ILU and ILUT based on sherman 5 matrix 
  in Harwell-Boeing collection on Sun Ultra 30 UPA/PCI (UltraSPARC-II 296MHz)
  Preconditioner & Factorization time  &  Number of Iteration \\ \hline
  SSOR        &   0.010577  & 41 \\
  ILU         &   0.019336  & 32 \\
  ILUT with 0 fill-in and threshold of 1.0e-6 & 0.343612 &  23 \\
  ILUT with 5 fill-in and threshold of 1.0e-6 & 0.343612 &  18 \\ \hline
*/

#include "gmm_precond.h"

namespace gmm {

  template<typename T> struct elt_rsvector_value_less_ {
    inline bool operator()(const elt_rsvector_<T>& a, 
			   const elt_rsvector_<T>& b) const
    { return (gmm::abs(a.e) > gmm::abs(b.e)); }
  };

  /** Incomplete LU with threshold and K fill-in Preconditioner.

  The algorithm of ILUT(A, 0, 1.0e-6) is slower than ILU(A). If No
  fill-in is arrowed, you can use ILU instead of ILUT.

  Notes: The idea under a concrete Preconditioner such as ilut is to
  create a Preconditioner object to use in iterative methods.
  */
  template <typename Matrix>
  class ilut_precond  {
  public :
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef wsvector<value_type> _wsvector;
    typedef rsvector<value_type> _rsvector;
    typedef row_matrix<_rsvector> LU_Matrix;

    bool invert;
    LU_Matrix L, U;

  protected:
    size_type K;
    double eps;    

    template<typename M> void do_ilut(const M&, row_major);
    void do_ilut(const Matrix&, col_major);

  public:
    void build_with(const Matrix& A, int k_ = -1, double eps_ = double(-1)) {
      if (k_ >= 0) K = k_;
      if (eps_ >= double(0)) eps = eps_;
      invert = false;
      gmm::resize(L, mat_nrows(A), mat_ncols(A));
      gmm::resize(U, mat_nrows(A), mat_ncols(A));
      do_ilut(A, typename principal_orientation_type<typename
	      linalg_traits<Matrix>::sub_orientation>::potype());
    }
    ilut_precond(const Matrix& A, int k_, double eps_) 
      : L(mat_nrows(A), mat_ncols(A)), U(mat_nrows(A), mat_ncols(A)),
	K(k_), eps(eps_) { build_with(A); }
    ilut_precond(size_type k_, double eps_) :  K(k_), eps(eps_) {}
    ilut_precond(void) { K = 10; eps = 1E-7; }
    size_type memsize() const { 
      return sizeof(*this) + (nnz(U)+nnz(L))*sizeof(value_type);
    }
  };

  template<typename Matrix> template<typename M> 
  void ilut_precond<Matrix>::do_ilut(const M& A, row_major) {
    typedef value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    
    size_type n = mat_nrows(A);
    if (n == 0) return;
    std::vector<T> indiag(n);
    _wsvector w(mat_ncols(A));
    _rsvector ww(mat_ncols(A)), wL(mat_ncols(A)), wU(mat_ncols(A));
    T tmp;
    gmm::clear(U); gmm::clear(L);
    R prec = default_tol(R()); 
    R max_pivot = gmm::abs(A(0,0)) * prec;

    for (size_type i = 0; i < n; ++i) {
      gmm::copy(mat_const_row(A, i), w);
      double norm_row = gmm::vect_norm2(w);

      typename _wsvector::iterator wkold = w.end();
      for (typename _wsvector::iterator wk = w.begin();
	   wk != w.end() && wk->first < i; ) {
	size_type k = wk->first;
	tmp = (wk->second) * indiag[k];
	if (gmm::abs(tmp) < eps * norm_row) w.erase(k);
	else { wk->second += tmp; gmm::add(scaled(mat_row(U, k), -tmp), w); }
	if (wkold == w.end()) wk = w.begin(); else { wk = wkold; ++wk; }
	if (wk != w.end() && wk->first == k)
	  { if (wkold == w.end()) wkold = w.begin(); else ++wkold; ++wk; }
      }
      tmp = w[i];

      if (gmm::abs(tmp) <= max_pivot) {
	GMM_WARNING2("pivot " << i << " too small. try with ilutp ?");
	w[i] = tmp = T(1);
      }

      max_pivot = std::max(max_pivot, std::min(gmm::abs(tmp) * prec, R(1)));
      indiag[i] = T(1) / tmp;
      gmm::clean(w, eps * norm_row);
      gmm::copy(w, ww);
      std::sort(ww.begin(), ww.end(), elt_rsvector_value_less_<T>());
      typename _rsvector::const_iterator wit = ww.begin(), wite = ww.end();

      size_type nnl = 0, nnu = 0;    
      wL.base_resize(K); wU.base_resize(K+1);
      typename _rsvector::iterator witL = wL.begin(), witU = wU.begin();
      for (; wit != wite; ++wit) 
	if (wit->c < i) { if (nnl < K) { *witL++ = *wit; ++nnl; } }
	else { if (nnu < K  || wit->c == i) { *witU++ = *wit; ++nnu; } }
      wL.base_resize(nnl); wU.base_resize(nnu);
      std::sort(wL.begin(), wL.end());
      std::sort(wU.begin(), wU.end());
      gmm::copy(wL, L.row(i));
      gmm::copy(wU, U.row(i));
    }

  }

  template<typename Matrix> 
  void ilut_precond<Matrix>::do_ilut(const Matrix& A, col_major) {
    do_ilut(gmm::transposed(A), row_major());
    invert = true;
  }

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const ilut_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    gmm::copy(v1, v2);
    if (P.invert) {
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
    else {
      gmm::lower_tri_solve(P.L, v2, true);
      gmm::upper_tri_solve(P.U, v2, false);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const ilut_precond<Matrix>& P,const V1 &v1,V2 &v2) {
    gmm::copy(v1, v2);
    if (P.invert) {
      gmm::lower_tri_solve(P.L, v2, true);
      gmm::upper_tri_solve(P.U, v2, false);
    }
    else {
      gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
      gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    }
  }

  template <typename Matrix, typename V1, typename V2> inline
  void left_mult(const ilut_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    copy(v1, v2);
    if (P.invert) gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
    else gmm::lower_tri_solve(P.L, v2, true);
  }

  template <typename Matrix, typename V1, typename V2> inline
  void right_mult(const ilut_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    copy(v1, v2);
    if (P.invert) gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
    else gmm::upper_tri_solve(P.U, v2, false);
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_left_mult(const ilut_precond<Matrix>& P, const V1 &v1,
			    V2 &v2) {
    copy(v1, v2);
    if (P.invert) gmm::upper_tri_solve(P.U, v2, false);
    else gmm::upper_tri_solve(gmm::transposed(P.L), v2, true);
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_right_mult(const ilut_precond<Matrix>& P, const V1 &v1,
			     V2 &v2) {
    copy(v1, v2);
    if (P.invert) gmm::lower_tri_solve(P.L, v2, true);
    else gmm::lower_tri_solve(gmm::transposed(P.U), v2, false);
  }

}

#endif 

