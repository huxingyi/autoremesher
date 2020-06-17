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

/**@file gmm_tri_solve.h
   @author Yves Renard
   @date October 13, 2002.
   @brief Solve triangular linear system for dense matrices.
*/

#ifndef GMM_TRI_SOLVE_H__
#define GMM_TRI_SOLVE_H__

#include "gmm_interface.h"

namespace gmm {

  template <typename TriMatrix, typename VecX>
  void upper_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				col_major, abstract_sparse, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type x_j;
    for (int j = int(k) - 1; j >= 0; --j) {
      typedef typename linalg_traits<TriMatrix>::const_sub_col_type COL;
      COL c = mat_const_col(T, j);
      typename linalg_traits<COL>::const_iterator 
	it = vect_const_begin(c), ite = vect_const_end(c);
      if (!is_unit) x[j] /= c[j];
      for (x_j = x[j]; it != ite ; ++it)
	if (int(it.index()) < j) x[it.index()] -= x_j * (*it);
    }    
  }

  template <typename TriMatrix, typename VecX>
  void upper_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				col_major, abstract_dense, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type x_j;
    for (int j = int(k) - 1; j >= 0; --j) {
      typedef typename linalg_traits<TriMatrix>::const_sub_col_type COL;
      COL c = mat_const_col(T, j);
      typename linalg_traits<COL>::const_iterator
	it = vect_const_begin(c), ite = it + j;
      typename linalg_traits<VecX>::iterator itx = vect_begin(x);
      if (!is_unit) x[j] /= c[j];
      for (x_j = x[j]; it != ite ; ++it, ++itx) *itx -= x_j * (*it);
    }
  }

  template <typename TriMatrix, typename VecX>
  void lower_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				col_major, abstract_sparse, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type x_j;
    // cout << "(lower col)The Tri Matrix = " << T << endl;
    // cout << "k = " << endl;
    for (int j = 0; j < int(k); ++j) {
      typedef typename linalg_traits<TriMatrix>::const_sub_col_type COL;
      COL c = mat_const_col(T, j);
      typename linalg_traits<COL>::const_iterator 
	it = vect_const_begin(c), ite = vect_const_end(c);
      if (!is_unit) x[j] /= c[j];
      for (x_j = x[j]; it != ite ; ++it)
	if (int(it.index()) > j && it.index() < k) x[it.index()] -= x_j*(*it);
    }    
  }
  
  template <typename TriMatrix, typename VecX>
  void lower_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				col_major, abstract_dense, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type x_j;
    for (int j = 0; j < int(k); ++j) {
      typedef typename linalg_traits<TriMatrix>::const_sub_col_type COL;
      COL c = mat_const_col(T, j);
      typename linalg_traits<COL>::const_iterator 
	it = vect_const_begin(c) + (j+1), ite = vect_const_begin(c) + k;
      typename linalg_traits<VecX>::iterator itx = vect_begin(x) + (j+1);
      if (!is_unit) x[j] /= c[j];
      for (x_j = x[j]; it != ite ; ++it, ++itx) *itx -= x_j * (*it);
    }    
  }
  

  template <typename TriMatrix, typename VecX>
  void upper_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				row_major, abstract_sparse, bool is_unit) {
    typedef typename linalg_traits<TriMatrix>::const_sub_row_type ROW;
    typename linalg_traits<TriMatrix>::value_type t;
    typename linalg_traits<TriMatrix>::const_row_iterator
      itr = mat_row_const_end(T);
    for (int i = int(k) - 1; i >= 0; --i) {
      --itr;
      ROW c = linalg_traits<TriMatrix>::row(itr);
      typename linalg_traits<ROW>::const_iterator 
	it = vect_const_begin(c), ite = vect_const_end(c);
      for (t = x[i]; it != ite; ++it)
	if (int(it.index()) > i && it.index() < k) t -= (*it) * x[it.index()];
      if (!is_unit) x[i] = t / c[i]; else x[i] = t;    
    }    
  }

  template <typename TriMatrix, typename VecX>
  void upper_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				row_major, abstract_dense, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type t;
   
    for (int i = int(k) - 1; i >= 0; --i) {
      typedef typename linalg_traits<TriMatrix>::const_sub_row_type ROW;
      ROW c = mat_const_row(T, i);
      typename linalg_traits<ROW>::const_iterator 
	it = vect_const_begin(c) + (i + 1), ite = vect_const_begin(c) + k;
      typename linalg_traits<VecX>::iterator itx = vect_begin(x) + (i+1);
      
      for (t = x[i]; it != ite; ++it, ++itx) t -= (*it) * (*itx);
      if (!is_unit) x[i] = t / c[i]; else x[i] = t;   
    }    
  }

  template <typename TriMatrix, typename VecX>
  void lower_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				row_major, abstract_sparse, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type t;
   
    for (int i = 0; i < int(k); ++i) {
      typedef typename linalg_traits<TriMatrix>::const_sub_row_type ROW;
      ROW c = mat_const_row(T, i);
      typename linalg_traits<ROW>::const_iterator 
	it = vect_const_begin(c), ite = vect_const_end(c);

      for (t = x[i]; it != ite; ++it)
	if (int(it.index()) < i) t -= (*it) * x[it.index()];
      if (!is_unit) x[i] = t / c[i]; else x[i] = t; 
    }    
  }

  template <typename TriMatrix, typename VecX>
  void lower_tri_solve__(const TriMatrix& T, VecX& x, size_t k,
				row_major, abstract_dense, bool is_unit) {
    typename linalg_traits<TriMatrix>::value_type t;
   
    for (int i = 0; i < int(k); ++i) {
      typedef typename linalg_traits<TriMatrix>::const_sub_row_type ROW;
      ROW c = mat_const_row(T, i);
      typename linalg_traits<ROW>::const_iterator 
	it = vect_const_begin(c), ite = it + i;
      typename linalg_traits<VecX>::iterator itx = vect_begin(x);

      for (t = x[i]; it != ite; ++it, ++itx) t -= (*it) * (*itx);
      if (!is_unit) x[i] = t / c[i]; else x[i] = t;
    }
  }


