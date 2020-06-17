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
/**@file gmm_vector.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Declaration of the vector types (gmm::rsvector, gmm::wsvector,
     gmm::slvector ,..)
*/
#ifndef GMM_VECTOR_H__
#define GMM_VECTOR_H__

#include <map>
#include "gmm_interface.h"

namespace gmm {

  /*************************************************************************/
  /*                                                                       */
  /* Class ref_elt_vector: reference on a vector component.                */
  /*                                                                       */
  /*************************************************************************/


  template<typename T, typename V> class ref_elt_vector {

    V *pm;
    size_type l;
    
    public :

    operator T() const { return pm->r(l); }
    ref_elt_vector(V *p, size_type ll) : pm(p), l(ll) {}
    inline ref_elt_vector &operator =(T v)
      { (*pm).w(l,v); return *this; }
    inline bool operator ==(T v) const { return ((*pm).r(l) == v); }
    inline bool operator !=(T v) const { return ((*pm).r(l) != v); }
    inline ref_elt_vector &operator +=(T v)
      { (*pm).w(l,(*pm).r(l) + v); return *this; }
    inline ref_elt_vector &operator -=(T v)
      { (*pm).w(l,(*pm).r(l) - v); return *this; }
    inline ref_elt_vector &operator /=(T v)
      { (*pm).w(l,(*pm).r(l) / v); return *this; }
    inline ref_elt_vector &operator *=(T v)
      { (*pm).w(l,(*pm).r(l) * v); return *this; }
    inline ref_elt_vector &operator =(const ref_elt_vector &re)
      { *this = T(re); return *this; }
    T operator +()    { return  T(*this);   } // necessary for unknow reason
    T operator -()    { return -T(*this);   } // necessary for unknow reason
    T operator +(T v) { return T(*this)+ v; } // necessary for unknow reason
    T operator -(T v) { return T(*this)- v; } // necessary for unknow reason
    T operator *(T v) { return T(*this)* v; } // necessary for unknow reason
    T operator /(T v) { return T(*this)/ v; } // necessary for unknow reason
  };  
  
  
  template<typename T, typename V> inline
  bool operator ==(T v, const ref_elt_vector<T, V> &re) { return (v==T(re)); }
  template<typename T, typename V> inline
  bool operator !=(T v, const ref_elt_vector<T, V> &re) { return (v!=T(re)); }
  template<typename T, typename V> inline
  T &operator +=(T &v, const ref_elt_vector<T, V> &re) 
  { v += T(re); return v; }
  template<typename T, typename V> inline
  T &operator -=(T &v, const ref_elt_vector<T, V> &re)
  { v -= T(re); return v; }
  template<typename T, typename V> inline
  T &operator *=(T &v, const ref_elt_vector<T, V> &re) 
  { v *= T(re); return v; }
  template<typename T, typename V> inline
  T &operator /=(T &v, const ref_elt_vector<T, V> &re)
  { v /= T(re); return v; }
  template<typename T, typename V> inline
  T operator +(const ref_elt_vector<T, V> &re) { return T(re); }
  template<typename T, typename V> inline
  T operator -(const ref_elt_vector<T, V> &re) { return -T(re); }
  template<typename T, typename V> inline
  T operator +(const ref_elt_vector<T, V> &re, T v) { return T(re)+ v; }
  template<typename T, typename V> inline
  T operator +(T v, const ref_elt_vector<T, V> &re) { return v+ T(re); }
  template<typename T, typename V> inline
  T operator -(const ref_elt_vector<T, V> &re, T v) { return T(re)- v; }
  template<typename T, typename V> inline
  T operator -(T v, const ref_elt_vector<T, V> &re) { return v- T(re); }
  template<typename T, typename V>  inline
  T operator *(const ref_elt_vector<T, V> &re, T v) { return T(re)* v; }
  template<typename T, typename V> inline
  T operator *(T v, const ref_elt_vector<T, V> &re) { return v* T(re); }
  template<typename T, typename V> inline
  T operator /(const ref_elt_vector<T, V> &re, T v) { return T(re)/ v; }
  template<typename T, typename V> inline
  T operator /(T v, const ref_elt_vector<T, V> &re) { return v/ T(re); }
  template<typename T, typename V> inline
  typename number_traits<T>::magnitude_type
  abs(const ref_elt_vector<T, V> &re) { return gmm::abs(T(re)); }
  template<typename T, typename V> inline
  T sqr(const ref_elt_vector<T, V> &re) { return gmm::sqr(T(re)); }
  template<typename T, typename V> inline
  typename number_traits<T>::magnitude_type
  abs_sqr(const ref_elt_vector<T, V> &re) { return gmm::abs_sqr(T(re)); }
  template<typename T, typename V> inline
  T conj(const ref_elt_vector<T, V> &re) { return gmm::conj(T(re)); }
  template<typename T, typename V> std::ostream &operator <<
  (std::ostream &o, const ref_elt_vector<T, V> &re) { o << T(re); return o; }
  template<typename T, typename V> inline
  typename number_traits<T>::magnitude_type
  real(const ref_elt_vector<T, V> &re) { return gmm::real(T(re)); }
  template<typename T, typename V> inline
  typename number_traits<T>::magnitude_type
  imag(const ref_elt_vector<T, V> &re) { return gmm::imag(T(re)); }

  

