/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2002-2012 Yves Renard
 
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

/**@file gmm_def.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Basic definitions and tools of GMM.
*/
#ifndef GMM_DEF_H__
#define GMM_DEF_H__

#include "gmm_ref.h"
#include <complex>

#ifndef M_PI
# define	M_E		2.7182818284590452354       /* e          */
# define	M_LOG2E		1.4426950408889634074       /* 1/ln(2)    */
# define	M_LOG10E	0.43429448190325182765      /* 1/ln(10)   */
# define	M_LN2		0.69314718055994530942      /* ln(2)      */
# define	M_LN10		2.30258509299404568402      /* ln(10)     */
# define	M_PI		3.14159265358979323846      /* pi         */
# define	M_PI_2		1.57079632679489661923      /* pi/2       */
# define	M_PI_4		0.78539816339744830962      /* pi/4       */
# define	M_1_PI		0.31830988618379067154      /* 1/pi       */
# define	M_2_PI		0.63661977236758134308      /* 2/pi       */
# define	M_2_SQRTPI	1.12837916709551257390      /* 2/sqrt(pi) */
# define	M_SQRT2		1.41421356237309504880      /* sqrt(2)    */
# define	M_SQRT1_2	0.70710678118654752440      /* sqrt(2)/2  */
#endif 

#ifndef M_PIl
# define M_PIl       3.1415926535897932384626433832795029L  /* pi         */
# define M_PI_2l     1.5707963267948966192313216916397514L  /* pi/2       */
# define M_PI_4l     0.7853981633974483096156608458198757L  /* pi/4       */
# define M_1_PIl     0.3183098861837906715377675267450287L  /* 1/pi       */
# define M_2_PIl     0.6366197723675813430755350534900574L  /* 2/pi       */
# define M_2_SQRTPIl 1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
#endif

namespace gmm {

  typedef size_t size_type;

  /* ******************************************************************** */
  /*		Specifier types                             		  */
  /* ******************************************************************** */
  /* not perfectly null, required by aCC 3.33                             */
  struct abstract_null_type { 
    abstract_null_type(int=0) {}
    template <typename A,typename B,typename C> void operator()(A,B,C) {}
  }; // specify an information lake.

  struct linalg_true {};
  struct linalg_false {};

  template <typename V, typename W> struct linalg_and
  { typedef linalg_false bool_type; };
  template <> struct linalg_and<linalg_true, linalg_true>
  { typedef linalg_true bool_type; };
  template <typename V, typename W> struct linalg_or
  { typedef linalg_true bool_type; };
  template <> struct linalg_and<linalg_false, linalg_false>
  { typedef linalg_false bool_type; };

  struct linalg_const {};       // A reference is either linalg_const,
  struct linalg_modifiable {};  //  linalg_modifiable or linalg_false.

  struct abstract_vector {};    // The object is a vector
  struct abstract_matrix {};    // The object is a matrix
  
  struct abstract_sparse {};    // sparse matrix or vector
  struct abstract_skyline {};   // 'sky-line' matrix or vector
  struct abstract_dense {};     // dense matrix or vector
  struct abstract_indirect {};  // matrix given by the product with a vector

  struct row_major {};          // matrix with a row access.
  struct col_major {};          // matrix with a column access
  struct row_and_col {};        // both accesses but row preference
  struct col_and_row {};        // both accesses but column preference

  template <typename T> struct transposed_type;
  template<> struct transposed_type<row_major>   {typedef col_major   t_type;};
  template<> struct transposed_type<col_major>   {typedef row_major   t_type;};
  template<> struct transposed_type<row_and_col> {typedef col_and_row t_type;};
  template<> struct transposed_type<col_and_row> {typedef row_and_col t_type;};

  template <typename T> struct principal_orientation_type
  { typedef abstract_null_type potype; };
  template<> struct principal_orientation_type<row_major>
  { typedef row_major potype; };
  template<> struct principal_orientation_type<col_major>
  { typedef col_major potype; };
  template<> struct principal_orientation_type<row_and_col>
  { typedef row_major potype; };
  template<> struct principal_orientation_type<col_and_row>
  { typedef col_major potype; };

  //  template <typename V> struct linalg_traits;
  template <typename V> struct linalg_traits {    
    typedef abstract_null_type this_type;
    typedef abstract_null_type linalg_type;
    typedef abstract_null_type value_type;
    typedef abstract_null_type is_reference;
    typedef abstract_null_type& reference;
    typedef abstract_null_type* iterator;
    typedef const abstract_null_type* const_iterator;
    typedef abstract_null_type index_sorted;
    typedef abstract_null_type storage_type;
    typedef abstract_null_type origin_type;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_row_iterator;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_col_iterator;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type sub_orientation;    
  };

  template <typename PT, typename V> struct vect_ref_type;
  template <typename P, typename V> struct vect_ref_type<P *, V> {
    typedef typename linalg_traits<V>::reference access_type;
    typedef typename linalg_traits<V>::iterator iterator;
  };
  template <typename P, typename V> struct vect_ref_type<const P *, V> {
    typedef typename linalg_traits<V>::value_type access_type;
    typedef typename linalg_traits<V>::const_iterator iterator;
  };
  
