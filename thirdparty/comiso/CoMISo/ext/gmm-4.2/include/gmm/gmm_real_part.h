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

/**@file gmm_real_part.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date September 18, 2003.
   @brief extract the real/imaginary part of vectors/matrices 
*/
#ifndef GMM_REAL_PART_H
#define GMM_REAL_PART_H

#include "gmm_def.h"
#include "gmm_vector.h"

namespace gmm {

  struct linalg_real_part {};
  struct linalg_imag_part {};
  template <typename R, typename PART> struct which_part {};
  
  template <typename C> typename number_traits<C>::magnitude_type 
  real_or_imag_part(C x, linalg_real_part) { return gmm::real(x); }
  template <typename C> typename number_traits<C>::magnitude_type 
  real_or_imag_part(C x, linalg_imag_part) { return gmm::imag(x); }
  template <typename T, typename C, typename OP> C
  complex_from(T x, C y, OP op, linalg_real_part) { return std::complex<T>(op(std::real(y), x), std::imag(y)); }
  template <typename T, typename C, typename OP> C
  complex_from(T x, C y, OP op,linalg_imag_part) { return std::complex<T>(std::real(y), op(std::imag(y), x)); }
  
  template<typename T> struct project2nd {
    T operator()(T , T b) const { return b; }
  };
  
  template<typename T, typename R, typename PART> class ref_elt_vector<T, which_part<R, PART> > {

    R r;
    
    public :

    operator T() const { return real_or_imag_part(std::complex<T>(r), PART()); }
    ref_elt_vector(R r_) : r(r_) {}
    inline ref_elt_vector &operator =(T v)
    { r = complex_from(v, std::complex<T>(r), gmm::project2nd<T>(), PART()); return *this; }
    inline bool operator ==(T v) const { return (r == v); }
    inline bool operator !=(T v) const { return (r != v); }
    inline ref_elt_vector &operator +=(T v)
    { r = complex_from(v, std::complex<T>(r), std::plus<T>(), PART()); return *this; }
    inline ref_elt_vector &operator -=(T v)
      { r = complex_from(v, std::complex<T>(r), std::minus<T>(), PART()); return *this; }
    inline ref_elt_vector &operator /=(T v)
      { r = complex_from(v, std::complex<T>(r), std::divides<T>(), PART()); return *this; }
    inline ref_elt_vector &operator *=(T v)
      { r = complex_from(v, std::complex<T>(r), std::multiplies<T>(), PART()); return *this; }
    inline ref_elt_vector &operator =(const ref_elt_vector &re)
      { *this = T(re); return *this; }
    T operator +()    { return  T(*this);   } // necessary for unknow reason
    T operator -()    { return -T(*this);   } // necessary for unknow reason
    T operator +(T v) { return T(*this)+ v; } // necessary for unknow reason
    T operator -(T v) { return T(*this)- v; } // necessary for unknow reason
    T operator *(T v) { return T(*this)* v; } // necessary for unknow reason
    T operator /(T v) { return T(*this)/ v; } // necessary for unknow reason
  };

  template<typename reference> struct ref_or_value_type {
    template <typename T, typename W>
    static W r(const T &x, linalg_real_part, W) {
      return gmm::real(x);
    }
    template <typename T, typename W>
    static W r(const T &x, linalg_imag_part, W) {
      return gmm::imag(x);
    }
  };
  
  template<typename U, typename R, typename PART> 
  struct ref_or_value_type<ref_elt_vector<U, which_part<R, PART> > > {
    template<typename T , typename W> 
    static const T &r(const T &x, linalg_real_part, W)
    { return x; }
    template<typename T, typename W> 
    static const T &r(const T &x, linalg_imag_part, W) {
      return x; 
    }
    template<typename T , typename W> 
    static T &r(T &x, linalg_real_part, W)
    { return x; }
    template<typename T, typename W> 
    static T &r(T &x, linalg_imag_part, W) {
      return x; 
    }
  };

  
  /* ********************************************************************* */
  /*	Reference to the real part of (complex) vectors            	   */
  /* ********************************************************************* */

