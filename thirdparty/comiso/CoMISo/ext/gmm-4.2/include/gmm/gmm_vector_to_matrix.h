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

/**@file gmm_vector_to_matrix.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date December 6, 2003.
   @brief View vectors as row or column matrices. */
#ifndef GMM_VECTOR_TO_MATRIX_H__
#define GMM_VECTOR_TO_MATRIX_H__

#include "gmm_interface.h"

namespace gmm {

  /* ********************************************************************* */
  /*	     row vector -> transform a vector in a (1, n) matrix.          */
  /* ********************************************************************* */

  template <typename PT> struct gen_row_vector {
    typedef gen_row_vector<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_V;
    typedef typename linalg_traits<this_type>::reference reference;

    simple_vector_ref<PT> vec;
    
    reference operator()(size_type, size_type j) const { return vec[j]; }
   
    size_type nrows(void) const { return 1; }
    size_type ncols(void) const { return vect_size(vec); }
    
    gen_row_vector(ref_V v) : vec(v) {}
    gen_row_vector() {}
    gen_row_vector(const gen_row_vector<CPT> &cr) : vec(cr.vec) {}
  };

  template <typename PT>
  struct gen_row_vector_iterator {
    typedef gen_row_vector<PT> this_type;
    typedef typename modifiable_pointer<PT>::pointer MPT;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef simple_vector_ref<PT> value_type;
    typedef const simple_vector_ref<PT> *pointer;
    typedef const simple_vector_ref<PT> &reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef std::random_access_iterator_tag  iterator_category;
    typedef gen_row_vector_iterator<PT> iterator;

    simple_vector_ref<PT> vec;
    bool isend;
    
    iterator &operator ++()   { isend = true; return *this; }
    iterator &operator --()   { isend = false; return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }
    iterator &operator +=(difference_type i)
    { if (i) isend = false; return *this; }
    iterator &operator -=(difference_type i)
    { if (i) isend = true; return *this;  }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const { 
      return (isend == true) ? ((i.isend == true) ? 0 : 1)
	                     : ((i.isend == true) ? -1 : 0);
    }

    const simple_vector_ref<PT>& operator *() const { return vec; }
    const simple_vector_ref<PT>& operator [](int i) { return vec; }

    bool operator ==(const iterator &i) const { return (isend == i.isend); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (*this - i < 0); }

    gen_row_vector_iterator(void) {}
    gen_row_vector_iterator(const gen_row_vector_iterator<MPT> &itm)
      : vec(itm.vec), isend(itm.isend) {}
    gen_row_vector_iterator(const gen_row_vector<PT> &m, bool iis_end)
      : vec(m.vec), isend(iis_end) { }
    
  };

