// Copyright (c) 2005  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesher/include/CGAL/Point_with_surface_index_geom_traits.h $
// $Id: Point_with_surface_index_geom_traits.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent RINEAU

#ifndef CGAL_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H
#define CGAL_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H

#include <CGAL/license/Surface_mesher.h>


#include <CGAL/Point_with_surface_index.h>

namespace CGAL {

template <class GT>
class Point_with_surface_index_geom_traits : public GT
{
  typedef typename GT::Point_3 Old_point_3;

public:
  typedef Point_with_surface_index<Old_point_3> Point_3;

};  // end Point_with_surface_index_geom_traits

} // end namespace CGAL

#endif // CGAL_POINT_WITH_SURFACE_INDEX_GEOM_TRAITS_H
