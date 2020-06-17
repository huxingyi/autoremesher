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

/**@file gmm_sub_vector.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Generic sub-vectors.
*/

#ifndef GMM_SUB_VECTOR_H__
#define GMM_SUB_VECTOR_H__

#include "gmm_interface.h"
#include "gmm_sub_index.h"

namespace gmm {

  /* ********************************************************************* */
  /*		sparse sub-vectors                                         */
  /* ********************************************************************* */

  template <typename IT, typename MIT, typename SUBI>
  struct sparse_sub_vector_iterator {

    IT itb, itbe;
    SUBI si;

    typedef std::iterator_traits<IT>                traits_type;
    typedef typename traits_type::value_type        value_type;
    typedef typename traits_type::pointer           pointer;
    typedef typename traits_type::reference         reference;
    typedef typename traits_type::difference_type   difference_type;
    typedef std::bidirectional_iterator_tag         iterator_category;
    typedef size_t                                  size_type;
    typedef sparse_sub_vector_iterator<IT, MIT, SUBI>    iterator;

    size_type index(void) const { return si.rindex(itb.index()); }
    void forward(void);
    void backward(void);
    iterator &operator ++()
    { ++itb; forward(); return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator &operator --()
    { --itb; backward(); return *this; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }
    reference operator *() const { return *itb; }

    bool operator ==(const iterator &i) const { return itb == i.itb; }
    bool operator !=(const iterator &i) const { return !(i == *this); }

    sparse_sub_vector_iterator(void) {}
    sparse_sub_vector_iterator(const IT &it, const IT &ite, const SUBI &s)
      : itb(it), itbe(ite), si(s) { forward(); }
    sparse_sub_vector_iterator(const sparse_sub_vector_iterator<MIT, MIT,
	 SUBI> &it) : itb(it.itb), itbe(it.itbe), si(it.si) {}
  };

  template <typename IT, typename MIT, typename SUBI>
  void  sparse_sub_vector_iterator<IT, MIT, SUBI>::forward(void)
  { while(itb!=itbe && index()==size_type(-1)) { ++itb; } }

  template <typename IT, typename MIT, typename SUBI>
  void  sparse_sub_vector_iterator<IT, MIT, SUBI>::backward(void)
  { while(itb!=itbe && index()==size_type(-1)) --itb; }

  template <typename PT, typename SUBI> struct sparse_sub_vector {
    typedef sparse_sub_vector<PT, SUBI> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * CPT;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
            typename linalg_traits<V>::iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    SUBI si;

    size_type size(void) const { return si.size(); }
   
    reference operator[](size_type i) const
    { return linalg_traits<V>::access(origin, begin_, end_, si.index(i)); }

    sparse_sub_vector(V &v, const SUBI &s) : begin_(vect_begin(v)),
       end_(vect_end(v)), origin(linalg_origin(v)), si(s) {}
    sparse_sub_vector(const V &v, const SUBI &s) 
      : begin_(vect_begin(const_cast<V &>(v))),
       end_(vect_end(const_cast<V &>(v))),
	origin(linalg_origin(const_cast<V &>(v))), si(s) {}
    sparse_sub_vector() {}
    sparse_sub_vector(const sparse_sub_vector<CPT, SUBI> &cr)
      : begin_(cr.begin_),end_(cr.end_),origin(cr.origin), si(cr.si) {} 
  };

  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_begin(sparse_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, sparse_sub_vector<PT, SUBI> *,
		    linalg_modifiable) {
    typedef sparse_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_begin(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    it.forward();
  }
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_begin(sparse_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, const sparse_sub_vector<PT, SUBI> *, 
		    linalg_modifiable) {
    typedef sparse_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_begin(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    it.forward();
  }
  
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_end(sparse_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, sparse_sub_vector<PT, SUBI> *, linalg_modifiable) {
    typedef sparse_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_end(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    it.forward();
  }
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_end(sparse_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, const sparse_sub_vector<PT, SUBI> *,
		  linalg_modifiable) {
    typedef sparse_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    set_to_end(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    it.forward();
  }

  template <typename PT, typename SUBI>
  struct linalg_traits<sparse_sub_vector<PT, SUBI> > {
    typedef sparse_sub_vector<PT, SUBI> this_type;
    typedef this_type * pthis_type;
    typedef PT pV;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename linalg_and<typename index_is_sorted<SUBI>::bool_type,
	    typename linalg_traits<V>::index_sorted>::bool_type index_sorted;
    typedef typename linalg_traits<V>::is_reference V_reference;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename select_ref<value_type, typename
            linalg_traits<V>::reference, PT>::ref_type reference;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
	    typename linalg_traits<V>::iterator, PT>::ref_type pre_iterator;
    typedef typename select_ref<abstract_null_type, 
	    sparse_sub_vector_iterator<pre_iterator, pre_iterator, SUBI>,
	    PT>::ref_type iterator;
    typedef sparse_sub_vector_iterator<typename linalg_traits<V>
            ::const_iterator, pre_iterator, SUBI> const_iterator;
    typedef abstract_sparse storage_type;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) {
      iterator it;
      it.itb = v.begin_; it.itbe = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_begin(it, v.origin, pthis_type(), is_reference());
      else it.forward();
      return it;
    }
    static const_iterator begin(const this_type &v) {
      const_iterator it; it.itb = v.begin_; it.itbe = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	{ set_to_begin(it, v.origin, pthis_type(), is_reference()); }
      else it.forward();
      return it;
    }
    static iterator end(this_type &v) {
      iterator it;
      it.itb = v.end_; it.itbe = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      else it.forward();
      return it;
    }
    static const_iterator end(const this_type &v) {
      const_iterator it; it.itb = v.end_; it.itbe = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      else it.forward();
      return it;
    }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type* o, const iterator &begin_,
		      const iterator &end_) {
      std::deque<size_type> ind;
      iterator it = begin_;
      for (; it != end_; ++it) ind.push_front(it.index());
      for (; !(ind.empty()); ind.pop_back())
	access(o, begin_, end_, ind.back()) = value_type(0);
    }
    static void do_clear(this_type &v) { clear(v.origin, begin(v), end(v)); }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it.itb, ite.itb, it.si.index(i)); }
    static reference access(origin_type *o, const iterator &it,
			    const iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it.itb, ite.itb, it.si.index(i)); }
  };

