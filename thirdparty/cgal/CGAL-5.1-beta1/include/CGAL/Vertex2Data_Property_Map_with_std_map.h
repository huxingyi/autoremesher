// Copyright (c) 2015  INRIA Sophia-Antipolis (France), INRIA Lorraine LORIA.
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Ridges_3/include/CGAL/Vertex2Data_Property_Map_with_std_map.h $
// $Id: Vertex2Data_Property_Map_with_std_map.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Marc Pouget and Frédéric Cazals

#ifndef CGAL_VERTEX2DATA_PROPERTY_MAP_WITH_STD_MAP_H
#define CGAL_VERTEX2DATA_PROPERTY_MAP_WITH_STD_MAP_H

#include <CGAL/license/Ridges_3.h>


#include <map>

namespace boost {
template <typename G>
class graph_traits;

template <typename T>
class associative_property_map;
}

//---------------------------------------------------------------------------
//Vertex2Data_Property_Map_with_std_map
// defines models for Vertex2FTPropertyMap and Vertex2VectorPropertyMap
//--------------------------------------------------------------------------
template < class TriangulatedSurfaceMesh >
class Vertex2Data_Property_Map_with_std_map
{
 public:
  typedef typename TriangulatedSurfaceMesh::Traits::FT        FT;
  typedef typename TriangulatedSurfaceMesh::Traits::Vector_3  Vector_3;
  typedef typename boost::graph_traits<TriangulatedSurfaceMesh>::vertex_descriptor vertex_descriptor;

  typedef std::map<vertex_descriptor, FT> Vertex2FT_map;
  typedef boost::associative_property_map< Vertex2FT_map > Vertex2FT_property_map;

  typedef std::map<vertex_descriptor, Vector_3> Vertex2Vector_map;
  typedef boost::associative_property_map< Vertex2Vector_map > Vertex2Vector_property_map;
};


#endif
