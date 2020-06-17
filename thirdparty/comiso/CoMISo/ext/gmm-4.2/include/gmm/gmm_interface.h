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


/**@file gmm_interface.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief gmm interface for STL vectors.
*/

#ifndef GMM_INTERFACE_H__
#define GMM_INTERFACE_H__

#include "gmm_blas.h"
#include "gmm_sub_index.h"

namespace gmm {

  /* ********************************************************************* */
  /*                                                                       */
  /* What is needed for a Vector type :                                    */
  /*   Vector v(n) defines a vector with n components.                     */
  /*   v[i] allows to access to the ith component of v.                    */
  /*   linalg_traits<Vector> should be filled with appropriate definitions */
  /*                                                                       */
  /*   for a dense vector : the minimum is two random iterators (begin and */
  /*                        end) and a pointer to a valid origin.          */
  /*   for a sparse vector : the minimum is two forward iterators, with    */
  /*                         a method it.index() which gives the index of  */
  /*                         a non zero element, an interface object       */
  /*                         should describe the method to add new non     */
  /*                         zero element, and  a pointer to a valid       */
  /*                         origin.                                       */
  /*                                                                       */
  /* What is needed for a Matrix type :                                    */
  /*   Matrix m(n, m) defines a matrix with n rows and m columns.          */
  /*   m(i, j) allows to access to the element at row i and column j.      */
  /*   linalg_traits<Matrix> should be filled with appropriate definitions */
  /*                                                                       */
  /* What is needed for an iterator on dense vector                        */
  /*    to be standard random access iterator                              */
  /*                                                                       */
  /* What is needed for an iterator on a sparse vector                     */
  /*    to be a standard bidirectional iterator                            */
  /*    elt should be sorted with increasing indices.                      */
  /*    it.index() gives the index of the non-zero element.                */
  /*                                                                       */
  /* Remark : If original iterators are not convenient, they could be      */
  /*   redefined and interfaced in linalg_traits<Vector> without changing  */
  /*   the original Vector type.                                           */
  /*                                                                       */
  /* ********************************************************************* */

  /* ********************************************************************* */
  /*		Simple references on vectors            		   */
  /* ********************************************************************* */

  template <typename PT> struct simple_vector_ref {
    typedef simple_vector_ref<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_V;
    typedef typename linalg_traits<this_type>::iterator iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    size_type size_;

    simple_vector_ref(ref_V v) : begin_(vect_begin(const_cast<V&>(v))), 
				 end_(vect_end(const_cast<V&>(v))), 
				 origin(linalg_origin(const_cast<V&>(v))),
				 size_(vect_size(v)) {}

    simple_vector_ref(const simple_vector_ref<CPT> &cr)
      : begin_(cr.begin_),end_(cr.end_),origin(cr.origin),size_(cr.size_) {}

    simple_vector_ref(void) {}

    reference operator[](size_type i) const
    { return linalg_traits<V>::access(origin, begin_, end_, i); }
  };

  template <typename IT, typename ORG, typename PT> inline
  void set_to_begin(IT &it, ORG o, simple_vector_ref<PT> *,linalg_modifiable) {
    typedef typename linalg_traits<simple_vector_ref<PT> >::V_reference ref_t;
    set_to_begin(it, o, PT(), ref_t());
  }

  template <typename IT, typename ORG, typename PT> inline
  void set_to_begin(IT &it, ORG o, const simple_vector_ref<PT> *,
		    linalg_modifiable) {
    typedef typename linalg_traits<simple_vector_ref<PT> >::V_reference ref_t;
    set_to_begin(it, o, PT(), ref_t());
  }

  template <typename IT, typename ORG, typename PT> inline
  void set_to_end(IT &it, ORG o, simple_vector_ref<PT> *, linalg_modifiable) {
    typedef typename linalg_traits<simple_vector_ref<PT> >::V_reference ref_t;
    set_to_end(it, o, PT(), ref_t());
  }

  template <typename IT, typename ORG, typename PT> inline
  void set_to_end(IT &it, ORG o, const simple_vector_ref<PT> *,
		  linalg_modifiable) {
    typedef typename linalg_traits<simple_vector_ref<PT> >::V_reference ref_t;
    set_to_end(it, o, PT(), ref_t());
  }


