/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard, Caroline Lecalvez
 
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

/**@file gmm_solver_idgmres.h
   @author  Caroline Lecalvez <Caroline.Lecalvez@gmm.insa-tlse.fr>
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 6, 2003.
   @brief Implicitly restarted and deflated Generalized Minimum Residual.
*/
#ifndef GMM_IDGMRES_H
#define GMM_IDGMRES_H

#include "gmm_kernel.h"
#include "gmm_iter.h"
#include "gmm_dense_sylvester.h"

namespace gmm {

  template <typename T> compare_vp {
    bool operator()(const std::pair<T, size_type> &a,
		    const std::pair<T, size_type> &b) const
    { return (gmm::abs(a.first) > gmm::abs(b.first)); }
  }

  struct idgmres_state {
    size_type m, tb_deb, tb_def, p, k, nb_want, nb_unwant;
    size_type nb_nolong, tb_deftot, tb_defwant, conv, nb_un, fin;
    bool ok;

    idgmres_state(size_type mm, size_type pp, size_type kk)
      : m(mm), tb_deb(1), tb_def(0), p(pp), k(kk), nb_want(0),
	nb_unwant(0), nb_nolong(0), tb_deftot(0), tb_defwant(0),
	conv(0), nb_un(0), fin(0), ok(false); {}
  }

    idgmres_state(size_type mm, size_type pp, size_type kk)
      : m(mm), tb_deb(1), tb_def(0), p(pp), k(kk), nb_want(0),
	nb_unwant(0), nb_nolong(0), tb_deftot(0), tb_defwant(0),
	conv(0), nb_un(0), fin(0), ok(false); {}
  

  template <typename CONT, typename IND>
  apply_permutation(CONT &cont, const IND &ind) {
    size_type m = ind.end() - ind.begin();
    std::vector<bool> sorted(m, false);
    
    for (size_type l = 0; l < m; ++l)
      if (!sorted[l] && ind[l] != l) {

	typeid(cont[0]) aux = cont[l];
	k = ind[l];
	cont[l] = cont[k];
	sorted[l] = true;
	
	for(k2 = ind[k]; k2 != l; k2 = ind[k]) {
	  cont[k] = cont[k2];
	  sorted[k] = true;
	  k = k2;
	}
	cont[k] = aux;
      }
  }


