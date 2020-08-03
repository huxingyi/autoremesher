// Copyright (c) 2008  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/AABB_tree/include/CGAL/internal/AABB_tree/AABB_drawing_traits.h $
// $Id: AABB_drawing_traits.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     :  Camille Wormser, Pierre Alliez

#ifndef CGAL_AABB_DRAWING_TRAITS_H
#define CGAL_AABB_DRAWING_TRAITS_H

#include <CGAL/license/AABB_tree.h>


#include <CGAL/Bbox_3.h>
#include <vector>

namespace CGAL {

template<typename Primitive, typename Node>
struct AABB_drawing_traits
{
  std::vector<float> *v_edges;
  double offset[3];

  typedef CGAL::Bbox_3 Bbox;
  bool go_further() { return true; }

  bool intersection(const int&, const Primitive&)
  {
    return true;
  }

  bool do_intersect(const int&, // unused
                    const Node& node)
  {
    gl_draw(node.bbox());
    return true;
  }

  // draw bbox
  void gl_draw(const Bbox& bb)
  {
    gl_draw_edge(bb.xmin(), bb.ymin(), bb.zmin(),
                 bb.xmax(), bb.ymin(), bb.zmin());
    gl_draw_edge(bb.xmin(), bb.ymin(), bb.zmin(),
                 bb.xmin(), bb.ymax(), bb.zmin());
    gl_draw_edge(bb.xmin(), bb.ymin(), bb.zmin(),
                 bb.xmin(), bb.ymin(), bb.zmax());

    gl_draw_edge(bb.xmax(), bb.ymin(), bb.zmin(),
                 bb.xmax(), bb.ymax(), bb.zmin());
    gl_draw_edge(bb.xmax(), bb.ymin(), bb.zmin(),
                 bb.xmax(), bb.ymin(), bb.zmax());

    gl_draw_edge(bb.xmin(), bb.ymax(), bb.zmin(),
                 bb.xmax(), bb.ymax(), bb.zmin());
    gl_draw_edge(bb.xmin(), bb.ymax(), bb.zmin(),
                 bb.xmin(), bb.ymax(), bb.zmax());

    gl_draw_edge(bb.xmin(), bb.ymin(), bb.zmax(),
                 bb.xmax(), bb.ymin(), bb.zmax());
    gl_draw_edge(bb.xmin(), bb.ymin(), bb.zmax(),
                 bb.xmin(), bb.ymax(), bb.zmax());

    gl_draw_edge(bb.xmax(), bb.ymax(), bb.zmax(),
                 bb.xmin(), bb.ymax(), bb.zmax());
    gl_draw_edge(bb.xmax(), bb.ymax(), bb.zmax(),
                 bb.xmax(), bb.ymin(), bb.zmax());
    gl_draw_edge(bb.xmax(), bb.ymax(), bb.zmax(),
                 bb.xmax(), bb.ymax(), bb.zmin());
  }

  void gl_draw_edge(double px, double py, double pz,
                           double qx, double qy, double qz)
  {
      v_edges->push_back((float)px+offset[0]);
      v_edges->push_back((float)py+offset[1]);
      v_edges->push_back((float)pz+offset[2]);

      v_edges->push_back((float)qx+offset[0]);
      v_edges->push_back((float)qy+offset[1]);
      v_edges->push_back((float)qz+offset[2]);
  }
}; // AABB_drawing_traits

} // end namespace CGAL

#endif // CGAL_AABB_DRAWING_TRAITS_H
