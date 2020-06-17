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

/**@file gmm_scaled.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date November 10, 2002.
   @brief get a scaled view of a vector/matrix.
*/
#ifndef GMM_SCALED_H__
#define GMM_SCALED_H__

#include "gmm_def.h"

namespace gmm {

  /* ********************************************************************* */
  /*		Scaled references on vectors            		   */
  /* ********************************************************************* */

  template <typename IT, typename S> struct scaled_const_iterator {
    typedef typename strongest_numeric_type<typename std::iterator_traits<IT>::value_type,
					    S>::T value_type;

    typedef typename std::iterator_traits<IT>::pointer         pointer;
    typedef typename std::iterator_traits<IT>::reference       reference;
    typedef typename std::iterator_traits<IT>::difference_type difference_type;
    typedef typename std::iterator_traits<IT>::iterator_category
    iterator_category;

    IT it;
    S r;
    
    scaled_const_iterator(void) {}
    scaled_const_iterator(const IT &i, S x) : it(i), r(x) {}
    
    inline size_type index(void) const { return it.index(); }
    inline scaled_const_iterator operator ++(int)
    { scaled_const_iterator tmp = *this; ++it; return tmp; }
    inline scaled_const_iterator operator --(int) 
    { scaled_const_iterator tmp = *this; --it; return tmp; }
    inline scaled_const_iterator &operator ++() { ++it; return *this; }
    inline scaled_const_iterator &operator --() { --it; return *this; }
    inline scaled_const_iterator &operator +=(difference_type i)
      { it += i; return *this; }
    inline scaled_const_iterator &operator -=(difference_type i)
      { it -= i; return *this; }
    inline scaled_const_iterator operator +(difference_type i) const
      { scaled_const_iterator itb = *this; return (itb += i); }
    inline scaled_const_iterator operator -(difference_type i) const
      { scaled_const_iterator itb = *this; return (itb -= i); }
    inline difference_type operator -(const scaled_const_iterator &i) const
      { return difference_type(it - i.it); }
    
    inline value_type operator  *() const { return (*it) * value_type(r); }
    inline value_type operator [](size_type ii) const { return it[ii] * r; }
    
    inline bool operator ==(const scaled_const_iterator &i) const
      { return (i.it == it); }
    inline bool operator !=(const scaled_const_iterator &i) const
      { return (i.it != it); }
    inline bool operator < (const scaled_const_iterator &i) const
      { return (it < i.it); }
  };

  template <typename V, typename S> struct scaled_vector_const_ref {
    typedef scaled_vector_const_ref<V,S> this_type;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<V>::const_iterator iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    size_type size_;
    S r;

    scaled_vector_const_ref(const V &v, S rr)
      : begin_(vect_const_begin(v)), end_(vect_const_end(v)),
	origin(linalg_origin(v)), size_(vect_size(v)), r(rr) {}

    reference operator[](size_type i) const
    { return value_type(r) * linalg_traits<V>::access(origin, begin_, end_, i); }
  };

