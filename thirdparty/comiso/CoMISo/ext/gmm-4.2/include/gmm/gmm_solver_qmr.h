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

// This file is a modified version of qmr.h from ITL.
// See http://osl.iu.edu/research/itl/
// Following the corresponding Copyright notice.
//===========================================================================
//
// Copyright (c) 1997-2001, The Trustees of Indiana University.
// All rights reserved.
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

/**@file gmm_solver_qmr.h
   @author Andrew Lumsdaine <lums@osl.iu.edu>
   @author Lie-Quan Lee     <llee@osl.iu.edu>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Quasi-Minimal Residual iterative solver.
*/
#ifndef GMM_QMR_H
#define GMM_QMR_H

#include "gmm_kernel.h"
#include "gmm_iter.h"

namespace gmm {

  /** Quasi-Minimal Residual.
     
     This routine solves the unsymmetric linear system Ax = b using
     the Quasi-Minimal Residual method.
   
     See: R. W. Freund and N. M. Nachtigal, A quasi-minimal residual
     method for non-Hermitian linear systems, Numerical Math.,
     60(1991), pp. 315-339
  
     Preconditioner -  Incomplete LU, Incomplete LU with threshold,
                       SSOR or identity_preconditioner.
  */
  template <typename Matrix, typename Vector, typename VectorB,
	    typename Precond1>
  void qmr(const Matrix &A, Vector &x, const VectorB &b, const Precond1 &M1,
	   iteration& iter) {

    typedef typename linalg_traits<Vector>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    T delta(0), ep(0), beta(0), theta_1(0), gamma_1(0);
    T theta(0), gamma(1), eta(-1);
    R rho_1(0), rho, xi;

    typedef typename temporary_vector<Vector>::vector_type TmpVec;
    size_type nn = vect_size(x);
    TmpVec r(nn), v_tld(nn), y(nn), w_tld(nn), z(nn), v(nn), w(nn);
    TmpVec y_tld(nn), z_tld(nn), p(nn), q(nn), p_tld(nn), d(nn), s(nn);

    iter.set_rhsnorm(double(gmm::vect_norm2(b)));
    if (iter.get_rhsnorm() == 0.0) { clear(x); return; }

    gmm::mult(A, gmm::scaled(x, T(-1)), b, r);
    gmm::copy(r, v_tld);

    gmm::left_mult(M1, v_tld, y);
    rho = gmm::vect_norm2(y);

    gmm::copy(r, w_tld);
    gmm::transposed_right_mult(M1, w_tld, z);
    xi = gmm::vect_norm2(z);
  
    while (! iter.finished_vect(r)) {
    
      if (rho == R(0) || xi == R(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "QMR failed to converge"); }
	else { GMM_WARNING1("QMR failed to converge"); return; }
      }
      gmm::copy(gmm::scaled(v_tld, T(R(1)/rho)), v);
      gmm::scale(y, T(R(1)/rho));

      gmm::copy(gmm::scaled(w_tld, T(R(1)/xi)), w);
      gmm::scale(z, T(R(1)/xi));

      delta = gmm::vect_sp(z, y);
      if (delta == T(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "QMR failed to converge"); }
	else { GMM_WARNING1("QMR failed to converge"); return; }
      }
      gmm::right_mult(M1, y, y_tld);		
      gmm::transposed_left_mult(M1, z, z_tld);

      if (iter.first()) {
	gmm::copy(y_tld, p);
	gmm::copy(z_tld, q);
      } else {
	gmm::add(y_tld, gmm::scaled(p, -(T(xi  * delta) / ep)), p);
	gmm::add(z_tld, gmm::scaled(q, -(T(rho * delta) / ep)), q);
      }
    
      gmm::mult(A, p, p_tld);

      ep = gmm::vect_sp(q, p_tld);
      if (ep == T(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "QMR failed to converge"); }
	else { GMM_WARNING1("QMR failed to converge"); return; }
      }
      beta = ep / delta;
      if (beta == T(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "QMR failed to converge"); }
	else { GMM_WARNING1("QMR failed to converge"); return; }
      }
      gmm::add(p_tld, gmm::scaled(v, -beta), v_tld);
      gmm::left_mult(M1, v_tld, y);

      rho_1 = rho;
      rho = gmm::vect_norm2(y);

      gmm::mult(gmm::transposed(A), q, w_tld);
      gmm::add(w_tld, gmm::scaled(w, -beta), w_tld);
      gmm::transposed_right_mult(M1, w_tld, z);

      xi = gmm::vect_norm2(z);

      gamma_1 = gamma;
      theta_1 = theta;

      theta = rho / (gamma_1 * beta);
      gamma = T(1) / gmm::sqrt(T(1) + gmm::sqr(theta));

      if (gamma == T(0)) {
	if (iter.get_maxiter() == size_type(-1)) 
	  { GMM_ASSERT1(false, "QMR failed to converge"); }
	else { GMM_WARNING1("QMR failed to converge"); return; }
      }
      eta = -eta * T(rho_1) * gmm::sqr(gamma) / (beta * gmm::sqr(gamma_1));

      if (iter.first()) {
	gmm::copy(gmm::scaled(p, eta), d);
	gmm::copy(gmm::scaled(p_tld, eta), s);
      } else {
	T tmp = gmm::sqr(theta_1 * gamma);
	gmm::add(gmm::scaled(p, eta), gmm::scaled(d, tmp), d);
	gmm::add(gmm::scaled(p_tld, eta), gmm::scaled(s, tmp), s);
      }
      gmm::add(d, x);
      gmm::add(gmm::scaled(s, T(-1)), r);

      ++iter;
    }
  }


}

#endif 

