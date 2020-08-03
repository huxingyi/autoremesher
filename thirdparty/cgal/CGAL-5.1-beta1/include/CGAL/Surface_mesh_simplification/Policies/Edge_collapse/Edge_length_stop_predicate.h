// Copyright (c) 2016  GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_simplification/include/CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_stop_predicate.h $
// $Id: Edge_length_stop_predicate.h ff09c5d 2019-10-25T16:35:53+02:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Sebastien Loriot <sebastien.loriot@geometryfactory.com>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_EDGE_LENGTH_STOP_PREDICATE_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_EDGE_LENGTH_STOP_PREDICATE_H

#include <CGAL/license/Surface_mesh_simplification.h>
#include <CGAL/squared_distance_3.h>

namespace CGAL {
namespace Surface_mesh_simplification {

template <class FT>
class Edge_length_stop_predicate
{
public:
  Edge_length_stop_predicate(const FT edge_length_threshold)
    : m_edge_sq_length_threshold(edge_length_threshold * edge_length_threshold)
  {}

  template <typename Profile>
  bool operator()(const FT& /*current_cost*/,
                  const Profile& profile,
                  std::size_t /*initial_edge_count*/,
                  std::size_t /*current_edge_count*/) const
  {
    const typename Profile::Geom_traits& gt = profile.geom_traits();
    return gt.compute_squared_distance_3_object()(profile.p0(), profile.p1()) > m_edge_sq_length_threshold;
  }

private:
  const FT m_edge_sq_length_threshold;
};

} // namespace Surface_mesh_simplification
} // namespace CGAL

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_EDGE_LENGTH_STOP_PREDICATE_H
