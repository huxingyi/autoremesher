/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard
 
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

/**@file gmm_conjugated.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date September 18, 2003.
   @brief handle conjugation of complex matrices/vectors.
*/
#ifndef GMM_CONJUGATED_H__
#define GMM_CONJUGATED_H__

#include "gmm_def.h"

namespace gmm {
  ///@cond DOXY_SHOW_ALL_FUNCTIONS

  /* ********************************************************************* */
  /*		Conjugated references on vectors            		   */
  /* ********************************************************************* */

  template <typename IT> struct conjugated_const_iterator {
    typedef typename std::iterator_traits<IT>::value_type      value_type;
    typedef typename std::iterator_traits<IT>::pointer         pointer;
    typedef typename std::iterator_traits<IT>::reference       reference;
    typedef typename std::iterator_traits<IT>::difference_type difference_type;
    typedef typename std::iterator_traits<IT>::iterator_category
    iterator_category;

    IT it;
    
    conjugated_const_iterator(void) {}
    conjugated_const_iterator(const IT &i) : it(i) {}
    
    inline size_type index(void) const { return it.index(); }
    conjugated_const_iterator operator ++(int)
    { conjugated_const_iterator tmp = *this; ++it; return tmp; }
    conjugated_const_iterator operator --(int) 
    { conjugated_const_iterator tmp = *this; --it; return tmp; }
    conjugated_const_iterator &operator ++() { ++it; return *this; }
    conjugated_const_iterator &operator --() { --it; return *this; }
    conjugated_const_iterator &operator +=(difference_type i)
      { it += i; return *this; }
    conjugated_const_iterator &operator -=(difference_type i)
      { it -= i; return *this; }
    conjugated_const_iterator operator +(difference_type i) const
      { conjugated_const_iterator itb = *this; return (itb += i); }
    conjugated_const_iterator operator -(difference_type i) const
      { conjugated_const_iterator itb = *this; return (itb -= i); }
    difference_type operator -(const conjugated_const_iterator &i) const
      { return difference_type(it - i.it); }
    
    value_type operator  *() const { return gmm::conj(*it); }
    value_type operator [](size_type ii) const { return gmm::conj(it[ii]); }
    
    bool operator ==(const conjugated_const_iterator &i) const
      { return (i.it == it); }
    bool operator !=(const conjugated_const_iterator &i) const
      { return (i.it != it); }
    bool operator < (const conjugated_const_iterator &i) const
      { return (it < i.it); }
  };

  template <typename V> struct conjugated_vector_const_ref {
    typedef conjugated_vector_const_ref<V> this_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename linalg_traits<V>::const_iterator iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    size_type size_;

    conjugated_vector_const_ref(const V &v)
      : begin_(vect_const_begin(v)), end_(vect_const_end(v)),
	origin(linalg_origin(v)),
	size_(vect_size(v)) {}

    reference operator[](size_type i) const
    { return gmm::conj(linalg_traits<V>::access(origin, begin_, end_, i)); }
  };