  /** Implicitly restarted and deflated Generalized Minimum Residual

      See: C. Le Calvez, B. Molina, Implicitly restarted and deflated
      FOM and GMRES, numerical applied mathematics,
      (30) 2-3 (1999) pp191-212.
      
      @param A Real or complex unsymmetric matrix.
      @param x initial guess vector and final result.
      @param b right hand side
      @param M preconditionner
      @param m size of the subspace between two restarts
      @param p number of converged ritz values seeked
      @param k size of the remaining Krylov subspace when the p ritz values
      have not yet converged 0 <= p <= k < m.
      @param tol_vp : tolerance on the ritz values.
      @param outer
      @param KS
  */
  template < typename Mat, typename Vec, typename VecB, typename Precond,
	     typename Basis >
  void idgmres(const Mat &A, Vec &x, const VecB &b, const Precond &M,
	     size_type m, size_type p, size_type k, double tol_vp,
	     iteration &outer, Basis& KS) {

    typedef typename linalg_traits<Mat>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    
    R a, beta;
    idgmres_state st(m, p, k);

    std::vector<T> w(vect_size(x)), r(vect_size(x)), u(vect_size(x));
    std::vector<T> c_rot(m+1), s_rot(m+1), s(m+1);
    std::vector<T> y(m+1), ztest(m+1), gam(m+1);
    std::vector<T> gamma(m+1);
    gmm::dense_matrix<T> H(m+1, m), Hess(m+1, m),
      Hobl(m+1, m), W(vect_size(x), m+1);

    gmm::clear(H);

    outer.set_rhsnorm(gmm::vect_norm2(b));
    if (outer.get_rhsnorm() == 0.0) { clear(x); return; }
    
    mult(A, scaled(x, -1.0), b, w);
    mult(M, w, r);
    beta = gmm::vect_norm2(r);

    iteration inner = outer;
    inner.reduce_noisy();
    inner.set_maxiter(m);
    inner.set_name("GMRes inner iter");
    
    while (! outer.finished(beta)) {
      
      gmm::copy(gmm::scaled(r, 1.0/beta), KS[0]);
      gmm::clear(s);
      s[0] = beta;
      gmm::copy(s, gamma);

      inner.set_maxiter(m - st.tb_deb + 1);
      size_type i = st.tb_deb - 1; inner.init();
      
      do {
	mult(A, KS[i], u);
	mult(M, u, KS[i+1]);
	orthogonalize_with_refinment(KS, mat_col(H, i), i);
	H(i+1, i) = a = gmm::vect_norm2(KS[i+1]);
	gmm::scale(KS[i+1], R(1) / a);

	gmm::copy(mat_col(H, i), mat_col(Hess, i));
	gmm::copy(mat_col(H, i), mat_col(Hobl, i));
	

	for (size_type l = 0; l < i; ++l)
	  Apply_Givens_rotation_left(H(l,i), H(l+1,i), c_rot[l], s_rot[l]);
	
	Givens_rotation(H(i,i), H(i+1,i), c_rot[i], s_rot[i]);
	Apply_Givens_rotation_left(H(i,i), H(i+1,i), c_rot[i], s_rot[i]);
	H(i+1, i) = T(0); 
	Apply_Givens_rotation_left(s[i], s[i+1], c_rot[i], s_rot[i]);
	
	++inner, ++outer, ++i;
      } while (! inner.finished(gmm::abs(s[i])));

      if (inner.converged()) {
	gmm::copy(s, y);
	upper_tri_solve(H, y, i, false);
	combine(KS, y, x, i);
	mult(A, gmm::scaled(x, T(-1)), b, w);
	mult(M, w, r);
	beta = gmm::vect_norm2(r); // + verif sur beta ... à faire
	break;
      }

      gmm::clear(gam); gam[m] = s[i];
      for (size_type l = m; l > 0; --l)
	Apply_Givens_rotation_left(gam[l-1], gam[l], gmm::conj(c_rot[l-1]),
				   -s_rot[l-1]);

      mult(KS.mat(), gam, r);
      beta = gmm::vect_norm2(r);
      
      mult(Hess, scaled(y, T(-1)), gamma, ztest);
      // En fait, d'après Caroline qui s'y connait ztest et gam devrait
      // être confondus
      // Quand on aura vérifié que ça marche, il faudra utiliser gam à la 
      // place de ztest.
      if (st.tb_def < p) {
        T nss = H(m,m-1) / ztest[m];
	nss /= gmm::abs(nss); // ns à calculer plus tard aussi
	gmm::copy(KS.mat(), W); gmm::copy(scaled(r, nss /beta), mat_col(W, m));
	
	// Computation of the oblique matrix
	sub_interval SUBI(0, m);
	add(scaled(sub_vector(ztest, SUBI), -Hobl(m, m-1) / ztest[m]),
	    sub_vector(mat_col(Hobl, m-1), SUBI));
	Hobl(m, m-1) *= nss * beta / ztest[m]; 

	/* **************************************************************** */
	/*  Locking                                                         */
	/* **************************************************************** */

	// Computation of the Ritz eigenpairs.
	std::vector<std::complex<R> > eval(m);
	dense_matrix<T> YB(m-st.tb_def, m-st.tb_def);
	std::vector<char> pure(m-st.tb_def, 0);
	gmm::clear(YB);

	select_eval(Hobl, eval, YB, pure, st);

	if (st.conv != 0) {
	  // DEFLATION using the QR Factorization of YB
	  
	  T alpha = Lock(W, Hobl,
			 sub_matrix(YB,  sub_interval(0, m-st.tb_def)),
			 sub_interval(st.tb_def, m-st.tb_def), 
			 (st.tb_defwant < p)); 
	  // ns *= alpha; // à calculer plus tard ??
	  //  V(:,m+1) = alpha*V(:, m+1); ça devait servir à qlq chose ...


	  //       Clean the portions below the diagonal corresponding
	  //       to the lock Schur vectors

	  for (size_type j = st.tb_def; j < st.tb_deftot; ++j) {
	    if ( pure[j-st.tb_def] == 0)
	      gmm::clear(sub_vector(mat_col(Hobl,j), sub_interval(j+1,m-j)));
	    else if (pure[j-st.tb_def] == 1) {
	      gmm::clear(sub_matrix(Hobl, sub_interval(j+2,m-j-1),
				    sub_interval(j, 2))); 
	      ++j;
	    }
	    else GMM_ASSERT3(false, "internal error");
	  }
	  
	  if (!st.ok) {

	    // attention si m = 0;
	    size_type mm = std::min(k+st.nb_unwant+st.nb_nolong, m-1);

	    if (eval_sort[m-mm-1].second != R(0)
		&& eval_sort[m-mm-1].second == -eval_sort[m-mm].second) ++mm;

	    std::vector<complex<R> > shifts(m-mm);
	    for (size_type i = 0; i < m-mm; ++i)
	      shifts[i] = eval_sort[i].second;

	    apply_shift_to_Arnoldi_factorization(W, Hobl, shifts, mm,
						 m-mm, true);

	    st.fin = mm;
	  }
	  else
	    st.fin = st.tb_deftot;


	  /* ************************************************************** */
	  /*  Purge                                                         */
	  /* ************************************************************** */

	  if (st.nb_nolong + st.nb_unwant > 0) {

	    std::vector<std::complex<R> > eval(m);
	    dense_matrix<T> YB(st.fin, st.tb_deftot);
	    std::vector<char> pure(st.tb_deftot, 0);
	    gmm::clear(YB);
	    st.nb_un = st.nb_nolong + st.nb_unwant;
	    
	    select_eval_for_purging(Hobl, eval, YB, pure, st);
	    
	    T alpha = Lock(W, Hobl, YB, sub_interval(0, st.fin), ok);

	    //       Clean the portions below the diagonal corresponding
	    //       to the unwanted lock Schur vectors
	    
	    for (size_type j = 0; j < st.tb_deftot; ++j) {
	      if ( pure[j] == 0)
		gmm::clear(sub_vector(mat_col(Hobl,j), sub_interval(j+1,m-j)));
	      else if (pure[j] == 1) {
		gmm::clear(sub_matrix(Hobl, sub_interval(j+2,m-j-1),
				      sub_interval(j, 2))); 
		++j;
	      }
	      else GMM_ASSERT3(false, "internal error");
	    }

	    gmm::dense_matrix<T> z(st.nb_un, st.fin - st.nb_un);
	    sub_interval SUBI(0, st.nb_un), SUBJ(st.nb_un, st.fin - st.nb_un);
	    sylvester(sub_matrix(Hobl, SUBI),
		      sub_matrix(Hobl, SUBJ),
		      sub_matrix(gmm::scaled(Hobl, -T(1)), SUBI, SUBJ), z);
	    
	  }

	}
	
      }
    }
  }
  