  template <typename PT> struct const_pointer;
  template <typename P> struct const_pointer<P *>
  { typedef const P* pointer; };
  template <typename P> struct const_pointer<const P *>
  { typedef const P* pointer; };

  template <typename PT> struct modifiable_pointer;
  template <typename P> struct modifiable_pointer<P *>
  { typedef P* pointer; };
  template <typename P> struct modifiable_pointer<const P *>
  { typedef P* pointer; };

  template <typename R> struct const_reference;
  template <typename R> struct const_reference<R &>
  { typedef const R &reference; };
  template <typename R> struct const_reference<const R &>
  { typedef const R  &reference; };


  inline bool is_sparse(abstract_sparse)   { return true;  }
  inline bool is_sparse(abstract_dense)    { return false; }
  inline bool is_sparse(abstract_skyline)  { return true;  }
  inline bool is_sparse(abstract_indirect) { return false; }

  template <typename L> inline bool is_sparse(const L &) 
  { return is_sparse(typename linalg_traits<L>::storage_type()); }

  inline bool is_row_matrix_(row_major)     { return true;  }
  inline bool is_row_matrix_(col_major)     { return false; }
  inline bool is_row_matrix_(row_and_col)   { return true;  }
  inline bool is_row_matrix_(col_and_row)   { return true;  }

  template <typename L> inline bool is_row_matrix(const L &) 
  { return is_row_matrix_(typename linalg_traits<L>::sub_orientation()); }

  inline bool is_col_matrix_(row_major)     { return false; }
  inline bool is_col_matrix_(col_major)     { return true;  }
  inline bool is_col_matrix_(row_and_col)   { return true;  }
  inline bool is_col_matrix_(col_and_row)   { return true;  }

  template <typename L> inline bool is_col_matrix(const L &) 
  { return is_col_matrix_(typename linalg_traits<L>::sub_orientation()); }

  inline bool is_col_matrix(row_major) { return false; }
  inline bool is_col_matrix(col_major) { return true; }
  inline bool is_row_matrix(row_major) { return true; }
  inline bool is_row_matrix(col_major) { return false; }

  template <typename L> inline bool is_const_reference(L) { return false; }
  inline bool is_const_reference(linalg_const) { return true; }  


  template <typename T> struct is_gmm_interfaced_ {
    typedef linalg_true result;
  };
  
  template<> struct is_gmm_interfaced_<abstract_null_type> {
    typedef linalg_false result;
  };
  
  template <typename T> struct is_gmm_interfaced {
    typedef typename is_gmm_interfaced_<typename gmm::linalg_traits<T>::this_type >::result result;
  };

  /* ******************************************************************** */
  /*  types to deal with const object representing a modifiable reference */
  /* ******************************************************************** */
  
  template <typename PT, typename R> struct mref_type_ 
  { typedef abstract_null_type return_type; };
  template <typename L, typename R> struct mref_type_<L *, R>
  { typedef L & return_type; };
  template <typename L, typename R> struct mref_type_<const L *, R>
  { typedef const L & return_type; };
  template <typename L> struct mref_type_<L *, linalg_const>
  { typedef const L & return_type; };
  template <typename L> struct mref_type_<const L *, linalg_const>
  { typedef const L & return_type; };
  template <typename L> struct mref_type_<const L *, linalg_modifiable>
  { typedef L & return_type; };
  template <typename L> struct mref_type_<L *, linalg_modifiable>
  { typedef L & return_type; };

  template <typename PT> struct mref_type {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename mref_type_<PT, 
      typename linalg_traits<L>::is_reference>::return_type return_type;
  };

  template <typename L> typename mref_type<const L *>::return_type 
  linalg_cast(const L &l)
  { return const_cast<typename mref_type<const L *>::return_type>(l); }

  template <typename L> typename mref_type<L *>::return_type linalg_cast(L &l)
  { return const_cast<typename mref_type<L *>::return_type>(l); }

  template <typename L, typename R> struct cref_type_
  { typedef abstract_null_type return_type; };
  template <typename L> struct cref_type_<L, linalg_modifiable>
  { typedef L & return_type; };
  template <typename L> struct cref_type {
    typedef typename cref_type_<L, 
      typename linalg_traits<L>::is_reference>::return_type return_type;
  };

  template <typename L> typename cref_type<L>::return_type 
  linalg_const_cast(const L &l)
  { return const_cast<typename cref_type<L>::return_type>(l); }