  template <typename PT> struct linalg_traits<simple_vector_ref<PT> > {
    typedef simple_vector_ref<PT> this_type;
    typedef this_type *pthis_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef V *pV;
    typedef typename linalg_traits<V>::is_reference V_reference;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename select_ref<value_type, typename
            linalg_traits<V>::reference, PT>::ref_type reference;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
	    typename linalg_traits<V>::iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<V>::const_iterator const_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size_; }
    static inline iterator begin(this_type &v) {
      iterator it = v.begin_;
      set_to_begin(it, v.origin, pthis_type(), is_reference()); 
      return it;
    }
    static inline const_iterator begin(const this_type &v) {
      const_iterator it = v.begin_;
      set_to_begin(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static inline iterator end(this_type &v) {
      iterator it = v.end_;
      set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static inline const_iterator end(const this_type &v) {
      const_iterator it = v.end_;
      set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type* o, const iterator &it, const iterator &ite)
    { linalg_traits<V>::clear(o, it, ite); }
    static void do_clear(this_type &v) { clear(v.origin, v.begin_, v.end_); }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it, ite, i); }
    static reference access(origin_type *o, const iterator &it,
			    const iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it, ite, i); }
  };

  template <typename PT>
  std::ostream &operator << (std::ostream &o, const simple_vector_ref<PT>& v)
  { gmm::write(o,v); return o; }

  /* ********************************************************************* */
  /*		                                         		   */
  /*		Traits for S.T.L. object                     		   */
  /*		                                         		   */
  /* ********************************************************************* */

  template <typename T, typename alloc>
  struct linalg_traits<std::vector<T, alloc> > {
    typedef std::vector<T, alloc> this_type;
    typedef this_type origin_type;
    typedef linalg_false is_reference;
    typedef abstract_vector linalg_type;
    typedef T value_type;
    typedef T& reference;
    typedef typename this_type::iterator iterator;
    typedef typename this_type::const_iterator const_iterator;
    typedef abstract_dense storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static origin_type* origin(this_type &v) { return &v; }
    static const origin_type* origin(const this_type &v) { return &v; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static void do_clear(this_type &v) { std::fill(v.begin(), v.end(), T(0)); }
    static value_type access(const origin_type *, const const_iterator &it,
			     const const_iterator &, size_type i)
    { return it[i]; }
    static reference access(origin_type *, const iterator &it,
			    const iterator &, size_type i)
    { return it[i]; }
    static void resize(this_type &v, size_type n) { v.resize(n); }
  };
}
namespace std {
  template <typename T> ostream &operator <<
  (std::ostream &o, const vector<T>& m) { gmm::write(o,m); return o; }
}
namespace gmm {

  template <typename T>
  inline size_type nnz(const std::vector<T>& l) { return l.size(); }

  /* ********************************************************************* */
  /*		                                         		   */
  /*		Traits for ref objects                     		   */
  /*		                                         		   */
  /* ********************************************************************* */

  template <typename IT, typename V>
  struct tab_ref_with_origin : public gmm::tab_ref<IT> {
    typedef tab_ref_with_origin<IT, V> this_type;
    // next line replaced by the 4 following lines in order to please aCC
    //typedef typename linalg_traits<this_type>::porigin_type porigin_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename std::iterator_traits<IT>::pointer PT;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
   

    porigin_type origin;
   
    tab_ref_with_origin(void) {}
    template <class PT> tab_ref_with_origin(const IT &b, const IT &e, PT p)
      : gmm::tab_ref<IT>(b,e), origin(porigin_type(p)) {}
    tab_ref_with_origin(const IT &b, const IT &e, porigin_type p)
      : gmm::tab_ref<IT>(b,e), origin(p) {}
   
    tab_ref_with_origin(const V &v, const sub_interval &si)
      : gmm::tab_ref<IT>(vect_begin(const_cast<V&>(v))+si.min,
			 vect_begin(const_cast<V&>(v))+si.max),
        origin(linalg_origin(const_cast<V&>(v))) {}
    tab_ref_with_origin(V &v, const sub_interval &si)
      : gmm::tab_ref<IT>(vect_begin(const_cast<V&>(v))+si.min,
			 vect_begin(const_cast<V&>(v))+si.max),
        origin(linalg_origin(const_cast<V&>(v))) {}
  };

  template <typename IT, typename V>
  struct linalg_traits<tab_ref_with_origin<IT, V> > {
    typedef typename std::iterator_traits<IT>::pointer PT;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef tab_ref_with_origin<IT, V> this_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename std::iterator_traits<IT>::value_type value_type;
    typedef typename std::iterator_traits<IT>::reference reference;
    typedef typename this_type::iterator iterator;
    typedef typename this_type::iterator const_iterator;
    typedef abstract_dense storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static inline void do_clear(this_type &v)
    { std::fill(v.begin(), v.end(), value_type(0)); }
    static value_type access(const origin_type *, const const_iterator &it,
			     const const_iterator &, size_type i)
    { return it[i]; }
    static reference access(origin_type *, const iterator &it, 
			    const iterator &, size_type i)
    { return it[i]; }
  };

  template <typename IT, typename V> std::ostream &operator <<
  (std::ostream &o, const tab_ref_with_origin<IT, V>& m)
  { gmm::write(o,m); return o; }


  template <typename IT, typename V>
  struct tab_ref_reg_spaced_with_origin : public gmm::tab_ref_reg_spaced<IT> {
    typedef  tab_ref_reg_spaced_with_origin<IT, V> this_type;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    porigin_type origin;
    
