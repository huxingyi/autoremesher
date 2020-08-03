// Copyright (c) 2005 Rijksuniversiteit Groningen (Netherlands)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Skin_surface_3/include/CGAL/make_union_of_balls_3.h $
// $Id: make_union_of_balls_3.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Nico Kruithof <Nico@cs.rug.nl>

#ifndef CGAL_MAKE_UNION_OF_BALLS_MESH_3_H
#define CGAL_MAKE_UNION_OF_BALLS_MESH_3_H

#include <CGAL/license/Skin_surface_3.h>

#include <CGAL/Skin_surface_traits_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/mesh_union_of_balls_3.h>
#include <CGAL/subdivide_union_of_balls_mesh_3.h>

#include <CGAL/make_union_of_balls_3.h>

namespace CGAL {

template <class WP_iterator, class Polyhedron_3>
void make_union_of_balls_mesh_3(Polyhedron_3 &p,
                                WP_iterator begin, WP_iterator end,
                                int nSubdivisions=0)
{
  typedef typename WP_iterator::value_type                Weighted_point;
  typedef typename Kernel_traits<Weighted_point>::Kernel  K;

  typedef Skin_surface_traits_3<K>                        Traits;
  typedef Union_of_balls_3<Traits>                        Union_of_balls;

  Union_of_balls union_of_balls(begin, end);

  CGAL::mesh_union_of_balls_3(union_of_balls, p);

  CGAL::subdivide_union_of_balls_mesh_3(union_of_balls, p, nSubdivisions);
}

} //namespace CGAL

#endif // CGAL_MAKE_UNION_OF_BALLS_MESH_3_H
