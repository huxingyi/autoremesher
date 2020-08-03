// Copyright (c) 2011  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Spatial_sorting/include/CGAL/Hilbert_sort_2.h $
// $Id: Hilbert_sort_2.h 5c41b10 2020-01-02T10:26:44+01:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Olivier Devillers

#ifndef CGAL_HILBERT_SORT_2_H
#define CGAL_HILBERT_SORT_2_H

#include <CGAL/Hilbert_policy_tags.h>
#include <CGAL/Hilbert_sort_median_2.h>
#include <CGAL/Hilbert_sort_middle_2.h>

namespace CGAL {

template <class K,  class Hilbert_policy, class ConcurrencyTag = Sequential_tag >
class Hilbert_sort_2;

template <class K, class ConcurrencyTag>
class Hilbert_sort_2<K, Hilbert_sort_median_policy, ConcurrencyTag >
  : public Hilbert_sort_median_2<K, ConcurrencyTag>
{
public:
  Hilbert_sort_2 (const K &k=K(), std::ptrdiff_t limit=1 )
    : Hilbert_sort_median_2<K, ConcurrencyTag> (k,limit)
  {}
};

template <class K, class ConcurrencyTag>
class Hilbert_sort_2<K, Hilbert_sort_middle_policy, ConcurrencyTag >
  : public Hilbert_sort_middle_2<K>
{
public:
  Hilbert_sort_2 (const K &k=K(), std::ptrdiff_t limit=1 )
    : Hilbert_sort_middle_2<K> (k,limit)
  {}
};

} // namespace CGAL

#endif//CGAL_HILBERT_SORT_2_H
