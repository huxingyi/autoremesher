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

// This file is a modified version of gmres.h from ITL.
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

/**@file gmm_solver_gmres.h
   @author  Andrew Lumsdaine <lums@osl.iu.edu>
   @author  Lie-Quan Lee     <llee@osl.iu.edu>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief GMRES (Generalized Minimum Residual) iterative solver.
*/
#ifndef GMM_KRYLOV_GMRES_H
#define GMM_KRYLOV_GMRES_H

#include "gmm_kernel.h"
#include "gmm_iter.h"
#include "gmm_modified_gram_schmidt.h"

namespace gmm {

  /** Generalized Minimum Residual
   
      This solve the unsymmetric linear system Ax = b using restarted GMRES.
      
      See: Y. Saad and M. Schulter. GMRES: A generalized minimum residual
      algorithm for solving nonsysmmetric linear systems, SIAM
      J. Sci. Statist. Comp.  7(1986), pp, 856-869
  */
  template <typename Mat, typename Vec, typename VecB, typename Precond,
	    typename Basis >
  void gmres(const Mat &A, Vec &x, const VecB &b, const Precond &M,
	     int restart, iteration &outer, Basis& KS) {

    typedef typename linalg_traits<Vec>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    std::vector<T> w(vect_size(x)), r(vect_size(x)), u(vect_size(x));
    std::vector<T> c_rot(restart+1), s_rot(restart+1), s(restart+1);
    gmm::dense_matrix<T> H(restart+1, restart);
#ifdef GMM_USES_MPI
      double t_ref, t_prec = MPI_Wtime(), t_tot = 0;
      static double tmult_tot = 0.0;
t_ref = MPI_Wtime();
    cout << "GMRES " << endl;
#endif
    mult(M,b,r);
    outer.set_rhsnorm(gmm::vect_norm2(r));
    if (outer.get_rhsnorm() == 0.0) { clear(x); return; }
    
    mult(A, scaled(x, T(-1)), b, w);
    mult(M, w, r);
    R beta = gmm::vect_norm2(r), beta_old = beta;
    int blocked = 0;

    iteration inner = outer;
    inner.reduce_noisy();
    inner.set_maxiter(restart);
    inner.set_name("GMRes inner");

    while (! outer.finished(beta)) {
      
      gmm::copy(gmm::scaled(r, R(1)/beta), KS[0]);
      gmm::clear(s);
      s[0] = beta;
      
      size_type i = 0; inner.init();
      
      do {
	mult(A, KS[i], u);
	mult(M, u, KS[i+1]);
	orthogonalize(KS, mat_col(H, i), i);
	R a = gmm::vect_norm2(KS[i+1]);
	H(i+1, i) = T(a);
	gmm::scale(KS[i+1], T(1) / a);
	for (size_type k = 0; k < i; ++k)
	  Apply_Givens_rotation_left(H(k,i), H(k+1,i), c_rot[k], s_rot[k]);
	
	Givens_rotation(H(i,i), H(i+1,i), c_rot[i], s_rot[i]);
	Apply_Givens_rotation_left(H(i,i), H(i+1,i), c_rot[i], s_rot[i]);
	Apply_Givens_rotation_left(s[i], s[i+1], c_rot[i], s_rot[i]);
	
	++inner, ++outer, ++i;
      } while (! inner.finished(gmm::abs(s[i])));

      upper_tri_solve(H, s, i, false);
      combine(KS, s, x, i);
      mult(A, gmm::scaled(x, T(-1)), b, w);
      mult(M, w, r);
      beta_old = std::min(beta, beta_old); beta = gmm::vect_norm2(r);
      if (int(inner.get_iteration()) < restart -1 || beta_old <= beta)
	++blocked; else blocked = 0;
      if (blocked > 10) {
	if (outer.get_noisy()) cout << "Gmres is blocked, exiting\n";
	break;
      }
#ifdef GMM_USES_MPI
	t_tot = MPI_Wtime() - t_ref;
	cout << "temps GMRES : " << t_tot << endl; 
#endif
    }
  }


  template <typename Mat, typename Vec, typename VecB, typename Precond >
  void gmres(const Mat &A, Vec &x, const VecB &b,
	     const Precond &M, int restart, iteration& outer) {
    typedef typename linalg_traits<Vec>::value_type T;
    modified_gram_schmidt<T> orth(restart, vect_size(x));
    gmres(A, x, b, M, restart, outer, orth); 
  }

}

#endif