  // To be used to select between a reference or a const refercence for
  // the return type of a function
  // select_return<C1, C2, L *> return C1 if L is a const reference,
  //                                   C2 otherwise.
  // select_return<C1, C2, const L *> return C2 if L is a modifiable reference
  //                                         C1 otherwise. 
  template <typename C1, typename C2, typename REF> struct select_return_ {
    typedef abstract_null_type return_type;
  };
  template <typename C1, typename C2, typename L>
  struct select_return_<C1, C2, const L &> { typedef C1 return_type; };
  template <typename C1, typename C2, typename L>
  struct select_return_<C1, C2, L &> { typedef C2 return_type; };
  template <typename C1, typename C2, typename PT> struct select_return {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return_<C1, C2, 
      typename mref_type<PT>::return_type>::return_type return_type;
  };

  
  // To be used to select between a reference or a const refercence inside
  // a structure or a linagl_traits
  // select_ref<C1, C2, L *> return C1 if L is a const reference,
  //                                C2 otherwise.
  // select_ref<C1, C2, const L *> return C2 in any case. 
  template <typename C1, typename C2, typename REF> struct select_ref_
  { typedef abstract_null_type ref_type; };
  template <typename C1, typename C2, typename L>
  struct select_ref_<C1, C2, const L &> { typedef C1 ref_type; };
  template <typename C1, typename C2, typename L>
  struct select_ref_<C1, C2, L &> { typedef C2 ref_type; };
  template <typename C1, typename C2, typename PT> struct select_ref {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_ref_<C1, C2, 
      typename mref_type<PT>::return_type>::ref_type ref_type;
  };
  template <typename C1, typename C2, typename L>
  struct select_ref<C1, C2, const L *>
  { typedef C1 ref_type; };


  template<typename R> struct is_a_reference_
  { typedef linalg_true reference; };
  template<> struct is_a_reference_<linalg_false>
  { typedef linalg_false reference; };

  template<typename L> struct is_a_reference {
    typedef typename is_a_reference_<typename linalg_traits<L>::is_reference>
      ::reference reference;
  };


  template <typename L> inline bool is_original_linalg(const L &) 
  { return is_original_linalg(typename is_a_reference<L>::reference()); }
  inline bool is_original_linalg(linalg_false) { return true; }
  inline bool is_original_linalg(linalg_true) { return false; }


  template <typename PT> struct which_reference 
  { typedef abstract_null_type is_reference; };
  template <typename PT> struct which_reference<PT *>
  { typedef linalg_modifiable is_reference; };
  template <typename PT> struct which_reference<const PT *>
  { typedef linalg_const is_reference; };


  template <typename C1, typename C2, typename R> struct select_orientation_
  { typedef abstract_null_type return_type; };
  template <typename C1, typename C2>
  struct select_orientation_<C1, C2, row_major>
  { typedef C1 return_type; };
  template <typename C1, typename C2>
  struct select_orientation_<C1, C2, col_major>
  { typedef C2 return_type; };
  template <typename C1, typename C2, typename L> struct select_orientation {
    typedef typename select_orientation_<C1, C2,
      typename principal_orientation_type<typename
      linalg_traits<L>::sub_orientation>::potype>::return_type return_type;
  };
  
  /* ******************************************************************** */
  /*		Operations on scalars                         		  */
  /* ******************************************************************** */

  template <typename T> inline T sqr(T a) { return T(a * a); }
  template <typename T> inline T abs(T a) { return (a < T(0)) ? T(-a) : a; }
  template <typename T> inline T abs(std::complex<T> a)
  { T x = a.real(), y = a.imag(); return T(::sqrt(x*x+y*y)); }
  template <typename T> inline T abs_sqr(T a) { return T(a*a); }
  template <typename T> inline T abs_sqr(std::complex<T> a)
  { return gmm::sqr(a.real()) + gmm::sqr(a.imag()); }
  template <typename T> inline T pos(T a) { return (a < T(0)) ? T(0) : a; }
  template <typename T> inline T neg(T a) { return (a < T(0)) ? T(-a) : T(0); }
  template <typename T> inline T sgn(T a) { return (a < T(0)) ? T(-1) : T(1); }
  template <typename T> inline T Heaviside(T a) { return (a < T(0)) ? T(0) : T(1); }
  inline double random() { return double(rand())/(RAND_MAX+0.5); }
  template <typename T> inline T random(T)
  { return T(rand()*2.0)/(T(RAND_MAX)+T(1)/T(2)) - T(1); }
  template <typename T> inline std::complex<T> random(std::complex<T>)
  { return std::complex<T>(gmm::random(T()), gmm::random(T())); }
  template <typename T> inline T irandom(T max)
  { return T(gmm::random() * double(max)); }
  template <typename T> inline T conj(T a) { return a; }
  template <typename T> inline std::complex<T> conj(std::complex<T> a)
  { return std::conj(a); }
  template <typename T> inline T real(T a) { return a; }
  template <typename T> inline T real(std::complex<T> a) { return a.real(); }
  template <typename T> inline T imag(T ) { return T(0); }
  template <typename T> inline T imag(std::complex<T> a) { return a.imag(); }  
  template <typename T> inline T sqrt(T a) { return T(::sqrt(a)); }
  template <typename T> inline std::complex<T> sqrt(std::complex<T> a) {
    T x = a.real(), y = a.imag();
    if (x == T(0)) {
      T t = T(::sqrt(gmm::abs(y) / T(2)));
      return std::complex<T>(t, y < T(0) ? -t : t);
    }
    T t = T(::sqrt(T(2) * (gmm::abs(a) + gmm::abs(x)))), u = t / T(2);
    return x > T(0) ? std::complex<T>(u, y / t)
      : std::complex<T>(gmm::abs(y) / t, y < T(0) ? -u : u);
  }
  using std::swap;


