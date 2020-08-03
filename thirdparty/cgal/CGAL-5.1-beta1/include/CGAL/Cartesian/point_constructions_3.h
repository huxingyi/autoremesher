// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Cartesian_kernel/include/CGAL/Cartesian/point_constructions_3.h $
// $Id: point_constructions_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_POINT_CONSTRUCTIONS_3_H
#define CGAL_CARTESIAN_POINT_CONSTRUCTIONS_3_H

#include <CGAL/Cartesian/Point_3.h>
#include <CGAL/constructions/kernel_ftC3.h>

namespace CGAL {

template <class K>
CGAL_KERNEL_LARGE_INLINE
PointC3<K>
point_on_plane(const PlaneC3<K> &p)
{
  typename K::FT x, y, z;
  point_on_planeC3(p.a(), p.b(), p.c(), p.d(), x, y, z);
  return PointC3<K>(x, y, z);
}

template <class K>
CGAL_KERNEL_LARGE_INLINE
PointC3<K>
projection_plane(const PointC3<K> &p,
                 const PlaneC3<K> &h)
{
  typename K::FT x, y, z;
  projection_planeC3(h.a(), h.b(), h.c(), h.d(),
                     p.x(), p.y(), p.z(),
                     x, y, z);
  return PointC3<K>(x, y, z);
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_POINT_CONSTRUCTIONS_3_H