  template <typename V, typename S> struct linalg_traits<scaled_vector_const_ref<V,S> > {
    typedef scaled_vector_const_ref<V,S> this_type;
    typedef linalg_const is_reference;
    typedef abstract_vector linalg_type;
    typedef typename strongest_numeric_type<S, typename linalg_traits<V>::value_type>::T value_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef value_type reference;
    typedef abstract_null_type iterator;
    typedef scaled_const_iterator<typename linalg_traits<V>::const_iterator, S>
      const_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    typedef typename linalg_traits<V>::index_sorted index_sorted;
    static size_type size(const this_type &v) { return v.size_; }
    static const_iterator begin(const this_type &v)
    { return const_iterator(v.begin_, v.r); }
    static const_iterator end(const this_type &v)
    { return const_iterator(v.end_, v.r); }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i)
    { return it.r * (linalg_traits<V>::access(o, it.it, ite.it, i)); }

  };

   template<typename V, typename S> std::ostream &operator <<
     (std::ostream &o, const scaled_vector_const_ref<V,S>& m)
  { gmm::write(o,m); return o; }

  /* ********************************************************************* */
  /*		Scaled references on matrices            		   */
  /* ********************************************************************* */

  template <typename M, typename S> struct scaled_row_const_iterator {
    typedef scaled_row_const_iterator<M,S> iterator;
    typedef typename linalg_traits<M>::const_row_iterator ITER;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    ITER it;
    S r;

    inline iterator operator ++(int) { iterator tmp=*this; it++; return tmp; }
    inline iterator operator --(int) { iterator tmp=*this; it--; return tmp; }
    inline iterator &operator ++()   { it++; return *this; }
    inline iterator &operator --()   { it--; return *this; }
    iterator &operator +=(difference_type i) { it += i; return *this; }
    iterator &operator -=(difference_type i) { it -= i; return *this; }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const
    { return it - i.it; }

    inline ITER operator *() const { return it; }
    inline ITER operator [](int i) { return it + i; }

    inline bool operator ==(const iterator &i) const { return (it == i.it); }
    inline bool operator !=(const iterator &i) const { return !(i == *this); }
    inline bool operator < (const iterator &i) const { return (it < i.it); }

    scaled_row_const_iterator(void) {}
    scaled_row_const_iterator(const ITER &i, S rr)
      : it(i), r(rr) { }

  };

  template <typename M, typename S> struct  scaled_row_matrix_const_ref {
    
    typedef scaled_row_matrix_const_ref<M,S> this_type;
    typedef typename linalg_traits<M>::const_row_iterator iterator;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    S r;
    size_type nr, nc;

    scaled_row_matrix_const_ref(const M &m, S rr)
      : begin_(mat_row_begin(m)), end_(mat_row_end(m)),
	origin(linalg_origin(m)), r(rr), nr(mat_nrows(m)), nc(mat_ncols(m)) {}

    value_type operator()(size_type i, size_type j) const
    { return r * linalg_traits<M>::access(begin_+i, j); }
  };

  template <typename M, typename S> struct linalg_traits<scaled_row_matrix_const_ref<M,S> > {
    typedef scaled_row_matrix_const_ref<M,S> this_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename strongest_numeric_type<S, typename linalg_traits<M>::value_type>::T value_type;
    typedef value_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef typename linalg_traits<M>::const_sub_row_type vector_type;
    typedef scaled_vector_const_ref<vector_type,S> sub_row_type;
    typedef scaled_vector_const_ref<vector_type,S> const_sub_row_type;
    typedef scaled_row_const_iterator<M,S> row_iterator;
    typedef scaled_row_const_iterator<M,S> const_row_iterator;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_col_iterator;
    typedef abstract_null_type col_iterator;
    typedef row_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type nrows(const this_type &m)
    { return m.nr; }
    static size_type ncols(const this_type &m)
    { return m.nc; }
    static const_sub_row_type row(const const_row_iterator &it)
    { return scaled(linalg_traits<M>::row(it.it), it.r); }
    static const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m.begin_, m.r); }
    static const_row_iterator row_end(const this_type &m)
    { return const_row_iterator(m.end_, m.r); }
    static const origin_type* origin(const this_type &m) { return m.origin; }
    static value_type access(const const_row_iterator &it, size_type i)
    { return it.r * (linalg_traits<M>::access(it.it, i)); }
  };

  template<typename M, typename S> std::ostream &operator <<
    (std::ostream &o, const scaled_row_matrix_const_ref<M,S>& m)
  { gmm::write(o,m); return o; }


  template <typename M, typename S> struct scaled_col_const_iterator {
    typedef scaled_col_const_iterator<M,S> iterator;
    typedef typename linalg_traits<M>::const_col_iterator ITER;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    ITER it;
    S r;

    iterator operator ++(int) { iterator tmp = *this; it++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; it--; return tmp; }
    iterator &operator ++()   { it++; return *this; }
    iterator &operator --()   { it--; return *this; }
    iterator &operator +=(difference_type i) { it += i; return *this; }
    iterator &operator -=(difference_type i) { it -= i; return *this; }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const
    { return it - i.it; }

    ITER operator *() const { return it; }
    ITER operator [](int i) { return it + i; }

    bool operator ==(const iterator &i) const { return (it == i.it); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (it < i.it); }

    scaled_col_const_iterator(void) {}
    scaled_col_const_iterator(const ITER &i, S rr)
      : it(i), r(rr) { }

  };

  template <typename M, typename S> struct  scaled_col_matrix_const_ref {
    
    typedef scaled_col_matrix_const_ref<M,S> this_type;
    typedef typename linalg_traits<M>::const_col_iterator iterator;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    S r;
    size_type nr, nc;

    scaled_col_matrix_const_ref(const M &m, S rr)
      : begin_(mat_col_begin(m)), end_(mat_col_end(m)),
	origin(linalg_origin(m)), r(rr), nr(mat_nrows(m)), nc(mat_ncols(m)) {}

    value_type operator()(size_type i, size_type j) const
    { return r * linalg_traits<M>::access(begin_+j, i); }
  };

  template <typename M, typename S> struct linalg_traits<scaled_col_matrix_const_ref<M,S> > {
    typedef scaled_col_matrix_const_ref<M,S> this_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename strongest_numeric_type<S, typename linalg_traits<M>::value_type>::T value_type;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef value_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef typename linalg_traits<M>::const_sub_col_type vector_type;
    typedef abstract_null_type sub_col_type;
    typedef scaled_vector_const_ref<vector_type,S> const_sub_col_type;
    typedef abstract_null_type  col_iterator;
    typedef scaled_col_const_iterator<M,S> const_col_iterator;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_row_iterator;
    typedef abstract_null_type row_iterator;
    typedef col_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type ncols(const this_type &m)
    { return m.nc; }
    static size_type nrows(const this_type &m)
    { return m.nr; }
    static const_sub_col_type col(const const_col_iterator &it)
    { return scaled(linalg_traits<M>::col(it.it), it.r); }
    static const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m.begin_, m.r); }
    static const_col_iterator col_end(const this_type &m)
    { return const_col_iterator(m.end_, m.r); }
    static const origin_type* origin(const this_type &m) { return m.origin; }
    static value_type access(const const_col_iterator &it, size_type i)
    { return it.r * (linalg_traits<M>::access(it.it, i)); }
  };

  template<typename M, typename S> std::ostream &operator <<
    (std::ostream &o, const scaled_col_matrix_const_ref<M,S>& m)
  { gmm::write(o,m); return o; }


  template <typename L, typename S, typename R> struct scaled_return__ {
    typedef abstract_null_type return_type;
  };
  template <typename L, typename S> struct scaled_return__<L, S, row_major> 
  { typedef scaled_row_matrix_const_ref<L,S> return_type; };
  template <typename L, typename S> struct scaled_return__<L, S, col_major> 
  { typedef scaled_col_matrix_const_ref<L,S> return_type; };
  

  template <typename L, typename S, typename LT> struct scaled_return_ {
    typedef abstract_null_type return_type;
  };
  template <typename L, typename S> struct scaled_return_<L, S, abstract_vector> 
  { typedef scaled_vector_const_ref<L,S> return_type; };
  template <typename L, typename S> struct scaled_return_<L, S, abstract_matrix> {
    typedef typename scaled_return__<L, S, 
      typename principal_orientation_type<typename
      linalg_traits<L>::sub_orientation>::potype>::return_type return_type;
  };

  template <typename L, typename S> struct scaled_return {
    typedef typename scaled_return_<L, S, typename
      linalg_traits<L>::linalg_type>::return_type return_type;
  };

  template <typename L, typename S> inline
  typename scaled_return<L,S>::return_type
  scaled(const L &v, S x)
  { return scaled(v, x, typename linalg_traits<L>::linalg_type()); }

  template <typename V, typename S> inline
  typename scaled_return<V,S>::return_type
  scaled(const V &v, S x, abstract_vector)
  { return scaled_vector_const_ref<V,S>(v, x); }

  template <typename M, typename S> inline
  typename scaled_return<M,S>::return_type
  scaled(const M &m, S x,abstract_matrix) {
    return scaled(m, x,  typename principal_orientation_type<typename
		  linalg_traits<M>::sub_orientation>::potype());
  }

  template <typename M, typename S> inline
  typename scaled_return<M,S>::return_type
  scaled(const M &m, S x, row_major) {
    return scaled_row_matrix_const_ref<M,S>(m, x);
  }

  template <typename M, typename S> inline
  typename scaled_return<M,S>::return_type
  scaled(const M &m, S x, col_major) {
    return scaled_col_matrix_const_ref<M,S>(m, x);
  }

  
  /* ******************************************************************** */
  /*	matrix or vector scale                                	          */
  /* ******************************************************************** */

  template <typename L> inline
  void scale(L& l, typename linalg_traits<L>::value_type a)
  { scale(l, a, typename linalg_traits<L>::linalg_type()); }

  template <typename L> inline
  void scale(const L& l, typename linalg_traits<L>::value_type a)
  { scale(linalg_const_cast(l), a); }

  template <typename L> inline
  void scale(L& l, typename linalg_traits<L>::value_type a, abstract_vector) {
    typename linalg_traits<L>::iterator it = vect_begin(l), ite = vect_end(l);
    for ( ; it != ite; ++it) *it *= a;
  }

  template <typename L> 
  void scale(L& l, typename linalg_traits<L>::value_type a, abstract_matrix) {
    scale(l, a, typename principal_orientation_type<typename
	  linalg_traits<L>::sub_orientation>::potype());
  }

  template <typename L> 
  void scale(L& l, typename linalg_traits<L>::value_type a, row_major) {
    typename linalg_traits<L>::row_iterator it = mat_row_begin(l),
      ite = mat_row_end(l);
    for ( ; it != ite; ++it) scale(linalg_traits<L>::row(it), a);
  }

  template <typename L> 
  void scale(L& l, typename linalg_traits<L>::value_type a, col_major) {
    typename linalg_traits<L>::col_iterator it = mat_col_begin(l),
      ite = mat_col_end(l);
    for ( ; it != ite; ++it) scale(linalg_traits<L>::col(it), a);
  }

}

#endif //  GMM_SCALED_H__
