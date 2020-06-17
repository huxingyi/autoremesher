/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard
 
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

/**@file gmm_precond_ildltt.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date June 30, 2003.
   @brief incomplete LDL^t (cholesky) preconditioner with fill-in and threshold.
*/

#ifndef GMM_PRECOND_ILDLTT_H
#define GMM_PRECOND_ILDLTT_H

// Store U = LT and D in indiag. On each line, the fill-in is the number
// of non-zero elements on the line of the original matrix plus K, except if
// the matrix is dense. In this case the fill-in is K on each line.

#include "gmm_precond_ilut.h"

namespace gmm {
  /** incomplete LDL^t (cholesky) preconditioner with fill-in and
      threshold. */
  template <typename Matrix>
  class ildltt_precond  {
  public :
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type magnitude_type;
    
    typedef rsvector<value_type> svector;

    row_matrix<svector> U;
    std::vector<magnitude_type> indiag;

  protected:
    size_type K;
    double eps;    

    template<typename M> void do_ildltt(const M&, row_major);
    void do_ildltt(const Matrix&, col_major);

  public:
    void build_with(const Matrix& A, int k_ = -1, double eps_ = double(-1)) {
      if (k_ >= 0) K = k_;
      if (eps_ >= double(0)) eps = eps_;
      gmm::resize(U, mat_nrows(A), mat_ncols(A));
      indiag.resize(std::min(mat_nrows(A), mat_ncols(A)));
      do_ildltt(A, typename principal_orientation_type<typename
		linalg_traits<Matrix>::sub_orientation>::potype());
    }
    ildltt_precond(const Matrix& A, int k_, double eps_) 
      : U(mat_nrows(A),mat_ncols(A)), K(k_), eps(eps_) { build_with(A); }
    ildltt_precond(void) { K=10; eps = 1E-7; }
    ildltt_precond(size_type k_, double eps_) :  K(k_), eps(eps_) {}
    size_type memsize() const { 
      return sizeof(*this) + nnz(U)*sizeof(value_type) + indiag.size() * sizeof(magnitude_type);
    }    
  };

  template<typename Matrix> template<typename M> 
  void ildltt_precond<Matrix>::do_ildltt(const M& A,row_major) {
    typedef value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type n = mat_nrows(A);
    if (n == 0) return;
    svector w(n);
    T tmp;
    R prec = default_tol(R()), max_pivot = gmm::abs(A(0,0)) * prec;

    gmm::clear(U);
    for (size_type i = 0; i < n; ++i) {
      gmm::copy(mat_const_row(A, i), w);
      double norm_row = gmm::vect_norm2(w);

      for (size_type krow = 0, k; krow < w.nb_stored(); ++krow) {
	typename svector::iterator wk = w.begin() + krow;
	if ((k = wk->c) >= i) break;
 	if (gmm::is_complex(wk->e)) {
 	  tmp = gmm::conj(U(k, i))/indiag[k]; // not completely satisfactory ..
 	  gmm::add(scaled(mat_row(U, k), -tmp), w);
 	}
 	else {
	  tmp = wk->e;
	  if (gmm::abs(tmp) < eps * norm_row) { w.sup(k); --krow; } 
	  else { wk->e += tmp; gmm::add(scaled(mat_row(U, k), -tmp), w); }
	}
      }
      tmp = w[i];

      if (gmm::abs(gmm::real(tmp)) <= max_pivot)
	{ GMM_WARNING2("pivot " << i << " is too small"); tmp = T(1); }

      max_pivot = std::max(max_pivot, std::min(gmm::abs(tmp) * prec, R(1)));
      indiag[i] = R(1) / gmm::real(tmp);
      gmm::clean(w, eps * norm_row);
      gmm::scale(w, T(indiag[i]));
      std::sort(w.begin(), w.end(), elt_rsvector_value_less_<T>());
      typename svector::const_iterator wit = w.begin(), wite = w.end();
      for (size_type nnu = 0; wit != wite; ++wit)  // copy to be optimized ...
	if (wit->c > i) { if (nnu < K) { U(i, wit->c) = wit->e; ++nnu; } }
    }
  }

  template<typename Matrix> 
  void ildltt_precond<Matrix>::do_ildltt(const Matrix& A, col_major)
  { do_ildltt(gmm::conjugated(A), row_major()); }

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const ildltt_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    gmm::copy(v1, v2);
    gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
    gmm::upper_tri_solve(P.U, v2, true);
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const ildltt_precond<Matrix>& P,const V1 &v1, V2 &v2)
  { mult(P, v1, v2); }

  template <typename Matrix, typename V1, typename V2> inline
  void left_mult(const ildltt_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    copy(v1, v2);
    gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
  }

  template <typename Matrix, typename V1, typename V2> inline
  void right_mult(const ildltt_precond<Matrix>& P, const V1 &v1, V2 &v2)
  { copy(v1, v2); gmm::upper_tri_solve(P.U, v2, true); }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_left_mult(const ildltt_precond<Matrix>& P, const V1 &v1,
			    V2 &v2) {
    copy(v1, v2);
    gmm::upper_tri_solve(P.U, v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_right_mult(const ildltt_precond<Matrix>& P, const V1 &v1,
			     V2 &v2)
  { copy(v1, v2); gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true); }


  // for compatibility with old versions

  template <typename Matrix>
  struct choleskyt_precond : public ildltt_precond<Matrix>{
    choleskyt_precond(const Matrix& A, int k_, double eps_)
      : ildltt_precond<Matrix>(A, k_, eps_) {}
    choleskyt_precond(void) {}
  } IS_DEPRECATED;

  template <typename Matrix, typename V1, typename V2> inline
  void mult(const choleskyt_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    gmm::copy(v1, v2);
    gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
    gmm::upper_tri_solve(P.U, v2, true);
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_mult(const choleskyt_precond<Matrix>& P,const V1 &v1, V2 &v2)
  { mult(P, v1, v2); }

  template <typename Matrix, typename V1, typename V2> inline
  void left_mult(const choleskyt_precond<Matrix>& P, const V1 &v1, V2 &v2) {
    copy(v1, v2);
    gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
  }

  template <typename Matrix, typename V1, typename V2> inline
  void right_mult(const choleskyt_precond<Matrix>& P, const V1 &v1, V2 &v2)
  { copy(v1, v2); gmm::upper_tri_solve(P.U, v2, true); }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_left_mult(const choleskyt_precond<Matrix>& P, const V1 &v1,
			    V2 &v2) {
    copy(v1, v2);
    gmm::upper_tri_solve(P.U, v2, true);
    for (size_type i = 0; i < P.indiag.size(); ++i) v2[i] *= P.indiag[i];
  }

  template <typename Matrix, typename V1, typename V2> inline
  void transposed_right_mult(const choleskyt_precond<Matrix>& P, const V1 &v1,
			     V2 &v2)
  { copy(v1, v2); gmm::lower_tri_solve(gmm::conjugated(P.U), v2, true); }

}

#endif 