  template <typename PT, typename SUBI> std::ostream &operator <<
  (std::ostream &o, const sparse_sub_vector<PT, SUBI>& m)
  { gmm::write(o,m); return o; }

  /* ********************************************************************* */
  /*		skyline sub-vectors                                        */
  /* ********************************************************************* */

    template <typename IT, typename MIT, typename SUBI>
  struct skyline_sub_vector_iterator {

    IT itb;
    SUBI si;

    typedef std::iterator_traits<IT>                traits_type;
    typedef typename traits_type::value_type        value_type;
    typedef typename traits_type::pointer           pointer;
    typedef typename traits_type::reference         reference;
    typedef typename traits_type::difference_type   difference_type;
    typedef std::bidirectional_iterator_tag         iterator_category;
    typedef size_t                                  size_type;
    typedef skyline_sub_vector_iterator<IT, MIT, SUBI>    iterator;

    size_type index(void) const
    { return (itb.index() - si.min + si.step() - 1) / si.step(); }
    void backward(void);
    iterator &operator ++()
    { itb += si.step(); return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator &operator --()
    { itb -= si.step(); return *this; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }

    iterator &operator +=(difference_type i)
    { itb += si.step() * i; return *this; }
    iterator &operator -=(difference_type i)
    { itb -= si.step() * i; return *this; }
    iterator operator +(difference_type i) const
    { iterator ii = *this; return (ii += i); }
    iterator operator -(difference_type i) const
    { iterator ii = *this; return (ii -= i); }
    difference_type operator -(const iterator &i) const
    { return (itb - i.itb) / si.step(); }

    reference operator *() const  { return *itb; }
    reference operator [](int ii) { return *(itb + ii * si.step());  }

    bool operator ==(const iterator &i) const { return index() == i.index();}
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return index()  < i.index();}

    skyline_sub_vector_iterator(void) {}
    skyline_sub_vector_iterator(const IT &it, const SUBI &s)
      : itb(it), si(s) {}
    skyline_sub_vector_iterator(const skyline_sub_vector_iterator<MIT, MIT,
	 SUBI> &it) : itb(it.itb), si(it.si) {}
  };

  template <typename IT, typename SUBI>
  void update_for_sub_skyline(IT &it, IT &ite, const SUBI &si) {
    if (it.index() >= si.max || ite.index() <= si.min) { it = ite; return; }
    ptrdiff_t dec1 = si.min - it.index(), dec2 = ite.index() - si.max;
    it  += (dec1 < 0) ? ((si.step()-((-dec1) % si.step())) % si.step()) : dec1;
    ite -= (dec2 < 0) ? -((-dec2) % si.step()) : dec2;
  }