  template <typename T> struct number_traits {
    typedef T magnitude_type;
  };
 
  template <typename T> struct number_traits<std::complex<T> > {
    typedef T magnitude_type;
  };

  template <typename T> inline T conj_product(T a, T b) { return a * b; }
  template <typename T> inline
  std::complex<T> conj_product(std::complex<T> a, std::complex<T> b)
  { return std::conj(a) * b; } // to be optimized ?

  template <typename T> inline bool is_complex(T) { return false; }
  template <typename T> inline bool is_complex(std::complex<T> )
  { return true; }

# define magnitude_of_linalg(M) typename number_traits<typename \
                    linalg_traits<M>::value_type>::magnitude_type
  
  template<typename T> inline std::complex<T> operator*(const std::complex<T>& a, int b) {
    return a*T(b);
  }
  template<typename T> inline std::complex<T> operator*(int b, const std::complex<T>& a) {
    return a*T(b);
  }

  /* ******************************************************************** */
  /*  types promotion                                                     */
  /* ******************************************************************** */

  /* should be completed for more specific cases <unsigned int, float> etc */
  template <typename T1, typename T2, bool c>
  struct strongest_numeric_type_aux {
    typedef T1 T;
  };
  template <typename T1, typename T2>
  struct strongest_numeric_type_aux<T1,T2,false> {
    typedef T2 T;
  };

  template <typename T1, typename T2>
  struct strongest_numeric_type {
    typedef typename
    strongest_numeric_type_aux<T1,T2,(sizeof(T1)>sizeof(T2))>::T T;
  };
  template <typename T1, typename T2>
  struct strongest_numeric_type<T1,std::complex<T2> > {
    typedef typename number_traits<T1>::magnitude_type R1;
    typedef std::complex<typename strongest_numeric_type<R1,T2>::T > T;
  };
  template <typename T1, typename T2>
  struct strongest_numeric_type<std::complex<T1>,T2 > {
    typedef typename number_traits<T2>::magnitude_type R2;
    typedef std::complex<typename strongest_numeric_type<T1,R2>::T > T;
  };
  template <typename T1, typename T2> 
  struct strongest_numeric_type<std::complex<T1>,std::complex<T2> > {
    typedef std::complex<typename strongest_numeric_type<T1,T2>::T > T;
  };

  template<> struct strongest_numeric_type<int,float>   { typedef float T;  };
  template<> struct strongest_numeric_type<float,int>   { typedef float T;  };
  template<> struct strongest_numeric_type<long,float>  { typedef float T;  };
  template<> struct strongest_numeric_type<float,long>  { typedef float T;  };
  template<> struct strongest_numeric_type<long,double> { typedef double T; };
  template<> struct strongest_numeric_type<double,long> { typedef double T; };

  template <typename V1, typename V2>
  struct strongest_value_type {
    typedef typename
    strongest_numeric_type<typename linalg_traits<V1>::value_type,
			   typename linalg_traits<V2>::value_type>::T
    value_type;
  };
  template <typename V1, typename V2, typename V3>
  struct strongest_value_type3 {
    typedef typename
    strongest_value_type<V1, typename
			 strongest_value_type<V2,V3>::value_type>::value_type
    value_type;
  };

  

  /* ******************************************************************** */
  /*		Basic vectors used                         		  */
  /* ******************************************************************** */
  
  template<typename T> struct dense_vector_type 
  { typedef std::vector<T> vector_type; };

  template <typename T> class wsvector;
  template <typename T> class rsvector;
  template<typename T> struct sparse_vector_type 
  { typedef wsvector<T> vector_type; };

  template <typename T> class slvector;
  template <typename T> class dense_matrix;
  template <typename VECT> class row_matrix;
  template <typename VECT> class col_matrix;
  

  /* ******************************************************************** */
  /*   Selects a temporary vector type                                    */
  /*   V if V is a valid vector type,                                     */
  /*   wsvector if V is a reference on a sparse vector,                   */
  /*   std::vector if V is a reference on a dense vector.                 */
  /* ******************************************************************** */

  
  template <typename R, typename S, typename L, typename V>
  struct temporary_vector_ {
    typedef abstract_null_type vector_type;
  };
  template <typename V, typename L>
  struct temporary_vector_<linalg_true, abstract_sparse, L, V>
  { typedef wsvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V, typename L>
  struct temporary_vector_<linalg_true, abstract_skyline, L, V>
  { typedef slvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V, typename L>
  struct temporary_vector_<linalg_true, abstract_dense, L, V>
  { typedef std::vector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename S, typename V>
  struct temporary_vector_<linalg_false, S, abstract_vector, V>
  { typedef V vector_type; };
  template <typename V>
  struct temporary_vector_<linalg_false, abstract_dense, abstract_matrix, V>
  { typedef std::vector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_vector_<linalg_false, abstract_sparse, abstract_matrix, V>
  { typedef wsvector<typename linalg_traits<V>::value_type> vector_type; };

