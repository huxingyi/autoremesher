/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard, Caroline Lecalvez
 
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

/**@file gmm_dense_Householder.h
   @author Caroline Lecalvez <Caroline.Lecalvez@gmm.insa-toulouse.fr>
   @author Yves Renard <Yves.Renard@insa-lyon.fr>
   @date June 5, 2003.
   @brief Householder for dense matrices.
*/

#ifndef GMM_DENSE_HOUSEHOLDER_H
#define GMM_DENSE_HOUSEHOLDER_H

#include "gmm_kernel.h"

namespace gmm {
  ///@cond DOXY_SHOW_ALL_FUNCTIONS

  /* ********************************************************************* */
  /*    Rank one update  (complex and real version)                        */
  /* ********************************************************************* */

  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_one_update(Matrix &A, const VecX& x,
			      const VecY& y, row_major) {
    typedef typename linalg_traits<Matrix>::value_type T;
    size_type N = mat_nrows(A);
    GMM_ASSERT2(N <= vect_size(x) && mat_ncols(A) <= vect_size(y),
	       "dimensions mismatch");
    typename linalg_traits<VecX>::const_iterator itx = vect_const_begin(x);
    for (size_type i = 0; i < N; ++i, ++itx) {
      typedef typename linalg_traits<Matrix>::sub_row_type row_type;
      row_type row = mat_row(A, i);
      typename linalg_traits<row_type>::iterator
	it = vect_begin(row), ite = vect_end(row);
      typename linalg_traits<VecY>::const_iterator ity = vect_const_begin(y);
      T tx = *itx;
      for (; it != ite; ++it, ++ity) *it += conj_product(*ity, tx);
    }
  }

  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_one_update(Matrix &A, const VecX& x,
			      const VecY& y, col_major) {
    typedef typename linalg_traits<Matrix>::value_type T;
    size_type M = mat_ncols(A);
    GMM_ASSERT2(mat_nrows(A) <= vect_size(x) && M <= vect_size(y),
		"dimensions mismatch");
    typename linalg_traits<VecY>::const_iterator ity = vect_const_begin(y);
    for (size_type i = 0; i < M; ++i, ++ity) {
      typedef typename linalg_traits<Matrix>::sub_col_type col_type;
      col_type col = mat_col(A, i);
      typename linalg_traits<col_type>::iterator
	it = vect_begin(col), ite = vect_end(col);
      typename linalg_traits<VecX>::const_iterator itx = vect_const_begin(x);
      T ty = *ity;
      for (; it != ite; ++it, ++itx) *it += conj_product(ty, *itx);
    }
  }
  
  ///@endcond
  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_one_update(const Matrix &AA, const VecX& x,
			      const VecY& y) {
    Matrix& A = const_cast<Matrix&>(AA);
    rank_one_update(A, x, y, typename principal_orientation_type<typename
		    linalg_traits<Matrix>::sub_orientation>::potype());
  }
  ///@cond DOXY_SHOW_ALL_FUNCTIONS

  /* ********************************************************************* */
  /*    Rank two update  (complex and real version)                        */
  /* ********************************************************************* */

  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_two_update(Matrix &A, const VecX& x,
			      const VecY& y, row_major) {
    typedef typename linalg_traits<Matrix>::value_type value_type;
    size_type N = mat_nrows(A);
    GMM_ASSERT2(N <= vect_size(x) && mat_ncols(A) <= vect_size(y),
		"dimensions mismatch");
    typename linalg_traits<VecX>::const_iterator itx1 = vect_const_begin(x);
    typename linalg_traits<VecY>::const_iterator ity2 = vect_const_begin(y);
    for (size_type i = 0; i < N; ++i, ++itx1, ++ity2) {
      typedef typename linalg_traits<Matrix>::sub_row_type row_type;
      row_type row = mat_row(A, i);
      typename linalg_traits<row_type>::iterator
	it = vect_begin(row), ite = vect_end(row);
      typename linalg_traits<VecX>::const_iterator itx2 = vect_const_begin(x);
      typename linalg_traits<VecY>::const_iterator ity1 = vect_const_begin(y);
      value_type tx = *itx1, ty = *ity2;
      for (; it != ite; ++it, ++ity1, ++itx2)
	*it += conj_product(*ity1, tx) + conj_product(*itx2, ty);
    }
  }

  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_two_update(Matrix &A, const VecX& x,
			      const VecY& y, col_major) {
    typedef typename linalg_traits<Matrix>::value_type value_type;
    size_type M = mat_ncols(A);
    GMM_ASSERT2(mat_nrows(A) <= vect_size(x) && M <= vect_size(y),
		"dimensions mismatch");
    typename linalg_traits<VecX>::const_iterator itx2 = vect_const_begin(x);
    typename linalg_traits<VecY>::const_iterator ity1 = vect_const_begin(y);
    for (size_type i = 0; i < M; ++i, ++ity1, ++itx2) {
      typedef typename linalg_traits<Matrix>::sub_col_type col_type;
      col_type col = mat_col(A, i);
      typename linalg_traits<col_type>::iterator
	it = vect_begin(col), ite = vect_end(col);
      typename linalg_traits<VecX>::const_iterator itx1 = vect_const_begin(x);
      typename linalg_traits<VecY>::const_iterator ity2 = vect_const_begin(y);
      value_type ty = *ity1, tx = *itx2;
      for (; it != ite; ++it, ++itx1, ++ity2)
	*it += conj_product(ty, *itx1) + conj_product(tx, *ity2); 
    }
  }
  
