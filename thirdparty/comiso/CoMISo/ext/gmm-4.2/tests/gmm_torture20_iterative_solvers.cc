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

#include "gmm/gmm.h"
using namespace std; // in order to test a using namespace std;

using gmm::size_type;

int print_debug = 0;
size_type nb_fault_allowed = 200;
size_type nb_fault = 0;
double ratio_max = 0.0;

struct la_stat {
  size_type nb_iter, nb_fault, nb_expe;
  la_stat(void) : nb_iter(0), nb_fault(0), nb_expe(0) {};
};

template <typename PS> 
  la_stat ps_stat(const PS &, size_type nb_iter, bool fault) {
  static la_stat la;
  if (nb_iter != size_type(-1)) {
    la.nb_expe++;
    la.nb_iter += nb_iter;
    la.nb_fault += (fault) ? 1 : 0;
  }
  return la;
}

template <typename PS>
void print_stat(const PS &ps, const char *name) {
  la_stat s = ps_stat(ps, size_type(-1), false);
  cout << name << "\t: " << s.nb_expe << " exp. with ";
  if (s.nb_fault > 1)  cout << s.nb_fault << " faults";
  else if (s.nb_fault == 1) cout << "1  fault";
  else cout << "no fault";
       
  if (s.nb_expe != 0) {
    double ratio = double(s.nb_fault) / double(s.nb_expe);
    ratio_max = std::max(ratio, ratio_max);
    cout << ", ratio = " << ratio;
    cout << ", average nb iter = " << double(s.nb_iter) / double(s.nb_expe);
  } 
  cout << endl;
}

struct LEAST_SQUARE_CG {
  template <typename MAT, typename VECT1, typename VECT2, typename PRECOND>
  void operator()(const MAT &m, VECT1 &v1, const VECT2 &v2, const PRECOND &,
		  gmm::iteration &iter) const
  { gmm::least_squares_cg(m, v1, v2, iter); }
};

struct BICGSTAB {
  template <typename MAT, typename VECT1, typename VECT2, typename PRECOND>
  void operator()(const MAT &m, VECT1 &v1, const VECT2 &v2, const PRECOND &P,
		  gmm::iteration &iter) const
  { gmm::bicgstab(m, v1, v2, P, iter); }
};

struct GMRES {
  template <typename MAT, typename VECT1, typename VECT2, typename PRECOND>
  void operator()(const MAT &m, VECT1 &v1, const VECT2 &v2, const PRECOND &P,
		  gmm::iteration &iter) const
  { gmm::gmres(m, v1, v2, P, 50, iter); }
};

struct QMR {
  template <typename MAT, typename VECT1, typename VECT2, typename PRECOND>
  void operator()(const MAT &m, VECT1 &v1, const VECT2 &v2, const PRECOND &P,
		  gmm::iteration &iter) const
  { gmm::qmr(m, v1, v2, P, iter); }
};

struct CG {
  template <typename MAT, typename VECT1, typename VECT2, typename PRECOND>
  void operator()(const MAT &m, VECT1 &v1, const VECT2 &v2, const PRECOND &P,
		  gmm::iteration &iter) const
  { gmm::cg(m, v1, v2, P, iter); }
};

template <typename SOLVER, typename PRECOND, typename MAT, typename VECT1,
	  typename VECT2, typename Rcond>
void do_test(const SOLVER &solver, const MAT &m1, VECT1 &v1,
	    VECT2 &v2, const PRECOND &P, Rcond cond) {

  typedef typename gmm::linalg_traits<MAT>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R()), error(0);
  size_type m = gmm::mat_nrows(m1);
  std::vector<T> v3(m);

  gmm::iteration iter((double(prec*cond))*100.0, (print_debug > 2) ? 1:0,
		      50*m);

  for (int i = 0; i < 30; ++i) {
    iter.init();
    gmm::fill_random(v1);
    if (i >= 4) gmm::fill_random(v2);
    if (i == 29) gmm::clear(v1);
    solver(m1, v1, v2, P, iter);
    gmm::mult(m1, v1, gmm::scaled(v2, T(-1)), v3);
    error = gmm::vect_norm2(v3) / gmm::vect_norm2(v1);
    // if (error * R(0) != R(0))
    //   GMM_ASSERT1(false, "Inconsistent error: " << error);
    if (error <= prec * cond * R(20000)) {
      ps_stat(solver, iter.get_iteration(), false);
      ps_stat(P, iter.get_iteration(), false);
      return;
    }
  }
  ps_stat(solver, iter.get_iteration(), true);
  ps_stat(P, iter.get_iteration(), true);
  ++nb_fault;
  if (nb_fault > nb_fault_allowed)
      GMM_ASSERT1(false, "Error too large: " << error);

}