  template <typename IT, typename MIT, typename PART>
  struct part_vector_iterator {
    typedef typename std::iterator_traits<IT>::value_type      vtype;
    typedef typename gmm::number_traits<vtype>::magnitude_type value_type;
    typedef value_type                                        *pointer;
    typedef ref_elt_vector<value_type, which_part<typename std::iterator_traits<IT>::reference, PART> > reference;
    typedef typename std::iterator_traits<IT>::difference_type difference_type;
    typedef typename std::iterator_traits<IT>::iterator_category
    iterator_category;

    IT it;
    
    part_vector_iterator(void) {}
    explicit part_vector_iterator(const IT &i) : it(i) {}
    part_vector_iterator(const part_vector_iterator<MIT, MIT, PART> &i) : it(i.it) {}
    

    size_type index(void) const { return it.index(); }
    part_vector_iterator operator ++(int)
    { part_vector_iterator tmp = *this; ++it; return tmp; }
    part_vector_iterator operator --(int) 
    { part_vector_iterator tmp = *this; --it; return tmp; }
    part_vector_iterator &operator ++() { ++it; return *this; }
    part_vector_iterator &operator --() { --it; return *this; }
    part_vector_iterator &operator +=(difference_type i)
      { it += i; return *this; }
    part_vector_iterator &operator -=(difference_type i)
      { it -= i; return *this; }
    part_vector_iterator operator +(difference_type i) const
      { part_vector_iterator itb = *this; return (itb += i); }
    part_vector_iterator operator -(difference_type i) const
      { part_vector_iterator itb = *this; return (itb -= i); }
    difference_type operator -(const part_vector_iterator &i) const
      { return difference_type(it - i.it); }
    
    reference operator  *() const { return reference(*it); }
    reference operator [](size_type ii) const { return reference(it[ii]); }
    
    bool operator ==(const part_vector_iterator &i) const
      { return (i.it == it); }
    bool operator !=(const part_vector_iterator &i) const
      { return (i.it != it); }
    bool operator < (const part_vector_iterator &i) const
      { return (it < i.it); }
  };


  template <typename PT, typename PART> struct part_vector {
    typedef part_vector<PT, PART> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * CPT;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
            typename linalg_traits<V>::iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    size_type size_;

    size_type size(void) const { return size_; }
   
    reference operator[](size_type i) const { 
      return reference(ref_or_value_type<reference>::r(
	     linalg_traits<V>::access(origin, begin_, end_, i),
	     PART(), value_type()));
    }

    part_vector(V &v)
      : begin_(vect_begin(v)),  end_(vect_end(v)),
	origin(linalg_origin(v)), size_(gmm::vect_size(v)) {}
    part_vector(const V &v) 
      : begin_(vect_begin(const_cast<V &>(v))),
       end_(vect_end(const_cast<V &>(v))),
	origin(linalg_origin(const_cast<V &>(v))), size_(gmm::vect_size(v)) {}
    part_vector() {}
    part_vector(const part_vector<CPT, PART> &cr)
      : begin_(cr.begin_),end_(cr.end_),origin(cr.origin), size_(cr.size_) {} 
  };

  template <typename IT, typename MIT, typename ORG, typename PT,
	    typename PART> inline
  void set_to_begin(part_vector_iterator<IT, MIT, PART> &it,
		    ORG o, part_vector<PT, PART> *, linalg_modifiable) {
    typedef part_vector<PT, PART> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_begin(it.it, o, typename linalg_traits<VECT>::pV(), ref_t());
  }
  template <typename IT, typename MIT, typename ORG, typename PT,
	    typename PART> inline
  void set_to_begin(part_vector_iterator<IT, MIT, PART> &it,
		    ORG o, const part_vector<PT, PART> *, linalg_modifiable) {
    typedef part_vector<PT, PART> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_begin(it.it, o, typename linalg_traits<VECT>::pV(), ref_t());
  }
  template <typename IT, typename MIT, typename ORG, typename PT,
	    typename PART> inline
  void set_to_end(part_vector_iterator<IT, MIT, PART> &it,
		    ORG o, part_vector<PT, PART> *, linalg_modifiable) {
    typedef part_vector<PT, PART> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_end(it.it, o, typename linalg_traits<VECT>::pV(), ref_t());
  }
  template <typename IT, typename MIT, typename ORG,
	    typename PT, typename PART> inline
  void set_to_end(part_vector_iterator<IT, MIT, PART> &it,
		  ORG o, const part_vector<PT, PART> *,
		  linalg_modifiable) {
    typedef part_vector<PT, PART> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_end(it.it, o, typename linalg_traits<VECT>::pV(), ref_t());
  }
  