    tab_ref_reg_spaced_with_origin(void) {}
    tab_ref_reg_spaced_with_origin(const IT &b, size_type n, size_type s,
				   const porigin_type p)
      : gmm::tab_ref_reg_spaced<IT>(b,n,s), origin(p) {}
    tab_ref_reg_spaced_with_origin(const V &v, const sub_slice &si)
      : gmm::tab_ref_reg_spaced<IT>(vect_begin(const_cast<V&>(v)) + si.min, 
				    si.N, (si.max - si.min)/si.N),
      origin(linalg_origin(const_cast<V&>(v))) {}
    tab_ref_reg_spaced_with_origin(V &v, const sub_slice &si)
      : gmm::tab_ref_reg_spaced<IT>(vect_begin(const_cast<V&>(v)) + si.min,
				    si.N, (si.max - si.min)/si.N),
	origin(linalg_origin(const_cast<V&>(v))) {}
  };

  template <typename IT, typename V> 
  struct linalg_traits<tab_ref_reg_spaced_with_origin<IT, V> > {
    typedef typename std::iterator_traits<IT>::pointer PT;
    typedef tab_ref_reg_spaced_with_origin<IT, V> this_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename std::iterator_traits<IT>::value_type value_type;
    typedef typename std::iterator_traits<IT>::reference reference;
    typedef typename this_type::iterator iterator;
    typedef typename this_type::iterator const_iterator;
    typedef abstract_dense storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static void do_clear(this_type &v)
    { std::fill(v.begin(), v.end(), value_type(0)); }
    static value_type access(const origin_type *, const const_iterator &it,
			     const const_iterator &, size_type i)
    { return it[i]; }
    static reference access(origin_type *, const iterator &it, 
			    const iterator &, size_type i)
    { return it[i]; }
  };
  
  template <typename IT, typename V> std::ostream &operator <<
  (std::ostream &o, const tab_ref_reg_spaced_with_origin<IT, V>& m)
  { gmm::write(o,m); return o; }


  template <typename IT, typename ITINDEX, typename V>
  struct tab_ref_index_ref_with_origin 
    : public gmm::tab_ref_index_ref<IT, ITINDEX> {
    typedef tab_ref_index_ref_with_origin<IT, ITINDEX, V> this_type;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    porigin_type origin;

    tab_ref_index_ref_with_origin(void) {}
    tab_ref_index_ref_with_origin(const IT &b, const ITINDEX &bi,
				  const ITINDEX &ei, porigin_type p)
      : gmm::tab_ref_index_ref<IT, ITINDEX>(b, bi, ei), origin(p) {}

    tab_ref_index_ref_with_origin(const V &v, const sub_index &si)
      : gmm::tab_ref_index_ref<IT, ITINDEX>(vect_begin(const_cast<V&>(v)),
					    si.begin(), si.end()),
      origin(linalg_origin(const_cast<V&>(v))) {}
    tab_ref_index_ref_with_origin(V &v, const sub_index &si)
      : gmm::tab_ref_index_ref<IT, ITINDEX>(vect_begin(const_cast<V&>(v)),
					    si.begin(), si.end()),
	origin(linalg_origin(const_cast<V&>(v))) {}
  };

  template <typename IT, typename ITINDEX, typename V>
  struct linalg_traits<tab_ref_index_ref_with_origin<IT, ITINDEX, V> > {
    typedef typename std::iterator_traits<IT>::pointer PT;
    typedef tab_ref_index_ref_with_origin<IT, ITINDEX, V> this_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename std::iterator_traits<IT>::value_type value_type;
    typedef typename std::iterator_traits<IT>::reference reference;
    typedef typename this_type::iterator iterator;
    typedef typename this_type::iterator const_iterator;
    typedef abstract_dense storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static void do_clear(this_type &v)
    { std::fill(v.begin(), v.end(), value_type(0)); }
    static value_type access(const origin_type *, const const_iterator &it,
			     const const_iterator &, size_type i)
    { return it[i]; }
    static reference access(origin_type *, const iterator &it,
			    const iterator &, size_type i)
    { return it[i]; }
  };

  template <typename IT, typename ITINDEX, typename V>
  std::ostream &operator <<
  (std::ostream &o, const tab_ref_index_ref_with_origin<IT, ITINDEX, V>& m)
  { gmm::write(o,m); return o; }


  template<typename ITER, typename MIT, typename PT> 
  struct dense_compressed_iterator {
    typedef ITER value_type;
    typedef ITER *pointer;
    typedef ITER &reference;
    typedef ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef size_t size_type;
    typedef dense_compressed_iterator<ITER, MIT, PT> iterator;
    typedef typename std::iterator_traits<PT>::value_type *MPT;

    ITER it;
    size_type N, nrows, ncols, i;
    PT origin;
    
