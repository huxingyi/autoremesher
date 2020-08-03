// Copyright (c) 2016 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Polygon_mesh_processing/include/CGAL/Polygon_mesh_processing/internal/Corefinement/Intersection_type.h $
// $Id: Intersection_type.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot

#ifndef CGAL_PMP_INTERNAL_COREFINEMENT_INTERSECTION_TYPE_H
#define CGAL_PMP_INTERNAL_COREFINEMENT_INTERSECTION_TYPE_H

#include <CGAL/license/Polygon_mesh_processing/corefinement.h>


#include <boost/graph/graph_traits.hpp>
#include <CGAL/property_map.h>

namespace CGAL{
namespace Polygon_mesh_processing {
namespace Corefinement{

enum Intersection_type {ON_FACE,ON_EDGE,ON_VERTEX,EMPTY,COPLANAR_TRIANGLES};

template <class TriangleMesh, class Exact_kernel>
struct Coplanar_intersection{
  typedef boost::graph_traits<TriangleMesh> GT;
  typedef typename GT::halfedge_descriptor halfedge_descriptor;

  Intersection_type type_1,type_2;   //intersection type for 1st and 2nd faces
  halfedge_descriptor info_1,info_2; //halfedge providing primitive indicated by type_1 and type_2
  typename Exact_kernel::Point_3 point;


  Coplanar_intersection()
  : type_1(EMPTY), type_2(EMPTY)
  , info_1(GT::null_halfedge()), info_2(info_1)
  {}

};

} } } // CGAL::Polygon_mesh_processing::Corefinement

#endif // CGAL_PMP_INTERNAL_COREFINEMENT_INTERSECTION_TYPE_H
