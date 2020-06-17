/*===========================================================================
 
 Copyright (C) 2007-2012 Yves Renard, Julien Pommier.
 
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
 
===========================================================================*/
// SQUARED_MATRIX_PARAM
// VECTOR_PARAM;
// VECTOR_PARAM;
// RECTANGULAR_MATRIX_PARAM;
// VECTOR_PARAM;
// VECTOR_PARAM;
// ENDPARAM;

#include "gmm/gmm_kernel.h"
#include "gmm/gmm_dense_lu.h"
#include "gmm/gmm_dense_qr.h"
#include "gmm/gmm_condition_number.h"

using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
using gmm::size_type;

template <typename MAT1 , typename MAT2, typename VECT1, typename VECT2,
	  typename VECT3, typename VECT4>
void test_procedure2(const MAT1 &m1_, const VECT1 &v1_, const VECT2 &v2_, 
		    const MAT2 &m2_, const VECT3 &v3_, const VECT4 &v4_) {
  VECT1 &v1 = const_cast<VECT1 &>(v1_);
  VECT2 &v2 = const_cast<VECT2 &>(v2_);
  VECT3 &v3 = const_cast<VECT3 &>(v3_);
  VECT4 &v4 = const_cast<VECT4 &>(v4_);
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  MAT2  &m2 = const_cast<MAT2  &>(m2_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());

  size_type m = gmm::vect_size(v1), n = gmm::vect_size(v3);
  size_type nn = std::min(m,n), mm = std::max(m, n);
  std::vector<T> v6(m);

  R det = gmm::abs(gmm::lu_det(m1)), error;
  R cond = gmm::condition_number(m1);

  if (prec * cond < R(1)/R(10000) && det != R(0)) {

    gmm::lu_solve(m1, v6, v2);
    gmm::mult(m1, v6, v1);
    gmm::add(gmm::scaled(v1, T(-1)), v2, v6);
    if (!((error = gmm::vect_norm2(v6)) <= prec * cond * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);
    
    gmm::lu_solve(gmm::transposed(m1), v6, v2);
    gmm::mult(gmm::transposed(m1), v6, v1);
    gmm::add(gmm::scaled(v1, T(-1)), v2, v6);
    if (!((error = gmm::vect_norm2(v6)) <= prec * cond * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);
    
    gmm::lu_solve(gmm::conjugated(m1), v6, v2);
    gmm::mult(gmm::conjugated(m1), v6, v1);
    gmm::add(gmm::scaled(v1, T(-1)), v2, v6);
    if (!((error = gmm::vect_norm2(v6)) <= prec * cond * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);
    
    gmm::lu_solve(gmm::transposed(gmm::conjugated(m1)), v6, v2);
    gmm::mult(gmm::transposed(gmm::conjugated(m1)), v6, v1);
    gmm::add(gmm::scaled(v1, T(-1)), v2, v6);
    if (!((error = gmm::vect_norm2(v6)) <= prec * cond * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);
    
    gmm::lu_solve(gmm::transposed(gmm::scaled(m1, T(-6))), v6, v2);
    gmm::mult(gmm::transposed(gmm::scaled(m1, T(-6))), v6, v1);
    gmm::add(gmm::scaled(v1, T(-1)), v2, v6);
    if (!((error = gmm::vect_norm2(v6)) <= prec * cond * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);

  }

  gmm::dense_matrix<T> q(mm, nn), r(nn, nn);
  if (m >= n) {
    std::vector<T> v5(m);
    gmm::mult(m2, v3, v2);
    gmm::qr_factor(m2, q, r);
    gmm::mult(r, v3, v4);
    gmm::mult(q, v4, gmm::scaled(v2, T(-1)), v5);
    if (!((error = gmm::vect_norm2(v5)) <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);

  }
  else {
    std::vector<T> v5(n);
    gmm::mult(gmm::conjugated(m2), v2, v3);
    gmm::qr_factor(gmm::conjugated(m2), q, r);
    gmm::mult(r, v2, v1);
    gmm::mult(q, v1, gmm::scaled(v3, T(-1)), v5);
    if (!((error = gmm::vect_norm2(v5)) <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: "<< error);

  }
  
}

template<typename MAT> void test_mat_swap(MAT &, gmm::linalg_modifiable) {}
template<typename MAT> void test_mat_swap(MAT &, gmm::linalg_const) {}
template<typename MAT> void test_mat_swap(MAT &M, gmm::linalg_false) {
  typedef typename gmm::linalg_traits<MAT>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  size_type m = gmm::mat_nrows(M), n = gmm::mat_ncols(M);
  MAT M2(m, n);
  gmm::dense_matrix<T> M3(m, n);
  gmm::copy(M, M3);
  std::swap(M, M2);
  gmm::add(gmm::scaled(M2, T(-1)), M3);
  if (gmm::mat_euclidean_norm(M3) > R(0) || gmm::mat_euclidean_norm(M) > R(0))
    GMM_ASSERT1(false, "Error in swap");
}

template<typename VECT> void test_vect_swap(VECT &, gmm::linalg_modifiable) {}
template<typename VECT> void test_vect_swap(VECT &, gmm::linalg_const) {}
template<typename VECT> void test_vect_swap(VECT &V, gmm::linalg_false) {
  typedef typename gmm::linalg_traits<VECT>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  size_type n = gmm::vect_size(V);
  VECT V2(n);
  std::vector<T> V3(n);
  gmm::copy(V, V3);
  std::swap(V, V2);
  gmm::add(gmm::scaled(V2, T(-1)), V3);
  if (gmm::vect_norm2(V3) > R(0) || gmm::vect_norm2(V) > R(0))
    GMM_ASSERT1(false, "Error in swap");
}


template <typename MAT1 , typename MAT2, typename VECT1, typename VECT2,
	  typename VECT3, typename VECT4>
bool test_procedure(const MAT1 &m1_, const VECT1 &v1_, const VECT2 &v2_, 
		    const MAT2 &m2_, const VECT3 &v3_, const VECT4 &v4_) {
  VECT1 &v1 = const_cast<VECT1 &>(v1_);
  VECT2 &v2 = const_cast<VECT2 &>(v2_);
  VECT3 &v3 = const_cast<VECT3 &>(v3_);
  VECT4 &v4 = const_cast<VECT4 &>(v4_);
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  MAT2  &m2 = const_cast<MAT2  &>(m2_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());
  static size_type nb_iter(0);
  ++nb_iter;

  test_procedure2(m1, v1, v2, m2, v3, v4);

  size_type m = gmm::vect_size(v1), n = gmm::vect_size(v3);
  gmm::csr_matrix<T> mm1(m, m);
  gmm::copy(m1, mm1);
  gmm::csc_matrix<T> mm2(m, n);
  gmm::copy(m2, mm2);
  test_procedure2(mm1, v1, v2, mm2, v3, v4);

  size_type mm = m / 2, nn = n / 2;
  gmm::sub_interval SUBI(0, mm), SUBJ(0, nn); 
  test_procedure2(gmm::sub_matrix(mm1, SUBI),
		  gmm::sub_vector(v1, SUBI),
		  gmm::sub_vector(v2, SUBI),
		  gmm::sub_matrix(mm2, SUBI, SUBJ),
		  gmm::sub_vector(v3, SUBJ),
		  gmm::sub_vector(v4, SUBJ));

  gmm::add(gmm::scaled(mm1, T(-1)), m1);
  gmm::add(gmm::scaled(mm2, T(-1)), m2);
  
  R error = gmm::mat_euclidean_norm(m1) + gmm::mat_euclidean_norm(m2);
  if (!(error <= prec * R(10000)))
    GMM_ASSERT1(false, "Error too large: "<< error);

  // test for row_vector and col_vector
  std::vector<T> v5(gmm::vect_size(v2));
  gmm::mult(m2, v3, v2);
  gmm::copy(v2, v5);
  gmm::mult(m2, gmm::col_vector(v3), gmm::col_vector(v2));
  gmm::add(gmm::scaled(v5, T(-1)), v2);
  error = gmm::vect_norm2(v2);
  if (!(error <= prec))
    GMM_ASSERT1(false, "Error too large: " << error);
  gmm::mult(gmm::row_vector(gmm::conjugated(v3)), gmm::conjugated(m2),
	    gmm::row_vector(v2));
  gmm::add(gmm::conjugated(gmm::scaled(v5, T(-1))), v2);
  error = gmm::vect_norm2(v2);
  if (!(error <= prec))
    GMM_ASSERT1(false, "Error too large: " << error);

  if (gmm::is_original_linalg(m1)) {
    size_type a = gmm::mat_nrows(m1), b = gmm::mat_ncols(m1);
    size_type a2 = gmm::irandom(size_type(a));
    size_type b2 = gmm::irandom(size_type(b));
    gmm::dense_matrix<T> m3(a, b);
    gmm::copy(m1, m3);
    gmm::resize(m1, a+a2, b+b2);
    for (size_type i = 0; i < a+a2; ++i)
      for (size_type j = 0; j < b+b2; ++j) {
	if (i < a && j < b) {
	  if (m3(i, j) != m1(i, j))
	    GMM_ASSERT1(false, "Error in resize");
	}
	else
	  if (m1(i, j) != T(0))
	    GMM_ASSERT1(false, "Error in resize");
      }
    gmm::resize(m1, a2, b2);
    for (size_type i = 0; i < a2; ++i)
      for (size_type j = 0; j < b2; ++j)
	if (m3(i, j) != m1(i, j))
	    GMM_ASSERT1(false, "Error in resize");
  }

  if (gmm::is_original_linalg(v1)) {
    size_type a = gmm::vect_size(v1);
    size_type a2 = gmm::irandom(size_type(a));
    std::vector<T> v6(a);
    gmm::copy(v1, v6);
    gmm::resize(v1, a+a2);
    for (size_type i = 0; i < a+a2; ++i) {
      if (i < a) {
	if (v1[i] != v6[i])
	    GMM_ASSERT1(false, "Error in resize");
	}
	else
	  if (v1[i] != T(0))
	    GMM_ASSERT1(false, "Error in resize");
    }
    gmm::resize(v1, a2);
    for (size_type i = 0; i < a2; ++i)
      if (v1[i] != v6[i])
	GMM_ASSERT1(false, "Error in resize");
  }

  test_mat_swap(m1, typename gmm::linalg_traits<MAT1>::is_reference());
  test_vect_swap(v1, typename gmm::linalg_traits<VECT1>::is_reference());
  
  if (nb_iter == 100) return true;
  return false;
}


  