    iterator operator ++(int) { iterator tmp = *this; i++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; i--; return tmp; }
    iterator &operator ++()   { ++i; return *this; }
    iterator &operator --()   { --i; return *this; }
    iterator &operator +=(difference_type ii) { i += ii; return *this; }
    iterator &operator -=(difference_type ii) { i -= ii; return *this; }
    iterator operator +(difference_type ii) const 
    { iterator itt = *this; return (itt += ii); }
    iterator operator -(difference_type ii) const
    { iterator itt = *this; return (itt -= ii); }
    difference_type operator -(const iterator &ii) const
    { return (N ? (it - ii.it) / N : 0) + i - ii.i; }

    ITER operator *() const { return it+i*N; }
    ITER operator [](int ii) const { return it + (i+ii) * N; }

    bool operator ==(const iterator &ii) const
    { return (*this - ii) == difference_type(0); }
    bool operator !=(const iterator &ii) const { return !(ii == *this); }
    bool operator < (const iterator &ii) const
    { return (*this - ii) < difference_type(0); }

    dense_compressed_iterator(void) {}
    dense_compressed_iterator(const dense_compressed_iterator<MIT,MIT,MPT> &ii)
      : it(ii.it), N(ii.N), nrows(ii.nrows), ncols(ii.ncols), i(ii.i),
	origin(ii.origin)  {}
    dense_compressed_iterator(const ITER &iter, size_type n, size_type r,
			      size_type c, size_type ii, PT o)
      : it(iter), N(n), nrows(r), ncols(c), i(ii), origin(o) { }
    
  };

  /* ******************************************************************** */
  /*	    Read only reference on a compressed sparse vector             */
  /* ******************************************************************** */

  template <typename PT1, typename PT2, int shift = 0>
  struct cs_vector_ref_iterator {
    PT1 pr;
    PT2 ir;

    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef PT1 pointer;
    typedef typename std::iterator_traits<PT1>::reference  reference;
    typedef size_t        size_type;
    typedef ptrdiff_t     difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef cs_vector_ref_iterator<PT1, PT2, shift> iterator;
    
    cs_vector_ref_iterator(void) {}
    cs_vector_ref_iterator(PT1 p1, PT2 p2) : pr(p1), ir(p2) {}

    inline size_type index(void) const { return (*ir) - shift; }
    iterator &operator ++() { ++pr; ++ir; return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator &operator --() { --pr; --ir; return *this; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }
    
    reference operator  *() const { return *pr; }
    pointer   operator ->() const { return pr; }
    
    bool operator ==(const iterator &i) const { return (i.pr==pr);}
    bool operator !=(const iterator &i) const { return (i.pr!=pr);}
  };
    
  template <typename PT1, typename PT2, int shift = 0> struct cs_vector_ref {
    PT1 pr;
    PT2 ir;
    size_type n, size_;

    typedef cs_vector_ref<PT1, PT2, shift> this_type;
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef typename linalg_traits<this_type>::const_iterator const_iterator;

    cs_vector_ref(PT1 pt1, PT2 pt2, size_type nnz, size_type ns)
      : pr(pt1), ir(pt2), n(nnz), size_(ns) {}
    cs_vector_ref(void) {}

    size_type size(void) const { return size_; }
    
    const_iterator begin(void) const { return const_iterator(pr, ir); }
    const_iterator end(void) const { return const_iterator(pr+n, ir+n); }
    
    value_type operator[](size_type i) const
    { return linalg_traits<this_type>::access(pr, begin(), end(),i); }
  };

  template <typename PT1, typename PT2, int shift>
  struct linalg_traits<cs_vector_ref<PT1, PT2, shift> > {
    typedef cs_vector_ref<PT1, PT2, shift> this_type;
    typedef linalg_const is_reference;
    typedef abstract_vector linalg_type;
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef value_type origin_type;
    typedef typename std::iterator_traits<PT1>::value_type reference;
    typedef cs_vector_ref_iterator<typename const_pointer<PT1>::pointer,
	    typename const_pointer<PT2>::pointer, shift>  const_iterator;
    typedef abstract_null_type iterator;
    typedef abstract_sparse storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static const origin_type* origin(const this_type &v) { return v.pr; }
    static value_type access(const origin_type *, const const_iterator &b,
			     const const_iterator &e, size_type i) {
      if (b.ir == e.ir) return value_type(0);
      PT2 p = std::lower_bound(b.ir, e.ir, i+shift);
      return (*p == i+shift && p != e.ir) ? b.pr[p-b.ir] : value_type(0);
    }
  };

  template <typename PT1, typename PT2, int shift>
  std::ostream &operator <<
  (std::ostream &o, const cs_vector_ref<PT1, PT2, shift>& m)
  { gmm::write(o,m); return o; }

  template <typename PT1, typename PT2, int shift>
  inline size_type nnz(const cs_vector_ref<PT1, PT2, shift>& l) { return l.n; }

  /* ******************************************************************** */
  /*	    Read only reference on a compressed sparse column matrix      */
  /* ******************************************************************** */

  template <typename PT1, typename PT2, typename PT3, int shift = 0>
  struct sparse_compressed_iterator {
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef sparse_compressed_iterator<PT1, PT2, PT3, shift> iterator;

