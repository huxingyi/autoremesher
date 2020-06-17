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
// RECTANGULAR_MATRIX_PARAM;
// RECTANGULAR_MATRIX_PARAM;
// ENDPARAM;

#include "gmm/gmm_kernel.h"

using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
using gmm::size_type;

template <typename MAT1, typename MAT2, typename MAT3>
bool test_procedure(const MAT1 &m1_, const MAT2 &m2_, const MAT3 &m3_) {
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  MAT2  &m2 = const_cast<MAT2  &>(m2_);
  MAT3  &m3 = const_cast<MAT3  &>(m3_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());
  static size_type nb_iter(0);
  ++nb_iter;

  size_type k = gmm::mat_nrows(m1);
  size_type l = std::max(gmm::mat_ncols(m1), gmm::mat_nrows(m2));
  size_type m = std::max(gmm::mat_ncols(m2), gmm::mat_nrows(m3));
  size_type n = gmm::mat_ncols(m3);

  gmm::dense_matrix<T> m4(k, m);
  gmm::mult(m1, m2, m4);

  R error = mat_euclidean_norm(m4)
    - mat_euclidean_norm(m1) * mat_euclidean_norm(m2);
  if (error > prec * R(100))
    GMM_ASSERT1(false, "Inconsistence of fröbenius norm" << error);

  error = mat_norm1(m4) - mat_norm1(m1) * mat_norm1(m2);
  if (error > prec * R(100))
    GMM_ASSERT1(false, "Inconsistence of norm1 for matrices"
	      << error);
  error = mat_norminf(m4) - mat_norminf(m1) * mat_norminf(m2);
  if (error > prec * R(100))
    GMM_ASSERT1(false, "Inconsistence of norminf for matrices"
		<< error);

  size_type mm = std::min(m, k);
  size_type nn = std::min(n, m);

  gmm::dense_matrix<T> m1bis(mm, l), m2bis(l, nn), m3bis(mm, nn);
  gmm::copy(gmm::sub_matrix(m1, gmm::sub_interval(0,mm),
			    gmm::sub_interval(0,l)), m1bis);
  gmm::copy(gmm::sub_matrix(m2, gmm::sub_interval(0,l),
			    gmm::sub_interval(0,nn)), m2bis);
  gmm::mult(m1bis, m2bis, m3bis);
  gmm::mult(gmm::sub_matrix(m1, gmm::sub_interval(0,mm),
			    gmm::sub_interval(0,l)),
	    gmm::sub_matrix(m2, gmm::sub_interval(0,l),
			    gmm::sub_interval(0,nn)),
	    gmm::sub_matrix(m3, gmm::sub_interval(0,mm),
			    gmm::sub_interval(0,nn)));
  gmm::add(gmm::scaled(m3bis, T(-1)),
	   gmm::sub_matrix(m3, gmm::sub_interval(0,mm),
			   gmm::sub_interval(0,nn)));
  
  error = gmm::mat_euclidean_norm(gmm::sub_matrix(m3, gmm::sub_interval(0,mm),
					   gmm::sub_interval(0,nn)));

  if (!(error <= prec * R(10000)))
    GMM_ASSERT1(false, "Error too large: " << error);

  if (nn <= gmm::mat_nrows(m3) && mm <= gmm::mat_ncols(m3)) {
    
    gmm::scale(m1, T(2));
    gmm::mult(gmm::scaled(gmm::sub_matrix(m1, gmm::sub_interval(0,mm),
					  gmm::sub_interval(0,l)), T(-1)),
	      gmm::sub_matrix(m2, gmm::sub_interval(0,l),
			      gmm::sub_interval(0,nn)),
	      gmm::sub_matrix(gmm::transposed(m3), gmm::sub_interval(0,mm),
			      gmm::sub_interval(0,nn)));
    gmm::add(gmm::scaled(m3bis, T(2)),
	     gmm::transposed(gmm::sub_matrix(m3, gmm::sub_interval(0,nn),
					     gmm::sub_interval(0,mm))));
    
    error = gmm::mat_euclidean_norm(gmm::sub_matrix(m3, gmm::sub_interval(0,nn),
					   gmm::sub_interval(0,mm)));
    
    if (!(error <= prec * R(10000)))
      GMM_ASSERT1(false, "Error too large: " << error);
  }
  if (nb_iter == 100) return true;
  return false;
  
}
