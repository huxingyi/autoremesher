// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_2/include/CGAL/Intersections_2/Line_2_Point_2.h $
// $Id: Line_2_Point_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Geert-Jan Giezeman


#ifndef CGAL_INTERSECTIONS_2_POINT_2_LINE_2_H
#define CGAL_INTERSECTIONS_2_POINT_2_LINE_2_H

#include <CGAL/Line_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Intersection_traits_2.h>

namespace CGAL {

namespace Intersections {

namespace internal {

template <class K>
inline bool
do_intersect(const typename K::Point_2 &pt,
             const typename K::Line_2 &line,
             const K&)
{
    return line.has_on(pt);
}

template <class K>
inline bool
do_intersect(const typename K::Line_2 &line,
             const typename K::Point_2 &pt,
             const K&)
{
    return line.has_on(pt);
}

template <class K>
typename CGAL::Intersection_traits
<K, typename K::Point_2, typename K::Line_2>::result_type
intersection(const typename K::Point_2 &pt,
             const typename K::Line_2 &line,
             const K& k)
{
    if (do_intersect(pt,line, k)) {
        return intersection_return<typename K::Intersect_2, typename K::Point_2, typename K::Line_2>(pt);
    }
    return intersection_return<typename K::Intersect_2, typename K::Point_2, typename K::Line_2>();
}

template <class K>
typename CGAL::Intersection_traits
<K, typename K::Line_2, typename K::Point_2>::result_type
intersection(const typename K::Line_2 &line,
             const typename K::Point_2 &pt,
             const K& k)
{
  return internal::intersection(pt, line, k);
}

} // namespace internal
} // namespace Intersections


CGAL_INTERSECTION_FUNCTION(Point_2, Line_2, 2)
CGAL_DO_INTERSECT_FUNCTION(Point_2, Line_2, 2)

} //namespace CGAL

#endif
