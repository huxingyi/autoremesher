// Copyright (c) 2019 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_2/include/CGAL/Intersections_2/Bbox_2_Bbox_2.h $
// $Id: Bbox_2_Bbox_2.h ce4cbe6 2020-03-19T11:41:57+01:00 Maxime Gimeno
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_INTERSECTIONS_BBOX_2_BBOX_2_H
#define CGAL_INTERSECTIONS_BBOX_2_BBOX_2_H

#include <CGAL/Bbox_2.h>

namespace CGAL {

bool
inline
do_intersect(const CGAL::Bbox_2& c,
             const CGAL::Bbox_2& bbox)
{
  return CGAL::do_overlap(c, bbox);
}

typename boost::optional< typename boost::variant<Bbox_2> >
inline
intersection(const CGAL::Bbox_2& a,
             const CGAL::Bbox_2& b)
{
  typedef typename boost::variant<Bbox_2> variant_type;
  typedef typename boost::optional<variant_type> result_type;

  if(!do_intersect(a, b))
    return result_type();

  double xmin = (std::max)(a.xmin(), b.xmin());
  double xmax = (std::min)(a.xmax(), b.xmax());

  double ymin = (std::max)(a.ymin(), b.ymin());
  double ymax = (std::min)(a.ymax(), b.ymax());

  return result_type(std::forward<Bbox_2>(Bbox_2(xmin, ymin, xmax, ymax)));
}

} // namespace CGAL

#endif // CGAL_INTERSECTIONS_BBOX_2_BBOX_2_H
