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

/**@file gmm_opt.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date July 9, 2003.
   @brief Optimization for some small cases (inversion of 2x2 matrices etc.)
*/
#ifndef GMM_OPT_H__
#define GMM_OPT_H__

namespace gmm {

  /* ********************************************************************* */
  /*    Optimized determinant and inverse for small matrices (2x2 and 3x3) */
  /*    with dense_matrix<T>.                                              */
  /* ********************************************************************* */

  template <typename T>  T lu_det(const dense_matrix<T> &A) {
    size_type n(mat_nrows(A));
    if (n) {
      const T *p = &(A(0,0));
      switch (n) {
      case 1 : return (*p);
      case 2 : return (*p) * (*(p+3)) - (*(p+1)) * (*(p+2));
// Not stable for nearly singular matrices
//       case 3 : return (*p) * ((*(p+4)) * (*(p+8)) - (*(p+5)) * (*(p+7)))
// 		 - (*(p+1)) * ((*(p+3)) * (*(p+8)) - (*(p+5)) * (*(p+6)))
// 		 + (*(p+2)) * ((*(p+3)) * (*(p+7)) - (*(p+4)) * (*(p+6)));
      default :
	{
	  dense_matrix<T> B(mat_nrows(A), mat_ncols(A));
	  std::vector<size_type> ipvt(mat_nrows(A));
	  gmm::copy(A, B);
	  lu_factor(B, ipvt);
	  return lu_det(B, ipvt);	
	}
      }
    }
    return T(1);
  }


  template <typename T> T lu_inverse(const dense_matrix<T> &A_) {
    dense_matrix<T>& A = const_cast<dense_matrix<T> &>(A_);
    size_type N = mat_nrows(A);
    T det(1);
    if (N) {
      T *p = &(A(0,0));
      if (N <= 2) {
	switch (N) {
	  case 1 : {
	    det = *p;
	    GMM_ASSERT1(det!=T(0), "non invertible matrix");
	    *p = T(1) / det; 
	  } break;
	  case 2 : {
	    det = (*p) * (*(p+3)) - (*(p+1)) * (*(p+2));
	    GMM_ASSERT1(det!=T(0), "non invertible matrix");
	    std::swap(*p, *(p+3));
	    *p++ /= det; *p++ /= -det; *p++ /= -det; *p++ /= det; 
	  } break;
// 	  case 3 : { // not stable for nearly singular matrices
// 	    T a, b, c, d, e, f, g, h, i;
// 	    a =   (*(p+4)) * (*(p+8)) - (*(p+5)) * (*(p+7));
// 	    b = - (*(p+1)) * (*(p+8)) + (*(p+2)) * (*(p+7));
// 	    c =   (*(p+1)) * (*(p+5)) - (*(p+2)) * (*(p+4));
// 	    d = - (*(p+3)) * (*(p+8)) + (*(p+5)) * (*(p+6));
// 	    e =   (*(p+0)) * (*(p+8)) - (*(p+2)) * (*(p+6));
// 	    f = - (*(p+0)) * (*(p+5)) + (*(p+2)) * (*(p+3));
// 	    g =   (*(p+3)) * (*(p+7)) - (*(p+4)) * (*(p+6));
// 	    h = - (*(p+0)) * (*(p+7)) + (*(p+1)) * (*(p+6));
// 	    i =   (*(p+0)) * (*(p+4)) - (*(p+1)) * (*(p+3));
// 	    det = (*p) * a + (*(p+1)) * d + (*(p+2)) * g;
// 	    GMM_ASSERT1(det!=T(0), "non invertible matrix");
// 	    *p++ = a / det; *p++ = b / det; *p++ = c / det; 
// 	    *p++ = d / det; *p++ = e / det; *p++ = f / det; 
// 	    *p++ = g / det; *p++ = h / det; *p++ = i / det; 
// 	  } break;
	}
      }
      else {
	dense_matrix<T> B(mat_nrows(A), mat_ncols(A));
	std::vector<int> ipvt(mat_nrows(A));
	gmm::copy(A, B);
	size_type info = lu_factor(B, ipvt);
	GMM_ASSERT1(!info, "non invertible matrix");
	lu_inverse(B, ipvt, A);
	return lu_det(B, ipvt);
      }
    }
    return det;
  }

  
}

#endif //  GMM_OPT_H__