  /*************************************************************************/
  /*                                                                       */
  /* Class wsvector: sparse vector optimized for random write operations.  */
  /*                                                                       */
  /*************************************************************************/
  
  template<typename T> struct wsvector_iterator
    : public std::map<size_type, T>::iterator {
    typedef typename std::map<size_type, T>::iterator base_it_type;
    typedef T                   value_type;
    typedef value_type*         pointer;
    typedef value_type&         reference;
    // typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    
    reference operator *() const { return (base_it_type::operator*()).second; }
    pointer operator->() const { return &(operator*()); }
    size_type index(void) const { return (base_it_type::operator*()).first; }

    wsvector_iterator(void) {}
    wsvector_iterator(const base_it_type &it) : base_it_type(it) {}
  };

  template<typename T> struct wsvector_const_iterator
    : public std::map<size_type, T>::const_iterator {
    typedef typename std::map<size_type, T>::const_iterator base_it_type;
    typedef T                   value_type;
    typedef const value_type*   pointer;
    typedef const value_type&   reference;
    // typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    
    reference operator *() const { return (base_it_type::operator*()).second; }
    pointer operator->() const { return &(operator*()); }
    size_type index(void) const { return (base_it_type::operator*()).first; }

    wsvector_const_iterator(void) {}
    wsvector_const_iterator(const wsvector_iterator<T> &it)
      : base_it_type(it) {}
    wsvector_const_iterator(const base_it_type &it) : base_it_type(it) {}
  };


  /**
     sparse vector built upon std::map.
     Read and write access are quite fast (log n)
  */
  template<typename T> class wsvector : public std::map<size_type, T> {
  public:
    
    typedef typename std::map<int, T>::size_type size_type;
    typedef std::map<size_type, T> base_type;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;

  protected:
    size_type nbl;
    
  public:
    void clean(double eps);
    void resize(size_type);
    
    inline ref_elt_vector<T, wsvector<T> > operator [](size_type c)
    { return ref_elt_vector<T, wsvector<T> >(this, c); }

    inline void w(size_type c, const T &e) {
      GMM_ASSERT2(c < nbl, "out of range");
      if (e == T(0)) { base_type::erase(c); }
      else base_type::operator [](c) = e;
    }

    inline T r(size_type c) const {
      GMM_ASSERT2(c < nbl, "out of range");
      const_iterator it = this->lower_bound(c);
      if (it != this->end() && c == it->first) return it->second;
      else return T(0);
    }

    inline T operator [](size_type c) const { return r(c); }
    
    size_type nb_stored(void) const { return base_type::size(); }
    size_type size(void) const { return nbl; }

    void swap(wsvector<T> &v)
    { std::swap(nbl, v.nbl); std::map<size_type, T>::swap(v); }
				       

    /* Constructeurs */
    void init(size_type l) { nbl = l; this->clear(); }
    explicit wsvector(size_type l){ init(l); }
    wsvector(void) { init(0); }
  };

  template<typename T>  void wsvector<T>::clean(double eps) {
    iterator it = this->begin(), itf = it, ite = this->end();
    while (it != ite) {
      ++itf; if (gmm::abs(it->second) <= eps) erase(it); it = itf;
    }
  }

  template<typename T>  void wsvector<T>::resize(size_type n) {
    if (n < nbl) {
      iterator it = this->begin(), itf = it, ite = this->end();
      while (it != ite) { ++itf; if (it->first >= n) this->erase(it); it=itf; }
    }
    nbl = n;
  }

