/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2000-2012 Yves Renard
 
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


#ifndef GMM_REF_H__
#define GMM_REF_H__

/** @file gmm_ref.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date August 26, 2000.
 *  @brief Provide some simple pseudo-containers.
 *  
 *  WARNING : modifiying the container infirm the validity of references.
 */


#include <iterator>
#include "gmm_except.h"

namespace gmm {

  /* ********************************************************************* */
  /* Simple reference.                                                     */
  /* ********************************************************************* */

  template<typename ITER> class tab_ref {

    protected :

      ITER begin_, end_;

    public :

      typedef typename std::iterator_traits<ITER>::value_type  value_type;
      typedef typename std::iterator_traits<ITER>::pointer     pointer;
      typedef typename std::iterator_traits<ITER>::pointer     const_pointer;
      typedef typename std::iterator_traits<ITER>::reference   reference;
      typedef typename std::iterator_traits<ITER>::reference   const_reference;
      typedef typename std::iterator_traits<ITER>::difference_type
	                                                       difference_type;
      typedef ITER                            iterator;
      typedef ITER                            const_iterator;
      typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
      typedef std::reverse_iterator<iterator> reverse_iterator;
      typedef size_t size_type;
    
      bool empty(void) const { return begin_ == end_; }
      size_type size(void) const { return end_ - begin_; }

      const iterator &begin(void) { return begin_; }
      const const_iterator &begin(void) const { return begin_; }
      const iterator &end(void) { return end_; }
      const const_iterator &end(void) const { return end_; }
      reverse_iterator rbegin(void) { return reverse_iterator(end()); }
      const_reverse_iterator rbegin(void) const
      { return const_reverse_iterator(end()); }
      reverse_iterator rend(void) { return reverse_iterator(begin()); }
      const_reverse_iterator rend(void) const
      { return const_reverse_iterator(begin()); }

      reference front(void) { return *begin(); }
      const_reference front(void) const { return *begin(); }
      reference back(void) { return *(--(end())); }
      const_reference back(void) const { return *(--(end())); }
      void pop_front(void) { ++begin_; }

      const_reference operator [](size_type ii) const { return *(begin_ + ii);}
      reference operator [](size_type ii) { return *(begin_ + ii); }

      tab_ref(void) {}
      tab_ref(const ITER &b, const ITER &e) : begin_(b), end_(e) {}
  };


