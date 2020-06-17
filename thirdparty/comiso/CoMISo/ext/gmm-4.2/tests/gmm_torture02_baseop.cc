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
// VECTOR_PARAM;
// ENDPARAM;

#include "gmm/gmm_kernel.h"

using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
using gmm::size_type;
bool print_debug = false;

template <typename MAT1, typename VECT1>
bool test_procedure(const MAT1 &m1_, const VECT1 &v1_) {
  VECT1 &v1 = const_cast<VECT1 &>(v1_);
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());

  size_type m = gmm::mat_nrows(m1);

  R norm = gmm::vect_norm2_sqr(v1);

  R normtest(0);

  for (size_type i = 0; i < m; ++i) {
    T x(1), y = v1[i];;
    x *= v1[i];
    x += v1[i];
    x += v1[i];
    x -= v1[i];
    x -= y;
    x *= v1[i];
    x /= v1[i];
    GMM_ASSERT1(y == v1[i], "Error in basic operations");
    normtest += gmm::abs_sqr(x);
  }
  
  GMM_ASSERT1(gmm::abs(norm - normtest) <= prec * R(100),
	      "Error in basic operations");
  
  return true;
}