  template <typename V> struct temporary_vector {
    typedef typename temporary_vector_<typename is_a_reference<V>::reference,
				       typename linalg_traits<V>::storage_type,
				       typename linalg_traits<V>::linalg_type,
				       V>::vector_type vector_type;
  };

  /* ******************************************************************** */
  /*   Selects a temporary matrix type                                    */
  /*   M if M is a valid matrix type,                                     */
  /*   row_matrix<wsvector> if M is a reference on a sparse matrix,       */
  /*   dense_matrix if M is a reference on a dense matrix.                */
  /* ******************************************************************** */

  
  template <typename R, typename S, typename L, typename V>
  struct temporary_matrix_ { typedef abstract_null_type matrix_type; };
  template <typename V, typename L>
  struct temporary_matrix_<linalg_true, abstract_sparse, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef row_matrix<wsvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_matrix_<linalg_true, abstract_skyline, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef row_matrix<slvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_matrix_<linalg_true, abstract_dense, L, V>
  { typedef dense_matrix<typename linalg_traits<V>::value_type> matrix_type; };
  template <typename S, typename V>
  struct temporary_matrix_<linalg_false, S, abstract_matrix, V>
  { typedef V matrix_type; };

  template <typename V> struct temporary_matrix {
    typedef typename temporary_matrix_<typename is_a_reference<V>::reference,
				       typename linalg_traits<V>::storage_type,
				       typename linalg_traits<V>::linalg_type,
				       V>::matrix_type matrix_type;
  };

  
  template <typename S, typename L, typename V>
  struct temporary_col_matrix_ { typedef abstract_null_type matrix_type; };
  template <typename V, typename L>
  struct temporary_col_matrix_<abstract_sparse, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef col_matrix<wsvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_col_matrix_<abstract_skyline, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef col_matrix<slvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_col_matrix_<abstract_dense, L, V>
  { typedef dense_matrix<typename linalg_traits<V>::value_type> matrix_type; };

  template <typename V> struct temporary_col_matrix {
    typedef typename temporary_col_matrix_<
      typename linalg_traits<V>::storage_type,
      typename linalg_traits<V>::linalg_type,
      V>::matrix_type matrix_type;
  };




  template <typename S, typename L, typename V>
  struct temporary_row_matrix_ { typedef abstract_null_type matrix_type; };
  template <typename V, typename L>
  struct temporary_row_matrix_<abstract_sparse, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef row_matrix<wsvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_row_matrix_<abstract_skyline, L, V> {
    typedef typename linalg_traits<V>::value_type T;
    typedef row_matrix<slvector<T> > matrix_type;
  };
  template <typename V, typename L>
  struct temporary_row_matrix_<abstract_dense, L, V>
  { typedef dense_matrix<typename linalg_traits<V>::value_type> matrix_type; };

  template <typename V> struct temporary_row_matrix {
    typedef typename temporary_row_matrix_<
      typename linalg_traits<V>::storage_type,
      typename linalg_traits<V>::linalg_type,
      V>::matrix_type matrix_type;
  };



  /* ******************************************************************** */
  /*   Selects a temporary dense vector type                              */
  /*   V if V is a valid dense vector type,                               */
  /*   std::vector if V is a reference or another type of vector          */
  /* ******************************************************************** */

