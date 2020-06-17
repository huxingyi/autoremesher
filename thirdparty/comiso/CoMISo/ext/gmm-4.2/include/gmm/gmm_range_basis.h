/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2009-2012 Yves Renard
 
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

/**@file gmm_range_basis.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date March 10, 2009.
   @brief Extract a basis of the range of a (large sparse) matrix from the
          columns of this matrix.
*/
#ifndef GMM_RANGE_BASIS_H
#define GMM_RANGE_BASIS_H
#include "gmm_dense_qr.h"
#include "gmm_dense_lu.h"

#include "gmm_kernel.h"
#include "gmm_iter.h"
#include <set>
#include <list>


namespace gmm {


  template <typename T, typename VECT, typename MAT1>
  void tridiag_qr_algorithm
  (std::vector<typename number_traits<T>::magnitude_type> diag,
   std::vector<T> sdiag, const VECT &eigval_, const MAT1 &eigvect_,
   bool compvect, tol_type_for_qr tol = default_tol_for_qr) {
    VECT &eigval = const_cast<VECT &>(eigval_);
    MAT1 &eigvect = const_cast<MAT1 &>(eigvect_);
    typedef typename number_traits<T>::magnitude_type R;

    if (compvect) gmm::copy(identity_matrix(), eigvect);

    size_type n = diag.size(), q = 0, p, ite = 0;
    if (n == 0) return;
    if (n == 1) { eigval[0] = gmm::real(diag[0]); return; }

    symmetric_qr_stop_criterion(diag, sdiag, p, q, tol);

    while (q < n) {
      sub_interval SUBI(p, n-p-q), SUBJ(0, mat_ncols(eigvect)), SUBK(p, n-p-q);
      if (!compvect) SUBK = sub_interval(0,0);

      symmetric_Wilkinson_qr_step(sub_vector(diag, SUBI),
                                  sub_vector(sdiag, SUBI),
                                  sub_matrix(eigvect, SUBJ, SUBK), compvect);

      symmetric_qr_stop_criterion(diag, sdiag, p, q, tol*R(3));
      ++ite;
      GMM_ASSERT1(ite < n*100, "QR algorithm failed.");
    }

    gmm::copy(diag, eigval);
  }

  // Range basis with a restarted Lanczos method
  template <typename Mat>
  void range_basis_eff_Lanczos(const Mat &BB, std::set<size_type> &columns,
                       double EPS=1E-12) {
    typedef std::set<size_type> TAB;
    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type nc_r = columns.size(), k;
    col_matrix< rsvector<T> > B(mat_nrows(BB), mat_ncols(BB));

    k = 0;
    for (TAB::iterator it = columns.begin(); it!=columns.end(); ++it, ++k){
      gmm::copy(scaled(mat_col(BB, *it), T(1)/vect_norm2(mat_col(BB, *it))),
                mat_col(B, *it));
    }
    std::vector<T> w(mat_nrows(B));
    size_type restart = 120;
    std::vector<T> sdiag(restart);
    std::vector<R> eigval(restart), diag(restart);
    dense_matrix<T> eigvect(restart, restart);

    R rho = R(-1), rho2;
    while (nc_r) {

      std::vector<T> v(nc_r), v0(nc_r), wl(nc_r);
      dense_matrix<T> lv(nc_r, restart);

      if (rho < R(0)) { // Estimate of the spectral radius of B^* B
        gmm::fill_random(v);
        for (size_type i = 0; i < 100; ++i) {
          gmm::scale(v, T(1)/vect_norm2(v));
          gmm::copy(v, v0);
          k = 0; gmm::clear(w);
          for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it, ++k)
            add(scaled(mat_col(B, *it), v[k]), w);
          k = 0;
          for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it, ++k)
            v[k] = vect_hp(w, mat_col(B, *it));
          rho = gmm::abs(vect_hp(v, v0) / vect_hp(v0, v0));
        }
        rho *= R(2);
      }