  ///@endcond
  template <typename Matrix, typename VecX, typename VecY>
  inline void rank_two_update(const Matrix &AA, const VecX& x,
			      const VecY& y) {
    Matrix& A = const_cast<Matrix&>(AA);
    rank_two_update(A, x, y, typename principal_orientation_type<typename
		    linalg_traits<Matrix>::sub_orientation>::potype());
  }
  ///@cond DOXY_SHOW_ALL_FUNCTIONS

  /* ********************************************************************* */
  /*    Householder vector computation (complex and real version)          */
  /* ********************************************************************* */

  template <typename VECT> void house_vector(const VECT &VV) {
    VECT &V = const_cast<VECT &>(VV);
    typedef typename linalg_traits<VECT>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    
    R mu = vect_norm2(V), abs_v0 = gmm::abs(V[0]);
    if (mu != R(0))
      gmm::scale(V, (abs_v0 == R(0)) ? T(R(1) / mu)
		 : (safe_divide(T(abs_v0), V[0]) / (abs_v0 + mu)));
    if (gmm::real(V[vect_size(V)-1]) * R(0) != R(0)) gmm::clear(V);
    V[0] = T(1);
  }

  template <typename VECT> void house_vector_last(const VECT &VV) {
    VECT &V = const_cast<VECT &>(VV);
    typedef typename linalg_traits<VECT>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type m = vect_size(V);
    R mu = vect_norm2(V), abs_v0 = gmm::abs(V[m-1]);
    if (mu != R(0))
      gmm::scale(V, (abs_v0 == R(0)) ? T(R(1) / mu)
		 : ((abs_v0 / V[m-1]) / (abs_v0 + mu)));
    if (gmm::real(V[0]) * R(0) != R(0)) gmm::clear(V);
    V[m-1] = T(1);
  }
  
  /* ********************************************************************* */
  /*    Householder updates  (complex and real version)                    */
  /* ********************************************************************* */

  // multiply A to the left by the reflector stored in V. W is a temporary.
  template <typename MAT, typename VECT1, typename VECT2> inline
    void row_house_update(const MAT &AA, const VECT1 &V, const VECT2 &WW) {
    VECT2 &W = const_cast<VECT2 &>(WW); MAT &A = const_cast<MAT &>(AA);
    typedef typename linalg_traits<MAT>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type magnitude_type;

    gmm::mult(conjugated(A),
	      scaled(V, value_type(magnitude_type(-2)/vect_norm2_sqr(V))), W);
    rank_one_update(A, V, W);
  }

  // multiply A to the right by the reflector stored in V. W is a temporary.
  template <typename MAT, typename VECT1, typename VECT2> inline
    void col_house_update(const MAT &AA, const VECT1 &V, const VECT2 &WW) {
    VECT2 &W = const_cast<VECT2 &>(WW); MAT &A = const_cast<MAT &>(AA);
    typedef typename linalg_traits<MAT>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type magnitude_type;
    
    gmm::mult(A,
	      scaled(V, value_type(magnitude_type(-2)/vect_norm2_sqr(V))), W);
    rank_one_update(A, W, V);
  }

  ///@endcond

  /* ********************************************************************* */
  /*    Hessemberg reduction with Householder.                             */
  /* ********************************************************************* */