  template <typename PT>
  struct linalg_traits<gen_row_vector<PT> > {
    typedef gen_row_vector<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<V>::reference, PT>::ref_type reference;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type const_col_iterator;
    typedef simple_vector_ref<const V *> const_sub_row_type;
    typedef typename select_ref<abstract_null_type, 
            simple_vector_ref<V *>, PT>::ref_type sub_row_type;
    typedef gen_row_vector_iterator<typename const_pointer<PT>::pointer>
            const_row_iterator;
    typedef typename select_ref<abstract_null_type, 
	    gen_row_vector_iterator<PT>, PT>::ref_type row_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    typedef row_major sub_orientation;
    typedef typename linalg_traits<V>::index_sorted index_sorted;
    static size_type nrows(const this_type &) { return 1; }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_sub_row_type row(const const_row_iterator &it) { return *it; }
    static sub_row_type row(const row_iterator &it) { return *it; }
    static const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m, false); }
    static row_iterator row_begin(this_type &m)
    { return row_iterator(m, false); }
    static const_row_iterator row_end(const this_type &m)
    { return const_row_iterator(m, true); }
    static row_iterator row_end(this_type &m)
    { return row_iterator(m, true); }
    static origin_type* origin(this_type &m) { return m.vec.origin; }
    static const origin_type* origin(const this_type &m)
    { return m.vec.origin; }
    static void do_clear(this_type &m)
    { clear(row(mat_row_begin(m))); }
    static value_type access(const const_row_iterator &itrow, size_type i)
    { return itrow.vec[i]; }
    static reference access(const row_iterator &itrow, size_type i)
    { return itrow.vec[i]; }
  };
  
  template <typename PT>
  std::ostream &operator <<(std::ostream &o, const gen_row_vector<PT>& m)
  { gmm::write(o,m); return o; }

  /* ********************************************************************* */
  /*	     col vector -> transform a vector in a (n, 1) matrix.          */
  /* ********************************************************************* */

  template <typename PT> struct gen_col_vector {
    typedef gen_col_vector<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef V * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_V;
    typedef typename linalg_traits<this_type>::reference reference;

    simple_vector_ref<PT> vec;
    
    reference operator()(size_type i, size_type) const { return vec[i]; }
   
    size_type ncols(void) const { return 1; }
    size_type nrows(void) const { return vect_size(vec); }
    
    gen_col_vector(ref_V v) : vec(v) {}
    gen_col_vector() {}
    gen_col_vector(const gen_col_vector<CPT> &cr) : vec(cr.vec) {}
  };

  template <typename PT>
  struct gen_col_vector_iterator {
    typedef gen_col_vector<PT> this_type;
    typedef typename modifiable_pointer<PT>::pointer MPT;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef simple_vector_ref<PT> value_type;
    typedef const simple_vector_ref<PT> *pointer;
    typedef const simple_vector_ref<PT> &reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef std::random_access_iterator_tag  iterator_category;
    typedef gen_col_vector_iterator<PT> iterator;

    simple_vector_ref<PT> vec;
    bool isend;
    
    iterator &operator ++()   { isend = true; return *this; }
    iterator &operator --()   { isend = false; return *this; }
    iterator operator ++(int) { iterator tmp = *this; ++(*this); return tmp; }
    iterator operator --(int) { iterator tmp = *this; --(*this); return tmp; }
    iterator &operator +=(difference_type i)
    { if (i) isend = false; return *this; }
    iterator &operator -=(difference_type i)
    { if (i) isend = true; return *this;  }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const { 
      return (isend == true) ? ((i.isend == true) ? 0 : 1)
	                     : ((i.isend == true) ? -1 : 0);
    }

    const simple_vector_ref<PT>& operator *() const { return vec; }
    const simple_vector_ref<PT>& operator [](int i) { return vec; }

    bool operator ==(const iterator &i) const { return (isend == i.isend); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (*this - i < 0); }

    gen_col_vector_iterator(void) {}
    gen_col_vector_iterator(const gen_col_vector_iterator<MPT> &itm)
      : vec(itm.vec), isend(itm.isend) {}
    gen_col_vector_iterator(const gen_col_vector<PT> &m, bool iis_end)
      : vec(m.vec), isend(iis_end) { }
    
  };

  template <typename PT>
  struct linalg_traits<gen_col_vector<PT> > {
    typedef gen_col_vector<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type V;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<V>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename linalg_traits<V>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<V>::reference, PT>::ref_type reference;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type const_row_iterator;
    typedef simple_vector_ref<const V *> const_sub_col_type;
    typedef typename select_ref<abstract_null_type, 
            simple_vector_ref<V *>, PT>::ref_type sub_col_type;
    typedef gen_col_vector_iterator<typename const_pointer<PT>::pointer>
            const_col_iterator;
    typedef typename select_ref<abstract_null_type, 
	    gen_col_vector_iterator<PT>, PT>::ref_type col_iterator;
    typedef typename linalg_traits<V>::storage_type storage_type;
    typedef col_major sub_orientation;
    typedef typename linalg_traits<V>::index_sorted index_sorted;
    static size_type ncols(const this_type &) { return 1; }
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static const_sub_col_type col(const const_col_iterator &it) { return *it; }
    static sub_col_type col(const col_iterator &it) { return *it; }
    static const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m, false); }
    static col_iterator col_begin(this_type &m)
    { return col_iterator(m, false); }
    static const_col_iterator col_end(const this_type &m)
    { return const_col_iterator(m, true); }
    static col_iterator col_end(this_type &m)
    { return col_iterator(m, true); }
    static origin_type* origin(this_type &m) { return m.vec.origin; }
    static const origin_type* origin(const this_type &m)
    { return m.vec.origin; }
    static void do_clear(this_type &m)
    { clear(col(mat_col_begin(m))); }
    static value_type access(const const_col_iterator &itcol, size_type i)
    { return itcol.vec[i]; }
    static reference access(const col_iterator &itcol, size_type i)
    { return itcol.vec[i]; }
  };
  
  template <typename PT>
  std::ostream &operator <<(std::ostream &o, const gen_col_vector<PT>& m)
  { gmm::write(o,m); return o; }

  /* ******************************************************************** */
  /*		col and row vectors                                       */
  /* ******************************************************************** */

  
  template <class V> inline
  typename select_return< gen_row_vector<const V *>, gen_row_vector<V *>,
			  const V *>::return_type
  row_vector(const V& v) {
    return typename select_return< gen_row_vector<const V *>,
      gen_row_vector<V *>, const V *>::return_type(linalg_cast(v));
  }

  template <class V> inline
  typename select_return< gen_row_vector<const V *>, gen_row_vector<V *>,
			  V *>::return_type
  row_vector(V& v) {
    return typename select_return< gen_row_vector<const V *>,
      gen_row_vector<V *>, V *>::return_type(linalg_cast(v));
  }
 
  template <class V> inline gen_row_vector<const V *>
  const_row_vector(V& v)
  { return gen_row_vector<const V *>(v); }
 

  template <class V> inline
  typename select_return< gen_col_vector<const V *>, gen_col_vector<V *>,
			  const V *>::return_type
  col_vector(const V& v) {
    return typename select_return< gen_col_vector<const V *>,
      gen_col_vector<V *>, const V *>::return_type(linalg_cast(v));
  }

  template <class V> inline
  typename select_return< gen_col_vector<const V *>, gen_col_vector<V *>,
			  V *>::return_type
  col_vector(V& v) {
    return typename select_return< gen_col_vector<const V *>,
      gen_col_vector<V *>, V *>::return_type(linalg_cast(v));
  }
 
  template <class V> inline gen_col_vector<const V *>
  const_col_vector(V& v)
  { return gen_col_vector<const V *>(v); }
 

}

#endif //  GMM_VECTOR_TO_MATRIX_H__
