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

/**@file gmm_sub_matrix.h
   @author Yves Renard <Yves.Renard@insa-lyon.fr>
   @date October 13, 2002.
   @brief Generic sub-matrices.
*/

#ifndef GMM_SUB_MATRIX_H__
#define GMM_SUB_MATRIX_H__

#include "gmm_sub_vector.h"

namespace gmm {

  /* ********************************************************************* */
  /*		sub row matrices type                                      */
  /* ********************************************************************* */

  template <typename PT, typename SUBI1, typename SUBI2>
  struct gen_sub_row_matrix {
    typedef gen_sub_row_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<M>
            ::const_row_iterator, typename linalg_traits<M>::row_iterator,
	    PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    SUBI1 si1;
    SUBI2 si2;
    iterator begin_;
    porigin_type origin;
    
    reference operator()(size_type i, size_type j) const 
    { return linalg_traits<M>::access(begin_ + si1.index(i), si2.index(j)); }
   
    size_type nrows(void) const { return si1.size(); }
    size_type ncols(void) const { return si2.size(); }
    
    gen_sub_row_matrix(ref_M m, const SUBI1 &s1, const SUBI2 &s2)
      : si1(s1), si2(s2), begin_(mat_row_begin(m)),
	origin(linalg_origin(m)) {}
    gen_sub_row_matrix() {}
    gen_sub_row_matrix(const gen_sub_row_matrix<CPT, SUBI1, SUBI2> &cr) :
      si1(cr.si1), si2(cr.si2), begin_(cr.begin_),origin(cr.origin) {}
  };

  template <typename PT, typename SUBI1, typename SUBI2>
  struct gen_sub_row_matrix_iterator {
    typedef gen_sub_row_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename modifiable_pointer<PT>::pointer MPT;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename select_ref<typename linalg_traits<M>
            ::const_row_iterator, typename linalg_traits<M>::row_iterator,
	    PT>::ref_type ITER;
    typedef ITER value_type;
    typedef ITER *pointer;
    typedef ITER &reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef std::random_access_iterator_tag  iterator_category;
    typedef gen_sub_row_matrix_iterator<PT, SUBI1, SUBI2> iterator;

    ITER it;
    SUBI1 si1;
    SUBI2 si2;
    size_type ii;
    
    iterator operator ++(int) { iterator tmp = *this; ii++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; ii--; return tmp; }
    iterator &operator ++()   { ii++; return *this; }
    iterator &operator --()   { ii--; return *this; }
    iterator &operator +=(difference_type i) { ii += i; return *this; }
    iterator &operator -=(difference_type i) { ii -= i; return *this; }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const { return ii - i.ii; }

    ITER operator *() const { return it + si1.index(ii); }
    ITER operator [](int i) { return it + si1.index(ii+i); }

    bool operator ==(const iterator &i) const { return (ii == i.ii); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (ii < i.ii); }

    gen_sub_row_matrix_iterator(void) {}
    gen_sub_row_matrix_iterator(const 
	     gen_sub_row_matrix_iterator<MPT, SUBI1, SUBI2> &itm)
      : it(itm.it), si1(itm.si1), si2(itm.si2), ii(itm.ii) {}
    gen_sub_row_matrix_iterator(const ITER &iter, const SUBI1 &s1,
				const SUBI2 &s2, size_type i)
      : it(iter), si1(s1), si2(s2), ii(i) { }
    
  };

