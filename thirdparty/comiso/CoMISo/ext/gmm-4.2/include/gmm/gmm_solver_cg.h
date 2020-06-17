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

// This file is a modified version of cg.h from ITL.
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

/**@file gmm_solver_cg.h
   @author  Andrew Lumsdaine <lums@osl.iu.edu>
   @author  Lie-Quan Lee <llee@osl.iu.edu>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Conjugate gradient iterative solver. 
*/
#ifndef GMM_SOLVER_CG_H__
#define GMM_SOLVER_CG_H__

#include "gmm_kernel.h"
#include "gmm_iter.h"

namespace gmm {

  /* ******************************************************************** */
  /*		conjugate gradient                           		  */
  /* (preconditionned, with parametrable additional scalar product)       */
  /* ******************************************************************** */

  template <typename Matrix, typename Matps, typename Precond, 
            typename Vector1, typename Vector2>
  void cg(const Matrix& A, Vector1& x, const Vector2& b, const Matps& PS,
	  const Precond &P, iteration &iter) {

    typedef typename temporary_dense_vector<Vector1>::vector_type temp_vector;
    typedef typename linalg_traits<Vector1>::value_type T;

    T rho, rho_1(0), a;
    temp_vector p(vect_size(x)), q(vect_size(x)), r(vect_size(x)),
      z(vect_size(x));
    iter.set_rhsnorm(gmm::sqrt(gmm::abs(vect_hp(PS, b, b))));

    if (iter.get_rhsnorm() == 0.0)
      clear(x);
    else {
      mult(A, scaled(x, T(-1)), b, r);
      mult(P, r, z);
      rho = vect_hp(PS, z, r);
      copy(z, p);

      while (!iter.finished_vect(r)) {

	if (!iter.first()) { 
	  mult(P, r, z);
	  rho = vect_hp(PS, z, r);
	  add(z, scaled(p, rho / rho_1), p);
	}
	mult(A, p, q);

	a = rho / vect_hp(PS, q, p);	
	add(scaled(p, a), x);
	add(scaled(q, -a), r);
	rho_1 = rho;

	++iter;
      }
    }
  }

  template <typename Matrix, typename Matps, typename Precond, 
            typename Vector1, typename Vector2>
  void cg(const Matrix& A, Vector1& x, const Vector2& b, const Matps& PS,
	  const gmm::identity_matrix &, iteration &iter) {

    typedef typename temporary_dense_vector<Vector1>::vector_type temp_vector;
    typedef typename linalg_traits<Vector1>::value_type T;

    T rho, rho_1(0), a;
    temp_vector p(vect_size(x)), q(vect_size(x)), r(vect_size(x));
    iter.set_rhsnorm(gmm::sqrt(gmm::abs(vect_hp(PS, b, b))));

    if (iter.get_rhsnorm() == 0.0)
      clear(x);
    else {
      mult(A, scaled(x, T(-1)), b, r);
      rho = vect_hp(PS, r, r);
      copy(r, p);

      while (!iter.finished_vect(r)) {

	if (!iter.first()) { 
	  rho = vect_hp(PS, r, r);
	  add(r, scaled(p, rho / rho_1), p);
	}	
	mult(A, p, q);
	a = rho / vect_hp(PS, q, p);	
	add(scaled(p, a), x);
	add(scaled(q, -a), r);
	rho_1 = rho;
	++iter;
      }
    }
  }

  template <typename Matrix, typename Matps, typename Precond, 
            typename Vector1, typename Vector2> inline 
  void cg(const Matrix& A, const Vector1& x, const Vector2& b, const Matps& PS,
	 const Precond &P, iteration &iter)
  { cg(A, linalg_const_cast(x), b, PS, P, iter); }

  template <typename Matrix, typename Precond, 
            typename Vector1, typename Vector2> inline
  void cg(const Matrix& A, Vector1& x, const Vector2& b,
	 const Precond &P, iteration &iter)
  { cg(A, x , b, identity_matrix(), P, iter); }

  template <typename Matrix, typename Precond, 
            typename Vector1, typename Vector2> inline
  void cg(const Matrix& A, const Vector1& x, const Vector2& b,
	 const Precond &P, iteration &iter)
  { cg(A, x , b , identity_matrix(), P , iter); }

}


#endif //  GMM_SOLVER_CG_H__
