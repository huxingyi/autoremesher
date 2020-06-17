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
// RECTANGULAR_MATRIX_PARAM
// SQUARED_MATRIX_PARAM
// ENDPARAM;

#include "gmm/gmm_kernel.h"
#include "gmm/gmm_dense_lu.h"
#include "gmm/gmm_dense_qr.h"
#include "gmm/gmm_condition_number.h"

using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
using gmm::size_type;
bool print_debug = false;

// template <typename MAT, typename T> void print_for_matlab(const MAT &m, T) { 
//   cout.precision(16);
//   cout << "[ ";
//   for (size_type i = 0; i < gmm::mat_nrows(m); ++i) {
//     for (size_type j = 0; j < gmm::mat_ncols(m); ++j) cout << " " << m(i,j);
//     if (i != gmm::mat_nrows(m)-1) cout << " ; \n";
//   }
//   cout << " ]" << endl;
// }

// template <typename MAT, typename T> void print_for_matlab(const MAT &m,
// 						    std::complex<T>) { 
//   cout.precision(16);
//   cout << "[ ";
//   for (size_type i = 0; i < gmm::mat_nrows(m); ++i) {
//     for (size_type j = 0; j < gmm::mat_ncols(m); ++j)
//       cout << " (" << m(i,j).real() << "+" << m(i,j).imag() << "*i)" ;
//     if (i != gmm::mat_nrows(m)-1) cout << " ; \n";
//   }
//   cout << " ]" << endl;
// }

// template <typename MAT> inline void print_for_matlab(const MAT &m)
// { print_for_matlab(m, gmm::linalg_traits<MAT>::value_type()); }

template <typename T> inline T real_or_complex(double a, double, T)
{ return T(a); }

template <typename T> inline
std::complex<T> real_or_complex(double a, double b, std::complex<T>) {
  typedef typename gmm::number_traits<T>::magnitude_type R;
  return std::complex<T>(R(a), R(b)); 
}

template <typename T> struct cmp_eval {
  bool operator()(T a, T b) {
    typedef typename gmm::number_traits<T>::magnitude_type R;
    // R prec = gmm::default_tol(R());
    R dr = gmm::real(a) - gmm::real(b);
    R di = gmm::imag(a) - gmm::imag(b);
    if (gmm::abs(dr) > gmm::abs(di)) return (dr<R(0)); else return (di<R(0));
  }
};

template <typename T> void sort_eval(std::vector<T> &v) {
  std::sort(v.begin(), v.end(), cmp_eval<T>());
} 


