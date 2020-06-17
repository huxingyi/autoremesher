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

/**@file gmm_iter_solvers.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Include standard gmm iterative solvers (cg, gmres, ...)
*/
#ifndef GMM_ITER_SOLVERS_H__
#define GMM_ITER_SOLVERS_H__

#include "gmm_iter.h"


namespace gmm {

  /** mixed method to find a zero of a real function G, a priori 
   * between a and b. If the zero is not between a and b, iterations
   * of secant are applied. When a convenient interval is found,
   * iterations of dichotomie and regula falsi are applied.
   */
  template <typename FUNC, typename T>
  T find_root(const FUNC &G, T a = T(0), T b = T(1),
	      T tol = gmm::default_tol(T())) {
    T c, Ga = G(a), Gb = G(b), Gc, d;
    d = gmm::abs(b - a);
#if 0
    for (int i = 0; i < 4; i++) { /* secant iterations.                   */
      if (d < tol) return (b + a) / 2.0;
      c = b - Gb * (b - a) / (Gb - Ga); Gc = G(c);
      a = b; b = c; Ga = Gb; Gb = Gc;
      d = gmm::abs(b - a);
    }
#endif
    while (Ga * Gb > 0.0) { /* secant iterations.                         */
      if (d < tol) return (b + a) / 2.0;
      c = b - Gb * (b - a) / (Gb - Ga); Gc = G(c);
      a = b; b = c; Ga = Gb; Gb = Gc;
      d = gmm::abs(b - a);
    }
    
    c = std::max(a, b); a = std::min(a, b); b = c;
    while (d > tol) {
      c = b - (b - a) * (Gb / (Gb - Ga)); /* regula falsi.     */
      if (c > b) c = b; if (c < a) c = a; 
      Gc = G(c);
      if (Gc*Gb > 0) { b = c; Gb = Gc; } else { a = c; Ga = Gc; }
      c = (b + a) / 2.0 ; Gc = G(c); /* Dichotomie.                       */
      if (Gc*Gb > 0) { b = c; Gb = Gc; } else { a = c; Ga = Gc; }
      d = gmm::abs(b - a); c = (b + a) / 2.0; if ((c == a) || (c == b)) d = 0.0;
    }
    return (b + a) / 2.0;
  }
  
}

#include "gmm_precond_diagonal.h"
#include "gmm_precond_ildlt.h"
#include "gmm_precond_ildltt.h"
#include "gmm_precond_mr_approx_inverse.h"
#include "gmm_precond_ilu.h"
#include "gmm_precond_ilut.h"
#include "gmm_precond_ilutp.h"



#include "gmm_solver_cg.h"
#include "gmm_solver_bicgstab.h"
#include "gmm_solver_qmr.h"
#include "gmm_solver_constrained_cg.h"
#include "gmm_solver_Schwarz_additive.h"
#include "gmm_modified_gram_schmidt.h"
#include "gmm_tri_solve.h"
#include "gmm_solver_gmres.h"
#include "gmm_solver_bfgs.h"
#include "gmm_least_squares_cg.h"

// #include "gmm_solver_idgmres.h"



#endif //  GMM_ITER_SOLVERS_H__
