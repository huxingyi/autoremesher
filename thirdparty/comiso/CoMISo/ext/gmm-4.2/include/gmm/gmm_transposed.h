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

/**@file gmm_transposed.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date November 10, 2002.
   @brief Generic transposed matrices
*/
#ifndef GMM_TRANSPOSED_H__
#define GMM_TRANSPOSED_H__

#include "gmm_def.h"

namespace gmm {

  /* ********************************************************************* */
  /*		transposed reference                    		   */
  /* ********************************************************************* */
  
  template <typename PT> struct  transposed_row_ref {
    
    typedef transposed_row_ref<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<this_type>
            ::const_col_iterator, typename linalg_traits<this_type>
            ::col_iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    iterator begin_, end_;
    porigin_type origin;
    size_type nr, nc;

    transposed_row_ref(ref_M m)
      : begin_(mat_row_begin(m)), end_(mat_row_end(m)),
	origin(linalg_origin(m)), nr(mat_ncols(m)), nc(mat_nrows(m)) {}

    transposed_row_ref(const transposed_row_ref<CPT> &cr) :
      begin_(cr.begin_),end_(cr.end_), origin(cr.origin),nr(cr.nr),nc(cr.nc) {}

    reference operator()(size_type i, size_type j) const
    { return linalg_traits<M>::access(begin_+j, i); }
  };

  template <typename PT> struct linalg_traits<transposed_row_ref<PT> > {
    typedef transposed_row_ref<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<M>::reference, PT>::ref_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_row_iterator;
    typedef typename linalg_traits<M>::const_sub_row_type const_sub_col_type;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::sub_row_type, PT>::ref_type sub_col_type;
    typedef typename linalg_traits<M>::const_row_iterator const_col_iterator;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::row_iterator, PT>::ref_type col_iterator;
    typedef col_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type ncols(const this_type &v) { return v.nc; }
    static size_type nrows(const this_type &v) { return v.nr; }
    static const_sub_col_type col(const const_col_iterator &it)
    { return linalg_traits<M>::row(it); }
    static sub_col_type col(const col_iterator &it)
    { return linalg_traits<M>::row(it); }
    static col_iterator col_begin(this_type &m) { return m.begin_; }
    static col_iterator col_end(this_type &m) { return m.end_; }
    static const_col_iterator col_begin(const this_type &m)
    { return m.begin_; }
    static const_col_iterator col_end(const this_type &m) { return m.end_; }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &v);
    static value_type access(const const_col_iterator &itcol, size_type i)
    { return linalg_traits<M>::access(itcol, i); }
    static reference access(const col_iterator &itcol, size_type i)
    { return linalg_traits<M>::access(itcol, i); }
  };
  
  template <typename PT> 
  void linalg_traits<transposed_row_ref<PT> >::do_clear(this_type &v) { 
    col_iterator it = mat_col_begin(v), ite = mat_col_end(v);
    for (; it != ite; ++it) clear(col(it));
  }
  
  template<typename PT> std::ostream &operator <<
  (std::ostream &o, const transposed_row_ref<PT>& m)
  { gmm::write(o,m); return o; }

  template <typename PT> struct  transposed_col_ref {
    
    typedef transposed_col_ref<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<this_type>
            ::const_row_iterator, typename linalg_traits<this_type>
            ::row_iterator, PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;
    
    iterator begin_, end_;
    porigin_type origin;
    size_type nr, nc;

    transposed_col_ref(ref_M m)
      : begin_(mat_col_begin(m)), end_(mat_col_end(m)),
	origin(linalg_origin(m)), nr(mat_ncols(m)), nc(mat_nrows(m)) {}

    transposed_col_ref(const transposed_col_ref<CPT> &cr) :
      begin_(cr.begin_),end_(cr.end_), origin(cr.origin),nr(cr.nr),nc(cr.nc) {}

    reference operator()(size_type i, size_type j) const
    { return linalg_traits<M>::access(begin_+i, j); }
  };

  template <typename PT> struct linalg_traits<transposed_col_ref<PT> > {
    typedef transposed_col_ref<PT> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<M>::reference, PT>::ref_type reference;
    typedef typename linalg_traits<M>::storage_type storage_type;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type const_col_iterator;
    typedef typename linalg_traits<M>::const_sub_col_type const_sub_row_type;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::sub_col_type, PT>::ref_type sub_row_type;
    typedef typename linalg_traits<M>::const_col_iterator const_row_iterator;
    typedef typename select_ref<abstract_null_type, typename
            linalg_traits<M>::col_iterator, PT>::ref_type row_iterator;
    typedef row_major sub_orientation;
    typedef typename linalg_traits<M>::index_sorted index_sorted;
    static size_type nrows(const this_type &v)
    { return v.nr; }
    static size_type ncols(const this_type &v)
    { return v.nc; }
    static const_sub_row_type row(const const_row_iterator &it)
    { return linalg_traits<M>::col(it); }
    static sub_row_type row(const row_iterator &it)
    { return linalg_traits<M>::col(it); }
    static row_iterator row_begin(this_type &m) { return m.begin_; }
    static row_iterator row_end(this_type &m) { return m.end_; }
    static const_row_iterator row_begin(const this_type &m)
    { return m.begin_; }
    static const_row_iterator row_end(const this_type &m) { return m.end_; }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &m);
    static value_type access(const const_row_iterator &itrow, size_type i)
    { return linalg_traits<M>::access(itrow, i); }
    static reference access(const row_iterator &itrow, size_type i)
    { return linalg_traits<M>::access(itrow, i); }
  };

  template <typename PT> 
  void linalg_traits<transposed_col_ref<PT> >::do_clear(this_type &v) { 
    row_iterator it = mat_row_begin(v), ite = mat_row_end(v);
    for (; it != ite; ++it) clear(row(it));
  }

  template<typename PT> std::ostream &operator <<
  (std::ostream &o, const transposed_col_ref<PT>& m)
  { gmm::write(o,m); return o; }

  template <typename TYPE, typename PT> struct transposed_return_ {
    typedef abstract_null_type return_type;
  };
  template <typename PT> struct transposed_return_<row_major, PT> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return<transposed_row_ref<const L *>,
            transposed_row_ref< L *>, PT>::return_type return_type;
  };
  template <typename PT> struct transposed_return_<col_major, PT> {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename select_return<transposed_col_ref<const L *>,
            transposed_col_ref< L *>, PT>::return_type return_type;
  };
  template <typename PT> struct transposed_return {
    typedef typename std::iterator_traits<PT>::value_type L;
    typedef typename transposed_return_<typename principal_orientation_type<
            typename linalg_traits<L>::sub_orientation>::potype,
	    PT>::return_type return_type;
  };

  template <typename L> inline 
  typename transposed_return<const L *>::return_type transposed(const L &l) {
    return typename transposed_return<const L *>::return_type
      (linalg_cast(const_cast<L &>(l)));
  }

  template <typename L> inline 
  typename transposed_return<L *>::return_type transposed(L &l)
  { return typename transposed_return<L *>::return_type(linalg_cast(l)); }

}

#endif //  GMM_TRANSPOSED_H__
