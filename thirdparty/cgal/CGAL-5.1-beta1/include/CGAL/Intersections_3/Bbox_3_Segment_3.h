// Copyright (c) 2010 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/Bbox_3_Segment_3.h $
// $Id: Bbox_3_Segment_3.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//

#ifndef CGAL_INTERSECTIONS_3_BBOX_3_SEGMENT_3_H
#define CGAL_INTERSECTIONS_3_BBOX_3_SEGMENT_3_H

#include <CGAL/Bbox_3.h>
#include <CGAL/Segment_3.h>
#include <CGAL/Intersections_3/internal/intersection_3_1_impl.h>
#include <CGAL/Intersections_3/internal/Bbox_3_Segment_3_do_intersect.h>

namespace CGAL {

template<typename K>
bool do_intersect(const CGAL::Bbox_3& a,
                  const Segment_3<K>& b) {
  return K().do_intersect_3_object()(a, b);
}

template<typename K>
bool do_intersect(const Segment_3<K>& a,
                  const CGAL::Bbox_3& b) {
  return K().do_intersect_3_object()(a, b);
}

template<typename K>
typename Intersection_traits<K, typename K::Segment_3, Bbox_3>::result_type
intersection(const CGAL::Bbox_3& a,
             const Segment_3<K>& b) {
  return K().intersect_3_object()(a, b);
}

template<typename K>
typename Intersection_traits<K, typename K::Segment_3, Bbox_3>::result_type
intersection(const Segment_3<K>& a,
             const CGAL::Bbox_3& b) {
  return K().intersect_3_object()(a, b);
}

} // namespace CGAL

#endif // CGAL_INTERSECTIONS_3_BBOX_3_SEGMENT_3_H
