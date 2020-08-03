// Copyright (c) 2010 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/Bbox_3_Iso_cuboid_3.h $
// $Id: Bbox_3_Iso_cuboid_3.h 90d2e03 2020-01-15T13:32:11+01:00 Maxime Gimeno
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//

#ifndef CGAL_INTERSECTIONS_3_BBOX_3_ISO_CUBOID_3_H
#define CGAL_INTERSECTIONS_3_BBOX_3_ISO_CUBOID_3_H

#include <CGAL/Bbox_3.h>
#include <CGAL/Iso_cuboid_3.h>

#include <CGAL/Intersections_3/internal/Bbox_3_Iso_cuboid_3_do_intersect.h>
#include <CGAL/Intersections_3/internal/intersection_3_1_impl.h>

namespace CGAL {

template<typename K>
bool do_intersect(const CGAL::Bbox_3& a,
                  const Iso_cuboid_3<K>& b) {
  return K().do_intersect_3_object()(a, b);
}

template<typename K>
bool do_intersect(const Iso_cuboid_3<K>& a,
                  const CGAL::Bbox_3& b) {
  return K().do_intersect_3_object()(b, a);
}

template<typename K>
typename Intersection_traits<K, typename K::Iso_cuboid_3, Bbox_3>::result_type
intersection(const CGAL::Bbox_3& a,
             const Iso_cuboid_3<K>& b) {
  return K().intersect_3_object()(a, b);
}

template<typename K>
typename Intersection_traits<K, typename K::Iso_cuboid_3, Bbox_3>::result_type
intersection(const Iso_cuboid_3<K>& a,
             const CGAL::Bbox_3& b) {
  return K().intersect_3_object()(a, b);
}
} // namespace CGAL

#endif // CGAL_INTERSECTIONS_3_BBOX_3_ISO_CUBOID_3_H