  template < typename Mat, typename Vec, typename VecB, typename Precond >
    void idgmres(const Mat &A, Vec &x, const VecB &b,
		 const Precond &M, size_type m, iteration& outer) {
    typedef typename linalg_traits<Mat>::value_type T;
    modified_gram_schmidt<T> orth(m, vect_size(x));
    gmres(A, x, b, M, m, outer, orth); 
  }


  // Lock stage of an implicit restarted Arnoldi process.
  // 1- QR factorization of YB through Householder matrices
  //    Q(Rl) = YB
  //     (0 )
  // 2- Update of the Arnoldi factorization.
  //    H <- Q*HQ,  W <- WQ
  // 3- Restore the Hessemberg form of H.

  template <typename T, typename MATYB>
    void Lock(gmm::dense_matrix<T> &W, gmm::dense_matrix<T> &H,
	      const MATYB &YB, const sub_interval SUB,
	      bool restore, T &ns) {

    size_type n = mat_nrows(W), m = mat_ncols(W) - 1;
    size_type ncols = mat_ncols(YB), nrows = mat_nrows(YB);
    size_type begin = min(SUB); end = max(SUB) - 1;
    sub_interval SUBR(0, nrows), SUBC(0, ncols);
    T alpha(1);

    GMM_ASSERT2(((end-begin) == ncols) && (m == mat_nrows(H)) 
		&& (m+1 == mat_ncols(H)), "dimensions mismatch");
    
    // DEFLATION using the QR Factorization of YB
	  
    dense_matrix<T> QR(n_rows, n_rows);
    gmmm::copy(YB, sub_matrix(QR, SUBR, SUBC));
    gmm::clear(submatrix(QR, SUBR, sub_interval(ncols, nrows-ncols)));
    qr_factor(QR); 


    apply_house_left(QR, sub_matrix(H, SUB));
    apply_house_right(QR, sub_matrix(H, SUBR, SUB));
    apply_house_right(QR, sub_matrix(W, sub_interval(0, n), SUB));
    
    //       Restore to the initial block hessenberg form
    
    if (restore) {
      
      // verifier quand m = 0 ...
      gmm::dense_matrix tab_p(end - st.tb_deftot, end - st.tb_deftot);
      gmm::copy(identity_matrix(), tab_p);
      
      for (size_type j = end-1; j >= st.tb_deftot+2; --j) {
	
	size_type jm = j-1;
	std::vector<T> v(jm - st.tb_deftot);
	sub_interval SUBtot(st.tb_deftot, jm - st.tb_deftot);
	sub_interval SUBtot2(st.tb_deftot, end - st.tb_deftot);
	gmm::copy(sub_vector(mat_row(H, j), SUBtot), v);
	house_vector_last(v);
	w.resize(end);
	col_house_update(sub_matrix(H, SUBI, SUBtot), v, w);
	w.resize(end - st.tb_deftot);
	row_house_update(sub_matrix(H, SUBtot, SUBtot2), v, w);
	gmm::clear(sub_vector(mat_row(H, j),
			      sub_interval(st.tb_deftot, j-1-st.tb_deftot)));
	w.resize(end - st.tb_deftot);
	col_house_update(sub_matrix(tab_p, sub_interval(0, end-st.tb_deftot),
				    sub_interval(0, jm-st.tb_deftot)), v, w);
	w.resize(n);
	col_house_update(sub_matrix(W, sub_interval(0, n), SUBtot), v, w);
      }
      
      //       restore positive subdiagonal elements
      
      std::vector<T> d(fin-st.tb_deftot); d[0] = T(1);
      
      // We compute d[i+1] in order 
      // (d[i+1] * H(st.tb_deftot+i+1,st.tb_deftoti)) / d[i] 
      // be equal to |H(st.tb_deftot+i+1,st.tb_deftot+i))|.
      for (size_type j = 0; j+1 < end-st.tb_deftot; ++j) {
	T e = H(st.tb_deftot+j, st.tb_deftot+j-1);
	d[j+1] = (e == T(0)) ? T(1) :  d[j] * gmm::abs(e) / e;
	scale(sub_vector(mat_row(H, st.tb_deftot+j+1),
			 sub_interval(st.tb_deftot, m-st.tb_deftot)), d[j+1]);
	scale(mat_col(H, st.tb_deftot+j+1), T(1) / d[j+1]);
	scale(mat_col(W, st.tb_deftot+j+1), T(1) / d[j+1]);
      }

      alpha = tab_p(end-st.tb_deftot-1, end-st.tb_deftot-1) / d[end-st.tb_deftot-1];
      alpha /= gmm::abs(alpha);
      scale(mat_col(W, m), alpha);
	    
    }
	 
    return alpha;
  }