  template <typename V> struct linalg_traits<conjugated_vector_const_ref<V> > {
    typedef conjugated_vector_const_ref<V> this_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef linalg_const is_reference;
    typedef abstract_vector linalg_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef value_type reference;
    typedef abstract_null_type iterator;
    typedef conjugated_const_iterator<typename
                   linalg_traits<V>::const_iterator> const_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    typedef typename linalg_traits<V>::index_sorted index_sorted;
    static size_type size(const this_type &v) { return v.size_; }
    static iterator begin(this_type &v) { return iterator(v.begin_); }
    static const_iterator begin(const this_type &v)
    { return const_iterator(v.begin_); }
    static iterator end(this_type &v)
    { return iterator(v.end_); }
    static const_iterator end(const this_type &v)
    { return const_iterator(v.end_); }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i)
    { return gmm::conj(linalg_traits<V>::access(o, it.it, ite.it, i)); }
    static const origin_type* origin(const this_type &v) { return v.origin; }
  };

  template<typename V> std::ostream &operator <<
    (std::ostream &o, const conjugated_vector_const_ref<V>& m)
  { gmm::write(o,m); return o; }

  /* ********************************************************************* */
  /*		Conjugated references on matrices            		   */
  /* ********************************************************************* */

  template <typename M> struct conjugated_row_const_iterator {
    typedef conjugated_row_const_iterator<M> iterator;
    typedef typename linalg_traits<M>::const_row_iterator ITER;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    ITER it;

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

    conjugated_row_const_iterator(void) {}
    conjugated_row_const_iterator(const ITER &i) : it(i) { }

  };

  template <typename M> struct  conjugated_row_matrix_const_ref {
    
    typedef conjugated_row_matrix_const_ref<M> this_type;
    typedef typename linalg_traits<M>::const_row_iterator iterator;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    size_type nr, nc;

    conjugated_row_matrix_const_ref(const M &m)
      : begin_(mat_row_begin(m)), end_(mat_row_end(m)),
	origin(linalg_origin(m)), nr(mat_ncols(m)), nc(mat_nrows(m)) {}

    value_type operator()(size_type i, size_type j) const
    { return gmm::conj(linalg_traits<M>::access(begin_+j, i)); }
  };

  template <typename M>
  struct linalg_traits<conjugated_row_matrix_const_ref<M> > {
    typedef conjugated_row_matrix_const_ref<M> this_type;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef value_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef typename linalg_traits<M>::const_sub_row_type vector_type;
    typedef conjugated_vector_const_ref<vector_type> sub_col_type;
    typedef conjugated_vector_const_ref<vector_type> const_sub_col_type;
    typedef conjugated_row_const_iterator<M> col_iterator;
    typedef conjugated_row_const_iterator<M> const_col_iterator;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_row_iterator;
    typedef abstract_null_type row_iterator;
    typedef col_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static inline size_type ncols(const this_type &m) { return m.nc; }
    static inline size_type nrows(const this_type &m) { return m.nr; }
    static inline const_sub_col_type col(const const_col_iterator &it)
    { return conjugated(linalg_traits<M>::row(it.it)); }
    static inline const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m.begin_); }
    static inline const_col_iterator col_end(const this_type &m)
    { return const_col_iterator(m.end_); }
    static inline const origin_type* origin(const this_type &m)
    { return m.origin; }
    static value_type access(const const_col_iterator &it, size_type i)
    { return gmm::conj(linalg_traits<M>::access(it.it, i)); }
  };

  template<typename M> std::ostream &operator <<
  (std::ostream &o, const conjugated_row_matrix_const_ref<M>& m)
  { gmm::write(o,m); return o; }


  template <typename M> struct conjugated_col_const_iterator {
    typedef conjugated_col_const_iterator<M> iterator;
    typedef typename linalg_traits<M>::const_col_iterator ITER;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    ITER it;

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

    conjugated_col_const_iterator(void) {}
    conjugated_col_const_iterator(const ITER &i) : it(i) { }

  };

  template <typename M> struct  conjugated_col_matrix_const_ref {
    
    typedef conjugated_col_matrix_const_ref<M> this_type;
    typedef typename linalg_traits<M>::const_col_iterator iterator;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename linalg_traits<this_type>::origin_type origin_type;

    iterator begin_, end_;
    const origin_type *origin;
    size_type nr, nc;

    conjugated_col_matrix_const_ref(const M &m)
      : begin_(mat_col_begin(m)), end_(mat_col_end(m)),
	origin(linalg_origin(m)), nr(mat_ncols(m)), nc(mat_nrows(m)) {}

    value_type operator()(size_type i, size_type j) const
    { return gmm::conj(linalg_traits<M>::access(begin_+i, j)); }
  };

  template <typename M>
  struct linalg_traits<conjugated_col_matrix_const_ref<M> > {
    typedef conjugated_col_matrix_const_ref<M> this_type;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef value_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef typename linalg_traits<M>::const_sub_col_type vector_type;
    typedef conjugated_vector_const_ref<vector_type> sub_row_type;
    typedef conjugated_vector_const_ref<vector_type> const_sub_row_type;
    typedef conjugated_col_const_iterator<M> row_iterator;
    typedef conjugated_col_const_iterator<M> const_row_iterator;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_col_iterator;
    typedef abstract_null_type col_iterator;
    typedef row_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static inline size_type nrows(const this_type &m) { return m.nr; }
    static inline size_type ncols(const this_type &m) { return m.nc; }
    static inline const_sub_row_type row(const const_row_iterator &it)
    { return conjugated(linalg_traits<M>::col(it.it)); }
    static inline const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m.begin_); }
    static inline const_row_iterator row_end(const this_type &m)
    { return const_row_iterator(m.end_); }
    static inline const origin_type* origin(const this_type &m)
    { return m.origin; }
    static value_type access(const const_row_iterator &it, size_type i)
    { return gmm::conj(linalg_traits<M>::access(it.it, i)); }
  };

  template<typename M> std::ostream &operator <<
  (std::ostream &o, const conjugated_col_matrix_const_ref<M>& m)
  { gmm::write(o,m); return o; }


  template <typename L, typename SO> struct conjugated_return__ {
    typedef conjugated_row_matrix_const_ref<L> return_type;
  };
  template <typename L> struct conjugated_return__<L, col_major> {
    typedef conjugated_col_matrix_const_ref<L> return_type;
  };
  template <typename L, typename T, typename LT> struct conjugated_return_ {
    typedef const L & return_type;
  };
  template <typename L, typename T>
  struct conjugated_return_<L, std::complex<T>, abstract_vector> {
    typedef conjugated_vector_const_ref<L> return_type;
  };
  template <typename L, typename T>
  struct conjugated_return_<L, T, abstract_matrix> {
    typedef typename conjugated_return__<L,
    typename principal_orientation_type<typename
    linalg_traits<L>::sub_orientation>::potype
    >::return_type return_type;
  };
  template <typename L> struct conjugated_return {
    typedef typename
    conjugated_return_<L, typename linalg_traits<L>::value_type,
		       typename linalg_traits<L>::linalg_type		       
		       >::return_type return_type;
  };

  ///@endcond
  /** return a conjugated view of the input matrix or vector. */
  template <typename L> inline
  typename conjugated_return<L>::return_type
  conjugated(const L &v) {
    return conjugated(v, typename linalg_traits<L>::value_type(),
		      typename linalg_traits<L>::linalg_type());
  }
  ///@cond DOXY_SHOW_ALL_FUNCTIONS

  template <typename L, typename T, typename LT> inline
  const L & conjugated(const L &v, T, LT) { return v; }

  template <typename L, typename T> inline
  conjugated_vector_const_ref<L> conjugated(const L &v, std::complex<T>,
					    abstract_vector)
  { return conjugated_vector_const_ref<L>(v); }

  template <typename L, typename T> inline
  typename conjugated_return__<L,
    typename principal_orientation_type<typename
    linalg_traits<L>::sub_orientation>::potype>::return_type
  conjugated(const L &v, T, abstract_matrix) {
    return conjugated(v, typename principal_orientation_type<typename
		      linalg_traits<L>::sub_orientation>::potype());
  }

  template <typename L> inline
  conjugated_row_matrix_const_ref<L> conjugated(const L &v, row_major)
  { return conjugated_row_matrix_const_ref<L>(v); }

  template <typename L> inline
  conjugated_col_matrix_const_ref<L> conjugated(const L &v, col_major)
  { return conjugated_col_matrix_const_ref<L>(v); }
  
  ///@endcond
  

}

#endif //  GMM_CONJUGATED_H__
