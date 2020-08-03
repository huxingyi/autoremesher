// Copyright (c) 2006-2007  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesher/include/CGAL/Surface_mesh_vertex_base_3.h $
// $Id: Surface_mesh_vertex_base_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_SURFACE_MESH_VERTEX_BASE_3_H
#define CGAL_SURFACE_MESH_VERTEX_BASE_3_H

#include <CGAL/license/Surface_mesher.h>


#include <CGAL/Triangulation_vertex_base_3.h>
#include <CGAL/Complex_2_in_triangulation_vertex_base_3.h>

namespace CGAL {

  template < class GT, class Vb = Triangulation_vertex_base_3 <GT> >
  class Surface_mesh_vertex_base_3
    : public Complex_2_in_triangulation_vertex_base_3<GT, Vb> {

  public:
    typedef Surface_mesh_vertex_base_3 <GT, Vb> Self;

    template < class TDS3 >
    struct Rebind_TDS {
      typedef typename Vb::template Rebind_TDS<TDS3>::Other  Vb3;
      typedef Surface_mesh_vertex_base_3 <GT, Vb3> Other;
    };

  public:
    Surface_mesh_vertex_base_3()
      : Complex_2_in_triangulation_vertex_base_3<GT, Vb>()
    {}
  };  // end Surface_mesh_vertex_base_3

}  // namespace CGAL

#endif  // CGAL_SURFACE_MESH_CELL_BASE_3_H