template <typename MAT1, typename MAT2>
bool test_procedure(const MAT1 &m1_, const MAT2 &m2_) {
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  MAT2  &m2 = const_cast<MAT2  &>(m2_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());
  R error;
  static size_type nb_iter(0);
  ++nb_iter;

  // gmm::qr_factor(A, Q, R) is tested in test_gmm_mult.C

  //
  // test for gmm::qr_factor(A), apply_house_right and apply_house_left
  //
  size_type m = gmm::mat_nrows(m1), n = gmm::mat_ncols(m1);
  size_type k = size_type(rand() % 50);

  if (print_debug) {
    static int nexpe = 0;
    cout << "Begin experiment " << ++nexpe << "\n\nwith " << m1 << "\n\n"; 
    gmm::set_warning_level(3);
  }
    
  gmm::dense_matrix<T> dm1(m, n);
  gmm::copy(m1, dm1);
  if (m >= n) {
    gmm::dense_matrix<T> q(k,m), qaux(k,m), q2(m,k), dm1aux(k,n), m1aux(k,n);
    gmm::fill_random(q); gmm::copy(q, qaux);
    gmm::mult(q, m1, m1aux);

    gmm::qr_factor(dm1);
    gmm::copy(dm1, m1);
    
    gmm::apply_house_right(dm1, q);
    for (size_type j = 0; j < n; ++j)
      for (size_type i = j+1; i < m; ++i)
	dm1(i, j) = T(0);
    gmm::mult(q, dm1, dm1aux);
    gmm::add(gmm::scaled(m1aux, T(-1)), dm1aux);
    error = gmm::mat_euclidean_norm(dm1aux);
    if (!(error <= prec * R(10000))) 
      GMM_ASSERT1(false, "Error too large: " << error);

    gmm::copy(gmm::identity_matrix(), q);
    gmm::apply_house_right(m1, q);
    size_type min_km = std::min(k, m);
    gmm::dense_matrix<T> a(min_km, min_km), b(min_km, min_km);
    gmm::copy(gmm::identity_matrix(), b);
    if (k > m) gmm::mult(gmm::conjugated(q), q, a);
    else gmm::mult(q, gmm::conjugated(q), a);
    gmm::add(gmm::scaled(b, T(-1)), a);
    error = gmm::mat_euclidean_norm(a);
    if (!(error <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: " << error);
      
    gmm::copy(gmm::conjugated(qaux), q2);
    gmm::apply_house_left(m1, q2);
    gmm::mult(gmm::conjugated(q2), dm1, dm1aux);
    gmm::add(gmm::scaled(m1aux, T(-1)), dm1aux);
    error = gmm::mat_euclidean_norm(dm1aux);
    if (!(error <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: " << error);

  }
  else {
    gmm::dense_matrix<T> q(k,n), qaux(k,n), q2(n,k), dm1aux(k,m), m1aux(k,m);
    gmm::fill_random(q); gmm::copy(q, qaux);
    gmm::mult(q, gmm::transposed(m1), m1aux);

    gmm::qr_factor(gmm::transposed(dm1));
    gmm::copy(dm1, m1);
    
    gmm::apply_house_right(gmm::transposed(dm1), q);
    for (size_type i = 0; i < m; ++i)
      for (size_type j = i+1; j < n; ++j)
	dm1(i, j) = T(0);
    gmm::mult(q, gmm::transposed(dm1), dm1aux);
    gmm::add(gmm::scaled(m1aux, T(-1)), dm1aux);
    error = gmm::mat_euclidean_norm(dm1aux);
    if (!(error <= prec * R(10000))) 
      GMM_ASSERT1(false, "Error too large: " << error);

    gmm::copy(gmm::identity_matrix(), q);
    gmm::apply_house_right(gmm::transposed(m1), q);
    size_type min_km = std::min(k, n);
    gmm::dense_matrix<T> a(min_km, min_km), b(min_km, min_km);
    gmm::copy(gmm::identity_matrix(), b);
    if (k > n) gmm::mult(gmm::conjugated(q), q, a);
    else gmm::mult(q, gmm::conjugated(q), a);
    gmm::add(gmm::scaled(b, T(-1)), a);
    error = gmm::mat_euclidean_norm(a);
    if (!(error <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: " << error);
      
    gmm::copy(gmm::conjugated(qaux), q2);
    gmm::apply_house_left(gmm::transposed(m1), q2);
    gmm::mult(gmm::conjugated(q2), gmm::transposed(dm1), dm1aux);
    gmm::add(gmm::scaled(m1aux, T(-1)), dm1aux);
    error = gmm::mat_euclidean_norm(dm1aux);
    if (!(error <= prec * R(10000))) 
      GMM_ASSERT1(false, "Error too large: " << error);

  }
  
  //
  // Test for implicit_qr_algorithm
  //

  

  m = gmm::mat_nrows(m2);
  gmm::dense_matrix<T> cq(m, m), cr(m, m), ca(m, m);  
  std::vector<T> cv(m);
  std::vector<std::complex<R> > eigc(m), cvc(m);


  gmm::fill_random(ca);
  std::complex<R> det1(gmm::lu_det(ca)), det2(1);
  implicit_qr_algorithm(ca, eigc, cq);
  for (size_type i = 0; i < m; ++i) det2 *= eigc[i];
  if (gmm::abs(det1 - det2) > (gmm::abs(det1)+gmm::abs(det2))/R(100))
    GMM_ASSERT1(false, "Error in QR or det. det lu: " << det1
	      << " det qr: " << det2);
  if (print_debug)
    cout << "det lu = " << det1 << "  det qr = " << det2 << endl;

  if (m > 0) do {
    gmm::fill_random(cq);
  } while (gmm::abs(gmm::lu_det(cq)) < sqrt(prec)
	   || gmm::condition_number(cq) > R(1000));
  gmm::copy(cq, cr);
  gmm::lu_inverse(cr);

  

  gmm::fill_random(cv);
  if (m >  0) cv[ 0] = real_or_complex(     0.0,  0.0, cv[0]);
  if (m >  1) cv[ 1] = real_or_complex(     0.0,  0.0, cv[0]);
  if (m >  2) cv[ 2] = real_or_complex(     0.01,-0.1, cv[0]);
  if (m >  3) cv[ 3] = real_or_complex(     0.01, 0.1, cv[0]);
  if (m >  4) cv[ 4] = real_or_complex(    -2.0,  3.0, cv[0]);
  if (m >  5) cv[ 5] = real_or_complex(    -2.0,  3.0, cv[0]);
  if (m >  6) cv[ 6] = real_or_complex(   -50.0,  3.0, cv[0]);
  if (m >  7) cv[ 7] = real_or_complex(   100.0,  1.0, cv[0]);
  if (m >  8) cv[ 8] = real_or_complex(   300.0,  1.0, cv[0]);
  if (m >  9) cv[ 9] = real_or_complex(   500.0,  1.0, cv[0]);
  if (m > 10) cv[10] = real_or_complex(  1000.0,  1.0, cv[0]);
  if (m > 11) cv[11] = real_or_complex(  4000.0,  1.0, cv[0]);
  if (m > 12) cv[12] = real_or_complex(  5000.0,  1.0, cv[0]);
  if (m > 13) cv[13] = real_or_complex( 10000.0,  1.0, cv[0]);
  if (m > 14) cv[14] = real_or_complex( 80000.0,  1.0, cv[0]);
  if (m > 15) cv[15] = real_or_complex(100000.0,  1.0, cv[0]);
  gmm::clear(m2);
  for (size_type l = 0; l < m; ++l) m2(l, l) = cv[l];
  gmm::mult(cq, m2, ca); 
  gmm::mult(ca, cr, ca);
  
  implicit_qr_algorithm(ca, eigc, cq);
  gmm::copy(cv, cvc);
  
  sort_eval(cvc);
  sort_eval(eigc);
  error = gmm::vect_dist2(cvc, eigc);
  if (!(error <= sqrt(prec) * gmm::vect_norm2(cv) * R(10)))
    GMM_ASSERT1(false, "Error in QR algorithm, error = " << error);

  gmm::dense_matrix<T> aa(m, m), bb(m, m);
  gmm::mult(gmm::conjugated(cq), ca, aa);
  gmm::mult(aa, cq, bb);
  for (size_type i = 0; i < m; ++i)
    for (size_type j = (i == 0) ? 0 : i-1; j < m; ++j)
      bb(i, j) = T(0);
  error = gmm::mat_maxnorm(bb);
  if (!(error <= sqrt(prec) * gmm::vect_norm2(cv) * R(10)))
    GMM_ASSERT1(false, "Error in Schur vectors, error = "<< error); 

  //
  // Test for symmetric_qr_algorithm
  //

  m = gmm::mat_nrows(m2);
  std::vector<R> cvr(m), eigcr(m);
  if (m > 0) do {
    gmm::fill_random(cr);
  } while (gmm::abs(gmm::lu_det(cr)) < sqrt(prec)
	   || gmm::condition_number(cr) > R(1000));
  
  gmm::qr_factor(cr, cq, ca);
  gmm::fill_random(cvr);

  gmm::copy(gmm::identity_matrix(), m2);

  if (m >  0) cvr[ 0] = R(     0.0 );
  if (m >  1) cvr[ 1] = R(     0.0 );
  if (m >  2) cvr[ 2] = R(     0.01);
  if (m >  3) cvr[ 3] = R(     0.01);
  if (m >  4) cvr[ 4] = R(    -2.0 );
  if (m >  5) cvr[ 5] = R(    -2.0 );
  if (m >  6) cvr[ 6] = R(   -50.0 );
  if (m >  7) cvr[ 7] = R(   100.0 );
  if (m >  8) cvr[ 8] = R(   300.0 );
  if (m >  9) cvr[ 9] = R(   500.0 );
  if (m > 10) cvr[10] = R(  1000.0 );
  if (m > 11) cvr[11] = R(  4000.0 );
  if (m > 12) cvr[12] = R(  5000.0 );
  if (m > 13) cvr[13] = R( 10000.0 );
  if (m > 14) cvr[14] = R( 80000.0 );
  if (m > 15) cvr[15] = R(100000.0 );
  gmm::clear(m2);
  for (size_type l = 0; l < m; ++l) m2(l, l) = cvr[l];

  gmm::mult(gmm::conjugated(cq), m2, ca); 
  gmm::mult(ca, cq, ca);
  
  symmetric_qr_algorithm(ca, eigcr, cq);

  for (size_type l = 0; l < m; ++l) {
     std::vector<T> vy(m);
     gmm::mult(ca, gmm::mat_col(cq, l),
	       gmm::scaled(gmm::mat_col(cq, l), -eigcr[l]), vy);
     error = gmm::vect_norm2(vy);
     if (!(error <= sqrt(prec) * gmm::vect_norm2(cvr) * R(10))) 
       GMM_ASSERT1(false, "Error too large: " << error);

  }

  sort_eval(cvr);
  sort_eval(eigcr);
  error = gmm::vect_dist2(cvr, eigcr);
  if (!(error <= sqrt(prec) * gmm::vect_norm2(cvr) * R(10)))
    GMM_ASSERT1(false, "Error in QR algorithm.");

  if (nb_iter == 100) return true;
  return false;

}