      // Computing vectors of the null space of de B^* B with restarted Lanczos
      rho2 = 0;
      gmm::fill_random(v);
      size_type iter = 0;
      for(;;++iter) {
        R rho_old = rho2;
        R beta = R(0), alpha;
        gmm::scale(v, T(1)/vect_norm2(v));
        size_type eff_restart = restart;
    if (sdiag.size() != restart) {
      sdiag.resize(restart); eigval.resize(restart); diag.resize(restart); gmm::resize(eigvect, restart, restart);
      gmm::resize(lv, nc_r, restart);
    }

        for (size_type i = 0; i < restart; ++i) { // Lanczos iterations
          gmm::copy(v, mat_col(lv, i));
          gmm::clear(w);
          k = 0;
          for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it, ++k)
            add(scaled(mat_col(B, *it), v[k]), w);

          k = 0;
          for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it, ++k)
            wl[k] = v[k]*rho - vect_hp(w, mat_col(B, *it)) - beta*v0[k];
          alpha = gmm::real(vect_hp(wl, v));
          diag[i] = alpha;
          gmm::add(gmm::scaled(v, -alpha), wl);
          sdiag[i] = beta = vect_norm2(wl);
          gmm::copy(v, v0);
      if (beta < EPS) { eff_restart = i+1; break; }
      gmm::copy(gmm::scaled(wl, T(1) / beta), v);
    }
    if (eff_restart != restart) {
      sdiag.resize(eff_restart); eigval.resize(eff_restart); diag.resize(eff_restart);
      gmm::resize(eigvect, eff_restart, eff_restart); gmm::resize(lv, nc_r, eff_restart);
    }
        tridiag_qr_algorithm(diag, sdiag, eigval, eigvect, true);

        size_type num = size_type(-1);
        rho2 = R(0);
        for (size_type j = 0; j < eff_restart; ++j)
          { R nvp=gmm::abs(eigval[j]); if (nvp > rho2) { rho2=nvp; num=j; }}

        GMM_ASSERT1(num != size_type(-1), "Internal error");

        gmm::mult(lv, mat_col(eigvect, num), v);

        if (gmm::abs(rho2-rho_old) < rho_old*R(EPS)) break;
        // if (gmm::abs(rho-rho2) < rho*R(gmm::sqrt(EPS))) break;
        if (gmm::abs(rho-rho2) < rho*R(EPS)*R(100)) break;
      }

      if (gmm::abs(rho-rho2) < rho*R(EPS*10.)) {
        size_type j_max = size_type(-1), j = 0;
        R val_max = R(0);
        for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it, ++j)
          if (gmm::abs(v[j]) > val_max)
            { val_max = gmm::abs(v[j]); j_max = *it; }
        columns.erase(j_max); nc_r = columns.size();
      }
      else break;
    }
  }

  // Range basis with LU decomposition. Not stable from a numerical viewpoint.
  // Complex version not verified
  template <typename Mat>
  void range_basis_eff_lu(const Mat &B, std::set<size_type> &columns,
                          std::vector<bool> &c_ortho, double EPS) {

    typedef std::set<size_type> TAB;
    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type nc_r = 0, nc_o = 0, nc = mat_ncols(B), nr = mat_nrows(B), i, j;

    for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it)
      if (!(c_ortho[*it])) ++nc_r; else nc_o++;

    if (nc_r > 0) {

      gmm::row_matrix< gmm::rsvector<T> > Hr(nc, nc_r), Ho(nc, nc_o);
      gmm::row_matrix< gmm::rsvector<T> > BBr(nr, nc_r), BBo(nr, nc_o);

      i = j = 0;
      for (TAB::iterator it=columns.begin(); it!=columns.end(); ++it)
        if (!(c_ortho[*it]))
          { Hr(*it, i) = T(1)/ vect_norminf(mat_col(B, *it)); ++i; }
        else
          { Ho(*it, j) = T(1)/ vect_norm2(mat_col(B, *it)); ++j; }

      gmm::mult(B, Hr, BBr);
      gmm::mult(B, Ho, BBo);
      gmm::dense_matrix<T> M(nc_r, nc_r), BBB(nc_r, nc_o), MM(nc_r, nc_r);
      gmm::mult(gmm::conjugated(BBr), BBr, M);
      gmm::mult(gmm::conjugated(BBr), BBo, BBB);
      gmm::mult(BBB, gmm::conjugated(BBB), MM);
      gmm::add(gmm::scaled(MM, T(-1)), M);

      std::vector<int> ipvt(nc_r);
      gmm::lu_factor(M, ipvt);

      R emax = R(0);
      for (i = 0; i < nc_r; ++i) emax = std::max(emax, gmm::abs(M(i,i)));

      i = 0;
      std::set<size_type> c = columns;
      for (TAB::iterator it = c.begin(); it != c.end(); ++it)
        if (!(c_ortho[*it])) {
          if (gmm::abs(M(i,i)) <= R(EPS)*emax) columns.erase(*it);
          ++i;
        }
    }
  }


  // Range basis with Gram-Schmidt orthogonalization (sparse version)
  // The sparse version is better when the sparsity is high and less efficient
  // than the dense version for high degree elements (P3, P4 ...)
  // Complex version not verified
  template <typename Mat>
  void range_basis_eff_Gram_Schmidt_sparse(const Mat &BB,
                                           std::set<size_type> &columns,
                                           std::vector<bool> &c_ortho,
                                           double EPS) {

    typedef std::set<size_type> TAB;
    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type nc = mat_ncols(BB), nr = mat_nrows(BB);
    std::set<size_type> c = columns, rc = columns;

    gmm::col_matrix< rsvector<T> > B(nr, nc);
    for (std::set<size_type>::iterator it = columns.begin();
         it != columns.end(); ++it) {
      gmm::copy(mat_col(BB, *it), mat_col(B, *it));
      gmm::scale(mat_col(B, *it), T(1)/vect_norm2(mat_col(B, *it)));
    }

    for (std::set<size_type>::iterator it = c.begin(); it != c.end(); ++it)
      if (c_ortho[*it]) {
        for (std::set<size_type>::iterator it2 = rc.begin();
             it2 != rc.end(); ++it2)
          if (!(c_ortho[*it2])) {
            T r = -vect_hp(mat_col(B, *it2), mat_col(B, *it));
            if (r != T(0)) add(scaled(mat_col(B, *it), r), mat_col(B, *it2));
          }
        rc.erase(*it);
      }

    while (rc.size()) {
      R nmax = R(0); size_type cmax = size_type(-1);
      for (std::set<size_type>::iterator it=rc.begin(); it != rc.end();) {
        TAB::iterator itnext = it; ++itnext;
        R n = vect_norm2(mat_col(B, *it));
        if (nmax < n) { nmax = n; cmax = *it; }
        if (n < R(EPS)) { columns.erase(*it); rc.erase(*it); }
        it = itnext;
      }

      if (nmax < R(EPS)) break;

      gmm::scale(mat_col(B, cmax), T(1)/vect_norm2(mat_col(B, cmax)));
      rc.erase(cmax);
      for (std::set<size_type>::iterator it=rc.begin(); it!=rc.end(); ++it) {
        T r = -vect_hp(mat_col(B, *it), mat_col(B, cmax));
        if (r != T(0)) add(scaled(mat_col(B, cmax), r), mat_col(B, *it));
      }
    }
    for (std::set<size_type>::iterator it=rc.begin(); it!=rc.end(); ++it)
      columns.erase(*it);
  }


  // Range basis with Gram-Schmidt orthogonalization (dense version)
  template <typename Mat>
  void range_basis_eff_Gram_Schmidt_dense(const Mat &B,
                                          std::set<size_type> &columns,
                                          std::vector<bool> &c_ortho,
                                          double EPS) {

    typedef std::set<size_type> TAB;
    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type nc_r = columns.size(), nc = mat_ncols(B), nr = mat_nrows(B), i;
    std::set<size_type> rc;

    row_matrix< gmm::rsvector<T> > H(nc, nc_r), BB(nr, nc_r);
    std::vector<T> v(nc_r);
    std::vector<size_type> ind(nc_r);

    i = 0;
    for (TAB::iterator it = columns.begin(); it != columns.end(); ++it, ++i)
      H(*it, i) = T(1) / vect_norm2(mat_col(B, *it));

    mult(B, H, BB);
    dense_matrix<T> M(nc_r, nc_r);
    mult(gmm::conjugated(BB), BB, M);

    i = 0;
    for (TAB::iterator it = columns.begin(); it != columns.end(); ++it, ++i)
      if (c_ortho[*it]) {
        gmm::copy(mat_row(M, i), v);
        rank_one_update(M, scaled(v, T(-1)), v);
        M(i, i) = T(1);
      }
      else { rc.insert(i); ind[i] = *it; }

    while (rc.size() > 0) {

      // Next pivot
      R nmax = R(0); size_type imax = size_type(-1);
      for (TAB::iterator it = rc.begin(); it != rc.end();) {
        TAB::iterator itnext = it; ++itnext;
        R a = gmm::abs(M(*it, *it));
        if (a > nmax) { nmax = a; imax = *it; }
        if (a < R(EPS)) { columns.erase(ind[*it]); rc.erase(*it); }
        it = itnext;
      }

      if (nmax < R(EPS)) break;

      // Normalization
      gmm::scale(mat_row(M, imax), T(1) / sqrt(nmax));
      gmm::scale(mat_col(M, imax), T(1) / sqrt(nmax));

      // orthogonalization
      copy(mat_row(M, imax), v);
      rank_one_update(M, scaled(v, T(-1)), v);
      M(imax, imax) = T(1);

      rc.erase(imax);
    }
    for (std::set<size_type>::iterator it=rc.begin(); it!=rc.end(); ++it)
      columns.erase(ind[*it]);
  }

  template <typename L> size_type nnz_eps(const L& l, double eps) {
    typename linalg_traits<L>::const_iterator it = vect_const_begin(l),
      ite = vect_const_end(l);
    size_type res(0);
    for (; it != ite; ++it) if (gmm::abs(*it) >= eps) ++res;
    return res;
  }

  template <typename L>
  bool reserve__rb(const L& l, std::vector<bool> &b, double eps) {
    typename linalg_traits<L>::const_iterator it = vect_const_begin(l),
      ite = vect_const_end(l);
    bool ok = true;
    for (; it != ite; ++it)
      if (gmm::abs(*it) >= eps && b[it.index()]) ok = false;
    if (ok) {
      for (it = vect_const_begin(l); it != ite; ++it)
        if (gmm::abs(*it) >= eps) b[it.index()] = true;
    }
    return ok;
  }

  template <typename Mat>
  void range_basis(const Mat &B, std::set<size_type> &columns,
                       double EPS, col_major, bool skip_init=false) {

    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;

    size_type nc = mat_ncols(B), nr = mat_nrows(B);

    std::vector<R> norms(nc);
    std::vector<bool> c_ortho(nc), booked(nr);
    std::vector< std::set<size_type> > nnzs(mat_nrows(B));

    if (!skip_init) {

      R norm_max = R(0);
      for (size_type i = 0; i < nc; ++i) {
        norms[i] = vect_norminf(mat_col(B, i));
        norm_max = std::max(norm_max, norms[i]);
      }

      columns.clear();
      for (size_type i = 0; i < nc; ++i)
        if (norms[i] > norm_max*R(EPS)) {
          columns.insert(i);
          nnzs[nnz_eps(mat_col(B, i), R(EPS) * norms[i])].insert(i);
        }

      for (size_type i = 1; i < nr; ++i)
        for (std::set<size_type>::iterator it = nnzs[i].begin();
             it != nnzs[i].end(); ++it)
          if (reserve__rb(mat_col(B, *it), booked, R(EPS) * norms[*it]))
            c_ortho[*it] = true;
    }

    size_type sizesm[7] = {125, 200, 350, 550, 800, 1100, 1500}, actsize;
    for (int k = 0; k < 7; ++k) {
      size_type nc_r = columns.size();
      std::set<size_type> c1, cres;
      actsize = sizesm[k];
      for (std::set<size_type>::iterator it = columns.begin();
           it != columns.end(); ++it) {
        c1.insert(*it);
        if (c1.size() >= actsize) {
          range_basis_eff_Gram_Schmidt_dense(B, c1, c_ortho, EPS);
          for (std::set<size_type>::iterator it2=c1.begin(); it2 != c1.end();
               ++it2) cres.insert(*it2);
          c1.clear();
        }
      }
      if (c1.size() > 10)
        range_basis_eff_Gram_Schmidt_dense(B, c1, c_ortho, EPS);
      for (std::set<size_type>::iterator it = c1.begin(); it != c1.end(); ++it)
        cres.insert(*it);
      columns = cres;
      if (nc_r <= actsize) return;
      if (columns.size() == nc_r) break;
      if (sizesm[k] >= 350 && columns.size() > (nc_r*19)/20) break;
    }
    // cout << "size of columns " << columns.size() << endl;
    if (columns.size() > std::max(size_type(10), actsize))
      range_basis_eff_Lanczos(B, columns, EPS);
    else
      range_basis_eff_Gram_Schmidt_dense(B, columns, c_ortho, EPS);
  }


  template <typename Mat>
  void range_basis(const Mat &B, std::set<size_type> &columns,
                   double EPS, row_major) {
    typedef typename  linalg_traits<Mat>::value_type T;
    gmm::col_matrix< rsvector<T> > BB(mat_nrows(B), mat_ncols(B));
    GMM_WARNING3("A copy of a row matrix is done into a column matrix "
                 "for range basis algorithm.");
    gmm::copy(B, BB);
    range_basis(BB, columns, EPS);
  }

  /** Range Basis :
    Extract a basis of the range of a (large sparse) matrix selecting some
    column vectors of this matrix. This is in particular useful to select
    an independent set of linear constraints.

    The algorithm is optimized for two cases :
       - when the (non trivial) kernel is small. An iterativ algorithm
         based on Lanczos method is applied
       - when the (non trivial) kernel is large and most of the dependencies
         can be detected locally. A block Gram-Schmidt is applied first then
         a restarted Lanczos method when the remaining kernel is greatly
         smaller.
    The restarted Lanczos method could be improved or replaced by a block
    Lanczos method, a block Wiedelann method (in order to be parallelized for
    instance) or simply could compute more than one vector of the null
    space at each iteration.
    The LU decomposition has been tested for local elimination but gives bad
    results : the algorithm is unstable and do not permit to give the right
    number of vector at the end of the process. Moreover, the number of final
    vector depend greatly on the number of vectors in a block of the local
    analysis.
  */
  template <typename Mat>
  void range_basis(const Mat &B, std::set<size_type> &columns,
                   double EPS=1E-12) {
    range_basis(B, columns, EPS,
                typename principal_orientation_type
                <typename linalg_traits<Mat>::sub_orientation>::potype());
}

}

#endif