  template <typename PT, typename SUBI> struct skyline_sub_vector {
    typedef skyline_sub_vector<PT, SUBI> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * pV;
    typedef typename select_ref<typename linalg_traits<V>::const_iterator,
            typename linalg_traits<V>::iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    SUBI si;

    size_type size(void) const { return si.size(); }
   
    reference operator[](size_type i) const
    { return linalg_traits<V>::access(origin, begin_, end_, si.index(i)); }

    skyline_sub_vector(V &v, const SUBI &s) : begin_(vect_begin(v)),
       end_(vect_end(v)), origin(linalg_origin(v)), si(s) {
      update_for_sub_skyline(begin_, end_, si);
    }
    skyline_sub_vector(const V &v, const SUBI &s)
      : begin_(vect_begin(const_cast<V &>(v))),
	end_(vect_end(const_cast<V &>(v))),
	origin(linalg_origin(const_cast<V &>(v))), si(s) {
      update_for_sub_skyline(begin_, end_, si);
    }
    skyline_sub_vector() {}
    skyline_sub_vector(const skyline_sub_vector<pV, SUBI> &cr)
      : begin_(cr.begin_),end_(cr.end_),origin(cr.origin), si(cr.si) {}
  };

  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_begin(skyline_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, skyline_sub_vector<PT, SUBI> *,
		    linalg_modifiable) {
    typedef skyline_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    IT itbe = it.itb;
    set_to_begin(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    update_for_sub_skyline(it.itb, itbe, it.si);
  }
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_begin(skyline_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, const skyline_sub_vector<PT, SUBI> *,
		    linalg_modifiable) {
    typedef skyline_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    IT itbe = it.itb;
    set_to_begin(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(itbe, o, typename linalg_traits<VECT>::pV(), ref_t());
    update_for_sub_skyline(it.itb, itbe, it.si);
  }
  
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_end(skyline_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, skyline_sub_vector<PT, SUBI> *,
		  linalg_modifiable) {
    typedef skyline_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    IT itb = it.itb;
    set_to_begin(itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    update_for_sub_skyline(itb, it.itb, it.si);
  }
  template <typename IT, typename MIT, typename SUBI, typename ORG,
	    typename PT> inline
  void set_to_end(skyline_sub_vector_iterator<IT, MIT, SUBI> &it,
		    ORG o, const skyline_sub_vector<PT, SUBI> *,
		  linalg_modifiable) {
    typedef skyline_sub_vector<PT, SUBI> VECT;
    typedef typename linalg_traits<VECT>::V_reference ref_t;
    IT itb = it.itb;
    set_to_begin(itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    set_to_end(it.itb, o, typename linalg_traits<VECT>::pV(), ref_t());
    update_for_sub_skyline(itb, it.itb, it.si);   
  }


  template <typename PT, typename SUBI>
  struct linalg_traits<skyline_sub_vector<PT, SUBI> > {
    typedef skyline_sub_vector<PT, SUBI> this_type;
    typedef this_type *pthis_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename linalg_traits<V>::is_reference V_reference;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef V * pV;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_vector linalg_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename select_ref<value_type, typename
            linalg_traits<V>::reference, PT>::ref_type reference;
    typedef typename linalg_traits<V>::const_iterator const_V_iterator;
    typedef typename linalg_traits<V>::iterator V_iterator;    
    typedef typename select_ref<const_V_iterator, V_iterator, 
				PT>::ref_type pre_iterator;
    typedef typename select_ref<abstract_null_type, 
	    skyline_sub_vector_iterator<pre_iterator, pre_iterator, SUBI>,
	    PT>::ref_type iterator;
    typedef skyline_sub_vector_iterator<const_V_iterator, pre_iterator, SUBI>
            const_iterator;
    typedef abstract_skyline storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) {
      iterator it;
      it.itb = v.begin_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_begin(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static const_iterator begin(const this_type &v) {
      const_iterator it; it.itb = v.begin_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	{ set_to_begin(it, v.origin, pthis_type(), is_reference()); }
      return it;
    }
    static iterator end(this_type &v) {
      iterator it;
      it.itb = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static const_iterator end(const this_type &v) {
      const_iterator it; it.itb = v.end_; it.si = v.si;
      if (!is_const_reference(is_reference()))
	set_to_end(it, v.origin, pthis_type(), is_reference());
      return it;
    }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void clear(origin_type*, const iterator &it, const iterator &ite)
    { std::fill(it, ite, value_type(0)); }
    static void do_clear(this_type &v) { clear(v.origin, begin(v), end(v)); }
    static value_type access(const origin_type *o, const const_iterator &it,
			     const const_iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it.itb, ite.itb, it.si.index(i)); }
    static reference access(origin_type *o, const iterator &it,
			    const iterator &ite, size_type i)
    { return linalg_traits<V>::access(o, it.itb, ite.itb, it.si.index(i)); }
  };

  template <typename PT, typename SUBI> std::ostream &operator <<
  (std::ostream &o, const skyline_sub_vector<PT, SUBI>& m)
  { gmm::write(o,m); return o; }

  /* ******************************************************************** */
  /*		sub vector.                                               */
  /* ******************************************************************** */
  /* sub_vector_type<PT, SUBI>::vector_type is the sub vector type        */
  /* returned by sub_vector(v, sub_index)                                 */
  /************************************************************************/

  template <typename PT, typename SUBI, typename st_type> struct svrt_ir {
    typedef abstract_null_type vector_type;
  };

  template <typename PT>
  struct svrt_ir<PT, sub_index, abstract_dense> {
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename vect_ref_type<PT,  V>::iterator iterator;
    typedef tab_ref_index_ref_with_origin<iterator,
      sub_index::const_iterator, V> vector_type;
  }; 

  template <typename PT>
  struct svrt_ir<PT, unsorted_sub_index, abstract_dense> {
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename vect_ref_type<PT,  V>::iterator iterator;
    typedef tab_ref_index_ref_with_origin<iterator,
      unsorted_sub_index::const_iterator, V> vector_type;
  }; 

  template <typename PT>
  struct svrt_ir<PT, sub_interval, abstract_dense> {
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename vect_ref_type<PT,  V>::iterator iterator;
    typedef tab_ref_with_origin<iterator, V> vector_type;
  }; 

  template <typename PT>
  struct svrt_ir<PT, sub_slice, abstract_dense> {
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename vect_ref_type<PT,  V>::iterator iterator;
    typedef tab_ref_reg_spaced_with_origin<iterator, V> vector_type;
  };

  template <typename PT, typename SUBI>
  struct svrt_ir<PT, SUBI, abstract_skyline> {
    typedef skyline_sub_vector<PT, SUBI> vector_type;
  };

  template <typename PT>
  struct svrt_ir<PT, sub_index, abstract_skyline> {
    typedef sparse_sub_vector<PT, sub_index> vector_type;
  };

  template <typename PT>
  struct svrt_ir<PT, unsorted_sub_index, abstract_skyline> {
    typedef sparse_sub_vector<PT, unsorted_sub_index> vector_type;
  };


  template <typename PT, typename SUBI>
  struct svrt_ir<PT, SUBI, abstract_sparse> {
    typedef sparse_sub_vector<PT, SUBI> vector_type;
  };

  template <typename PT, typename SUBI>
  struct sub_vector_type {
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename svrt_ir<PT, SUBI,
      typename linalg_traits<V>::storage_type>::vector_type vector_type;
  };

  template <typename V, typename SUBI>
  typename select_return<
    typename sub_vector_type<const V *, SUBI>::vector_type,
    typename sub_vector_type<V *, SUBI>::vector_type, const V *>::return_type
  sub_vector(const V &v, const SUBI &si) {
    GMM_ASSERT2(si.last() <= vect_size(v), "sub vector too large");
    return typename select_return<
      typename sub_vector_type<const V *, SUBI>::vector_type,
      typename sub_vector_type<V *, SUBI>::vector_type, const V *>::return_type
      (linalg_cast(v), si);
  }

  template <typename V, typename SUBI>
  typename select_return<
    typename sub_vector_type<const V *, SUBI>::vector_type,
    typename sub_vector_type<V *, SUBI>::vector_type, V *>::return_type
  sub_vector(V &v, const SUBI &si) {
    GMM_ASSERT2(si.last() <= vect_size(v), "sub vector too large");
    return  typename select_return<
      typename sub_vector_type<const V *, SUBI>::vector_type,
      typename sub_vector_type<V *, SUBI>::vector_type, V *>::return_type
      (linalg_cast(v), si);
  }

}

#endif //  GMM_SUB_VECTOR_H__