    PT1 pr;
    PT2 ir;
    PT3 jc;
    size_type n;
    const value_type *origin;
    
    iterator operator ++(int) { iterator tmp = *this; jc++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; jc--; return tmp; }
    iterator &operator ++()   { jc++; return *this; }
    iterator &operator --()   { jc--; return *this; }
    iterator &operator +=(difference_type i) { jc += i; return *this; }
    iterator &operator -=(difference_type i) { jc -= i; return *this; }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const { return jc - i.jc; }

    reference operator *() const { return pr + *jc - shift; }
    reference operator [](int ii) { return pr + *(jc+ii) - shift; }

    bool operator ==(const iterator &i) const { return (jc == i.jc); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (jc < i.jc); }

    sparse_compressed_iterator(void) {}
    sparse_compressed_iterator(PT1 p1, PT2 p2, PT3 p3, size_type nn,
			       const value_type *o)
      : pr(p1), ir(p2), jc(p3), n(nn), origin(o) { }
    
  };

  template <typename PT1, typename PT2, typename PT3, int shift = 0>
  struct csc_matrix_ref {
    PT1 pr; // values.
    PT2 ir; // row indexes.
    PT3 jc; // column repartition on pr and ir.
    size_type nc, nr;
    
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    csc_matrix_ref(PT1 pt1, PT2 pt2, PT3 pt3, size_type nrr, size_type ncc)
      : pr(pt1), ir(pt2), jc(pt3), nc(ncc), nr(nrr) {}
    csc_matrix_ref(void) {}
    
    size_type nrows(void) const { return nr; }
    size_type ncols(void) const { return nc; }
   
    value_type operator()(size_type i, size_type j) const
      { return mat_col(*this, j)[i]; }
  };

