/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2002-2012 Yves Renard, Benjamin Schleimer
 
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

/**@file gmm_leastsquares_cg.h
   @author Benjamin Schleimer <bensch128  (at) yahoo (dot) com>
   @date January 23, 2007.
   @brief Conjugate gradient least squares algorithm. 
   Algorithm taken from http://www.stat.washington.edu/wxs/Stat538-w05/Notes/conjugate-gradients.pdf page 6
*/
#ifndef GMM_LEAST_SQUARES_CG_H__
#define GMM_LEAST_SQUARES_CG_H__

#include "gmm_kernel.h"
#include "gmm_iter.h"
#include "gmm_conjugated.h"

namespace gmm {

  template <typename Matrix, typename Vector1, typename Vector2>
  void least_squares_cg(const Matrix& C, Vector1& x, const Vector2& y,
			iteration &iter) {

    typedef typename temporary_dense_vector<Vector1>::vector_type temp_vector;
    typedef typename linalg_traits<Vector1>::value_type T;

    T rho, rho_1(0), a;
    temp_vector p(vect_size(x)), q(vect_size(y)), g(vect_size(x));
    temp_vector r(vect_size(y));
    iter.set_rhsnorm(gmm::sqrt(gmm::abs(vect_hp(y, y))));

    if (iter.get_rhsnorm() == 0.0)
      clear(x);
    else {
      mult(C, scaled(x, T(-1)), y, r);
      mult(conjugated(C), r, g);
      rho = vect_hp(g, g);
      copy(g, p);

      while (!iter.finished_vect(g)) {

	if (!iter.first()) { 
	  rho = vect_hp(g, g);
	  add(g, scaled(p, rho / rho_1), p);
	}

	mult(C, p, q);

	a = rho / vect_hp(q, q);	
	add(scaled(p, a), x);
	add(scaled(q, -a), r);
	// NOTE: how do we minimize the impact to the transpose?
	mult(conjugated(C), r, g);
	rho_1 = rho;

	++iter;
      }
    }
  }

  template <typename Matrix, typename Precond, 
            typename Vector1, typename Vector2> inline 
  void least_squares_cg(const Matrix& C, const Vector1& x, const Vector2& y,
			iteration &iter)
  { least_squares_cg(C, linalg_const_cast(x), y, iter); }
}


#endif //  GMM_SOLVER_CG_H__