  template <typename R, typename S, typename V>
  struct temporary_dense_vector_ { typedef abstract_null_type vector_type; };
  template <typename S, typename V>
  struct temporary_dense_vector_<linalg_true, S, V>
  { typedef std::vector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_dense_vector_<linalg_false, abstract_sparse, V>
  { typedef std::vector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_dense_vector_<linalg_false, abstract_skyline, V>
  { typedef std::vector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_dense_vector_<linalg_false, abstract_dense, V>
  { typedef V vector_type; };

  template <typename V> struct temporary_dense_vector {
    typedef typename temporary_dense_vector_<typename
    is_a_reference<V>::reference,
    typename linalg_traits<V>::storage_type, V>::vector_type vector_type;
  };

  /* ******************************************************************** */
  /*   Selects a temporary sparse vector type                             */
  /*   V if V is a valid sparse vector type,                              */
  /*   wsvector if V is a reference or another type of vector             */
  /* ******************************************************************** */

  template <typename R, typename S, typename V>
  struct temporary_sparse_vector_ { typedef abstract_null_type vector_type; };
  template <typename S, typename V>
  struct temporary_sparse_vector_<linalg_true, S, V>
  { typedef wsvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_sparse_vector_<linalg_false, abstract_sparse, V>
  { typedef V vector_type; };
  template <typename V>
  struct temporary_sparse_vector_<linalg_false, abstract_dense, V>
  { typedef wsvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_sparse_vector_<linalg_false, abstract_skyline, V>
  { typedef wsvector<typename linalg_traits<V>::value_type> vector_type; };

  template <typename V> struct temporary_sparse_vector {
    typedef typename temporary_sparse_vector_<typename
    is_a_reference<V>::reference,
    typename linalg_traits<V>::storage_type, V>::vector_type vector_type;
  };

  /* ******************************************************************** */
  /*   Selects a temporary sky-line vector type                           */
  /*   V if V is a valid sky-line vector type,                            */
  /*   slvector if V is a reference or another type of vector             */
  /* ******************************************************************** */

  template <typename R, typename S, typename V>
  struct temporary_skyline_vector_
  { typedef abstract_null_type vector_type; };
  template <typename S, typename V>
  struct temporary_skyline_vector_<linalg_true, S, V>
  { typedef slvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_skyline_vector_<linalg_false, abstract_skyline, V>
  { typedef V vector_type; };
  template <typename V>
  struct temporary_skyline_vector_<linalg_false, abstract_dense, V>
  { typedef slvector<typename linalg_traits<V>::value_type> vector_type; };
  template <typename V>
  struct temporary_skyline_vector_<linalg_false, abstract_sparse, V>
  { typedef slvector<typename linalg_traits<V>::value_type> vector_type; };

  template <typename V> struct temporary_skylines_vector {
    typedef typename temporary_skyline_vector_<typename
    is_a_reference<V>::reference,
    typename linalg_traits<V>::storage_type, V>::vector_type vector_type;
  };

  /* ********************************************************************* */
  /*  Definition & Comparison of origins.                                  */
  /* ********************************************************************* */

  template <typename L> 
  typename select_return<const typename linalg_traits<L>::origin_type *,
			 typename linalg_traits<L>::origin_type *,
			 L *>::return_type
  linalg_origin(L &l)
  { return linalg_traits<L>::origin(linalg_cast(l)); }

  template <typename L> 
  typename select_return<const typename linalg_traits<L>::origin_type *,
			 typename linalg_traits<L>::origin_type *,
			 const L *>::return_type
  linalg_origin(const L &l)
  { return linalg_traits<L>::origin(linalg_cast(l)); }

  template <typename PT1, typename PT2>
  bool same_porigin(PT1, PT2) { return false; }

  template <typename PT>
  bool same_porigin(PT pt1, PT pt2) { return (pt1 == pt2); }

  template <typename L1, typename L2>
  bool same_origin(const L1 &l1, const L2 &l2)
  { return same_porigin(linalg_origin(l1), linalg_origin(l2)); }


  /* ******************************************************************** */
  /*		Miscellaneous                           		  */
  /* ******************************************************************** */

  template <typename V> inline size_type vect_size(const V &v)
  { return linalg_traits<V>::size(v); }

  template <typename MAT> inline size_type mat_nrows(const MAT &m)
  { return linalg_traits<MAT>::nrows(m); }

  template <typename MAT> inline size_type mat_ncols(const MAT &m)
  { return linalg_traits<MAT>::ncols(m); }


  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
           typename linalg_traits<V>::iterator, V *>::return_type
  vect_begin(V &v)
  { return linalg_traits<V>::begin(linalg_cast(v)); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
	   typename linalg_traits<V>::iterator, const V *>::return_type
  vect_begin(const V &v)
  { return linalg_traits<V>::begin(linalg_cast(v)); }

  template <typename V> inline
  typename linalg_traits<V>::const_iterator
  vect_const_begin(const V &v)
  { return linalg_traits<V>::begin(v); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
    typename linalg_traits<V>::iterator, V *>::return_type
  vect_end(V &v)
  { return linalg_traits<V>::end(linalg_cast(v)); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
    typename linalg_traits<V>::iterator, const V *>::return_type
  vect_end(const V &v)
  { return linalg_traits<V>::end(linalg_cast(v)); }

  template <typename V> inline
  typename linalg_traits<V>::const_iterator
  vect_const_end(const V &v)
  { return linalg_traits<V>::end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, M *>::return_type
  mat_row_begin(M &m) { return linalg_traits<M>::row_begin(linalg_cast(m)); }
  
  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, const M *>::return_type
  mat_row_begin(const M &m)
  { return linalg_traits<M>::row_begin(linalg_cast(m)); }
  
  template <typename M> inline typename linalg_traits<M>::const_row_iterator
  mat_row_const_begin(const M &m)
  { return linalg_traits<M>::row_begin(m); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, M *>::return_type
  mat_row_end(M &v) {
    return linalg_traits<M>::row_end(linalg_cast(v));
  }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, const M *>::return_type
  mat_row_end(const M &v) {
    return linalg_traits<M>::row_end(linalg_cast(v));
  }

  template <typename M> inline
  typename linalg_traits<M>::const_row_iterator
  mat_row_const_end(const M &v)
  { return linalg_traits<M>::row_end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
    typename linalg_traits<M>::col_iterator, M *>::return_type
  mat_col_begin(M &v) {
    return linalg_traits<M>::col_begin(linalg_cast(v));
  }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
    typename linalg_traits<M>::col_iterator, const M *>::return_type
  mat_col_begin(const M &v) {
    return linalg_traits<M>::col_begin(linalg_cast(v));
  }

  template <typename M> inline
  typename linalg_traits<M>::const_col_iterator
  mat_col_const_begin(const M &v)
  { return linalg_traits<M>::col_begin(v); }

  template <typename M> inline
  typename linalg_traits<M>::const_col_iterator
  mat_col_const_end(const M &v)
  { return linalg_traits<M>::col_end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
                         typename linalg_traits<M>::col_iterator,
                         M *>::return_type
  mat_col_end(M &m)
  { return linalg_traits<M>::col_end(linalg_cast(m)); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
                         typename linalg_traits<M>::col_iterator,
                         const M *>::return_type
  mat_col_end(const M &m)
  { return linalg_traits<M>::col_end(linalg_cast(m)); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_row_type,
                         typename linalg_traits<MAT>::sub_row_type,
                         const MAT *>::return_type
  mat_row(const MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_begin(m) + i); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_row_type,
                         typename linalg_traits<MAT>::sub_row_type,
                         MAT *>::return_type
  mat_row(MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_begin(m) + i); }

  template <typename MAT> inline
  typename linalg_traits<MAT>::const_sub_row_type
  mat_const_row(const MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_const_begin(m) + i); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_col_type,
                         typename linalg_traits<MAT>::sub_col_type,
                         const MAT *>::return_type
  mat_col(const MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_begin(m) + i); }


  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_col_type,
                         typename linalg_traits<MAT>::sub_col_type,
                         MAT *>::return_type
  mat_col(MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_begin(m) + i); }
  
  template <typename MAT> inline
  typename linalg_traits<MAT>::const_sub_col_type
  mat_const_col(const MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_const_begin(m) + i); }
  
  /* ********************************************************************* */
  /* Set to begin end set to end for iterators on non-const sparse vectors.*/
  /* ********************************************************************* */

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &it, ORG o, VECT *, linalg_false)
  { it = vect_begin(*o); }

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &it, ORG o, const VECT *, linalg_false) 
  { it = vect_const_begin(*o); }

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &it, ORG o, VECT *, linalg_false)
  { it = vect_end(*o); }
  
  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &it, ORG o, const VECT *, linalg_false)
  { it = vect_const_end(*o); }


  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &, ORG, VECT *, linalg_const) { }

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &, ORG, const VECT *, linalg_const) { }

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &, ORG, VECT *, linalg_const) { }
  
  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &, ORG, const VECT *, linalg_const) { }


  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &, ORG, VECT *v, linalg_modifiable)
  { GMM_ASSERT3(!is_sparse(*v), "internal_error"); v = 0; }

  template <typename IT, typename ORG, typename VECT> inline
  void set_to_begin(IT &, ORG, const VECT *v, linalg_modifiable)
  { GMM_ASSERT3(!is_sparse(*v), "internal_error"); v = 0; }
 
  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &, ORG, VECT *v, linalg_modifiable)
  { GMM_ASSERT3(!is_sparse(*v), "internal_error"); v = 0; }
  
