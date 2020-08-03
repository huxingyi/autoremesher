// Copyright (c) 2018 GeometryFactory Sarl
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/internal/Triangle_3_Sphere_3_do_intersect.h $
// $Id: Triangle_3_Sphere_3_do_intersect.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_TRIANGLE_3_SPHERE_3_DO_INTERSECT_H
#define CGAL_TRIANGLE_3_SPHERE_3_DO_INTERSECT_H

#include <CGAL/squared_distance_3_2.h>
#include <CGAL/Intersection_traits_3.h>

namespace CGAL {

template <class K>
class Triangle_3;

template <class K>
class Sphere_3;

template <class K>
class Line_3;

namespace Intersections {

namespace internal {

template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Sphere_3 &sp,
             const typename K::Triangle_3 &tr,
             const K & /* k */)
{
  return squared_distance(sp.center(), tr) <= sp.squared_radius();
}

template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Triangle_3 &tr,
             const typename K::Sphere_3 &sp,
             const K & /* k */)
{
  return squared_distance(sp.center(), tr) <= sp.squared_radius();
}
template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Sphere_3 &sp,
             const typename K::Line_3 &lin,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}


template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Line_3 &lin,
             const typename K::Sphere_3 &sp,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}



template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Sphere_3 &sp,
             const typename K::Ray_3 &lin,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}


template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Ray_3 &lin,
             const typename K::Sphere_3 &sp,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}

template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Sphere_3 &sp,
             const typename K::Segment_3 &lin,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}


template <class K>
inline
typename K::Boolean
do_intersect(const typename K::Segment_3 &lin,
             const typename K::Sphere_3 &sp,
             const K & /* k */)
{
  return squared_distance(sp.center(), lin) <= sp.squared_radius();
}

} // namespace internal
} // namespace Intersections
} // namespace CGAL

#endif // CGAL_TRIANGLE_3_SPHERE_3_DO_INTERSECT_H