  template <typename PT, typename PART>
  struct linalg_traits<part_vector<PT, PART> > {
    typedef part_vector<PT, PART> this_type;
    typedef this_type * pthis_type;
    typedef PT pV;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename linalg_traits<V>::index_sorted index_sorted;
    typedef typename linalg_traits<V>::is_reference V_reference;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename linalg_traits<V>::value_type vtype;
    typedef typename number_traits<vtype>::magnitude_type value_type;
    typedef typename select_ref<value_type, ref_elt_vector<value_type,
		     which_part<typename linalg_traits<V>::reference,
				PART> >, PT>::ref_type reference;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
	    typename linalg_traits<V>::iterator, PT>::ref_type pre_iterator;
    typedef typename select_ref<abstract_null_type, 
	    part_vector_iterator<pre_iterator, pre_iterator, PART>,
	    PT>::ref_type iterator;
    typedef part_vector_iterator<typename linalg_traits<V>::const_iterator,
				 pre_iterator, PART> const_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) {
      iterator it; it.it = v.begin_;
      if (!is_const_reference(is_reference()) && is_sparse(storage_type()))
	set_to_begin(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static const_iterator begin(const this_type &v) {
      const_iterator it(v.begin_);
      if (!is_const_reference(is_reference()) && is_sparse(storage_type()))
	{ set_to_begin(it, v.origin, pthis_type(), is_reference()); }
      return it;
    }
    static iterator end(this_type &v) {
      iterator it(v.end_);
      if (!is_const_reference(is_reference()) && is_sparse(storage_type()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static const_iterator end(const this_type &v) {
      const_iterator it(v.end_);
      if (!is_const_reference(is_reference()) && is_sparse(storage_type()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }

    static void clear(origin_type* o, const iterator &begin_,
		      const iterator &end_, abstract_sparse) {
      std::deque<size_type> ind;
      iterator it = begin_;
      for (; it != end_; ++it) ind.push_front(it.index());
      for (; !(ind.empty()); ind.pop_back())
	access(o, begin_, end_, ind.back()) = value_type(0);
    }
    static void clear(origin_type* o, const iterator &begin_,
		      const iterator &end_, abstract_skyline) {
      clear(o, begin_, end_, abstract_sparse());
    }
    static void clear(origin_type* o, const iterator &begin_,
		      const iterator &end_, abstract_dense) {
      for (iterator it = begin_; it != end_; ++it) *it = value_type(0);
    }

   static void clear(origin_type* o, const iterator &begin_,
		      const iterator &end_) 
    { clear(o, begin_, end_, storage_type()); }
    static void do_clear(this_type &v) { clear(v.origin, begin(v), end(v)); }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i) { 
      return  real_or_imag_part(linalg_traits<V>::access(o, it.it, ite.it,i),
				PART());
    }
    static reference access(origin_type *o, const iterator &it,
			    const iterator &ite, size_type i)
    { return reference(linalg_traits<V>::access(o, it.it, ite.it,i)); }
  };

  template <typename PT, typename PART> std::ostream &operator <<
    (std::ostream &o, const part_vector<PT, PART>& m)
  { gmm::write(o,m); return o; }


  /* ********************************************************************* */
  /*	Reference to the real or imaginary part of (complex) matrices      */
  /* ********************************************************************* */


  template <typename PT, typename PART> struct  part_row_ref {
    
    typedef part_row_ref<PT, PART> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<this_type>
            ::const_row_iterator, typename linalg_traits<this_type>
            ::row_iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    size_type nr, nc;

    part_row_ref(ref_M m)
      : begin_(mat_row_begin(m)), end_(mat_row_end(m)),
	origin(linalg_origin(m)), nr(mat_nrows(m)), nc(mat_ncols(m)) {}

    part_row_ref(const part_row_ref<CPT, PART> &cr) :
      begin_(cr.begin_),end_(cr.end_), origin(cr.origin),nr(cr.nr),nc(cr.nc) {}

    reference operator()(size_type i, size_type j) const {
      return reference(ref_or_value_type<reference>::r(
					 linalg_traits<M>::access(begin_+i, j),
					 PART(), value_type()));
    }
  };

  template <typename PT, typename PART>
  struct linalg_traits<part_row_ref<PT, PART> > {
    typedef part_row_ref<PT, PART> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type vtype;
    typedef typename number_traits<vtype>::magnitude_type value_type;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type const_col_iterator;
    typedef typename linalg_traits<M>::const_sub_row_type
            pre_const_sub_row_type;
    typedef typename linalg_traits<M>::sub_row_type pre_sub_row_type;
    typedef part_vector<const pre_const_sub_row_type *, PART>
            const_sub_row_type;
    typedef typename select_ref<abstract_null_type,
	    part_vector<pre_sub_row_type *, PART>, PT>::ref_type sub_row_type;
    typedef typename linalg_traits<M>::const_row_iterator const_row_iterator;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::row_iterator, PT>::ref_type row_iterator;
    typedef typename select_ref<
            typename linalg_traits<const_sub_row_type>::reference,
	    typename linalg_traits<sub_row_type>::reference,
				PT>::ref_type reference;
    typedef row_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type ncols(const this_type &v) { return v.nc; }
    static size_type nrows(const this_type &v) { return v.nr; }
    static const_sub_row_type row(const const_row_iterator &it)
    { return const_sub_row_type(linalg_traits<M>::row(it)); }
    static sub_row_type row(const row_iterator &it)
    { return sub_row_type(linalg_traits<M>::row(it)); }
    static row_iterator row_begin(this_type &m) { return m.begin_; }
    static row_iterator row_end(this_type &m) { return m.end_; }
    static const_row_iterator row_begin(const this_type &m)
    { return m.begin_; }
    static const_row_iterator row_end(const this_type &m) { return m.end_; }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &v);
    static value_type access(const const_row_iterator &itrow, size_type i)
    { return real_or_imag_part(linalg_traits<M>::access(itrow, i), PART()); }
    static reference access(const row_iterator &itrow, size_type i) {
      return reference(ref_or_value_type<reference>::r(
					 linalg_traits<M>::access(itrow, i),
					 PART(), value_type()));
    }
  };
   
  template <typename PT, typename PART> 
  void linalg_traits<part_row_ref<PT, PART> >::do_clear(this_type &v) { 
    row_iterator it = mat_row_begin(v), ite = mat_row_end(v);
    for (; it != ite; ++it) clear(row(it));
  }
  
  template<typename PT, typename PART> std::ostream &operator <<
    (std::ostream &o, const part_row_ref<PT, PART>& m)
  { gmm::write(o,m); return o; }

  template <typename PT, typename PART> struct  part_col_ref {
    
    typedef part_col_ref<PT, PART> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<this_type>
            ::const_col_iterator, typename linalg_traits<this_type>
            ::col_iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::value_type value_type;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    size_type nr, nc;

    part_col_ref(ref_M m)
      : begin_(mat_col_begin(m)), end_(mat_col_end(m)),
	origin(linalg_origin(m)), nr(mat_nrows(m)), nc(mat_ncols(m)) {}

    part_col_ref(const part_col_ref<CPT, PART> &cr) :
      begin_(cr.begin_),end_(cr.end_), origin(cr.origin),nr(cr.nr),nc(cr.nc) {}

    reference operator()(size_type i, size_type j) const {
      return reference(ref_or_value_type<reference>::r(
					 linalg_traits<M>::access(begin_+j, i),
					 PART(), value_type()));
    }
  };

  template <typename PT, typename PART>
  struct linalg_traits<part_col_ref<PT, PART> > {
    typedef part_col_ref<PT, PART> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type vtype;
    typedef typename number_traits<vtype>::magnitude_type value_type;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_row_iterator;
    typedef typename linalg_traits<M>::const_sub_col_type
            pre_const_sub_col_type;
    typedef typename linalg_traits<M>::sub_col_type pre_sub_col_type;
    typedef part_vector<const pre_const_sub_col_type *, PART>
            const_sub_col_type;
    typedef typename select_ref<abstract_null_type,
	    part_vector<pre_sub_col_type *, PART>, PT>::ref_type sub_col_type;
    typedef typename linalg_traits<M>::const_col_iterator const_col_iterator;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::col_iterator, PT>::ref_type col_iterator;
    typedef typename select_ref<
            typename linalg_traits<const_sub_col_type>::reference,
	    typename linalg_traits<sub_col_type>::reference,
				PT>::ref_type reference;
    typedef col_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type nrows(const this_type &v) { return v.nr; }
    static size_type ncols(const this_type &v) { return v.nc; }
    static const_sub_col_type col(const const_col_iterator &it)
    { return const_sub_col_type(linalg_traits<M>::col(it)); }
    static sub_col_type col(const col_iterator &it)
    { return sub_col_type(linalg_traits<M>::col(it)); }
    static col_iterator col_begin(this_type &m) { return m.begin_; }
    static col_iterator col_end(this_type &m) { return m.end_; }
    static const_col_iterator col_begin(const this_type &m)
    { return m.begin_; }
    static const_col_iterator col_end(const this_type &m) { return m.end_; }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &v);
    static value_type access(const const_col_iterator &itcol, size_type i)
    { return real_or_imag_part(linalg_traits<M>::access(itcol, i), PART()); }
    static reference access(const col_iterator &itcol, size_type i) {
      return reference(ref_or_value_type<reference>::r(
					 linalg_traits<M>::access(itcol, i),
					 PART(), value_type()));
    }
  };
   
  template <typename PT, typename PART> 
  void linalg_traits<part_col_ref<PT, PART> >::do_clear(this_type &v) { 
    col_iterator it = mat_col_begin(v), ite = mat_col_end(v);
    for (; it != ite; ++it) clear(col(it));
  }
  
  template<typename PT, typename PART> std::ostream &operator <<
    (std::ostream &o, const part_col_ref<PT, PART>& m)
  { gmm::write(o,m); return o; }

  




template <typename TYPE, typename PART, typename PT>
  struct part_return_ {
    typedef abstract_null_type return_type;
  };
  template <typename PT, typename PART>
  struct part_return_<row_major, PART, PT> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return<part_row_ref<const L *, PART>,
		     part_row_ref< L *, PART>, PT>::return_type return_type;
  };
  template <typename PT, typename PART>
  struct part_return_<col_major, PART, PT> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return<part_col_ref<const L *, PART>,
		     part_col_ref<L *, PART>, PT>::return_type return_type;
  };

  template <typename PT, typename PART, typename LT> struct part_return__{
    typedef abstract_null_type return_type;
  };

  template <typename PT, typename PART>
  struct part_return__<PT, PART, abstract_matrix> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename part_return_<typename principal_orientation_type<
      typename linalg_traits<L>::sub_orientation>::potype, PART,
      PT>::return_type return_type;
  };