// Triangular Solve:  x <-- T^{-1} * x

  template <typename TriMatrix, typename VecX> inline
  void upper_tri_solve(const TriMatrix& T, VecX &x_, bool is_unit = false)
  { upper_tri_solve(T, x_, mat_nrows(T), is_unit); }
  
  template <typename TriMatrix, typename VecX> inline
  void lower_tri_solve(const TriMatrix& T, VecX &x_, bool is_unit = false)
  { lower_tri_solve(T, x_, mat_nrows(T), is_unit); }

  template <typename TriMatrix, typename VecX> inline
  void upper_tri_solve(const TriMatrix& T, VecX &x_, size_t k,
		       bool is_unit) {
    VecX& x = const_cast<VecX&>(x_);
    GMM_ASSERT2(mat_nrows(T) >= k && vect_size(x) >= k
		&& mat_ncols(T) >= k && !is_sparse(x_), "dimensions mismatch");
    upper_tri_solve__(T, x, k, 
		      typename principal_orientation_type<typename
		      linalg_traits<TriMatrix>::sub_orientation>::potype(),
		      typename linalg_traits<TriMatrix>::storage_type(),
		      is_unit);
  }
  
  template <typename TriMatrix, typename VecX> inline
  void lower_tri_solve(const TriMatrix& T, VecX &x_, size_t k,
		       bool is_unit) {
    VecX& x = const_cast<VecX&>(x_);
    GMM_ASSERT2(mat_nrows(T) >= k && vect_size(x) >= k
		&& mat_ncols(T) >= k && !is_sparse(x_), "dimensions mismatch");
    lower_tri_solve__(T, x, k, 
		      typename principal_orientation_type<typename
		      linalg_traits<TriMatrix>::sub_orientation>::potype(),
		      typename linalg_traits<TriMatrix>::storage_type(),
		      is_unit);
  }


 



}


#endif //  GMM_TRI_SOLVE_H__