  /* ********************************************************************* */
  /* Reference with index.                                                 */
  /* ********************************************************************* */

//   template<typename ITER> struct tab_ref_index_iterator_
//     : public dynamic_array<size_t>::const_iterator
//   {
//     typedef typename std::iterator_traits<ITER>::value_type  value_type;
//     typedef typename std::iterator_traits<ITER>::pointer     pointer;
//     typedef typename std::iterator_traits<ITER>::reference   reference;
//     typedef typename std::iterator_traits<ITER>::difference_type  
//     difference_type;
//     typedef std::random_access_iterator_tag iterator_category;
//     typedef size_t size_type;
//     typedef dynamic_array<size_type>::const_iterator dnas_iterator_;
//     typedef tab_ref_index_iterator_<ITER> iterator;
    

//     ITER piter;
    
//     iterator operator ++(int)
//     { iterator tmp = *this; ++(*((dnas_iterator_ *)(this))); return tmp; }
//     iterator operator --(int)
//     { iterator tmp = *this; --(*((dnas_iterator_ *)(this))); return tmp; }
//     iterator &operator ++()
//     { ++(*((dnas_iterator_ *)(this))); return *this; }
//     iterator &operator --()
//     { --(*((dnas_iterator_ *)(this))); return *this; }
//     iterator &operator +=(difference_type i)
//     { (*((dnas_iterator_ *)(this))) += i; return *this; }
//     iterator &operator -=(difference_type i)
//     { (*((dnas_iterator_ *)(this))) -= i; return *this; }
//     iterator operator +(difference_type i) const
//     { iterator it = *this; return (it += i); }
//     iterator operator -(difference_type i) const
//     { iterator it = *this; return (it -= i); }
//     difference_type operator -(const iterator &i) const
//     { return *((dnas_iterator_ *)(this)) - *((dnas_iterator_ *)(&i)); }
	
//     reference operator *() const
//     { return *(piter + *((*((dnas_iterator_ *)(this))))); }
//     reference operator [](int ii)
//     { return *(piter + *((*((dnas_iterator_ *)(this+ii))))); }
    
//     bool operator ==(const iterator &i) const
//     { 
//       return ((piter) == ((i.piter))
//        && *((dnas_iterator_ *)(this)) == *((*((dnas_iterator_ *)(this)))));
//     }
//     bool operator !=(const iterator &i) const
//     { return !(i == *this); }
//     bool operator < (const iterator &i) const
//     { 
//       return ((piter) == ((i.piter))
// 	 && *((dnas_iterator_ *)(this)) < *((*((dnas_iterator_ *)(this)))));
//     }

//     tab_ref_index_iterator_(void) {}
//     tab_ref_index_iterator_(const ITER &iter, const dnas_iterator_ &dnas_iter)
//       : dnas_iterator_(dnas_iter), piter(iter) {}
//   };


//   template<typename ITER> class tab_ref_index
//   {
//     public :

//       typedef typename std::iterator_traits<ITER>::value_type value_type;
//       typedef typename std::iterator_traits<ITER>::pointer    pointer;
//       typedef typename std::iterator_traits<ITER>::pointer    const_pointer;
//       typedef typename std::iterator_traits<ITER>::reference  reference;
//       typedef typename std::iterator_traits<ITER>::reference  const_reference;
//       typedef typename std::iterator_traits<ITER>::difference_type
// 	                                                       difference_type;
//       typedef size_t size_type; 
//       typedef tab_ref_index_iterator_<ITER> iterator;
//       typedef iterator                          const_iterator;
//       typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
//       typedef std::reverse_iterator<iterator> reverse_iterator;
    
//     protected :

//       ITER begin_;
//       dynamic_array<size_type> index_;

//     public :

//       bool empty(void) const { return index_.empty(); }
//       size_type size(void) const { return index_.size(); }


//       iterator begin(void) { return iterator(begin_, index_.begin()); }
//       const_iterator begin(void) const
//       { return iterator(begin_, index_.begin()); }
//       iterator end(void) { return iterator(begin_, index_.end()); }
//       const_iterator end(void) const { return iterator(begin_, index_.end()); }
//       reverse_iterator rbegin(void) { return reverse_iterator(end()); }
//       const_reverse_iterator rbegin(void) const
//       { return const_reverse_iterator(end()); }
//       reverse_iterator rend(void) { return reverse_iterator(begin()); }
//       const_reverse_iterator rend(void) const
//       { return const_reverse_iterator(begin()); }


//       reference front(void) { return *(begin_ +index_[0]); }
//       const_reference front(void) const { return *(begin_ +index_[0]); }
//       reference back(void) { return *(--(end())); }
//       const_reference back(void) const { return *(--(end())); }
   
//       tab_ref_index(void) {}
//       tab_ref_index(const ITER &b, const dynamic_array<size_type> &ind)
//       { begin_ = b; index_ = ind; }

//     // to be changed in a const_reference ?
//       value_type operator [](size_type ii) const
//       { return *(begin_ + index_[ii]);}
//       reference operator [](size_type ii) { return *(begin_ + index_[ii]); }

//   };


  /// iterator over a gmm::tab_ref_index_ref<ITER,ITER_INDEX>
  template<typename ITER, typename ITER_INDEX>
    struct tab_ref_index_ref_iterator_
    {
      typedef typename std::iterator_traits<ITER>::value_type value_type;
      typedef typename std::iterator_traits<ITER>::pointer    pointer;
      typedef typename std::iterator_traits<ITER>::reference  reference;
      typedef typename std::iterator_traits<ITER>::difference_type
                                                              difference_type;
      typedef std::random_access_iterator_tag iterator_category;
      typedef tab_ref_index_ref_iterator_<ITER, ITER_INDEX> iterator;
      typedef size_t size_type;

      ITER piter;
      ITER_INDEX iter_index;
      
      iterator operator ++(int)
      { iterator tmp = *this; ++iter_index; return tmp; }
      iterator operator --(int)
      { iterator tmp = *this; --iter_index; return tmp; }
      iterator &operator ++() { ++iter_index; return *this; }
      iterator &operator --() { --iter_index; return *this; }
      iterator &operator +=(difference_type i)
      { iter_index += i; return *this; }
      iterator &operator -=(difference_type i)
      { iter_index -= i; return *this; }
      iterator operator +(difference_type i) const
      { iterator it = *this; return (it += i); }
      iterator operator -(difference_type i) const
      { iterator it = *this; return (it -= i); }
      difference_type operator -(const iterator &i) const
      { return iter_index - i.iter_index; }
	
      reference operator *() const
      { return *(piter + *iter_index); }
      reference operator [](size_type ii) const
      { return *(piter + *(iter_index+ii)); }
      
      bool operator ==(const iterator &i) const
      { return ((piter) == ((i.piter)) && iter_index == i.iter_index); }
      bool operator !=(const iterator &i) const { return !(i == *this); }
      bool operator < (const iterator &i) const
      { return ((piter) == ((i.piter)) && iter_index < i.iter_index); }

      tab_ref_index_ref_iterator_(void) {}
      tab_ref_index_ref_iterator_(const ITER &iter, 
				  const ITER_INDEX &dnas_iter)
	: piter(iter), iter_index(dnas_iter) {}
      
    };