  template <typename MAT1, typename MAT2>
    void Hessenberg_reduction(const MAT1& AA, const MAT2 &QQ, bool compute_Q){
    MAT1& A = const_cast<MAT1&>(AA); MAT2& Q = const_cast<MAT2&>(QQ);
    typedef typename linalg_traits<MAT1>::value_type value_type;
    if (compute_Q) gmm::copy(identity_matrix(), Q);
    size_type n = mat_nrows(A); if (n < 2) return;
    std::vector<value_type> v(n), w(n);
    sub_interval SUBK(0,n);
    for (size_type k = 1; k+1 < n; ++k) {
      sub_interval SUBI(k, n-k), SUBJ(k-1,n-k+1);
      v.resize(n-k);
      for (size_type j = k; j < n; ++j) v[j-k] = A(j, k-1);
      house_vector(v);
      row_house_update(sub_matrix(A, SUBI, SUBJ), v, sub_vector(w, SUBJ));
      col_house_update(sub_matrix(A, SUBK, SUBI), v, w);
      // is it possible to "unify" the two on the common part of the matrix?
      if (compute_Q) col_house_update(sub_matrix(Q, SUBK, SUBI), v, w);
    }
  }

  /* ********************************************************************* */
  /*    Householder tridiagonalization for symmetric matrices              */
  /* ********************************************************************* */

  template <typename MAT1, typename MAT2> 
  void Householder_tridiagonalization(const MAT1 &AA, const MAT2 &QQ,
				     bool compute_q) {
    MAT1 &A = const_cast<MAT1 &>(AA); MAT2 &Q = const_cast<MAT2 &>(QQ);
    typedef typename linalg_traits<MAT1>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type n = mat_nrows(A); if (n < 2) return;
    std::vector<T> v(n), p(n), w(n), ww(n);
    sub_interval SUBK(0,n);

    for (size_type k = 1; k+1 < n; ++k) { // not optimized ...
      sub_interval SUBI(k, n-k);
      v.resize(n-k); p.resize(n-k); w.resize(n-k); 
      for (size_type l = k; l < n; ++l) 
	{ v[l-k] = w[l-k] = A(l, k-1); A(l, k-1) = A(k-1, l) = T(0); }
      house_vector(v);
      R norm = vect_norm2_sqr(v);
      A(k-1, k) = gmm::conj(A(k, k-1) = w[0] - T(2)*v[0]*vect_hp(w, v)/norm);

      gmm::mult(sub_matrix(A, SUBI), gmm::scaled(v, T(-2) / norm), p);
      gmm::add(p, gmm::scaled(v, -vect_hp(v, p) / norm), w);
      rank_two_update(sub_matrix(A, SUBI), v, w);
      // it should be possible to compute only the upper or lower part

      if (compute_q) col_house_update(sub_matrix(Q, SUBK, SUBI), v, ww);
    }
  }

  /* ********************************************************************* */
  /*    Real and complex Givens rotations                                  */
  /* ********************************************************************* */

  template <typename T> void Givens_rotation(T a, T b, T &c, T &s) {
    typedef typename number_traits<T>::magnitude_type R;
    R aa = gmm::abs(a), bb = gmm::abs(b);
    if (bb == R(0)) { c = T(1); s = T(0);   return; }
    if (aa == R(0)) { c = T(0); s = b / bb; return; }
    if (bb > aa)
      { T t = -safe_divide(a,b); s = T(R(1) / (sqrt(R(1)+gmm::abs_sqr(t)))); c = s * t; }
    else
      { T t = -safe_divide(b,a); c = T(R(1) / (sqrt(R(1)+gmm::abs_sqr(t)))); s = c * t; }
  }

  // Apply Q* v
  template <typename T> inline
  void Apply_Givens_rotation_left(T &x, T &y, T c, T s)
  { T t1=x, t2=y; x = gmm::conj(c)*t1 - gmm::conj(s)*t2; y = c*t2 + s*t1; }

  // Apply v^T Q
  template <typename T> inline
  void Apply_Givens_rotation_right(T &x, T &y, T c, T s)
  { T t1=x, t2=y; x = c*t1 - s*t2; y = gmm::conj(c)*t2 + gmm::conj(s)*t1; }

  template <typename MAT, typename T>
  void row_rot(const MAT &AA, T c, T s, size_type i, size_type k) {
    MAT &A = const_cast<MAT &>(AA); // can be specialized for row matrices
    for (size_type j = 0; j < mat_ncols(A); ++j)
      Apply_Givens_rotation_left(A(i,j), A(k,j), c, s);
  }

  template <typename MAT, typename T>
  void col_rot(const MAT &AA, T c, T s, size_type i, size_type k) {
    MAT &A = const_cast<MAT &>(AA); // can be specialized for column matrices
    for (size_type j = 0; j < mat_nrows(A); ++j)
      Apply_Givens_rotation_right(A(j,i), A(j,k), c, s);
  }
  
}

#endif

