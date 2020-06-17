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

// This file is a modified version of bicgstab.h from ITL.
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

/**@file gmm_solver_bicgstab.h
   @author Andrew Lumsdaine <lums@osl.iu.edu>
   @author Lie-Quan Lee <llee@osl.iu.edu>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief BiCGStab iterative solver.
*/

#ifndef GMM_SOLVER_BICGSTAB_H__
#define GMM_SOLVER_BICGSTAB_H__

#include "gmm_kernel.h"
#include "gmm_iter.h"

namespace gmm {

  /* ******************************************************************** */
  /*		BiConjugate Gradient Stabilized               		  */
  /* (preconditionned, with parametrable scalar product)        	  */
  /* ******************************************************************** */

  template <typename Matrix, typename Vector, typename VectorB,
	    typename Preconditioner>
  void bicgstab(const Matrix& A, Vector& x, const VectorB& b,
	       const Preconditioner& M, iteration &iter) {

    typedef typename linalg_traits<Vector>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    typedef typename temporary_dense_vector<Vector>::vector_type temp_vector;
    
    T rho_1, rho_2(0), alpha(0), beta, omega(0);
    temp_vector p(vect_size(x)), phat(vect_size(x)), s(vect_size(x)),
      shat(vect_size(x)), 
      t(vect_size(x)), v(vect_size(x)), r(vect_size(x)), rtilde(vect_size(x));
    
    gmm::mult(A, gmm::scaled(x, -T(1)), b, r);	  
    gmm::copy(r, rtilde);
    R norm_r = gmm::vect_norm2(r);
    iter.set_rhsnorm(gmm::vect_norm2(b));

    if (iter.get_rhsnorm() == 0.0) { clear(x); return; }
    
    while (!iter.finished(norm_r)) {
      
      rho_1 = gmm::vect_sp(rtilde, r);
      if (rho_1 == T(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "Bicgstab failed to converge"); }
	else { GMM_WARNING1("Bicgstab failed to converge"); return; }
      }
      
      if (iter.first())
	gmm::copy(r, p);
      else {
	if (omega == T(0)) {
	  if (iter.get_maxiter() == size_type(-1))
	    { GMM_ASSERT1(false, "Bicgstab failed to converge"); }
	  else { GMM_WARNING1("Bicgstab failed to converge"); return; }
	}
	
	beta = (rho_1 / rho_2) * (alpha / omega);
	
	gmm::add(gmm::scaled(v, -omega), p);
	gmm::add(r, gmm::scaled(p, beta), p);      
      }
      gmm::mult(M, p, phat);
      gmm::mult(A, phat, v);	
      alpha = rho_1 / gmm::vect_sp(v, rtilde);
      gmm::add(r, gmm::scaled(v, -alpha), s);
      
      if (iter.finished_vect(s)) 
	{ gmm::add(gmm::scaled(phat, alpha), x); break; }
      
      gmm::mult(M, s, shat);	
      gmm::mult(A, shat, t);
      omega = gmm::vect_sp(t, s) / gmm::vect_norm2_sqr(t);
      
      gmm::add(gmm::scaled(phat, alpha), x); 
      gmm::add(gmm::scaled(shat, omega), x);
      gmm::add(s, gmm::scaled(t, -omega), r); 
      norm_r = gmm::vect_norm2(r);
      rho_2 = rho_1;
      
      ++iter;
    }
  }
  
  template <typename Matrix, typename Vector, typename VectorB,
	    typename Preconditioner>
  void bicgstab(const Matrix& A, const Vector& x, const VectorB& b,
	       const Preconditioner& M, iteration &iter)
  { bicgstab(A, linalg_const_cast(x), b, M, iter); }
  
}


#endif //  GMM_SOLVER_BICGSTAB_H__