  template <typename T> struct linalg_traits<wsvector<T> > {
    typedef wsvector<T> this_type;
    typedef this_type origin_type;
    typedef linalg_false is_reference;
    typedef abstract_vector linalg_type;
    typedef T value_type;
    typedef ref_elt_vector<T, wsvector<T> > reference;
    typedef wsvector_iterator<T>  iterator;
    typedef wsvector_const_iterator<T> const_iterator;
    typedef abstract_sparse storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return v.begin(); }
    static const_iterator begin(const this_type &v) { return v.begin(); }
    static iterator end(this_type &v) { return v.end(); }
    static const_iterator end(const this_type &v) { return v.end(); }
    static origin_type* origin(this_type &v) { return &v; }
    static const origin_type* origin(const this_type &v) { return &v; }
    static void clear(origin_type* o, const iterator &, const iterator &)
    { o->clear(); }
    static void do_clear(this_type &v) { v.clear(); }
    static value_type access(const origin_type *o, const const_iterator &,
			     const const_iterator &, size_type i)
    { return (*o)[i]; }
    static reference access(origin_type *o, const iterator &, const iterator &,
			    size_type i)
    { return (*o)[i]; }
    static void resize(this_type &v, size_type n) { v.resize(n); }
  };

  template<typename T> std::ostream &operator <<
  (std::ostream &o, const wsvector<T>& v) { gmm::write(o,v); return o; }

  /******* Optimized BLAS for wsvector<T> **********************************/

  template <typename T> inline void copy(const wsvector<T> &v1,
					 wsvector<T> &v2) {
    GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
    v2 = v1;
  }
  template <typename T> inline
  void copy(const wsvector<T> &v1, const simple_vector_ref<wsvector<T> *> &v2){
    simple_vector_ref<wsvector<T> *>
      *svr = const_cast<simple_vector_ref<wsvector<T> *> *>(&v2);
    wsvector<T>
      *pv = const_cast<wsvector<T> *>(v2.origin);
    GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
    *pv = v1; svr->begin_ = vect_begin(*pv); svr->end_ = vect_end(*pv);
  }
  template <typename T> inline
  void copy(const simple_vector_ref<const wsvector<T> *> &v1,
	    wsvector<T> &v2)
  { copy(*(v1.origin), v2); }
  template <typename T> inline
  void copy(const simple_vector_ref<wsvector<T> *> &v1, wsvector<T> &v2)
  { copy(*(v1.origin), v2); }

  template <typename T> inline void clean(wsvector<T> &v, double eps) {
    typedef typename number_traits<T>::magnitude_type R;
    typename wsvector<T>::iterator it = v.begin(), ite = v.end(), itc;
    while (it != ite) 
      if (gmm::abs((*it).second) <= R(eps))
	{ itc=it; ++it; v.erase(itc); } else ++it; 
  }

  template <typename T>
  inline void clean(const simple_vector_ref<wsvector<T> *> &l, double eps) {
    simple_vector_ref<wsvector<T> *>
      *svr = const_cast<simple_vector_ref<wsvector<T> *> *>(&l);
    wsvector<T>
      *pv = const_cast<wsvector<T> *>((l.origin));
    clean(*pv, eps);
    svr->begin_ = vect_begin(*pv); svr->end_ = vect_end(*pv);
  }

  template <typename T>
  inline size_type nnz(const wsvector<T>& l) { return l.nb_stored(); }

  /*************************************************************************/
  /*                                                                       */
  /*    rsvector: sparse vector optimized for linear algebra operations.   */
  /*                                                                       */
  /*************************************************************************/

  template<typename T> struct elt_rsvector_ {
    size_type c; T e;
    /* e is initialized by default to avoid some false warnings of valgrind..
       (from http://valgrind.org/docs/manual/mc-manual.html:
      
       When memory is read into the CPU's floating point registers, the
       relevant V bits are read from memory and they are immediately
       checked. If any are invalid, an uninitialised value error is
       emitted. This precludes using the floating-point registers to copy
       possibly-uninitialised memory, but simplifies Valgrind in that it
       does not have to track the validity status of the floating-point
       registers.
    */
    elt_rsvector_(void) : e(0) {}
    elt_rsvector_(size_type cc) : c(cc), e(0) {}
    elt_rsvector_(size_type cc, const T &ee) : c(cc), e(ee) {}
    bool operator < (const elt_rsvector_ &a) const { return c < a.c; }
    bool operator == (const elt_rsvector_ &a) const { return c == a.c; }
    bool operator != (const elt_rsvector_ &a) const { return c != a.c; }
  };

  template<typename T> struct rsvector_iterator {
    typedef typename std::vector<elt_rsvector_<T> >::iterator IT;
    typedef T                   value_type;
    typedef value_type*         pointer;
    typedef value_type&         reference;
    typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef rsvector_iterator<T> iterator;

    IT it;

    reference operator *() const { return it->e; }
    pointer operator->() const { return &(operator*()); }

    iterator &operator ++() { ++it; return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator &operator --() { --it; return *this; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }

    bool operator ==(const iterator &i) const { return it == i.it; }
    bool operator !=(const iterator &i) const { return !(i == *this); }

    size_type index(void) const { return it->c; }
    rsvector_iterator(void) {}
    rsvector_iterator(const IT &i) : it(i) {}
  };

  template<typename T> struct rsvector_const_iterator {
    typedef typename std::vector<elt_rsvector_<T> >::const_iterator IT;
    typedef T                   value_type;
    typedef const value_type*   pointer;
    typedef const value_type&   reference;
    typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;
    typedef std::forward_iterator_tag iterator_category;
    typedef rsvector_const_iterator<T> iterator;

    IT it;

    reference operator *() const { return it->e; }
    pointer operator->() const { return &(operator*()); }
    size_type index(void) const { return it->c; }

    iterator &operator ++() { ++it; return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator &operator --() { --it; return *this; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }

    bool operator ==(const iterator &i) const { return it == i.it; }
    bool operator !=(const iterator &i) const { return !(i == *this); }

    rsvector_const_iterator(void) {}
    rsvector_const_iterator(const rsvector_iterator<T> &i) : it(i.it) {}
    rsvector_const_iterator(const IT &i) : it(i) {}
  };

  /**
     sparse vector built upon std::vector. Read access is fast,
     but insertion is O(n) 
  */
  template<typename T> class rsvector : public std::vector<elt_rsvector_<T> > {
  public:
    
    typedef std::vector<elt_rsvector_<T> > base_type_;
    typedef typename base_type_::iterator iterator;
    typedef typename base_type_::const_iterator const_iterator;
    typedef typename base_type_::size_type size_type;
    typedef T value_type;

  protected:
    size_type nbl;    	/* size of the vector.	          	  */
    
  public:

    void sup(size_type j);
    void base_resize(size_type n) { base_type_::resize(n); }
    void resize(size_type);
    
    ref_elt_vector<T, rsvector<T> > operator [](size_type c)
    { return ref_elt_vector<T, rsvector<T> >(this, c); }

    void w(size_type c, const T &e);
    T r(size_type c) const;
    void swap_indices(size_type i, size_type j);

    inline T operator [](size_type c) const { return r(c); }
    
    size_type nb_stored(void) const { return base_type_::size(); }
    size_type size(void) const { return nbl; }
    void clear(void) { base_type_::resize(0); }
    void swap(rsvector<T> &v)
    { std::swap(nbl, v.nbl); std::vector<elt_rsvector_<T> >::swap(v); }

    /* Constructeurs */
    explicit rsvector(size_type l) : nbl(l) { }
    rsvector(void) : nbl(0) { }
  };

  template <typename T>
  void rsvector<T>::swap_indices(size_type i, size_type j) {
    if (i > j) std::swap(i, j);
    if (i != j) {
      int situation = 0;
      elt_rsvector_<T> ei(i), ej(j), a;
      iterator it, ite, iti, itj;
      iti = std::lower_bound(this->begin(), this->end(), ei);
      if (iti != this->end() && iti->c == i) situation += 1;
      itj = std::lower_bound(this->begin(), this->end(), ej);
      if (itj != this->end() && itj->c == j) situation += 2;

      switch (situation) {
      case 1 : a = *iti; a.c = j; it = iti; ++it; ite = this->end();
	       for (; it != ite && it->c <= j; ++it, ++iti) *iti = *it;
	       *iti = a;
	       break;
      case 2 : a = *itj; a.c = i; it = itj; ite = this->begin();
	if (it != ite) {
	  --it;
	  while (it->c >= i) { *itj = *it;  --itj; if (it==ite) break; --it; }
	}
	*itj = a;
	break;
      case 3 : std::swap(iti->e, itj->e);
	       break;
      }
    }
  }

  template <typename T> void rsvector<T>::sup(size_type j) {
    if (nb_stored() != 0) {
      elt_rsvector_<T> ev(j);
      iterator it = std::lower_bound(this->begin(), this->end(), ev);
      if (it != this->end() && it->c == j) {
	for (iterator ite = this->end() - 1; it != ite; ++it) *it = *(it+1);
	base_type_::resize(nb_stored()-1);
      }
    }
  }

  template<typename T>  void rsvector<T>::resize(size_type n) {
    if (n < nbl) {
      for (size_type i = 0; i < nb_stored(); ++i)
	if (base_type_::operator[](i).c >= n) { base_resize(i); break; }
    }
    nbl = n;
  }

  template <typename T> void rsvector<T>::w(size_type c, const T &e) {
    GMM_ASSERT2(c < nbl, "out of range");
    if (e == T(0)) sup(c);
    else {
      elt_rsvector_<T> ev(c, e);
      if (nb_stored() == 0) {
	base_type_::resize(1,ev);
      }
      else {
	iterator it = std::lower_bound(this->begin(), this->end(), ev);
	if (it != this->end() && it->c == c) it->e = e;
	else {
	  size_type ind = it - this->begin();
	  base_type_::resize(nb_stored()+1, ev);
	  if (ind != nb_stored() - 1) {
	    it = this->begin() + ind;
	    for (iterator ite = this->end() - 1; ite != it; --ite)
	      *ite = *(ite-1);
	    *it = ev;
	  }
	}
      }
    }
  }
  
  template <typename T> T rsvector<T>::r(size_type c) const {
    GMM_ASSERT2(c < nbl, "out of range");
    if (nb_stored() != 0) {
      elt_rsvector_<T> ev(c);
      const_iterator it = std::lower_bound(this->begin(), this->end(), ev);
      if (it != this->end() && it->c == c) return it->e;
    }
    return T(0);
  }

  template <typename T> struct linalg_traits<rsvector<T> > {
    typedef rsvector<T> this_type;
    typedef this_type origin_type;
    typedef linalg_false is_reference;
    typedef abstract_vector linalg_type;
    typedef T value_type;
    typedef ref_elt_vector<T, rsvector<T> > reference;
    typedef rsvector_iterator<T>  iterator;
    typedef rsvector_const_iterator<T> const_iterator;
    typedef abstract_sparse storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v) { return iterator(v.begin()); }
    static const_iterator begin(const this_type &v)
    { return const_iterator(v.begin()); }
    static iterator end(this_type &v) { return iterator(v.end()); }
    static const_iterator end(const this_type &v)
      { return const_iterator(v.end()); }
    static origin_type* origin(this_type &v) { return &v; }
    static const origin_type* origin(const this_type &v) { return &v; }
    static void clear(origin_type* o, const iterator &, const iterator &)
    { o->clear(); }
    static void do_clear(this_type &v) { v.clear(); }
    static value_type access(const origin_type *o, const const_iterator &,
			     const const_iterator &, size_type i)
    { return (*o)[i]; }
    static reference access(origin_type *o, const iterator &, const iterator &,
			    size_type i)
    { return (*o)[i]; }
    static void resize(this_type &v, size_type n) { v.resize(n); }
  };

  template<typename T> std::ostream &operator <<
  (std::ostream &o, const rsvector<T>& v) { gmm::write(o,v); return o; }

  /******* Optimized operations for rsvector<T> ****************************/

  template <typename T> inline void copy(const rsvector<T> &v1,
 					 rsvector<T> &v2) {
    GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
    v2 = v1;
  }
  template <typename T> inline
  void copy(const rsvector<T> &v1, const simple_vector_ref<rsvector<T> *> &v2){
    simple_vector_ref<rsvector<T> *>
      *svr = const_cast<simple_vector_ref<rsvector<T> *> *>(&v2);
    rsvector<T>
      *pv = const_cast<rsvector<T> *>((v2.origin));
    GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
    *pv = v1; svr->begin_ = vect_begin(*pv); svr->end_ = vect_end(*pv);
  }
  template <typename T> inline
  void copy(const simple_vector_ref<const rsvector<T> *> &v1,
	    rsvector<T> &v2)
  { copy(*(v1.origin), v2); }
  template <typename T> inline
  void copy(const simple_vector_ref<rsvector<T> *> &v1, rsvector<T> &v2)
  { copy(*(v1.origin), v2); }

  template <typename V, typename T> inline void add(const V &v1,
						    rsvector<T> &v2) {
    if ((const void *)(&v1) != (const void *)(&v2)) {
      GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
	add_rsvector(v1, v2, typename linalg_traits<V>::storage_type());
    }
  }

  template <typename V, typename T> 
  inline void add_rsvector(const V &v1, rsvector<T> &v2, abstract_dense)
  { add(v1, v2, abstract_dense(), abstract_sparse()); }

  template <typename V, typename T> 
  inline void add_rsvector(const V &v1, rsvector<T> &v2, abstract_skyline)
  { add(v1, v2, abstract_skyline(), abstract_sparse()); }

  template <typename V, typename T> 
  void add_rsvector(const V &v1, rsvector<T> &v2, abstract_sparse) {
    add_rsvector(v1, v2, typename linalg_traits<V>::index_sorted());
  }

  template <typename V, typename T> 
  void add_rsvector(const V &v1, rsvector<T> &v2, linalg_false) {
    add(v1, v2, abstract_sparse(), abstract_sparse());
  }

  template <typename V, typename T> 
  void add_rsvector(const V &v1, rsvector<T> &v2, linalg_true) {
    typename linalg_traits<V>::const_iterator it1 = vect_const_begin(v1),
      ite1 = vect_const_end(v1);
    typename rsvector<T>::iterator it2 = v2.begin(), ite2 = v2.end(), it3;
    size_type nbc = 0, old_nbc = v2.nb_stored();
    for (; it1 != ite1 && it2 != ite2 ; ++nbc)
      if (it1.index() == it2->c) { ++it1; ++it2; }
      else if (it1.index() < it2->c) ++it1; else ++it2;
    for (; it1 != ite1; ++it1) ++nbc;
    for (; it2 != ite2; ++it2) ++nbc;

    v2.base_resize(nbc);
    it3 = v2.begin() + old_nbc;
    it2 = v2.end(); ite2 = v2.begin();
    it1 = vect_end(v1); ite1 = vect_const_begin(v1);
    while (it1 != ite1 && it3 != ite2) {
      --it3; --it1; --it2;
      if (it3->c > it1.index()) { *it2 = *it3; ++it1; }
      else if (it3->c == it1.index()) { *it2=*it3; it2->e+=*it1; }
      else { it2->c = it1.index(); it2->e = *it1; ++it3; }
    }
    while (it1 != ite1) { --it1; --it2; it2->c = it1.index(); it2->e = *it1; }
  }

  template <typename V, typename T> void copy(const V &v1, rsvector<T> &v2) {
    if ((const void *)(&v1) != (const void *)(&v2)) {
      GMM_ASSERT2(vect_size(v1) == vect_size(v2), "dimensions mismatch");
      if (same_origin(v1, v2))
	GMM_WARNING2("a conflict is possible in vector copy\n");
      copy_rsvector(v1, v2, typename linalg_traits<V>::storage_type());
    }
  }

  template <typename V, typename T> 
  void copy_rsvector(const V &v1, rsvector<T> &v2, abstract_dense)
  { copy_vect(v1, v2, abstract_dense(), abstract_sparse()); }

  template <typename V, typename T> 
  void copy_rsvector(const V &v1, rsvector<T> &v2, abstract_skyline)
  { copy_vect(v1, v2, abstract_skyline(), abstract_sparse()); }

  template <typename V, typename T>
  void copy_rsvector(const V &v1, rsvector<T> &v2, abstract_sparse) {
    copy_rsvector(v1, v2, typename linalg_traits<V>::index_sorted());
  }
  
  template <typename V, typename T2>
  void copy_rsvector(const V &v1, rsvector<T2> &v2, linalg_true) {
    typedef typename linalg_traits<V>::value_type T1;
    typename linalg_traits<V>::const_iterator it = vect_const_begin(v1),
      ite = vect_const_end(v1);
    v2.base_resize(nnz(v1));
    typename rsvector<T2>::iterator it2 = v2.begin();
    size_type nn = 0;
    for (; it != ite; ++it)
      if ((*it) != T1(0)) { it2->c = it.index(); it2->e = *it; ++it2; ++nn; }
    v2.base_resize(nn);
  }

  template <typename V, typename T2>
  void copy_rsvector(const V &v1, rsvector<T2> &v2, linalg_false) {
    typedef typename linalg_traits<V>::value_type T1;
    typename linalg_traits<V>::const_iterator it = vect_const_begin(v1),
      ite = vect_const_end(v1);
    v2.base_resize(nnz(v1));
    typename rsvector<T2>::iterator it2 = v2.begin();
    size_type nn = 0;
    for (; it != ite; ++it)
      if ((*it) != T1(0)) { it2->c = it.index(); it2->e = *it; ++it2; ++nn; }
    v2.base_resize(nn);
    std::sort(v2.begin(), v2.end());
  }
  
  template <typename T> inline void clean(rsvector<T> &v, double eps) {
    typedef typename number_traits<T>::magnitude_type R;
    typename rsvector<T>::iterator it = v.begin(), ite = v.end();
    for (; it != ite; ++it) if (gmm::abs((*it).e) <= eps) break;
    if (it != ite) {
      typename rsvector<T>::iterator itc = it;
      size_type erased = 1;
      for (++it; it != ite; ++it)
	{ *itc = *it; if (gmm::abs((*it).e) <= R(eps)) ++erased; else ++itc; }
      v.base_resize(v.nb_stored() - erased);
    }
  }

  template <typename T>
  inline void clean(const simple_vector_ref<rsvector<T> *> &l, double eps) {
    simple_vector_ref<rsvector<T> *>
      *svr = const_cast<simple_vector_ref<rsvector<T> *> *>(&l);
    rsvector<T>
      *pv = const_cast<rsvector<T> *>((l.origin));
    clean(*pv, eps);
    svr->begin_ = vect_begin(*pv); svr->end_ = vect_end(*pv);
  }
  
  template <typename T>
  inline size_type nnz(const rsvector<T>& l) { return l.nb_stored(); }

  /*************************************************************************/
  /*                                                                       */
  /* Class slvector: 'sky-line' vector.                                    */
  /*                                                                       */
  /*************************************************************************/

  template<typename T> struct slvector_iterator {
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef size_t size_type;
    typedef slvector_iterator<T> iterator;
    typedef typename std::vector<T>::iterator base_iterator;

    base_iterator it;
    size_type shift;
    
   
    iterator &operator ++()
    { ++it; ++shift; return *this; }
    iterator &operator --()
    { --it; --shift; return *this; }
    iterator operator ++(int)
    { iterator tmp = *this; ++(*(this)); return tmp; }
    iterator operator --(int)
    { iterator tmp = *this; --(*(this)); return tmp; }
    iterator &operator +=(difference_type i)
    { it += i; shift += i; return *this; }
    iterator &operator -=(difference_type i)
    { it -= i; shift -= i; return *this; }
    iterator operator +(difference_type i) const
    { iterator tmp = *this; return (tmp += i); }
    iterator operator -(difference_type i) const
    { iterator tmp = *this; return (tmp -= i); }
    difference_type operator -(const iterator &i) const
    { return it - i.it; }
	
    reference operator *() const
    { return *it; }
    reference operator [](int ii)
    { return *(it + ii); }
    
    bool operator ==(const iterator &i) const
    { return it == i.it; }
    bool operator !=(const iterator &i) const
    { return !(i == *this); }
    bool operator < (const iterator &i) const
    { return it < i.it; }
    size_type index(void) const { return shift; }

    slvector_iterator(void) {}
    slvector_iterator(const base_iterator &iter, size_type s)
      : it(iter), shift(s) {}
  };

  template<typename T> struct slvector_const_iterator {
    typedef T value_type;
    typedef const T *pointer;
    typedef value_type reference;
    typedef ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef size_t size_type;
    typedef slvector_const_iterator<T> iterator;
    typedef typename std::vector<T>::const_iterator base_iterator;

    base_iterator it;
    size_type shift;
    
   
    iterator &operator ++()
    { ++it; ++shift; return *this; }
    iterator &operator --()
    { --it; --shift; return *this; }
    iterator operator ++(int)
    { iterator tmp = *this; ++(*(this)); return tmp; }
    iterator operator --(int)
    { iterator tmp = *this; --(*(this)); return tmp; }
    iterator &operator +=(difference_type i)
    { it += i; shift += i; return *this; }
    iterator &operator -=(difference_type i)
    { it -= i; shift -= i; return *this; }
    iterator operator +(difference_type i) const
    { iterator tmp = *this; return (tmp += i); }
    iterator operator -(difference_type i) const
    { iterator tmp = *this; return (tmp -= i); }
    difference_type operator -(const iterator &i) const
    { return it - i.it; }
	
    value_type operator *() const
    { return *it; }
    value_type operator [](int ii)
    { return *(it + ii); }
    
    bool operator ==(const iterator &i) const
    { return it == i.it; }
    bool operator !=(const iterator &i) const
    { return !(i == *this); }
    bool operator < (const iterator &i) const
    { return it < i.it; }
    size_type index(void) const { return shift; }

    slvector_const_iterator(void) {}
    slvector_const_iterator(const slvector_iterator<T>& iter)
      : it(iter.it), shift(iter.shift) {}
    slvector_const_iterator(const base_iterator &iter, size_type s)
      : it(iter), shift(s) {}
  };


  /** skyline vector.
   */
  template <typename T> class slvector {
    
  public :
    typedef slvector_iterator<T> iterators;
    typedef slvector_const_iterator<T> const_iterators;
    typedef typename std::vector<T>::size_type size_type;
    typedef T value_type;

  protected :
    std::vector<T> data;
    size_type shift;
    size_type size_;


  public :

    size_type size(void) const { return size_; }
    size_type first(void) const { return shift; }
    size_type last(void) const { return shift + data.size(); }
    ref_elt_vector<T, slvector<T> > operator [](size_type c)
    { return ref_elt_vector<T, slvector<T> >(this, c); }

    typename std::vector<T>::iterator data_begin(void) { return data.begin(); }
    typename std::vector<T>::iterator data_end(void) { return data.end(); }
    typename std::vector<T>::const_iterator data_begin(void) const
      { return data.begin(); }
    typename std::vector<T>::const_iterator data_end(void) const
      { return data.end(); }

    void w(size_type c, const T &e);
    T r(size_type c) const {
      GMM_ASSERT2(c < size_, "out of range");
      if (c < shift || c >= shift + data.size()) return T(0);
      return data[c - shift];
    }

    inline T operator [](size_type c) const { return r(c); }
    void resize(size_type);
    void clear(void) { data.resize(0); shift = 0; }
    void swap(slvector<T> &v) {
      std::swap(data, v.data);
      std::swap(shift, v.shift);
      std::swap(size_, v.size_);
    }


    slvector(void) : data(0), shift(0), size_(0) {}
    explicit slvector(size_type l) : data(0), shift(0), size_(l) {}
    slvector(size_type l, size_type d, size_type s)
      : data(d), shift(s), size_(l) {}

  };

  template<typename T>  void slvector<T>::resize(size_type n) {
    if (n < last()) {
      if (shift >= n) clear(); else { data.resize(n-shift); }
    }
    size_ = n;
  }

  template<typename T>  void slvector<T>::w(size_type c, const T &e) {
    GMM_ASSERT2(c < size_, "out of range");
    size_type s = data.size();
    if (!s) { data.resize(1); shift = c; }
    else if (c < shift) {
      data.resize(s + shift - c); 
      typename std::vector<T>::iterator it = data.begin(),it2=data.end()-1;
      typename std::vector<T>::iterator it3 = it2 - shift + c;
      for (; it3 >= it; --it3, --it2) *it2 = *it3;
      std::fill(it, it + shift - c, T(0));
      shift = c;
    }
    else if (c >= shift + s) {
      data.resize(c - shift + 1);
      std::fill(data.begin() + s, data.end(), T(0));
    }
    data[c - shift] = e;
  }
  
  template <typename T> struct linalg_traits<slvector<T> > {
    typedef slvector<T> this_type;
    typedef this_type origin_type;
    typedef linalg_false is_reference;
    typedef abstract_vector linalg_type;
    typedef T value_type;
    typedef ref_elt_vector<T, slvector<T> > reference;
    typedef slvector_iterator<T>  iterator;
    typedef slvector_const_iterator<T> const_iterator;
    typedef abstract_skyline storage_type;
    typedef linalg_true index_sorted;
    static size_type size(const this_type &v) { return v.size(); }
    static iterator begin(this_type &v)
      { return iterator(v.data_begin(), v.first()); }
    static const_iterator begin(const this_type &v)
      { return const_iterator(v.data_begin(), v.first()); }
    static iterator end(this_type &v)
      { return iterator(v.data_end(), v.last()); }
    static const_iterator end(const this_type &v)
      { return const_iterator(v.data_end(), v.last()); }
    static origin_type* origin(this_type &v) { return &v; }
    static const origin_type* origin(const this_type &v) { return &v; }
    static void clear(origin_type* o, const iterator &, const iterator &)
    { o->clear(); }
    static void do_clear(this_type &v) { v.clear(); }
    static value_type access(const origin_type *o, const const_iterator &,
			     const const_iterator &, size_type i)
    { return (*o)[i]; }
    static reference access(origin_type *o, const iterator &, const iterator &,
			    size_type i)
    { return (*o)[i]; }
    static void resize(this_type &v, size_type n) { v.resize(n); }
  };

  template<typename T> std::ostream &operator <<
  (std::ostream &o, const slvector<T>& v) { gmm::write(o,v); return o; }

  template <typename T>
  inline size_type nnz(const slvector<T>& l) { return l.last() - l.first(); }

}

namespace std {
  template <typename T> void swap(gmm::wsvector<T> &v, gmm::wsvector<T> &w)
  { v.swap(w);}
  template <typename T> void swap(gmm::rsvector<T> &v, gmm::rsvector<T> &w)
  { v.swap(w);}
  template <typename T> void swap(gmm::slvector<T> &v, gmm::slvector<T> &w)
  { v.swap(w);}
}



#endif /* GMM_VECTOR_H__ */
