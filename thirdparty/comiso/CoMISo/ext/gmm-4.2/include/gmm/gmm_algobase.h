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

/** @file gmm_algobase.h 
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date September 28, 2000.
    @brief Miscelleanous algorithms on containers.
*/

#ifndef GMM_ALGOBASE_H__
#define GMM_ALGOBASE_H__
#include "gmm_std.h"
#include "gmm_except.h"
#include <functional>

namespace gmm {

  /* ********************************************************************* */
  /* Definitition de classes de comparaison.                               */
  /* retournant un int.                                                    */
  /* ********************************************************************* */
  
  template <class T>
    struct less : public std::binary_function<T, T, int> {
    inline int operator()(const T& x, const T& y) const
    { return (x < y) ? -1 : ((y < x) ? 1 : 0); }
  };

  template<> struct less<int> : public std::binary_function<int, int, int>
  { int operator()(int x, int y) const { return x-y; } };
  template<> struct less<char> : public std::binary_function<char, char, int>
  { int operator()(char x, char y) const { return int(x-y); } };
  template<> struct less<short> : public std::binary_function<short,short,int>
  { int operator()(short x, short y) const { return int(x-y); } };
  template<> struct less<unsigned char>
     : public std::binary_function<unsigned char, unsigned char, int> {
    int operator()(unsigned char x, unsigned char y) const
    { return int(x)-int(y); }
  };
  

  template <class T>
    struct greater : public std::binary_function<T, T, int> {
    inline int operator()(const T& x, const T& y) const
    { return (y < x) ? -1 : ((x < y) ? 1 : 0); }
  };

  template<> struct greater<int> : public std::binary_function<int, int, int>
  { int operator()(int x, int y) const { return y-x; } };
  template<> struct greater<char> : public std::binary_function<char,char,int>
  { int operator()(char x, char y) const { return int(y-x); } };
  template<> struct greater<short>
      : public std::binary_function<short, short, int>
  { int operator()(short x, short y) const { return int(y-x); } };
  template<> struct greater<unsigned char>
    : public std::binary_function<unsigned char, unsigned char, int> {
    int operator()(unsigned char x, unsigned char y) const
      { return int(y)-int(x); }
  };

  template <typename T> inline T my_abs(T a) { return (a < T(0)) ? T(-a) : a; }
  
  template <class T>
    struct approx_less : public std::binary_function<T, T, int> { 
    double eps;
    inline int operator()(const T &x, const T &y) const
    { if (my_abs(x - y) <= eps) return 0; if (x < y) return -1; return 1; }
    approx_less(double e = 1E-13) { eps = e; }
  };

  template <class T>
    struct approx_greater : public std::binary_function<T, T, int> { 
    double eps;
    inline int operator()(const T &x, const T &y) const
    { if (my_abs(x - y) <= eps) return 0; if (x > y) return -1; return 1; }
    approx_greater(double e = 1E-13) { eps = e; }
  };

  template<class ITER1, class ITER2, class COMP>
    int lexicographical_compare(ITER1 b1, const ITER1 &e1,
				ITER2 b2, const ITER2 &e2, const COMP &c)  {
    int i;
    for ( ; b1 != e1 && b2 != e2; ++b1, ++b2)
      if ((i = c(*b1, *b2)) != 0) return i;
    if (b1 != e1) return 1; if (b2 != e2) return -1; return 0; 
  }

  template<class CONT, class COMP = gmm::less<typename CONT::value_type> >
    struct lexicographical_less : public std::binary_function<CONT, CONT, int>
  { 
    COMP c;
    int operator()(const CONT &x, const CONT &y) const {
      return gmm::lexicographical_compare(x.begin(), x.end(),
					  y.begin(), y.end(), c);
    }
    lexicographical_less(const COMP &d = COMP()) { c = d; }
  };

  template<class CONT, class COMP = gmm::less<typename CONT::value_type> >
  struct lexicographical_greater
    : public std::binary_function<CONT, CONT, int> { 
    COMP c;
    int operator()(const CONT &x, const CONT &y) const {
      return -gmm::lexicographical_compare(x.begin(), x.end(),
					   y.begin(), y.end(), c);
    }
    lexicographical_greater(const COMP &d = COMP()) { c = d; }
  };
  

  /* ********************************************************************* */
  /* "Virtual" iterators on sequences.                                     */
  /* The class T represent a class of sequence.                            */
  /* ********************************************************************* */

  template<class T> struct sequence_iterator {
    
    typedef T             value_type;
    typedef value_type*   pointer;
    typedef value_type&   reference;
    typedef const value_type& const_reference;
    typedef std::forward_iterator_tag iterator_category;

    T Un;

    sequence_iterator(T U0 = T(0)) { Un = U0; }
    
    sequence_iterator &operator ++()
    { ++Un; return *this; }
    sequence_iterator operator ++(int)
    { sequence_iterator tmp = *this; (*this)++; return tmp; }
	
    const_reference operator *() const { return Un; }
    reference operator *() { return Un; }
    
    bool operator ==(const sequence_iterator &i) const { return (i.Un==Un);}
    bool operator !=(const sequence_iterator &i) const { return (i.Un!=Un);}
  };

  /* ********************************************************************* */
  /* generic algorithms.                                                   */
  /* ********************************************************************* */

  template <class ITER1, class SIZE, class ITER2>
  ITER2 copy_n(ITER1 first, SIZE count, ITER2 result) {
    for ( ; count > 0; --count, ++first, ++result) *result = *first;
    return result;
  }

  template<class ITER>
    typename std::iterator_traits<ITER>::value_type
      mean_value(ITER first, const ITER &last) {
    GMM_ASSERT2(first != last, "mean value of empty container");
    size_t n = 1;
    typename std::iterator_traits<ITER>::value_type res = *first++;
    while (first != last) { res += *first; ++first; ++n; }
    res /= float(n);
    return res;
  }

  template<class CONT>
    typename CONT::value_type
  mean_value(const CONT &c) { return mean_value(c.begin(), c.end()); }

  template<class ITER> /* hum ... */
    void minmax_box(typename std::iterator_traits<ITER>::value_type &pmin,
		    typename std::iterator_traits<ITER>::value_type &pmax,
		    ITER first, const ITER &last) {
    typedef typename std::iterator_traits<ITER>::value_type PT;
    if (first != last) { pmin = pmax = *first; ++first; }
    while (first != last) {
      typename PT::const_iterator b = (*first).begin(), e = (*first).end();
      typename PT::iterator b1 = pmin.begin(), b2 = pmax.begin();
      while (b != e)
	{ *b1 = std::min(*b1, *b); *b2 = std::max(*b2, *b); ++b; ++b1; ++b2; }
    }
  }

  template<typename VEC> struct sorted_indexes_aux {
    const VEC &v;
  public:
    sorted_indexes_aux(const VEC& v_) : v(v_) {}
    template <typename IDX>
    bool operator()(const IDX &ia, const IDX &ib) const
    { return v[ia] < v[ib]; }
  };

  template<typename VEC, typename IVEC> 
  void sorted_indexes(const VEC &v, IVEC &iv) {
    iv.clear(); iv.resize(v.size());
    for (size_t i=0; i < v.size(); ++i) iv[i] = i;
    std::sort(iv.begin(), iv.end(), sorted_indexes_aux<VEC>(v));
  }

}


#endif /* GMM_ALGOBASE_H__ */
