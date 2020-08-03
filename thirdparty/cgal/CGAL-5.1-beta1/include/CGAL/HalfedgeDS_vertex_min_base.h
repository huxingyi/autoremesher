// Copyright (c) 1997
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/HalfedgeDS/include/CGAL/HalfedgeDS_vertex_min_base.h $
// $Id: HalfedgeDS_vertex_min_base.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>

#ifndef CGAL_HALFEDGEDS_VERTEX_MIN_BASE_H
#define CGAL_HALFEDGEDS_VERTEX_MIN_BASE_H 1

#include <CGAL/basic.h>

namespace CGAL {

template < class Refs>
class HalfedgeDS_vertex_min_base {
public:
    typedef Refs                                 HalfedgeDS;
    typedef HalfedgeDS_vertex_min_base< Refs>    Base;
    typedef Tag_false                            Supports_vertex_halfedge;
    typedef Tag_false                            Supports_vertex_point;
    typedef typename Refs::Vertex_handle         Vertex_handle;
    typedef typename Refs::Vertex_const_handle   Vertex_const_handle;
    typedef typename Refs::Halfedge_handle       Halfedge_handle;
    typedef typename Refs::Halfedge_const_handle Halfedge_const_handle;
    typedef typename Refs::Face_handle           Face_handle;
    typedef typename Refs::Face_const_handle     Face_const_handle;
    typedef typename Refs::Halfedge              Halfedge;
    typedef typename Refs::Face                  Face;
};

} //namespace CGAL

#endif // CGAL_HALFEDGEDS_VERTEX_MIN_BASE_H //
// EOF //