  template <typename PT, typename SUBI1, typename SUBI2>
  struct linalg_traits<gen_sub_row_matrix<PT, SUBI1, SUBI2> > {
    typedef gen_sub_row_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
				PT>::ref_type porigin_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<M>::reference, PT>::ref_type reference;
    typedef abstract_null_type sub_col_type;
    typedef abstract_null_type col_iterator;
    typedef abstract_null_type const_sub_col_type;
    typedef abstract_null_type const_col_iterator;
    typedef typename sub_vector_type<const typename
            linalg_traits<M>::const_sub_row_type *, SUBI2>::vector_type
            const_sub_row_type;
    typedef typename select_ref<abstract_null_type, 
            typename sub_vector_type<typename linalg_traits<M>::sub_row_type *,
	    SUBI2>::vector_type, PT>::ref_type sub_row_type;
    typedef gen_sub_row_matrix_iterator<typename const_pointer<PT>::pointer,
	    SUBI1, SUBI2> const_row_iterator;
    typedef typename select_ref<abstract_null_type, 
	    gen_sub_row_matrix_iterator<PT, SUBI1, SUBI2>, PT>::ref_type
            row_iterator;
    typedef typename linalg_traits<const_sub_row_type>::storage_type
            storage_type;
    typedef row_major sub_orientation;
    typedef linalg_true index_sorted;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_sub_row_type row(const const_row_iterator &it)
    { return const_sub_row_type(linalg_traits<M>::row(*it), it.si2); }
    static sub_row_type row(const row_iterator &it)
    { return sub_row_type(linalg_traits<M>::row(*it), it.si2); }
    static const_row_iterator row_begin(const this_type &m)
    { return const_row_iterator(m.begin_, m.si1, m.si2, 0); }
    static row_iterator row_begin(this_type &m)
    { return row_iterator(m.begin_, m.si1, m.si2, 0); }
    static const_row_iterator row_end(const this_type &m)
    { return const_row_iterator(m.begin_, m.si1, m.si2,  m.nrows()); }
    static row_iterator row_end(this_type &m)
    { return row_iterator(m.begin_, m.si1, m.si2, m.nrows()); }
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &m) {
      row_iterator it = mat_row_begin(m), ite = mat_row_end(m);
      for (; it != ite; ++it) clear(row(it));
    }
    static value_type access(const const_row_iterator &itrow, size_type i)
    { return linalg_traits<M>::access(*itrow, itrow.si2.index(i)); }
    static reference access(const row_iterator &itrow, size_type i)
    { return linalg_traits<M>::access(*itrow, itrow.si2.index(i)); }
  };
  
  template <typename PT, typename SUBI1, typename SUBI2>
  std::ostream &operator <<(std::ostream &o,
			    const gen_sub_row_matrix<PT, SUBI1, SUBI2>& m)
  { gmm::write(o,m); return o; }


  /* ********************************************************************* */
  /*		sub column matrices type                                   */
  /* ********************************************************************* */

  template <typename PT, typename SUBI1, typename SUBI2>
  struct gen_sub_col_matrix {
    typedef gen_sub_col_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef M * CPT;
    typedef typename std::iterator_traits<PT>::reference ref_M;
    typedef typename select_ref<typename linalg_traits<M>
            ::const_col_iterator, typename linalg_traits<M>::col_iterator,
	    PT>::ref_type iterator;
    typedef typename linalg_traits<this_type>::reference reference;
    typedef typename linalg_traits<this_type>::porigin_type porigin_type;

    SUBI1 si1;
    SUBI2 si2;
    iterator begin_;
    porigin_type origin;
    
    reference operator()(size_type i, size_type j) const
    { return linalg_traits<M>::access(begin_ + si2.index(j), si1.index(i)); }

    size_type nrows(void) const { return si1.size(); }
    size_type ncols(void) const { return si2.size(); }
    
    gen_sub_col_matrix(ref_M m, const SUBI1 &s1, const SUBI2 &s2)
      : si1(s1), si2(s2), begin_(mat_col_begin(m)),
        origin(linalg_origin(m)) {}
    gen_sub_col_matrix() {}
    gen_sub_col_matrix(const gen_sub_col_matrix<CPT, SUBI1, SUBI2> &cr) :
      si1(cr.si1), si2(cr.si2), begin_(cr.begin_),origin(cr.origin) {}
  };

  template <typename PT, typename SUBI1, typename SUBI2>
  struct gen_sub_col_matrix_iterator {
    typedef gen_sub_col_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename modifiable_pointer<PT>::pointer MPT;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename select_ref<typename linalg_traits<M>::const_col_iterator,
				typename linalg_traits<M>::col_iterator,
				PT>::ref_type ITER;
    typedef ITER value_type;
    typedef ITER *pointer;
    typedef ITER &reference;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef std::random_access_iterator_tag  iterator_category;
    typedef gen_sub_col_matrix_iterator<PT, SUBI1, SUBI2> iterator;

    ITER it;
    SUBI1 si1;
    SUBI2 si2;
    size_type ii;
    
    iterator operator ++(int) { iterator tmp = *this; ii++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; ii--; return tmp; }
    iterator &operator ++()   { ii++; return *this; }
    iterator &operator --()   { ii--; return *this; }
    iterator &operator +=(difference_type i) { ii += i; return *this; }
    iterator &operator -=(difference_type i) { ii -= i; return *this; }
    iterator operator +(difference_type i) const 
    { iterator itt = *this; return (itt += i); }
    iterator operator -(difference_type i) const
    { iterator itt = *this; return (itt -= i); }
    difference_type operator -(const iterator &i) const { return ii - i.ii; }

    ITER operator *() const { return it + si2.index(ii); }
    ITER operator [](int i) { return it + si2.index(ii+i); }

    bool operator ==(const iterator &i) const { return (ii == i.ii); }
    bool operator !=(const iterator &i) const { return !(i == *this); }
    bool operator < (const iterator &i) const { return (ii < i.ii); }

    gen_sub_col_matrix_iterator(void) {}
    gen_sub_col_matrix_iterator(const 
	gen_sub_col_matrix_iterator<MPT, SUBI1, SUBI2> &itm)
      : it(itm.it), si1(itm.si1), si2(itm.si2), ii(itm.ii) {}
    gen_sub_col_matrix_iterator(const ITER &iter, const SUBI1 &s1,
				const SUBI2 &s2, size_type i)
      : it(iter), si1(s1), si2(s2), ii(i) { }
  };

  template <typename PT, typename SUBI1, typename SUBI2>
  struct linalg_traits<gen_sub_col_matrix<PT, SUBI1, SUBI2> > {
    typedef gen_sub_col_matrix<PT, SUBI1, SUBI2> this_type;
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename linalg_traits<M>::origin_type origin_type;
    typedef typename select_ref<const origin_type *, origin_type *,
			        PT>::ref_type porigin_type;
    typedef typename which_reference<PT>::is_reference is_reference;
    typedef abstract_matrix linalg_type;
    typedef typename linalg_traits<M>::value_type value_type;
    typedef typename select_ref<value_type,
            typename linalg_traits<M>::reference, PT>::ref_type reference;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type const_row_iterator;
    typedef typename sub_vector_type<const typename
            linalg_traits<M>::const_sub_col_type *, SUBI1>::vector_type
            const_sub_col_type;
    typedef typename select_ref<abstract_null_type, 
            typename sub_vector_type<typename linalg_traits<M>::sub_col_type *,
	    SUBI1>::vector_type, PT>::ref_type sub_col_type;
    typedef gen_sub_col_matrix_iterator<typename const_pointer<PT>::pointer,
	    SUBI1, SUBI2> const_col_iterator;
    typedef typename select_ref<abstract_null_type, 
	    gen_sub_col_matrix_iterator<PT, SUBI1, SUBI2>, PT>::ref_type
            col_iterator;
    typedef col_major sub_orientation;
    typedef linalg_true index_sorted;
    typedef typename linalg_traits<const_sub_col_type>::storage_type
    storage_type;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_sub_col_type col(const const_col_iterator &it)
    { return const_sub_col_type(linalg_traits<M>::col(*it), it.si1); }
    static sub_col_type col(const col_iterator &it)
    { return sub_col_type(linalg_traits<M>::col(*it), it.si1); }
    static const_col_iterator col_begin(const this_type &m)
    { return const_col_iterator(m.begin_, m.si1, m.si2, 0); }
    static col_iterator col_begin(this_type &m)
    { return col_iterator(m.begin_, m.si1, m.si2, 0); }
    static const_col_iterator col_end(const this_type &m)
    { return const_col_iterator(m.begin_, m.si1, m.si2,  m.ncols()); }
    static col_iterator col_end(this_type &m)
    { return col_iterator(m.begin_, m.si1, m.si2, m.ncols()); } 
    static origin_type* origin(this_type &v) { return v.origin; }
    static const origin_type* origin(const this_type &v) { return v.origin; }
    static void do_clear(this_type &m) {
      col_iterator it = mat_col_begin(m), ite = mat_col_end(m);
      for (; it != ite; ++it) clear(col(it));
    }
    static value_type access(const const_col_iterator &itcol, size_type i)
    { return linalg_traits<M>::access(*itcol, itcol.si1.index(i)); }
    static reference access(const col_iterator &itcol, size_type i)
    { return linalg_traits<M>::access(*itcol, itcol.si1.index(i)); }
  };

  template <typename PT, typename SUBI1, typename SUBI2> std::ostream &operator <<
  (std::ostream &o, const gen_sub_col_matrix<PT, SUBI1, SUBI2>& m)
  { gmm::write(o,m); return o; }

  /* ******************************************************************** */
  /*		sub matrices                                              */
  /* ******************************************************************** */
  
  template <typename PT, typename SUBI1, typename SUBI2, typename ST>
  struct sub_matrix_type_ {
    typedef abstract_null_type return_type;
  };
  template <typename PT, typename SUBI1, typename SUBI2>
  struct sub_matrix_type_<PT, SUBI1, SUBI2, col_major>
  { typedef gen_sub_col_matrix<PT, SUBI1, SUBI2> matrix_type; };
  template <typename PT, typename SUBI1, typename SUBI2>
  struct sub_matrix_type_<PT, SUBI1, SUBI2, row_major>
  { typedef gen_sub_row_matrix<PT, SUBI1, SUBI2> matrix_type; };
  template <typename PT, typename SUBI1, typename SUBI2>
  struct sub_matrix_type {
    typedef typename std::iterator_traits<PT>::value_type M;
    typedef typename sub_matrix_type_<PT, SUBI1, SUBI2,
        typename principal_orientation_type<typename
        linalg_traits<M>::sub_orientation>::potype>::matrix_type matrix_type;
  };

  template <typename M, typename SUBI1, typename SUBI2>  inline
    typename select_return<typename sub_matrix_type<const M *, SUBI1, SUBI2>
    ::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI2>::matrix_type,
    M *>::return_type
  sub_matrix(M &m, const SUBI1 &si1, const SUBI2 &si2) {
    GMM_ASSERT2(si1.last() <= mat_nrows(m) && si2.last() <= mat_ncols(m),
		"sub matrix too large");
    return typename select_return<typename sub_matrix_type<const M *, SUBI1,
      SUBI2>::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI2>
      ::matrix_type, M *>::return_type(linalg_cast(m), si1, si2);
  }

  template <typename M, typename SUBI1, typename SUBI2>  inline
    typename select_return<typename sub_matrix_type<const M *, SUBI1, SUBI2>
    ::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI2>::matrix_type,
    const M *>::return_type
  sub_matrix(const M &m, const SUBI1 &si1, const SUBI2 &si2) {
    GMM_ASSERT2(si1.last() <= mat_nrows(m) && si2.last() <= mat_ncols(m),
		"sub matrix too large");
    return typename select_return<typename sub_matrix_type<const M *, SUBI1,
      SUBI2>::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI2>
      ::matrix_type, const M *>::return_type(linalg_cast(m), si1, si2);
  }

  template <typename M, typename SUBI1>  inline
    typename select_return<typename sub_matrix_type<const M *, SUBI1, SUBI1>
    ::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI1>::matrix_type,
    M *>::return_type
  sub_matrix(M &m, const SUBI1 &si1) {
    GMM_ASSERT2(si1.last() <= mat_nrows(m) && si1.last() <= mat_ncols(m),
		"sub matrix too large");
    return typename select_return<typename sub_matrix_type<const M *, SUBI1,
      SUBI1>::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI1>
      ::matrix_type, M *>::return_type(linalg_cast(m), si1, si1);
  }

  template <typename M, typename SUBI1>  inline
    typename select_return<typename sub_matrix_type<const M *, SUBI1, SUBI1>
    ::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI1>::matrix_type,
    const M *>::return_type
  sub_matrix(const M &m, const SUBI1 &si1) {
    GMM_ASSERT2(si1.last() <= mat_nrows(m) && si1.last() <= mat_ncols(m),
		"sub matrix too large");
    return typename select_return<typename sub_matrix_type<const M *, SUBI1,
      SUBI1>::matrix_type, typename sub_matrix_type<M *, SUBI1, SUBI1>
      ::matrix_type, const M *>::return_type(linalg_cast(m), si1, si1);
  }

}

#endif //  GMM_SUB_MATRIX_H__
