/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2004-2012 Yves Renard
 
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

/**@file gmm_solver_bfgs.h 
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 14 2004.
   @brief Implements BFGS (Broyden, Fletcher, Goldfarb, Shanno) algorithm.
 */
#ifndef GMM_BFGS_H
#define GMM_BFGS_H

#include "gmm_kernel.h"
#include "gmm_iter.h"

namespace gmm {

  // BFGS algorithm (Broyden, Fletcher, Goldfarb, Shanno)
  // Quasi Newton method for optimization problems.
  // with Wolfe Line search.


  // delta[k] = x[k+1] - x[k]
  // gamma[k] = grad f(x[k+1]) - grad f(x[k])
  // H[0] = I
  // BFGS : zeta[k] = delta[k] - H[k] gamma[k]
  // DFP  : zeta[k] = H[k] gamma[k]
  // tau[k] = gamma[k]^T zeta[k]
  // rho[k] = 1 / gamma[k]^T delta[k]
  // BFGS : H[k+1] = H[k] + rho[k](zeta[k] delta[k]^T + delta[k] zeta[k]^T)
  //                 - rho[k]^2 tau[k] delta[k] delta[k]^T
  // DFP  : H[k+1] = H[k] + rho[k] delta[k] delta[k]^T 
  //                 - (1/tau[k])zeta[k] zeta[k]^T 

  // Object representing the inverse of the Hessian
  template <typename VECTOR> struct bfgs_invhessian {
    
    typedef typename linalg_traits<VECTOR>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    std::vector<VECTOR> delta, gamma, zeta;
    std::vector<T> tau, rho;
    int version;

    template<typename VEC1, typename VEC2> void hmult(const VEC1 &X, VEC2 &Y) {
      copy(X, Y);
      for (size_type k = 0 ; k < delta.size(); ++k) {
	T xdelta = vect_sp(X, delta[k]), xzeta = vect_sp(X, zeta[k]);
	switch (version) {
	case 0 : // BFGS
	  add(scaled(zeta[k], rho[k]*xdelta), Y);
	  add(scaled(delta[k], rho[k]*(xzeta-rho[k]*tau[k]*xdelta)), Y);
	  break;
	case 1 : // DFP
	  add(scaled(delta[k], rho[k]*xdelta), Y);
	  add(scaled(zeta[k], -xzeta/tau[k]), Y);
	  break;
	}
      }
    }
    
    void restart(void) {
      delta.resize(0); gamma.resize(0); zeta.resize(0); 
      tau.resize(0); rho.resize(0);
    }
    
    template<typename VECT1, typename VECT2>
    void update(const VECT1 &deltak, const VECT2 &gammak) {
      size_type N = vect_size(deltak), k = delta.size();
      VECTOR Y(N);
      hmult(gammak, Y);
      delta.resize(k+1); gamma.resize(k+1); zeta.resize(k+1);
      tau.resize(k+1); rho.resize(k+1);
      resize(delta[k], N); resize(gamma[k], N); resize(zeta[k], N); 
      gmm::copy(deltak, delta[k]);
      gmm::copy(gammak, gamma[k]);
      rho[k] = R(1) / vect_sp(deltak, gammak);
      if (version == 0)
	add(delta[k], scaled(Y, -1), zeta[k]);
      else
	gmm::copy(Y, zeta[k]);
      tau[k] = vect_sp(gammak,  zeta[k]);
    }
    
    bfgs_invhessian(int v = 0) { version = v; }
  };


  template <typename FUNCTION, typename DERIVATIVE, typename VECTOR> 
  void bfgs(FUNCTION f, DERIVATIVE grad, VECTOR &x,
	    int restart, iteration& iter, int version = 0,
	    double lambda_init=0.001, double print_norm=1.0) {

    typedef typename linalg_traits<VECTOR>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    
    bfgs_invhessian<VECTOR> invhessian(version);
    VECTOR r(vect_size(x)), d(vect_size(x)), y(vect_size(x)), r2(vect_size(x));
    grad(x, r);
    R lambda = lambda_init, valx = f(x), valy;
    int nb_restart(0);
    
    if (iter.get_noisy() >= 1) cout << "value " << valx / print_norm << " ";
    while (! iter.finished_vect(r)) {

      invhessian.hmult(r, d); gmm::scale(d, T(-1));
      
      // Wolfe Line search
      R derivative = gmm::vect_sp(r, d);    
      R lambda_min(0), lambda_max(0), m1 = 0.27, m2 = 0.57;
      bool unbounded = true, blocked = false, grad_computed = false;
      
      for(;;) {
	add(x, scaled(d, lambda), y);
	valy = f(y);
	if (iter.get_noisy() >= 2) {
	  cout.precision(15);
	  cout << "Wolfe line search, lambda = " << lambda 
 	       << " value = " << valy /print_norm << endl;
// 	       << " derivative = " << derivative
// 	       << " lambda min = " << lambda_min << " lambda max = "
// 	       << lambda_max << endl; getchar();
	}
	if (valy <= valx + m1 * lambda * derivative) {
	  grad(y, r2); grad_computed = true;
	  T derivative2 = gmm::vect_sp(r2, d);
	  if (derivative2 >= m2*derivative) break;
	  lambda_min = lambda;
	}
	else {
	  lambda_max = lambda;
	  unbounded = false;
	}
	if (unbounded) lambda *= R(10);
	else  lambda = (lambda_max + lambda_min) / R(2);
	if (lambda == lambda_max || lambda == lambda_min) break;
	// valy <= R(2)*valx replaced by
	// valy <= valx + gmm::abs(derivative)*lambda_init
	// for compatibility with negative values (08.24.07).
	if (valy <= valx + R(2)*gmm::abs(derivative)*lambda &&
	    (lambda < R(lambda_init*1E-8) ||
	     (!unbounded && lambda_max-lambda_min < R(lambda_init*1E-8))))
	{ blocked = true; lambda = lambda_init; break; }
      }

      // Rank two update
      ++iter;
      if (!grad_computed) grad(y, r2);
      gmm::add(scaled(r2, -1), r);
      if (iter.get_iteration() % restart == 0 || blocked) { 
	if (iter.get_noisy() >= 1) cout << "Restart\n";
	invhessian.restart();
	if (++nb_restart > 10) {
	  if (iter.get_noisy() >= 1) cout << "BFGS is blocked, exiting\n";
	  return;
	}
      }
      else {
	invhessian.update(gmm::scaled(d,lambda), gmm::scaled(r,-1));
	nb_restart = 0;
      }
      copy(r2, r); copy(y, x); valx = valy;
      if (iter.get_noisy() >= 1)
	cout << "BFGS value " << valx/print_norm << "\t";
    }

  }


  template <typename FUNCTION, typename DERIVATIVE, typename VECTOR> 
  inline void dfp(FUNCTION f, DERIVATIVE grad, VECTOR &x,
	    int restart, iteration& iter, int version = 1) {
    bfgs(f, grad, x, restart, iter, version);

  }


}

#endif 

