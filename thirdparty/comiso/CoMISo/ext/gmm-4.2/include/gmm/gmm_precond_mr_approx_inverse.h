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


// This file is a modified version of approximate_inverse.h from ITL.
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

/**@file gmm_precond_mr_approx_inverse.h
   @author Andrew Lumsdaine <lums@osl.iu.edu>
   @author Lie-Quan Lee     <llee@osl.iu.edu>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date June 5, 2003.
   @brief Approximate inverse via MR iteration.
*/

#ifndef GMM_PRECOND_MR_APPROX_INVERSE_H
#define GMM_PRECOND_MR_APPROX_INVERSE_H


#include "gmm_precond.h"

namespace gmm {

  /** Approximate inverse via MR iteration (see P301 of Saad book).
   */
  template <typename Matrix>
  struct mr_approx_inverse_precond {

    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type magnitude_type;
    typedef typename principal_orientation_type<typename
      linalg_traits<Matrix>::sub_orientation>::potype sub_orientation;
    typedef wsvector<value_type> VVector;
    typedef col_matrix<VVector> MMatrix;

    MMatrix M;
    size_type nb_it;
    magnitude_type threshold;

    void build_with(const Matrix& A);
    mr_approx_inverse_precond(const Matrix& A, size_type nb_it_,
			      magnitude_type threshold_)
      : M(mat_nrows(A), mat_ncols(A))
    { threshold = threshold_; nb_it = nb_it_; build_with(A); }
    mr_approx_inverse_precond(void)
    { threshold = magnitude_type(1E-7); nb_it = 5; }
    mr_approx_inverse_precond(size_type nb_it_, magnitude_type threshold_)
    { threshold = threshold_; nb_it = nb_it_; } 
    const MMatrix &approx_inverse(void) const { return M; }
  };

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const mr_approx_inverse_precond<Matrix>& P, const V1 &v1, V2 &v2)
  { mult(P.M, v1, v2); }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const mr_approx_inverse_precond<Matrix>& P,
		       const V1 &v1,V2 &v2)
  { mult(gmm::conjugated(P.M), v1, v2); }

  template <typename Matrix>
  void mr_approx_inverse_precond<Matrix>::build_with(const Matrix& A) {
    gmm::resize(M, mat_nrows(A), mat_ncols(A));
    typedef value_type T;
    typedef magnitude_type R;
    VVector m(mat_ncols(A)),r(mat_ncols(A)),ei(mat_ncols(A)),Ar(mat_ncols(A)); 
    T alpha = mat_trace(A)/ mat_euclidean_norm_sqr(A);
    if (alpha == T(0)) alpha = T(1);
    
    for (size_type i = 0; i < mat_nrows(A); ++i) {
      gmm::clear(m); gmm::clear(ei); 
      m[i] = alpha;
      ei[i] = T(1);
      
      for (size_type j = 0; j < nb_it; ++j) {
	gmm::mult(A, gmm::scaled(m, T(-1)), r);
	gmm::add(ei, r);
	gmm::mult(A, r, Ar);
	T nAr = vect_sp(Ar,Ar);
	if (gmm::abs(nAr) > R(0)) {
	  gmm::add(gmm::scaled(r, gmm::safe_divide(vect_sp(r, Ar), vect_sp(Ar, Ar))), m);
	  gmm::clean(m, threshold * gmm::vect_norm2(m));
	} else gmm::clear(m);
      }
      if (gmm::vect_norm2(m) == R(0)) m[i] = alpha;
      gmm::copy(m, M.col(i));
    }
  }
}

#endif 

