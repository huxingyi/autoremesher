// Copyright (c) 2006  GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_simplification/include/CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h $
// $Id: Count_stop_predicate.h ff09c5d 2019-10-25T16:35:53+02:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H

#include <CGAL/license/Surface_mesh_simplification.h>

#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

namespace CGAL {
namespace Surface_mesh_simplification {

//*******************************************************************************************************************
//                                -= stopping condition predicate =-
//
// Determines whether the simplification has finished.
// The arguments are (current_cost,vertex,vertex,is_edge,initial_pair_count,current_pair_count,surface) and the result is bool
//
//*******************************************************************************************************************

// Stops when the number of edges left falls below a given number.
template<class TM_>
class Count_stop_predicate
{
public:
  typedef TM_                                                                   TM;
  typedef typename boost::graph_traits<TM>::edges_size_type                     size_type;

  Count_stop_predicate(const std::size_t edge_count_threshold)
    : m_edge_count_threshold(edge_count_threshold)
  { }

  template <typename F, typename Profile>
  bool operator()(const F& /*current_cost*/,
                  const Profile& /*profile*/,
                  std::size_t /*initial_edge_count*/,
                  std::size_t current_edge_count) const
  {
    return current_edge_count < m_edge_count_threshold;
  }

private:
  std::size_t m_edge_count_threshold;
};

} // namespace Surface_mesh_simplification
} // namespace CGAL

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_COUNT_STOP_PREDICATE_H
