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

/** @file gmm_domain_decomp.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date May 21, 2004.
    @brief Domain decomposition.
*/
#ifndef GMM_DOMAIN_DECOMP_H__
#define GMM_DOMAIN_DECOMP_H__

#include "gmm_kernel.h"
#include <map>


namespace gmm {

  /** This function separates into small boxes of size msize with a ratio
   * of overlap (in [0,1[) a set of points. The result is given into a
   * vector of sparse matrices vB.
   */
  template <typename Matrix, typename Point>
  void rudimentary_regular_decomposition(std::vector<Point> pts,
					 double msize,
					 double overlap,
					 std::vector<Matrix> &vB) {
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef abstract_null_type void_type;
    typedef std::map<size_type, void_type> map_type;

    size_type nbpts = pts.size();
    if (!nbpts || pts[0].size() == 0) { vB.resize(0); return; }
    int dim = int(pts[0].size());

    // computation of the global box and the number of sub-domains
    Point pmin = pts[0], pmax = pts[0];
    for (size_type i = 1; i < nbpts; ++i)
      for (int k = 0; k < dim; ++k) {
	pmin[k] = std::min(pmin[k], pts[i][k]);
	pmax[k] = std::max(pmax[k], pts[i][k]);
      }
    
    std::vector<size_type> nbsub(dim), mult(dim);
    std::vector<int> pts1(dim), pts2(dim);
    size_type nbtotsub = 1;
    for (int k = 0; k < dim; ++k) {
      nbsub[k] = size_type((pmax[k] - pmin[k]) / msize)+1;
      mult[k] = nbtotsub; nbtotsub *= nbsub[k];
    }
    
    std::vector<map_type> subs(nbtotsub);
    // points ventilation
    std::vector<size_type> ns(dim), na(dim), nu(dim);
    for (size_type i = 0; i < nbpts; ++i) {
      for (int k = 0; k < dim; ++k) {
	register double a = (pts[i][k] - pmin[k]) / msize;
	ns[k] = size_type(a) - 1; na[k] = 0;
	pts1[k] = int(a + overlap); pts2[k] = int(ceil(a-1.0-overlap));
      }
      size_type sum = 0;
      do {
	bool ok = 1;
	for (int k = 0; k < dim; ++k)
	  if ((ns[k] >= nbsub[k]) || (pts1[k] < int(ns[k]))
	      || (pts2[k] > int(ns[k]))) { ok = false; break; }
	if (ok) {
	  size_type ind = ns[0];
	  for (int k=1; k < dim; ++k) ind += ns[k]*mult[k];
	  subs[ind][i] = void_type();
	}
	for (int k = 0; k < dim; ++k) {
	  if (na[k] < 2) { na[k]++; ns[k]++; ++sum; break; }
	  na[k] = 0; ns[k] -= 2; sum -= 2;
	}
      } while (sum);
    }
    // delete too small domains.
    size_type nbmaxinsub = 0;
    for (size_type i = 0; i < nbtotsub; ++i)
      nbmaxinsub = std::max(nbmaxinsub, subs[i].size());
    
    std::fill(ns.begin(), ns.end(), size_type(0));
    for (size_type i = 0; i < nbtotsub; ++i) {
      if (subs[i].size() > 0 && subs[i].size() < nbmaxinsub / 10) {
	
	for (int k = 0; k < dim; ++k) nu[k] = ns[k];
	size_type nbmax = 0, imax = 0;
	
	for (int l = 0; l < dim; ++l) {
	  nu[l]--;
	  for (int m = 0; m < 2; ++m, nu[l]+=2) {
	    bool ok = true;
	    for (int k = 0; k < dim && ok; ++k) 
	      if (nu[k] >= nbsub[k]) ok = false;
	    if (ok) {
	      size_type ind = ns[0];
	      for (int k=1; k < dim; ++k) ind += ns[k]*mult[k];
	      if (subs[ind].size() > nbmax)
		{ nbmax = subs[ind].size(); imax = ind; }
	    }
	  }
	  nu[l]--;
	}
	
	if (nbmax > subs[i].size()) {
	  for (map_type::iterator it=subs[i].begin(); it!=subs[i].end(); ++it)
	    subs[imax][it->first] = void_type();
	  subs[i].clear();
	}
      }
      for (int k = 0; k < dim; ++k)
	{ ns[k]++; if (ns[k] < nbsub[k]) break; ns[k] = 0; }
    }
    
    // delete empty domains.
    size_type effnb = 0;
    for (size_type i = 0; i < nbtotsub; ++i) {
      if (subs[i].size() > 0)
	{ if (i != effnb) std::swap(subs[i], subs[effnb]); ++effnb; }
    }

    // build matrices
    subs.resize(effnb);
    vB.resize(effnb);
    for (size_type i = 0; i < effnb; ++i) {
      clear(vB[i]); resize(vB[i], nbpts, subs[i].size());
      size_type j = 0;
      for (map_type::iterator it=subs[i].begin(); it!=subs[i].end(); ++it, ++j)
	vB[i](it->first, j) = value_type(1);
    }
  }
  

}


#endif