  // Apply p implicit shifts to the Arnoldi factorization
  // AV = VH+H(k+p+1,k+p) V(:,k+p+1) e_{k+p}*
  // and produces the following new Arnoldi factorization
  // A(VQ) = (VQ)(Q*HQ)+H(k+p+1,k+p) V(:,k+p+1) e_{k+p}* Q
  // where only the first k columns are relevant.
  //
  // Dan Sorensen and Richard J. Radke, 11/95
  template<typename T, typename C>
    apply_shift_to_Arnoldi_factorization(dense_matrix<T> V, dense_matrix<T> H,
					 std::vector<C> Lambda, size_type &k,
					 size_type p, bool true_shift = false) {


    size_type k1 = 0, num = 0, kend = k+p, kp1 = k + 1;
    bool mark = false;
    T c, s, x, y, z;

    dense_matrix<T> q(1, kend);
    gmm::clear(q); q(0,kend-1) = T(1);
    std::vector<T> hv(3), w(std::max(kend, mat_nrows(V)));

    for(size_type jj = 0; jj < p; ++jj) {
      //     compute and apply a bulge chase sweep initiated by the
      //     implicit shift held in w(jj)
   
      if (abs(Lambda[jj].real()) == 0.0) {
	//       apply a real shift using 2 by 2 Givens rotations

	for (size_type k1 = 0, k2 = 0; k2 != kend-1; k1 = k2+1) {
	  k2 = k1;
	  while (h(k2+1, k2) != T(0) && k2 < kend-1) ++k2;

	  Givens_rotation(H(k1, k1) - Lambda[jj], H(k1+1, k1), c, s);
	  
	  for (i = k1; i <= k2; ++i) {
            if (i > k1) Givens_rotation(H(i, i-1), H(i+1, i-1), c, s);
            
	    // Ne pas oublier de nettoyer H(i+1,i-1) (le mettre à zéro).
	    // Vérifier qu'au final H(i+1,i) est bien un réel positif.

            // apply rotation from left to rows of H
	    row_rot(sub_matrix(H, sub_interval(i,2), sub_interval(i, kend-i)),
		    c, s, 0, 0);
	    
	    // apply rotation from right to columns of H
            size_type ip2 = std::min(i+2, kend);
            col_rot(sub_matrix(H, sub_interval(0, ip2), sub_interval(i, 2))
		    c, s, 0, 0);
            
            // apply rotation from right to columns of V
	    col_rot(V, c, s, i, i+1);
            
            // accumulate e'  Q so residual can be updated k+p
	    Apply_Givens_rotation_left(q(0,i), q(0,i+1), c, s);
	    // peut être que nous utilisons G au lieu de G* et que
	    // nous allons trop loin en k2.
	  }
	}
	
	num = num + 1;
      }
      else {
      
	// Apply a double complex shift using 3 by 3 Householder 
	// transformations
      
	if (jj == p || mark)
	  mark = false;     // skip application of conjugate shift
	else {
	  num = num + 2;    // mark that a complex conjugate
	  mark = true;      // pair has been applied

	  // Indices de fin de boucle à surveiller... de près !
	  for (size_type k1 = 0, k3 = 0; k3 != kend-2; k1 = k3+1) {
	    k3 = k1;
	    while (h(k3+1, k3) != T(0) && k3 < kend-2) ++k3;
	    size_type k2 = k1+1;


            x = H(k1,k1) * H(k1,k1) + H(k1,k2) * H(k2,k1)
	      - 2.0*Lambda[jj].real() * H(k1,k1) + gmm::abs_sqr(Lambda[jj]);
	    y = H(k2,k1) * (H(k1,k1) + H(k2,k2) - 2.0*Lambda[jj].real());
	    z = H(k2+1,k2) * H(k2,k1);

	    for (size_type i = k1; i <= k3; ++i) {
	      if (i > k1) {
		x = H(i, i-1);
		y = H(i+1, i-1);
		z = H(i+2, i-1);
		// Ne pas oublier de nettoyer H(i+1,i-1) et H(i+2,i-1) 
		// (les mettre à zéro).
	      }

	      hv[0] = x; hv[1] = y; hv[2] = z;
	      house_vector(v);

	      // Vérifier qu'au final H(i+1,i) est bien un réel positif

	      // apply transformation from left to rows of H
	      w.resize(kend-i);
	      row_house_update(sub_matrix(H, sub_interval(i, 2),
					  sub_interval(i, kend-i)), v, w);
               
	      // apply transformation from right to columns of H
               
	      size_type ip3 = std::min(kend, i + 3);
	      w.resize(ip3);
              col_house_update(sub_matrix(H, sub_interval(0, ip3),
					  sub_interval(i, 2)), v, w);
               
	      // apply transformation from right to columns of V
	      
	      w.resize(mat_nrows(V));
	      col_house_update(sub_matrix(V, sub_interval(0, mat_nrows(V)),
					  sub_interval(i, 2)), v, w);
               
	      // accumulate e' Q so residual can be updated  k+p

	      w.resize(1);
	      col_house_update(sub_matrix(q, sub_interval(0,1),
					  sub_interval(i,2)), v, w);
               
	    }
	  }
         
	  //           clean up step with Givens rotation

	  i = kend-2;
	  c = x; s = y;
	  if (i > k1) Givens_rotation(H(i, i-1), H(i+1, i-1), c, s);
            
	  // Ne pas oublier de nettoyer H(i+1,i-1) (le mettre à zéro).
	  // Vérifier qu'au final H(i+1,i) est bien un réel positif.

	  // apply rotation from left to rows of H
	  row_rot(sub_matrix(H, sub_interval(i,2), sub_interval(i, kend-i)),
		    c, s, 0, 0);
	    
	  // apply rotation from right to columns of H
	  size_type ip2 = std::min(i+2, kend);
	  col_rot(sub_matrix(H, sub_interval(0, ip2), sub_interval(i, 2))
		  c, s, 0, 0);
            
	  // apply rotation from right to columns of V
	  col_rot(V, c, s, i, i+1);
            
	  // accumulate e'  Q so residual can be updated k+p
	  Apply_Givens_rotation_left(q(0,i), q(0,i+1), c, s);

	}
      }
    }

    //  update residual and store in the k+1 -st column of v

    k = kend - num;
    scale(mat_col(V, kend), q(0, k));
    
    if (k < mat_nrows(H)) {
      if (true_shift)
	gmm::copy(mat_col(V, kend), mat_col(V, k));
      else
	   //   v(:,k+1) = v(:,kend+1) + v(:,k+1)*h(k+1,k);
	   //   v(:,k+1) = v(:,kend+1) ;
	gmm::add(scaled(mat_col(V, kend), H(kend, kend-1)), 
		 scaled(mat_col(V, k), H(k, k-1)), mat_col(V, k));
    }

    H(k, k-1) = vect_norm2(mat_col(V, k));
    scale(mat_col(V, kend), T(1) / H(k, k-1));
  }



