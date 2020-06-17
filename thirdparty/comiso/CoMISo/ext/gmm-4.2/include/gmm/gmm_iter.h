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

/**@file gmm_iter.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date February 10, 2003.
   @brief Iteration object.
*/

#ifndef GMM_ITER_H__
#define GMM_ITER_H__

#include "gmm_kernel.h"

namespace gmm {

  /**  The Iteration object calculates whether the solution has reached the
       desired accuracy, or whether the maximum number of iterations has
       been reached. 

       The method finished() checks the convergence.  The first()
       method is used to determine the first iteration of the loop.
  */
  class iteration {
  protected :
    double rhsn;       /* Right hand side norm.                            */
    size_type maxiter; /* Max. number of iterations.                       */
    int noise;         /* if noise > 0 iterations are printed.             */
    double resmax;     /* maximum residu.                                  */
    double resminreach, resadd;
    double diverged_res; /* Threshold beyond which the iterative           */
                       /* is considered to diverge.                        */
    size_type nit;     /* iteration number.                                */
    double res;        /* last computed residu.                            */
    std::string name;  /* eventually, name of the method.                  */
    bool written;
    void (*callback)(const gmm::iteration&);
  public :

    void init(void) { 
      nit = 0; res = 0.0; written = false; 
      resminreach = 1E200; resadd = 0.0; 
      callback = 0;
    }

    iteration(double r = 1.0E-8, int noi = 0, size_type mit = size_type(-1),
	      double div_res = 1E200)
      : rhsn(1.0), maxiter(mit), noise(noi), resmax(r), diverged_res(div_res)
    { init(); }

    void  operator ++(int) {  nit++; written = false; resadd += res; }
    void  operator ++() { (*this)++; }

    bool first(void) { return nit == 0; }

    /* get/set the "noisyness" (verbosity) of the solvers */
    int get_noisy(void) const { return noise; }
    void set_noisy(int n) { noise = n; }
    void reduce_noisy(void) { if (noise > 0) noise--; }

    double get_resmax(void) const { return resmax; }
    void set_resmax(double r) { resmax = r; }

    double get_res() const { return res; }
    void enforce_converged(bool c = true)
    { if (c) res = double(0); else res = rhsn * resmax + double(1); }

    /* change the user-definable callback, called after each iteration */
    void set_callback(void (*t)(const gmm::iteration&)) {
      callback = t;
    }

    double get_diverged_residual(void) const { return diverged_res; }
    void set_diverged_residual(double r) { diverged_res = r; }

    size_type get_iteration(void) const { return nit; }
    void set_iteration(size_type i) { nit = i; }
    
    size_type get_maxiter(void) const { return maxiter; }
    void set_maxiter(size_type i) { maxiter = i; }

    double get_rhsnorm(void) const { return rhsn; }
    void set_rhsnorm(double r) { rhsn = r; }
    
    bool converged(void) { return res <= rhsn * resmax; }
    bool converged(double nr) { 
      res = gmm::abs(nr); resminreach = std::min(resminreach, res);
      return converged();
    }
    template <typename VECT> bool converged(const VECT &v)
    { return converged(gmm::vect_norm2(v)); }
    bool diverged(void)
    { return (nit>=maxiter) || (res>=rhsn*diverged_res && nit > 4); }
    bool diverged(double nr) { 
      res = gmm::abs(nr); resminreach = std::min(resminreach, res);
      return diverged();
    }

    bool finished(double nr) {
      if (callback) callback(*this);
      if (noise > 0 && !written) {
	double a = (rhsn == 0) ? 1.0 : rhsn;
	converged(nr);
	cout << name << " iter " << nit << " residual "
	     << gmm::abs(nr) / a;
// 	if (nit % 100 == 0 && nit > 0) {
// 	  cout << " (residual min " << resminreach / a << " mean val "
// 	       << resadd / (100.0 * a) << " )";
// 	  resadd = 0.0;
// 	}
	cout <<  endl;
	written = true;
      }
      return (converged(nr) || diverged(nr));
    }
    template <typename VECT> bool finished_vect(const VECT &v)
    { return finished(double(gmm::vect_norm2(v))); }


    void set_name(const std::string &n) { name = n; }
    const std::string &get_name(void) const { return name; }

  };

}

#endif /* GMM_ITER_H__ */
