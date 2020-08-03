// Copyright (c) 2018  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/Point_3_Point_3.h $
// $Id: Point_3_Point_3.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_INTERSECTIONS_3_POINT_3_POINT_3_H
#define CGAL_INTERSECTIONS_3_POINT_3_POINT_3_H

#include <CGAL/Point_3.h>
#include <CGAL/Intersection_traits_3.h>

namespace CGAL {

namespace Intersections {

namespace internal {

template <class K>
inline bool
do_intersect(const typename K::Point_3 &pt1,
             const typename K::Point_3 &pt2,
             const K&)
{
    return pt1 == pt2;
}

template <class K>
typename CGAL::Intersection_traits
<K, typename K::Point_3, typename K::Point_3>::result_type
intersection(const typename K::Point_3 &pt1,
             const typename K::Point_3 &pt2,
             const K&)
{
  if (pt1 == pt2)
    return intersection_return<typename K::Intersect_3, typename K::Point_3, typename K::Point_3>(pt1);
  return intersection_return<typename K::Intersect_3, typename K::Point_3, typename K::Point_3>();
}

} // namespace internal
} // namespace Intersections

CGAL_INTERSECTION_FUNCTION_SELF(Point_3, 3)
CGAL_DO_INTERSECT_FUNCTION_SELF(Point_3, 3)
}//nmaespace cgal

#endif // CGAL_INTERSECTIONS_3_POINT_3_POINT_3_H