  template<typename MAT, typename EVAL, typename PURE>
  void select_eval(const MAT &Hobl, EVAL &eval, MAT &YB, PURE &pure,
		   idgmres_state &st) {

    typedef typename linalg_traits<MAT>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    size_type m = st.m;

    // Computation of the Ritz eigenpairs.
    
    col_matrix< std::vector<T> > evect(m-st.tb_def, m-st.tb_def);
    // std::vector<std::complex<R> > eval(m);
    std::vector<R> ritznew(m, T(-1));
	
    // dense_matrix<T> evect_lock(st.tb_def, st.tb_def);
    
    sub_interval SUB1(st.tb_def, m-st.tb_def);
    implicit_qr_algorithm(sub_matrix(Hobl, SUB1),
			  sub_vector(eval, SUB1), evect);
    sub_interval SUB2(0, st.tb_def);
    implicit_qr_algorithm(sub_matrix(Hobl, SUB2),
			  sub_vector(eval, SUB2), /* evect_lock */);
    
    for (size_type l = st.tb_def; l < m; ++l)
      ritznew[l] = gmm::abs(evect(m-st.tb_def-1, l-st.tb_def) * Hobl(m, m-1));
    
    std::vector< std::pair<T, size_type> > eval_sort(m);
    for (size_type l = 0; l < m; ++l)
      eval_sort[l] = std::pair<T, size_type>(eval[l], l);
    std::sort(eval_sort.begin(), eval_sort.end(), compare_vp());
    
    std::vector<size_type> index(m);
    for (size_type l = 0; l < m; ++l) index[l] = eval_sort[l].second;
    
    std::vector<bool> kept(m, false);
    std::fill(kept.begin(), kept.begin()+st.tb_def, true);

    apply_permutation(eval, index);
    apply_permutation(evect, index);
    apply_permutation(ritznew, index);
    apply_permutation(kept, index);

    //	Which are the eigenvalues that converged ?
    //
    //	nb_want is the number of eigenvalues of 
    //	Hess(tb_def+1:n,tb_def+1:n) that converged and are WANTED
    //
    //	nb_unwant is the number of eigenvalues of 
    //	Hess(tb_def+1:n,tb_def+1:n) that converged and are UNWANTED
    //
    //	nb_nolong is the number of eigenvalues of 
    //	Hess(1:tb_def,1:tb_def) that are NO LONGER WANTED. 
    //
    //	tb_deftot is the number of the deflated eigenvalues
    //	that is tb_def + nb_want + nb_unwant
    //
    //	tb_defwant is the number of the wanted deflated eigenvalues
    //	that is tb_def + nb_want - nb_nolong
    
    st.nb_want = 0, st.nb_unwant = 0, st.nb_nolong = 0;
    size_type j, ind;
    
    for (j = 0, ind = 0; j < m-p; ++j) {
      if (ritznew[j] == R(-1)) {
	if (std::imag(eval[j]) != R(0)) {
	  st.nb_nolong += 2; ++j; //  à adapter dans le cas complexe ...
	} 
	else st.nb_nolong++;
      }
      else {
	if (ritznew[j]
	    < tol_vp * gmm::abs(eval[j])) {
	  
	  for (size_type l = 0, l < m-st.tb_def; ++l)
	    YB(l, ind) = std::real(evect(l, j));
	  kept[j] = true;
	  ++j; ++st.nb_unwant; ind++;
	  
	  if (std::imag(eval[j]) != R(0)) {
	    for (size_type l = 0, l < m-st.tb_def; ++l)
	      YB(l, ind) = std::imag(evect(l, j));
	    pure[ind-1] = 1;
	    pure[ind] = 2;
	    
	    kept[j] = true;
	    
	    st.nb_unwant++;
	    ++ind;
	  }
	}
      }
    }
    
    
    for (; j < m; ++j) {
      if (ritznew[j] != R(-1)) {

	for (size_type l = 0, l < m-st.tb_def; ++l)
	  YB(l, ind) = std::real(evect(l, j));
	pure[ind] = 1;
	++ind;
	kept[j] = true;
	++st.nb_want;
	
	if (ritznew[j]
	    < tol_vp * gmm::abs(eval[j])) {
	  for (size_type l = 0, l < m-st.tb_def; ++l)
	    YB(l, ind) = std::imag(evect(l, j));
	  pure[ind] = 2;
	  
	  j++;
	  kept[j] = true;
	  
	  st.nb_want++;
	  ++ind;	      
	}
      }
    }
    
    std::vector<T> shift(m - st.tb_def - st.nb_want - st.nb_unwant);
    for (size_type j = 0, i = 0; j < m; ++j)
      if (!kept[j]) shift[i++] = eval[j];
    
    // st.conv (st.nb_want+st.nb_unwant) is the number of eigenpairs that
    //   have just converged.
    // st.tb_deftot is the total number of eigenpairs that have converged.
    
    size_type st.conv = ind;
    size_type st.tb_deftot = st.tb_def + st.conv;
    size_type st.tb_defwant = st.tb_def + st.nb_want - st.nb_nolong;
    
    sub_interval SUBYB(0, st.conv);
    
    if ( st.tb_defwant >= p ) { // An invariant subspace has been found.
      
      st.nb_unwant = 0;
      st.nb_want = p + st.nb_nolong - st.tb_def;
      st.tb_defwant = p;
      
      if ( pure[st.conv - st.nb_want + 1] == 2 ) {
	++st.nb_want; st.tb_defwant = ++p;// il faudrait que ce soit un p local
      }
      
      SUBYB = sub_interval(st.conv - st.nb_want, st.nb_want);
      // YB = YB(:, st.conv-st.nb_want+1 : st.conv); // On laisse en suspend ..
      // pure = pure(st.conv-st.nb_want+1 : st.conv,1); // On laisse suspend ..
      st.conv = st.nb_want;
      st.tb_deftot = st.tb_def + st.conv;
      st.ok = true;
    }
    
  }



