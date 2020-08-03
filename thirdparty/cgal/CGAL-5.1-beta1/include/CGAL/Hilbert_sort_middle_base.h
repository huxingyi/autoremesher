// Copyright (c) 2011  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Spatial_sorting/include/CGAL/Hilbert_sort_middle_base.h $
// $Id: Hilbert_sort_middle_base.h 5c41b10 2020-01-02T10:26:44+01:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     :  Olivier Devillers

#ifndef CGAL_HILBERT_SORT_MIDDLE_BASE_H
#define CGAL_HILBERT_SORT_MIDDLE_BASE_H

#include <CGAL/config.h>
#include <algorithm>

namespace CGAL {

namespace internal {

template <class RandomAccessIterator, class Cmp>
RandomAccessIterator
fixed_hilbert_split (RandomAccessIterator begin, RandomAccessIterator end,
                     Cmp cmp = Cmp ())
{
  if (begin >= end)
    return begin;

  return std::partition (begin, end, cmp);
}

} // namespace internal

} // namespace CGAL

#endif//CGAL_HILBERT_SORT_MIDDLE_BASE_H