  /** 
      convenience template function for quick obtention of a indexed iterator
      without having to specify its (long) typename
  */
  template<typename ITER, typename ITER_INDEX>
  tab_ref_index_ref_iterator_<ITER,ITER_INDEX>
  index_ref_iterator(ITER it, ITER_INDEX it_i) {
    return tab_ref_index_ref_iterator_<ITER,ITER_INDEX>(it, it_i);
  }

  /** indexed array reference (given a container X, and a set of indexes I, 
      this class provides a pseudo-container Y such that
      @code Y[i] = X[I[i]] @endcode
  */
  template<typename ITER, typename ITER_INDEX> class tab_ref_index_ref {
  public :
    
    typedef std::iterator_traits<ITER>            traits_type;
    typedef typename traits_type::value_type      value_type;
    typedef typename traits_type::pointer         pointer;
    typedef typename traits_type::pointer         const_pointer;
    typedef typename traits_type::reference       reference;
    typedef typename traits_type::reference       const_reference;
    typedef typename traits_type::difference_type difference_type;
    typedef size_t                                size_type;
    typedef tab_ref_index_ref_iterator_<ITER, ITER_INDEX>   iterator;
    typedef iterator                              const_iterator;
    typedef std::reverse_iterator<const_iterator>     const_reverse_iterator;
    typedef std::reverse_iterator<iterator>           reverse_iterator;
    
  protected :

    ITER begin_;
    ITER_INDEX index_begin_, index_end_;

  public :
    
    bool empty(void) const { return index_begin_ == index_end_; }
    size_type size(void) const { return index_end_ - index_begin_; }
    
    iterator begin(void) { return iterator(begin_, index_begin_); }
    const_iterator begin(void) const
    { return iterator(begin_, index_begin_); }
    iterator end(void) { return iterator(begin_, index_end_); }
    const_iterator end(void) const { return iterator(begin_, index_end_); }
    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const
    { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const
    { return const_reverse_iterator(begin()); }
    
    reference front(void) { return *(begin_ + *index_begin_); }
    const_reference front(void) const { return *(begin_ + *index_begin_); }
    reference back(void) { return *(--(end())); }
    const_reference back(void) const { return *(--(end())); }
    void pop_front(void) { ++index_begin_; }
    
    tab_ref_index_ref(void) {}
    tab_ref_index_ref(const ITER &b, const ITER_INDEX &bi,
		      const ITER_INDEX &ei)
      : begin_(b), index_begin_(bi), index_end_(ei) {}
    
    // to be changed in a const_reference ?
    const_reference operator [](size_type ii) const
    { return *(begin_ + index_begin_[ii]);}
    reference operator [](size_type ii)
    { return *(begin_ + index_begin_[ii]); }

  };


  /* ********************************************************************* */
  /* Reference on regularly spaced elements.                               */
  /* ********************************************************************* */

  template<typename ITER> struct tab_ref_reg_spaced_iterator_ {
    
    typedef typename std::iterator_traits<ITER>::value_type value_type;
    typedef typename std::iterator_traits<ITER>::pointer    pointer;
    typedef typename std::iterator_traits<ITER>::reference  reference;
    typedef typename std::iterator_traits<ITER>::difference_type
                                                            difference_type;
    typedef typename std::iterator_traits<ITER>::iterator_category
                                                            iterator_category;
    typedef size_t size_type;
    typedef tab_ref_reg_spaced_iterator_<ITER> iterator;
    
    ITER it;
    size_type N, i;
    
    iterator operator ++(int) { iterator tmp = *this; i++; return tmp; }
    iterator operator --(int) { iterator tmp = *this; i--; return tmp; }
    iterator &operator ++()   { i++; return *this; }
    iterator &operator --()   { i--; return *this; }
    iterator &operator +=(difference_type ii) { i+=ii; return *this; }
    iterator &operator -=(difference_type ii) { i-=ii; return *this; }
    iterator operator +(difference_type ii) const 
    { iterator itt = *this; return (itt += ii); }
    iterator operator -(difference_type ii) const
    { iterator itt = *this; return (itt -= ii); }
    difference_type operator -(const iterator &ii) const
    { return (N ? (it - ii.it) / N : 0) + i - ii.i; }

    reference operator *() const { return *(it + i*N); }
    reference operator [](size_type ii) const { return *(it + (i+ii)*N); }

    bool operator ==(const iterator &ii) const
    { return (*this - ii) == difference_type(0); }
    bool operator !=(const iterator &ii) const
    { return  (*this - ii) != difference_type(0); }
    bool operator < (const iterator &ii) const
    { return (*this - ii) < difference_type(0); }

    tab_ref_reg_spaced_iterator_(void) {}
    tab_ref_reg_spaced_iterator_(const ITER &iter, size_type n, size_type ii)
      : it(iter), N(n), i(ii) { }
    
  };

  /** 
      convenience template function for quick obtention of a strided iterator
      without having to specify its (long) typename
  */
  template<typename ITER> tab_ref_reg_spaced_iterator_<ITER> 
  reg_spaced_iterator(ITER it, size_t stride) {
    return tab_ref_reg_spaced_iterator_<ITER>(it, stride);
  }

  /**
     provide a "strided" view a of container
  */
  template<typename ITER> class tab_ref_reg_spaced {
  public :

    typedef typename std::iterator_traits<ITER>::value_type value_type;
    typedef typename std::iterator_traits<ITER>::pointer    pointer;
    typedef typename std::iterator_traits<ITER>::pointer    const_pointer;
    typedef typename std::iterator_traits<ITER>::reference  reference;
    typedef typename std::iterator_traits<ITER>::reference  const_reference;
    typedef typename std::iterator_traits<ITER>::difference_type
            difference_type;
    typedef size_t size_type;
    typedef tab_ref_reg_spaced_iterator_<ITER> iterator;
    typedef iterator                          const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    
  protected :

    ITER begin_;
    size_type N, size_;
    
  public :
    
    bool empty(void) const { return size_ == 0; }
    size_type size(void) const { return size_; }
    
    iterator begin(void) { return iterator(begin_, N, 0); }
    const_iterator begin(void) const { return iterator(begin_, N, 0); }
    iterator end(void) { return iterator(begin_, N, size_); }
    const_iterator end(void) const { return iterator(begin_, N, size_); }
    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const
    { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const
    { return const_reverse_iterator(begin()); }
    
    reference front(void) { return *begin_; }
    const_reference front(void) const { return *begin_; }
    reference back(void) { return *(begin_ + N * (size_-1)); }
    const_reference back(void) const { return *(begin_ + N * (size_-1)); }
    void pop_front(void) { begin_ += N; }
    
    tab_ref_reg_spaced(void) {}
    tab_ref_reg_spaced(const ITER &b, size_type n, size_type s)
      : begin_(b), N(n), size_(s) {}
    
    
    const_reference operator [](size_type ii) const
    { return *(begin_ + ii * N);}
    reference operator [](size_type ii) { return *(begin_ + ii * N); }
    
  };

  /// iterator over a tab_ref_with_selection
  template<typename ITER, typename COND> 
  struct tab_ref_with_selection_iterator_ : public ITER {
    typedef typename std::iterator_traits<ITER>::value_type value_type;
    typedef typename std::iterator_traits<ITER>::pointer    pointer;
    typedef typename std::iterator_traits<ITER>::reference  reference;
    typedef typename std::iterator_traits<ITER>::difference_type
                                                              difference_type;
    typedef std::forward_iterator_tag iterator_category;
    typedef tab_ref_with_selection_iterator_<ITER, COND> iterator;
    const COND cond;
    
    void forward(void) { while (!(cond)(*this)) ITER::operator ++(); }
    iterator &operator ++()
    { ITER::operator ++(); forward(); return *this; }
    iterator operator ++(int)
    { iterator tmp = *this; ++(*this); return tmp; }
    
    tab_ref_with_selection_iterator_(void) {}
    tab_ref_with_selection_iterator_(const ITER &iter, const COND c)
      : ITER(iter), cond(c) {}
    
  };

  /**
     given a container X and a predicate P, provide pseudo-container Y
     of all elements of X such that P(X[i]).
  */
  template<typename ITER, typename COND> class tab_ref_with_selection {
    
  protected :
    
    ITER begin_, end_;
    COND cond;
    
  public :
    
    typedef typename std::iterator_traits<ITER>::value_type value_type;
    typedef typename std::iterator_traits<ITER>::pointer    pointer;
    typedef typename std::iterator_traits<ITER>::pointer    const_pointer;
    typedef typename std::iterator_traits<ITER>::reference  reference;
    typedef typename std::iterator_traits<ITER>::reference  const_reference;
    typedef size_t  size_type;
    typedef tab_ref_with_selection_iterator_<ITER, COND> iterator;
    typedef iterator   const_iterator;
    
    iterator begin(void) const
    { iterator it(begin_, cond); it.forward(); return it; }
    iterator end(void) const { return iterator(end_, cond); }
    bool empty(void) const { return begin_ == end_; }
    
    value_type front(void) const { return *begin(); }
    void pop_front(void) { ++begin_; begin_ = begin(); }
    
    COND &condition(void) { return cond; }
    const COND &condition(void) const { return cond; }
    
    tab_ref_with_selection(void) {}
    tab_ref_with_selection(const ITER &b, const ITER &e, const COND &c)
      : begin_(b), end_(e), cond(c) { begin_ = begin(); }
    
  };

}

#endif /* GMM_REF_H__  */