template <typename MAT1, typename VECT1, typename VECT2>
bool test_procedure(const MAT1 &m1_, const VECT1 &v1_, const VECT2 &v2_) {
  VECT1 &v1 = const_cast<VECT1 &>(v1_);
  VECT2 &v2 = const_cast<VECT2 &>(v2_);
  MAT1  &m1 = const_cast<MAT1  &>(m1_);
  typedef typename gmm::linalg_traits<MAT1>::value_type T;
  typedef typename gmm::number_traits<T>::magnitude_type R;
  R prec = gmm::default_tol(R());
  size_type m = gmm::mat_nrows(m1);
  if (m == 0) return true;
  static int nexpe = 0, effexpe = 0;
  ++nexpe;
  gmm::set_warning_level(0);

  gmm::clean(v1, 0.01);
  for (size_type i = 0; i < gmm::vect_size(v1); ++i)
    if (v1[i] != T(0) && gmm::abs(v1[i]) < R(1) / R(101))
      GMM_ASSERT1(false, "Error in clean");

  if (print_debug) {
    cout << "Begin experiment " << nexpe << "\n\nwith " << m1 << "\n\n"; 
    gmm::set_warning_level(3);
  }
  
  R det = gmm::abs(gmm::lu_det(m1)), cond = gmm::condest(m1);
  
  if (print_debug)
    cout << "condition number = " << cond << " det = " << det << endl;
  if (det == R(0) && cond < R(1) / prec && cond != R(0))
    GMM_ASSERT1(false, "Inconsistent condition number: " << cond);

  if (sqrt(prec) * cond >= R(1)/R(100) || det < sqrt(prec)*R(10)) return false;
    
  ++effexpe; cout << "."; cout.flush();
  
  gmm::identity_matrix P1;
  gmm::diagonal_precond<MAT1> P2(m1);
  gmm::mr_approx_inverse_precond<MAT1> P3(m1, 10, prec);
  gmm::ilu_precond<MAT1> P4(m1);
  gmm::ilut_precond<MAT1> P5(m1, 20, prec);
  gmm::ilutp_precond<MAT1> P5b(m1, 20, prec);
  
  R detmr = gmm::abs(gmm::lu_det(P3.approx_inverse()));

  if (sizeof(R) > 4 || m < 15) {
    
    if (print_debug) cout << "\nLeast square CG with no preconditionner\n";
    do_test(LEAST_SQUARE_CG(), m1, v1, v2, P1, cond);

    if (print_debug) cout << "\nBicgstab with no preconditionner\n";
    do_test(BICGSTAB(), m1, v1, v2, P1, cond);
    
    if (print_debug) cout << "\nBicgstab with diagonal preconditionner\n";
    do_test(BICGSTAB(), m1, v1, v2, P2, cond);
    
    if (detmr > prec * R(100)) {
      if (print_debug) cout << "\nBicgstab with mr preconditionner\n";
      do_test(BICGSTAB(), m1, v1, v2, P3, cond);
    }
    
    if (print_debug) cout << "\nBicgstab with ilu preconditionner\n";
    do_test(BICGSTAB(), m1, v1, v2, P4, cond);
    
    if (print_debug) cout << "\nBicgstab with ilut preconditionner\n";
    do_test(BICGSTAB(), m1, v1, v2, P5, cond); 

    if (print_debug) cout << "\nBicgstab with ilutp preconditionner\n";
    do_test(BICGSTAB(), m1, v1, v2, P5b, cond); 
  }

  if (print_debug) cout << "\nGmres with no preconditionner\n";
  do_test(GMRES(), m1, v1, v2, P1, cond);
  
  if (print_debug) cout << "\nGmres with diagonal preconditionner\n";
  do_test(GMRES(), m1, v1, v2, P2, cond);
  
  if (detmr > prec * R(100)) {
    if (print_debug) cout << "\nGmres with mr preconditionner\n";
    do_test(GMRES(), m1, v1, v2, P3, cond);
  }
  
  if (print_debug) cout << "\nGmres with ilu preconditionner\n";
  do_test(GMRES(), m1, v1, v2, P4, cond);
  
  if (print_debug) cout << "\nGmres with ilut preconditionner\n";
  do_test(GMRES(), m1, v1, v2, P5, cond);
  
  if (print_debug) cout << "\nGmres with ilutp preconditionner\n";
  do_test(GMRES(), m1, v1, v2, P5b, cond);
  
  if (sizeof(R) > 4 || m < 20) {

    if (print_debug) cout << "\nQmr with no preconditionner\n";
    do_test(QMR(), m1, v1, v2, P1, cond);
    
    if (print_debug) cout << "\nQmr with diagonal preconditionner\n";
    do_test(QMR(), m1, v1, v2, P2, cond);
    
    if (print_debug) cout << "\nQmr with ilu preconditionner\n";
    do_test(QMR(), m1, v1, v2, P4, cond);
    
    if (print_debug) cout << "\nQmr with ilut preconditionner\n";
    do_test(QMR(), m1, v1, v2, P5, cond);  

    if (print_debug) cout << "\nQmr with ilutp preconditionner\n";
    do_test(QMR(), m1, v1, v2, P5b, cond);  
  }

  gmm::dense_matrix<T> m2(m, m), m3(m, m);
  gmm::mult(gmm::conjugated(m1), m1, m2);
  gmm::copy(m1, m3);
  gmm::add(gmm::conjugated(m1), m3);
  gmm::copy(m2, m1);
  gmm::cholesky_precond<MAT1> P6(m1);
  gmm::choleskyt_precond<MAT1> P7(m1, 10, prec);
  
  if (!is_hermitian(m1, prec*R(100)))
    GMM_ASSERT1(false, "The matrix is not hermitian");
  
  if (print_debug) cout << "\nCG with no preconditionner\n";
  do_test(CG(), m1, v1, v2, P1, cond*cond);
  
  if (print_debug) cout << "\nCG with diagonal preconditionner\n";
  do_test(CG(), m1, v1, v2, P2, cond*cond);
  
  if (print_debug) cout << "\nCG with ildlt preconditionner\n";
  do_test(CG(), m1, v1, v2, P6, cond*cond);
  
  if (print_debug) cout << "\nCG with ildltt preconditionner\n";
  do_test(CG(), m1, v1, v2, P7, cond*cond);

  if (effexpe == 50) {
    cout << "\n\n" << effexpe << " effective experiments with ";
    if (nb_fault > 1)  cout << nb_fault << " faults";
    else if (nb_fault == 1) cout << "1  fault";
    else cout << "no fault";
    cout << ", size = " << m << " base type : " << typeid(T).name() << endl;

    cout.precision(3);
    print_stat(LEAST_SQUARE_CG(), "solver least square cg");
    print_stat(BICGSTAB(), "solver bicgstab");
    print_stat(GMRES(), "solver gmres");
    print_stat(QMR(), "solver qmr");
    print_stat(CG(), "solver cg");
    print_stat(P1, "no precond");
    print_stat(P2, "diag precond");
    print_stat(P3, "mr precond");
    print_stat(P4, "ilu precond");
    print_stat(P5, "ilut precond");
    print_stat(P5b, "ilutp precond");
    print_stat(P6, "ildlt precond");
    print_stat(P7, "ildltt precond");
    if (ratio_max > 0.2) GMM_ASSERT1(false, "something wrong ..");
    return true;
  }
 
  return false;
}