  template <typename PT, typename PART>
  struct part_return__<PT, PART, abstract_vector> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return<part_vector<const L *, PART>,
      part_vector<L *, PART>, PT>::return_type return_type;
  };

  template <typename PT, typename PART> struct part_return {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename part_return__<PT, PART,
      typename linalg_traits<L>::linalg_type>::return_type return_type;
  };

  template <typename L> inline 
  typename part_return<const L *, linalg_real_part>::return_type
  real_part(const L &l) {
    return typename part_return<const L *, linalg_real_part>::return_type
      (linalg_cast(const_cast<L &>(l)));
  }

  template <typename L> inline 
  typename part_return<L *, linalg_real_part>::return_type
  real_part(L &l) {
    return typename part_return<L *, linalg_real_part>::return_type(linalg_cast(l));
  }

  template <typename L> inline 
  typename part_return<const L *, linalg_imag_part>::return_type
  imag_part(const L &l) {
    return typename part_return<const L *, linalg_imag_part>::return_type
      (linalg_cast(const_cast<L &>(l)));
  }

  template <typename L> inline 
  typename part_return<L *, linalg_imag_part>::return_type
  imag_part(L &l) {
    return typename part_return<L *, linalg_imag_part>::return_type(linalg_cast(l));
  }



}

#endif //  GMM_REAL_PART_H