  template <typename PT1, typename PT2, typename PT3, int shift>
  struct linalg_traits<csc_matrix_ref<PT1, PT2, PT3, shift> > {
    typedef csc_matrix_ref<PT1, PT2, PT3, shift> this_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef typename std::iterator_traits<PT1>::value_type reference;
    typedef value_type origin_type;
    typedef abstract_sparse storage_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_row_iterator;
    typedef abstract_null_type sub_col_type;
    typedef cs_vector_ref<typename const_pointer<PT1>::pointer,
            typename const_pointer<PT2>::pointer, shift> const_sub_col_type;
    typedef sparse_compressed_iterator<typename const_pointer<PT1>::pointer,
				       typename const_pointer<PT2>::pointer,
				       typename const_pointer<PT3>::pointer,
				       shift>  const_col_iterator;
    typedef abstract_null_type col_iterator;
    typedef col_major sub_orientation;
    typedef linalg_true index_sorted;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m.pr, m.ir, m.jc, m.nr, m.pr); }
    static const_col_iterator col_end(const this_type &m)
    { return const_col_iterator(m.pr, m.ir, m.jc + m.nc, m.nr, m.pr); }
    static const_sub_col_type col(const const_col_iterator &it) {
      return const_sub_col_type(it.pr + *(it.jc) - shift,
	     it.ir + *(it.jc) - shift, *(it.jc + 1) - *(it.jc), it.n);
    }
    static const origin_type* origin(const this_type &m) { return m.pr; }
    static value_type access(const const_col_iterator &itcol, size_type j)
    { return col(itcol)[j]; }
  };


  template <typename PT1, typename PT2, typename PT3, int shift>
  std::ostream &operator <<
  (std::ostream &o, const csc_matrix_ref<PT1, PT2, PT3, shift>& m)
  { gmm::write(o,m); return o; }

  /* ******************************************************************** */
  /*	   Read only reference on a compressed sparse row matrix          */
  /* ******************************************************************** */

  template <typename PT1, typename PT2, typename PT3, int shift = 0>
  struct csr_matrix_ref {
    PT1 pr; // values.
    PT2 ir; // column indexes.
    PT3 jc; // row repartition on pr and ir.
    size_type nc, nr;
    
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    csr_matrix_ref(PT1 pt1, PT2 pt2, PT3 pt3, size_type nrr, size_type ncc)
      : pr(pt1), ir(pt2), jc(pt3), nc(ncc), nr(nrr) {}
    csr_matrix_ref(void) {}
    
    size_type nrows(void) const { return nr; }
    size_type ncols(void) const { return nc; }
   
    value_type operator()(size_type i, size_type j) const
      { return mat_row(*this, i)[j]; }
  };
  
  template <typename PT1, typename PT2, typename PT3, int shift>
  struct linalg_traits<csr_matrix_ref<PT1, PT2, PT3, shift> > {
    typedef csr_matrix_ref<PT1, PT2, PT3, shift> this_type;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename std::iterator_traits<PT1>::value_type value_type;
    typedef typename std::iterator_traits<PT1>::value_type reference;
    typedef value_type origin_type;
    typedef abstract_sparse storage_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type const_col_iterator;
    typedef abstract_null_type sub_row_type;
    typedef cs_vector_ref<typename const_pointer<PT1>::pointer,
			  typename const_pointer<PT2>::pointer, shift>
            const_sub_row_type;
    typedef sparse_compressed_iterator<typename const_pointer<PT1>::pointer,
				       typename const_pointer<PT2>::pointer,
				       typename const_pointer<PT3>::pointer,
				       shift>  const_row_iterator;
    typedef abstract_null_type row_iterator;
    typedef row_major sub_orientation;
    typedef linalg_true index_sorted;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m.pr, m.ir, m.jc, m.nc, m.pr); }
    static const_row_iterator row_end(const this_type &m)
    { return const_row_iterator(m.pr, m.ir, m.jc + m.nr, m.nc, m.pr); }
    static const_sub_row_type row(const const_row_iterator &it) {
      return const_sub_row_type(it.pr + *(it.jc) - shift,
	     it.ir + *(it.jc) - shift, *(it.jc + 1) - *(it.jc), it.n);
    }
    static const origin_type* origin(const this_type &m) { return m.pr; }
    static value_type access(const const_row_iterator &itrow, size_type j)
    { return row(itrow)[j]; }
  };

  template <typename PT1, typename PT2, typename PT3, int shift>
  std::ostream &operator <<
  (std::ostream &o, const csr_matrix_ref<PT1, PT2, PT3, shift>& m)
  { gmm::write(o,m); return o; }

  /* ********************************************************************* */
  /*		                                         		   */
  /*		Simple interface for C arrays                     	   */
  /*		                                         		   */
  /* ********************************************************************* */

  template <class PT> struct array1D_reference {

    typedef typename std::iterator_traits<PT>::value_type value_type;

    PT begin, end;
    
    const value_type &operator[](size_type i) const { return *(begin+i); }
    value_type &operator[](size_type i) { return *(begin+i); }

    array1D_reference(PT begin_, size_type s) : begin(begin_), end(begin_+s) {}
  };

  template <typename PT>
  struct linalg_traits<array1D_reference<PT> > {
    typedef array1D_reference<PT> this_type;
    typedef this_type origin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename std::iterator_traits<PT>::value_type value_type;
    typedef typename std::iterator_traits<PT>::reference reference;
    typedef PT iterator;
    typedef PT const_iterator;
    typedef abstract_dense storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.end - v.begin; }
    static iterator begin(this_type &v) { return v.begin; }
    static const_iterator begin(const this_type &v) { return v.begin; }
    static iterator end(this_type &v) { return v.end; }
    static const_iterator end(const this_type &v) { return v.end; }
    static origin_type* origin(this_type &v) { return &v; }
    static const origin_type* origin(const this_type &v) { return &v; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static void do_clear(this_type &v)
    { std::fill(v.begin, v.end, value_type(0)); }
    static value_type access(const origin_type *, const const_iterator &it,
			     const const_iterator &, size_type i)
    { return it[i]; }
    static reference access(origin_type *, const iterator &it,
			    const iterator &, size_type i)
    { return it[i]; }
    static void resize(this_type &, size_type )
    { GMM_ASSERT1(false, "Not resizable vector"); }
  };

  template<typename PT> std::ostream &operator <<
  (std::ostream &o, const array1D_reference<PT>& v)
  { gmm::write(o,v); return o; }
  
  template <class PT> struct array2D_col_reference {

    typedef typename std::iterator_traits<PT>::value_type T;
    typedef typename std::iterator_traits<PT>::reference reference;
    typedef typename const_reference<reference>::reference const_reference;
    typedef PT iterator;
    typedef typename const_pointer<PT>::pointer const_iterator;
    
    PT begin_;
    size_type nbl, nbc;

    inline const_reference operator ()(size_type l, size_type c) const {
      GMM_ASSERT2(l < nbl && c < nbc, "out of range");
      return *(begin_ + c*nbl+l);
    }
    inline reference operator ()(size_type l, size_type c) {
      GMM_ASSERT2(l < nbl && c < nbc, "out of range");
      return *(begin_ + c*nbl+l);
    }
    
    void resize(size_type, size_type);
    void reshape(size_type m, size_type n) {
      GMM_ASSERT2(n*m == nbl*nbc, "dimensions mismatch");
      nbl = m; nbc = n;
    }
    
    void fill(T a, T b = T(0)) { 
      std::fill(begin_, begin_+nbc*nbl, b);
      iterator p = begin_, e = begin_+nbc*nbl;
      while (p < e) { *p = a; p += nbl+1; }
    }
    inline size_type nrows(void) const { return nbl; }
    inline size_type ncols(void) const { return nbc; }

    iterator begin(void) { return begin_; }
    const_iterator begin(void) const { return begin_; }
    iterator end(void) { return begin_+nbl*nbc; }
    const_iterator end(void) const { return begin_+nbl*nbc; }

    array2D_col_reference(PT begin__, size_type nrows_, size_type ncols_)
      : begin_(begin__), nbl(nrows_), nbc(ncols_) {}
  };

  template <typename PT> struct linalg_traits<array2D_col_reference<PT> > {
    typedef array2D_col_reference<PT> this_type;
    typedef this_type origin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename std::iterator_traits<PT>::value_type value_type;
    typedef typename std::iterator_traits<PT>::reference reference;
    typedef abstract_dense storage_type;
    typedef tab_ref_reg_spaced_with_origin<typename this_type::iterator,
					   this_type> sub_row_type;
    typedef tab_ref_reg_spaced_with_origin<typename this_type::const_iterator,
					   this_type> const_sub_row_type;
    typedef dense_compressed_iterator<typename this_type::iterator,
				      typename this_type::iterator,
				      this_type *> row_iterator;
    typedef dense_compressed_iterator<typename this_type::const_iterator,
				      typename this_type::iterator,
				      const this_type *> const_row_iterator;
    typedef tab_ref_with_origin<typename this_type::iterator, 
				this_type> sub_col_type;
    typedef tab_ref_with_origin<typename this_type::const_iterator,
				this_type> const_sub_col_type;
    typedef dense_compressed_iterator<typename this_type::iterator,
				      typename this_type::iterator,
				      this_type *> col_iterator;
    typedef dense_compressed_iterator<typename this_type::const_iterator,
				      typename this_type::iterator,
				      const this_type *> const_col_iterator;
    typedef col_and_row sub_orientation;
    typedef linalg_true index_sorted;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_sub_row_type row(const const_row_iterator &it)
    { return const_sub_row_type(*it, it.nrows, it.ncols, it.origin); }
    static const_sub_col_type col(const const_col_iterator &it)
    { return const_sub_col_type(*it, *it + it.nrows, it.origin); }
    static sub_row_type row(const row_iterator &it)
    { return sub_row_type(*it, it.nrows, it.ncols, it.origin); }
    static sub_col_type col(const col_iterator &it)
    { return sub_col_type(*it, *it + it.nrows, it.origin); }
    static row_iterator row_begin(this_type &m)
    { return row_iterator(m.begin(), 1, m.nrows(), m.ncols(), 0, &m); }
    static row_iterator row_end(this_type &m)
    { return row_iterator(m.begin(), 1, m.nrows(), m.ncols(), m.nrows(), &m); }
    static const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m.begin(), 1, m.nrows(), m.ncols(), 0, &m); }
    static const_row_iterator row_end(const this_type &m) {
      return const_row_iterator(m.begin(), 1, m.nrows(),
				m.ncols(), m.nrows(), &m);
    }
    static col_iterator col_begin(this_type &m)
    { return col_iterator(m.begin(), m.nrows(), m.nrows(), m.ncols(), 0, &m); }
    static col_iterator col_end(this_type &m) {
      return col_iterator(m.begin(), m.nrows(), m.nrows(), m.ncols(),
			  m.ncols(), &m);
    }
    static const_col_iterator col_begin(const this_type &m) {
      return const_col_iterator(m.begin(), m.nrows(), m.nrows(),
				m.ncols(), 0, &m);
    }
    static const_col_iterator col_end(const this_type &m) {
      return const_col_iterator(m.begin(), m.nrows(),m.nrows(),m.ncols(),
				m.ncols(), &m);
    }
    static origin_type* origin(this_type &m) { return &m; }
    static const origin_type* origin(const this_type &m) { return &m; }
    static void do_clear(this_type &m) { m.fill(value_type(0)); }
    static value_type access(const const_col_iterator &itcol, size_type j)
    { return (*itcol)[j]; }
    static reference access(const col_iterator &itcol, size_type j)
    { return (*itcol)[j]; }
    static void resize(this_type &v, size_type m, size_type n)
    { v.resize(m,n); }
    static void reshape(this_type &v, size_type m, size_type n)
    { v.reshape(m, n); }
  };

  template<typename PT> std::ostream &operator <<
    (std::ostream &o, const array2D_col_reference<PT>& m)
  { gmm::write(o,m); return o; }



  template <class PT> struct array2D_row_reference {
    
    typedef typename std::iterator_traits<PT>::value_type T;
    typedef typename std::iterator_traits<PT>::reference reference;
    typedef typename const_reference<reference>::reference const_reference;
    typedef PT iterator;
    typedef typename const_pointer<PT>::pointer const_iterator;
    
    PT begin_;
    size_type nbl, nbc;

    inline const_reference operator ()(size_type l, size_type c) const {
      GMM_ASSERT2(l < nbl && c < nbc, "out of range");
      return *(begin_ + l*nbc+c);
    }
    inline reference operator ()(size_type l, size_type c) {
      GMM_ASSERT2(l < nbl && c < nbc, "out of range");
      return *(begin_ + l*nbc+c);
    }
    
    void resize(size_type, size_type);
    void reshape(size_type m, size_type n) {
      GMM_ASSERT2(n*m == nbl*nbc, "dimensions mismatch");
      nbl = m; nbc = n;
    }
    
    void fill(T a, T b = T(0)) { 
      std::fill(begin_, begin_+nbc*nbl, b);
      iterator p = begin_, e = begin_+nbc*nbl;
      while (p < e) { *p = a; p += nbc+1; }
    }
    inline size_type nrows(void) const { return nbl; }
    inline size_type ncols(void) const { return nbc; }

    iterator begin(void) { return begin_; }
    const_iterator begin(void) const { return begin_; }
    iterator end(void) { return begin_+nbl*nbc; }
    const_iterator end(void) const { return begin_+nbl*nbc; }

    array2D_row_reference(PT begin__, size_type nrows_, size_type ncols_)
      : begin_(begin__), nbl(nrows_), nbc(ncols_) {}
  };

  template <typename PT> struct linalg_traits<array2D_row_reference<PT> > {
    typedef array2D_row_reference<PT> this_type;
    typedef this_type origin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename std::iterator_traits<PT>::value_type value_type;
    typedef typename std::iterator_traits<PT>::reference reference;
    typedef abstract_dense storage_type;
    typedef tab_ref_reg_spaced_with_origin<typename this_type::iterator,
					   this_type> sub_col_type;
    typedef tab_ref_reg_spaced_with_origin<typename this_type::const_iterator,
					   this_type> const_sub_col_type;
    typedef dense_compressed_iterator<typename this_type::iterator,
				      typename this_type::iterator,
				      this_type *> col_iterator;
    typedef dense_compressed_iterator<typename this_type::const_iterator,
				      typename this_type::iterator,
				      const this_type *> const_col_iterator;
    typedef tab_ref_with_origin<typename this_type::iterator, 
				this_type> sub_row_type;
    typedef tab_ref_with_origin<typename this_type::const_iterator,
				this_type> const_sub_row_type;
    typedef dense_compressed_iterator<typename this_type::iterator,
				      typename this_type::iterator,
				      this_type *> row_iterator;
    typedef dense_compressed_iterator<typename this_type::const_iterator,
				      typename this_type::iterator,
				      const this_type *> const_row_iterator;
    typedef col_and_row sub_orientation;
    typedef linalg_true index_sorted;
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static const_sub_col_type col(const const_col_iterator &it)
    { return const_sub_col_type(*it, it.ncols, it.nrows, it.origin); }
    static const_sub_row_type row(const const_row_iterator &it)
    { return const_sub_row_type(*it, *it + it.ncols, it.origin); }
    static sub_col_type col(const col_iterator &it)
    { return sub_col_type(*it, *it, it.ncols, it.nrows, it.origin); }
    static sub_row_type row(const row_iterator &it)
    { return sub_row_type(*it, *it + it.ncols, it.origin); }
    static col_iterator col_begin(this_type &m)
    { return col_iterator(m.begin(), 1, m.ncols(), m.nrows(), 0, &m); }
    static col_iterator col_end(this_type &m)
    { return col_iterator(m.begin(), 1, m.ncols(), m.nrows(), m.ncols(), &m); }
    static const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m.begin(), 1, m.ncols(), m.nrows(), 0, &m); }
    static const_col_iterator col_end(const this_type &m) {
      return const_col_iterator(m.begin(), 1, m.ncols(),
				m.nrows(), m.ncols(), &m);
    }
    static row_iterator row_begin(this_type &m)
    { return row_iterator(m.begin(), m.ncols(), m.ncols(), m.nrows(), 0, &m); }
    static row_iterator row_end(this_type &m) {
      return row_iterator(m.begin(), m.ncols(), m.ncols(), m.nrows(),
			  m.nrows(), &m);
    }
    static const_row_iterator row_begin(const this_type &m) {
      return const_row_iterator(m.begin(), m.ncols(), m.ncols(), m.nrows(),
				0, &m);
    }
    static const_row_iterator row_end(const this_type &m) {
      return const_row_iterator(m.begin(), m.ncols(), m.ncols(), m.nrows(),
				m.nrows(), &m);
    }
    static origin_type* origin(this_type &m) { return &m; }
    static const origin_type* origin(const this_type &m) { return &m; }
    static void do_clear(this_type &m) { m.fill(value_type(0)); }
    static value_type access(const const_row_iterator &itrow, size_type j)
    { return (*itrow)[j]; }
    static reference access(const row_iterator &itrow, size_type j)
    { return (*itrow)[j]; }
    static void resize(this_type &v, size_type m, size_type n)
    { v.resize(m,n); }
    static void reshape(this_type &v, size_type m, size_type n)
    { v.reshape(m, n); }
  };

  template<typename PT> std::ostream &operator <<
    (std::ostream &o, const array2D_row_reference<PT>& m)
  { gmm::write(o,m); return o; }






}


#endif //  GMM_INTERFACE_H__