  template <typename IT, typename ORG, typename VECT> inline
  void set_to_end(IT &, ORG, const VECT *v, linalg_modifiable)
  { GMM_ASSERT3(!is_sparse(*v), "internal_error"); v = 0; }

  /* ******************************************************************** */
  /*		General index for certain algorithms.         		  */
  /* ******************************************************************** */

  template<class IT> 
  size_type index_of_it(const IT &it, size_type, abstract_sparse)
  { return it.index(); }
  template<class IT> 
  size_type index_of_it(const IT &it, size_type, abstract_skyline)
  { return it.index(); }
  template<class IT> 
  size_type index_of_it(const IT &, size_type k, abstract_dense)
  { return k; }

  /* ********************************************************************* */
  /* Numeric limits.                                                       */
  /* ********************************************************************* */
  
  template<typename T> inline T default_tol(T) {
    using namespace std;
    static T tol(10);
    if (tol == T(10)) {
      if (numeric_limits<T>::is_specialized)
	tol = numeric_limits<T>::epsilon();
      else {
	int i=sizeof(T)/4; while(i-- > 0) tol*=T(1E-8); 
	GMM_WARNING1("The numeric type " << typeid(T).name()
		    << " has no numeric_limits defined !!\n"
		    << "Taking " << tol << " as default tolerance");
      }
    }
    return tol;
  }
  template<typename T> inline T default_tol(std::complex<T>)
  { return default_tol(T()); }

  template<typename T> inline T default_min(T) {
    using namespace std;
    static T mi(10);
    if (mi == T(10)) {
      if (numeric_limits<T>::is_specialized)
	mi = std::numeric_limits<T>::min();
      else {
	mi = T(0);
	GMM_WARNING1("The numeric type " << typeid(T).name()
		    << " has no numeric_limits defined !!\n"
		    << "Taking 0 as default minimum");
      }
    }
    return mi;
  }
  template<typename T> inline T default_min(std::complex<T>)
  { return default_min(T()); }

