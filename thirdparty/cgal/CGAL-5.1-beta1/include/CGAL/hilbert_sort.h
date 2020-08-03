// Copyright (c) 2007-2011  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Spatial_sorting/include/CGAL/hilbert_sort.h $
// $Id: hilbert_sort.h 8bb22d5 2020-03-26T14:23:37+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Christophe Delage
//

#ifndef CGAL_HILBERT_SORT_H
#define CGAL_HILBERT_SORT_H

#include <CGAL/config.h>

#include <CGAL/Hilbert_policy_tags.h>
#include <CGAL/Hilbert_sort_2.h>
#include <CGAL/Hilbert_sort_3.h>
#include <CGAL/Hilbert_sort_d.h>
#include <CGAL/algorithm.h>

#include <boost/random/random_number_generator.hpp>
#include <boost/random/linear_congruential.hpp>

#include <algorithm>

namespace CGAL {
namespace internal {

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator, class Kernel, class Policy>
void hilbert_sort (RandomAccessIterator begin,
                   RandomAccessIterator end,
                   const Kernel &k,
                   Policy /*policy*/,
                   typename Kernel::Point_2 *)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::difference_type Diff_t;
  boost::rand48 random;
  boost::random_number_generator<boost::rand48, Diff_t> rng(random);
  CGAL::cpp98::random_shuffle(begin,end, rng);
  (Hilbert_sort_2<Kernel, Policy, ConcurrencyTag> (k))(begin, end);
}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator, class Kernel, class Policy>
void hilbert_sort (RandomAccessIterator begin,
                   RandomAccessIterator end,
                   const Kernel &k,
                   Policy /*policy*/,
                   typename Kernel::Point_3 *)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::difference_type Diff_t;
  boost::rand48 random;
  boost::random_number_generator<boost::rand48, Diff_t> rng(random);
  CGAL::cpp98::random_shuffle(begin,end, rng);
  (Hilbert_sort_3<Kernel, Policy, ConcurrencyTag> (k))(begin, end);
}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator, class Kernel, class Policy>
void hilbert_sort (RandomAccessIterator begin,
                   RandomAccessIterator end,
                   const Kernel &k,
                   Policy /*policy*/,
                   typename Kernel::Point_d *)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::difference_type Diff_t;
  boost::rand48 random;
  boost::random_number_generator<boost::rand48, Diff_t> rng(random);
  CGAL::cpp98::random_shuffle(begin,end, rng);
  (Hilbert_sort_d<Kernel, Policy> (k))(begin, end);
}

} // namespace internal

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end)
{

  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::value_type               value_type;
  typedef CGAL::Kernel_traits<value_type>            KTraits;
  typedef typename KTraits::Kernel                   Kernel;

  internal::hilbert_sort<ConcurrencyTag>(begin, end, Kernel(), Hilbert_sort_median_policy(),
                                         static_cast<value_type *> (0));

}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator, class Kernel>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
                   const Kernel &k)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::value_type               value_type;

  internal::hilbert_sort<ConcurrencyTag>(begin, end, k, Hilbert_sort_median_policy(),
                                         static_cast<value_type *> (0));
}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
                   Hilbert_sort_median_policy policy)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::value_type               value_type;
  typedef CGAL::Kernel_traits<value_type>            KTraits;
  typedef typename KTraits::Kernel                   Kernel;

  internal::hilbert_sort<ConcurrencyTag>(begin, end, Kernel(), policy,
                                         static_cast<value_type *> (0));
}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
                   Hilbert_sort_middle_policy policy)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::value_type               value_type;
  typedef CGAL::Kernel_traits<value_type>            KTraits;
  typedef typename KTraits::Kernel                   Kernel;

  internal::hilbert_sort<ConcurrencyTag>(begin, end, Kernel(), policy,
                                         static_cast<value_type *> (0));
}

template <class ConcurrencyTag = Sequential_tag, class RandomAccessIterator, class Kernel, class Policy>
void hilbert_sort (RandomAccessIterator begin, RandomAccessIterator end,
                   const Kernel &k, Policy policy)
{
  typedef std::iterator_traits<RandomAccessIterator> ITraits;
  typedef typename ITraits::value_type               value_type;

  internal::hilbert_sort<ConcurrencyTag>(begin, end,
                                         k, policy, static_cast<value_type *> (0));
}

} // namespace CGAL

#endif//CGAL_HILBERT_SORT_H

