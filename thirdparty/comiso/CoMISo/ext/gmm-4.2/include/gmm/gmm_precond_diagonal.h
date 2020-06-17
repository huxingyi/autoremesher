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

/**@file gmm_precond_diagonal.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date June 5, 2003.
   @brief Diagonal matrix preconditoner.
*/

#ifndef GMM_PRECOND_DIAGONAL_H
#define GMM_PRECOND_DIAGONAL_H

#include "gmm_precond.h"

namespace gmm {

  /** Diagonal preconditioner. */
  template<typename Matrix> struct diagonal_precond {
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type magnitude_type;

    std::vector<magnitude_type> diag;

    void build_with(const Matrix &M) {
      diag.resize(mat_nrows(M));
      for (size_type i = 0; i < mat_nrows(M); ++i) {
	magnitude_type x = gmm::abs(M(i, i));
	if (x == magnitude_type(0)) {
	  x = magnitude_type(1);
	  GMM_WARNING2("The matrix has a zero on its diagonal");
	}
	diag[i] = magnitude_type(1) / x;
      }
    }
    size_type memsize() const { return sizeof(*this) + diag.size() * sizeof(value_type); }
    diagonal_precond(const Matrix &M) { build_with(M); }
    diagonal_precond(void) {}
  };

  template <typename Matrix, typename V2> inline
  void mult_diag_p(const diagonal_precond<Matrix>& P, V2 &v2, abstract_sparse){
    typename linalg_traits<V2>::iterator it = vect_begin(v2),
      ite = vect_end(v2);
    for (; it != ite; ++it) *it *= P.diag[it.index()];
  }

  template <typename Matrix, typename V2> inline
  void mult_diag_p(const diagonal_precond<Matrix>& P,V2 &v2, abstract_skyline)
    { mult_diag_p(P, v2, abstract_sparse()); }

  template <typename Matrix, typename V2> inline
  void mult_diag_p(const diagonal_precond<Matrix>& P, V2 &v2, abstract_dense){
    for (size_type i = 0; i < P.diag.size(); ++i) v2[i] *= P.diag[i];
  }

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const diagonal_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    GMM_ASSERT2(P.diag.size() == vect_size(v2),"dimensions mismatch");
    copy(v1, v2);
    mult_diag_p(P, v2, typename linalg_traits<V2>::storage_type());
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const diagonal_precond<Matrix>& P,const V1 &v1,V2 &v2) {
    mult(P, v1, v2);
  }
  
  // # define DIAG_LEFT_MULT_SQRT
  
  template <typename Matrix, typename V1, typename V2> inline
  void left_mult(const diagonal_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    GMM_ASSERT2(P.diag.size() == vect_size(v2), "dimensions mismatch");
    copy(v1, v2);
#   ifdef DIAG_LEFT_MULT_SQRT
    for (size_type i= 0; i < P.diag.size(); ++i) v2[i] *= gmm::sqrt(P.diag[i]);
#   else
    for (size_type i= 0; i < P.diag.size(); ++i) v2[i] *= P.diag[i];
#   endif
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_left_mult(const diagonal_precond<Matrix>& P,
			    const V1 &v1, V2 &v2)
    { left_mult(P, v1, v2); }

  template <typename Matrix, typename V1, typename V2> inline
  void right_mult(const diagonal_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    typedef typename linalg_traits<Matrix>::value_type T;
    GMM_ASSERT2(P.diag.size() == vect_size(v2), "dimensions mismatch");
    copy(v1, v2);
#   ifdef DIAG_LEFT_MULT_SQRT    
    for (size_type i= 0; i < P.diag.size(); ++i) v2[i] *= gmm::sqrt(P.diag[i]);
#   endif
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_right_mult(const diagonal_precond<Matrix>& P,
			    const V1 &v1, V2 &v2)
    { right_mult(P, v1, v2); }

}

#endif 