  template<typename T> inline T default_max(T) {
    using namespace std;
    static T mi(10);
    if (mi == T(10)) {
      if (numeric_limits<T>::is_specialized)
	mi = std::numeric_limits<T>::max();
      else {
	mi = T(1);
	GMM_WARNING1("The numeric type " << typeid(T).name()
		    << " has no numeric_limits defined !!\n"
		    << "Taking 1 as default maximum !");
      }
    }
    return mi;
  }
  template<typename T> inline T default_max(std::complex<T>)
  { return default_max(T()); }

  
  /*
    use safe_divide to avoid NaNs when dividing very small complex
    numbers, for example
    std::complex<float>(1e-23,1e-30)/std::complex<float>(1e-23,1e-30)
  */
  template<typename T> inline T safe_divide(T a, T b) { return a/b; }
  template<typename T> inline std::complex<T>
  safe_divide(std::complex<T> a, std::complex<T> b) {
    T m = std::max(gmm::abs(b.real()), gmm::abs(b.imag()));
    a = std::complex<T>(a.real()/m, a.imag()/m);
    b = std::complex<T>(b.real()/m, b.imag()/m);
    return a / b;
  }


  /* ******************************************************************** */
  /*		Write                                   		  */
  /* ******************************************************************** */

  template <typename T> struct cast_char_type { typedef T return_type; };
  template <> struct cast_char_type<signed char> { typedef int return_type; };
  template <> struct cast_char_type<unsigned char>
  { typedef unsigned int return_type; };
  template <typename T> inline typename cast_char_type<T>::return_type
  cast_char(const T &c) { return typename cast_char_type<T>::return_type(c); }


  template <typename L> inline void write(std::ostream &o, const L &l)
  { write(o, l, typename linalg_traits<L>::linalg_type()); }

  template <typename L> void write(std::ostream &o, const L &l,
				       abstract_vector) {
    o << "vector(" << vect_size(l) << ") [";
    write(o, l, typename linalg_traits<L>::storage_type());
    o << " ]";
  }

  template <typename L> void write(std::ostream &o, const L &l,
				       abstract_sparse) {
    typename linalg_traits<L>::const_iterator it = vect_const_begin(l),
      ite = vect_const_end(l);
    for (; it != ite; ++it) 
      o << " (r" << it.index() << "," << cast_char(*it) << ")";
  }

  template <typename L> void write(std::ostream &o, const L &l,
				       abstract_dense) {
    typename linalg_traits<L>::const_iterator it = vect_const_begin(l),
      ite = vect_const_end(l);
    if (it != ite) o << " " << cast_char(*it++);
    for (; it != ite; ++it) o << ", " << cast_char(*it);
  }

  template <typename L> void write(std::ostream &o, const L &l,
				       abstract_skyline) {
    typedef typename linalg_traits<L>::const_iterator const_iterator;
    const_iterator it = vect_const_begin(l), ite = vect_const_end(l);
    if (it != ite) {
      o << "<r+" << it.index() << ">";
      if (it != ite) o << " " << cast_char(*it++);
      for (; it != ite; ++it) { o << ", " << cast_char(*it); }
    }
  }

  template <typename L> inline void write(std::ostream &o, const L &l,
				       abstract_matrix) {
    write(o, l, typename linalg_traits<L>::sub_orientation());
  }


  template <typename L> void write(std::ostream &o, const L &l,
				       row_major) {
    o << "matrix(" << mat_nrows(l) << ", " << mat_ncols(l) << ")" << endl;
    for (size_type i = 0; i < mat_nrows(l); ++i) {
      o << "(";
      write(o, mat_const_row(l, i), typename linalg_traits<L>::storage_type());
      o << " )\n";
    }
  }

  template <typename L> inline
  void write(std::ostream &o, const L &l, row_and_col) 
  { write(o, l, row_major()); }

  template <typename L> inline
  void write(std::ostream &o, const L &l, col_and_row)
  { write(o, l, row_major()); }

  template <typename L> void write(std::ostream &o, const L &l, col_major) {
    o << "matrix(" << mat_nrows(l) << ", " << mat_ncols(l) << ")" << endl;
    for (size_type i = 0; i < mat_nrows(l); ++i) {
      o << "(";
      if (is_sparse(l)) { // not optimized ...
	for (size_type j = 0; j < mat_ncols(l); ++j)
	  if (l(i,j) != typename linalg_traits<L>::value_type(0)) 
	    o << " (r" << j << ", " << l(i,j) << ")";
      }
      else {
	if (mat_ncols(l) != 0) o << ' ' << l(i, 0);
	for (size_type j = 1; j < mat_ncols(l); ++j) o << ", " << l(i, j); 
      }
      o << " )\n";
    }
  }

}

#endif //  GMM_DEF_H__
