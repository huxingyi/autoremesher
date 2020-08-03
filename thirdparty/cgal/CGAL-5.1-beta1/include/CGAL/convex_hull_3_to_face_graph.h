// Copyright (c) 2011  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Convex_hull_3/include/CGAL/convex_hull_3_to_face_graph.h $
// $Id: convex_hull_3_to_face_graph.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//

#ifndef CGAL_CONVEX_HULL_3_TO_FACE_GRAPH_3_H
#define CGAL_CONVEX_HULL_3_TO_FACE_GRAPH_3_H

#include <CGAL/license/Convex_hull_3.h>


#include <CGAL/link_to_face_graph.h>

namespace CGAL {


template<class Triangulation_3,class PolygonMesh>
void convex_hull_3_to_face_graph(const Triangulation_3& T,PolygonMesh& P){
  link_to_face_graph(T,T.infinite_vertex(), P);
}

} //namespace CGAL

#endif //CGAL_CONVEX_HULL_3_TO_FACE_GRAPH_3_H
