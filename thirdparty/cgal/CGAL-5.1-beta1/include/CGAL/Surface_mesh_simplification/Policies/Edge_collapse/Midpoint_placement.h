// Copyright (c) 2006  GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_simplification/include/CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h $
// $Id: Midpoint_placement.h ff09c5d 2019-10-25T16:35:53+02:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_MIDPOINT_PLACEMENT_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_MIDPOINT_PLACEMENT_H

#include <CGAL/license/Surface_mesh_simplification.h>


#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

namespace CGAL {
namespace Surface_mesh_simplification {

template<class TM_>
class Midpoint_placement
{
public:
  typedef TM_                                                     TM;

  Midpoint_placement() {}

  template <typename Profile>
  boost::optional<typename Profile::Point> operator()(const Profile& profile) const
  {
    typedef boost::optional<typename Profile::Point>              result_type;
    return result_type(profile.geom_traits().construct_midpoint_3_object()(profile.p0(), profile.p1()));
  }
};

} // namespace Surface_mesh_simplification
} // namespace CGAL

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_MIDPOINT_PLACEMENT_H
