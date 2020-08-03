// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Cartesian_kernel/include/CGAL/Cartesian/predicates_on_points_3.h $
// $Id: predicates_on_points_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Herve Bronnimann

#ifndef CGAL_CARTESIAN_PREDICATES_ON_POINTS_3_H
#define CGAL_CARTESIAN_PREDICATES_ON_POINTS_3_H

#include <CGAL/predicates/kernel_ftC3.h>
#include <CGAL/Cartesian/Point_3.h>

namespace CGAL {

template < class K >
inline
bool
equal_xy(const PointC3<K> &p, const PointC3<K> &q)
{
  return K().equal_xy_3_object()(p, q);
}

template < class K >
inline
bool
equal_xyz(const PointC3<K> &p, const PointC3<K> &q)
{
  return p.x() == q.x() && p.y() == q.y() && p.z() == q.z();
}

template < class K >
inline
Comparison_result
compare_xy(const PointC3<K> &p, const PointC3<K> &q)
{
  return K().compare_xy_3_object()(p, q);
}

template < class K >
inline
Comparison_result
compare_lexicographically_xy(const PointC3<K> &p, const PointC3<K> &q)
{
  return K().compare_xy_3_object()(p, q);
}

template < class K >
inline
bool
lexicographically_xy_smaller_or_equal(const PointC3<K> &p,
                                      const PointC3<K> &q)
{
  return compare_lexicographically_xy(p, q) != LARGER;
}

template < class K >
inline
bool
lexicographically_xy_smaller(const PointC3<K> &p,
                             const PointC3<K> &q)
{
  return K().less_xy_3_object()(p, q);
}

template < class K >
inline
bool
strict_dominance(const PointC3<K> &p,
                 const PointC3<K> &q)
{
  return strict_dominanceC3(p.x(), p.y(), p.z(),
                            q.x(), q.y(), q.z());
}

template < class K >
inline
bool
dominance(const PointC3<K> &p,
          const PointC3<K> &q)
{
  return dominanceC3(p.x(), p.y(), p.z(),
                     q.x(), q.y(), q.z());
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_PREDICATES_ON_POINTS_3_H