  template<typename MAT, typename EVAL, typename PURE>
  void select_eval_for_purging(const MAT &Hobl, EVAL &eval, MAT &YB,
			       PURE &pure, idgmres_state &st) {

    typedef typename linalg_traits<MAT>::value_type T;
    typedef typename number_traits<T>::magnitude_type R;
    size_type m = st.m;

    // Computation of the Ritz eigenpairs.
    
    col_matrix< std::vector<T> > evect(st.tb_deftot, st.tb_deftot);
    
    sub_interval SUB1(0, st.tb_deftot);
    implicit_qr_algorithm(sub_matrix(Hobl, SUB1),
			  sub_vector(eval, SUB1), evect);
    std::fill(eval.begin() + st.tb_deftot, eval.end(), std::complex<R>(0));
    
    std::vector< std::pair<T, size_type> > eval_sort(m);
    for (size_type l = 0; l < m; ++l)
      eval_sort[l] = std::pair<T, size_type>(eval[l], l);
    std::sort(eval_sort.begin(), eval_sort.end(), compare_vp());

    std::vector<bool> sorted(m);
    std::fill(sorted.begin(), sorted.end(), false);
    
    std::vector<size_type> ind(m);
    for (size_type l = 0; l < m; ++l) ind[l] = eval_sort[l].second;
    
    std::vector<bool> kept(m, false);
    std::fill(kept.begin(), kept.begin()+st.tb_def, true);

    apply_permutation(eval, ind);
    apply_permutation(evect, ind);
    
    size_type j;
    for (j = 0; j < st.tb_deftot; ++j) {
	  
      for (size_type l = 0, l < st.tb_deftot; ++l)
	YB(l, j) = std::real(evect(l, j));
      
      if (std::imag(eval[j]) != R(0)) {
	for (size_type l = 0, l < m-st.tb_def; ++l)
	  YB(l, j+1) = std::imag(evect(l, j));
	pure[j] = 1;
	pure[j+1] = 2;
	
	j += 2;
      }
      else ++j;
    }
  }
  





}

#endif
