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
// SQUARED_MATRIX_PARAM;
// DENSE_VECTOR_PARAM;
// VECTOR_PARAM;
// ENDPARAM;

#include "gmm/gmm_kernel.h"
#include "gmm/gmm_dense_lu.h"
#include "gmm/gmm_condition_number.h"

using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
using gmm::size_type;
bool print_debug = false;

template <typename MAT1, typename VECT1, typename VECT2>
bool test_procedure(const MAT1 &m1_, const VECT1 &v1_, const VECT2 &v2_) {
  VECT1 &v1 = const_cast<VECT1 &>(v1_);
  VECT2 &v2 = const_cast<VECT2 &>(v2_);
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());
  static size_type nb_iter = 0;

  size_type m = gmm::mat_nrows(m1);
  std::vector<T> v3(m);

  R det = gmm::abs(gmm::lu_det(m1)), error;
  R cond = gmm::condition_number(m1);

  if (print_debug) cout << "cond = " << cond << " det = " << det << endl;
  GMM_ASSERT1(det != R(0) || cond >= R(0.01) / prec || cond == R(0),
	      "Inconsistent condition number: " << cond);

  if (prec * cond < R(1)/R(10000) && det != R(0)) {
    ++nb_iter;

    gmm::lu_solve(m1, v1, v2);
    gmm::mult(m1, v1, gmm::scaled(v2, T(-1)), v3);

    error = gmm::vect_norm2(v3);
    GMM_ASSERT1(error <= prec * cond * R(20000), "Error too large: " << error);

    gmm::lu_inverse(m1);
    gmm::mult(m1, v2, v1);
    gmm::lu_inverse(m1);
    gmm::mult(m1, v1, gmm::scaled(v2, T(-1)), v3);
    
    error = gmm::vect_norm2(v3);
    GMM_ASSERT1(error <= prec * cond * R(20000), "Error too large: " << error);

    if (nb_iter == 100) return true;
  }
  return false;
}
