// Copyright (c) 2016  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_parameterization/include/CGAL/Surface_mesh_parameterization/internal/Containers_filler.h $
// $Id: Containers_filler.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_CONTAINERS_FILLER_H
#define CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_CONTAINERS_FILLER_H

#include <CGAL/license/Surface_mesh_parameterization.h>

#include <CGAL/disable_warnings.h>

#include "boost/tuple/tuple.hpp"
#include <boost/unordered_set.hpp>
#include <boost/graph/graph_traits.hpp>
#include <vector>

namespace CGAL {

namespace Surface_mesh_parameterization {

namespace internal {

// Custom output iterator that fills 'faces' and 'vertices' containers from a mesh
template<typename TriangleMesh,
         typename Vertex_set =
             boost::unordered_set<typename boost::graph_traits<TriangleMesh>::vertex_descriptor>,
         typename Face_vector =
             std::vector<typename boost::graph_traits<TriangleMesh>::face_descriptor> >
class Containers_filler
{
  typedef typename boost::graph_traits<TriangleMesh>::vertex_descriptor   vertex_descriptor;
  typedef typename boost::graph_traits<TriangleMesh>::halfedge_descriptor halfedge_descriptor;
  typedef typename boost::graph_traits<TriangleMesh>::face_descriptor     face_descriptor;

  const TriangleMesh& mesh;

  Vertex_set& vertices;
  Face_vector* faces;

public:
  Containers_filler(const TriangleMesh& mesh_,
                    Vertex_set& vertices_,
                    Face_vector* faces_)
    : mesh(mesh_), vertices(vertices_), faces(faces_)
  { }

  Containers_filler(const TriangleMesh& mesh_,
                    Vertex_set& vertices_)
    : mesh(mesh_), vertices(vertices_), faces(nullptr)
  { }

  void operator()(face_descriptor fd)
  {
    halfedge_descriptor hd = halfedge(fd, mesh);
    for(vertex_descriptor vd : vertices_around_face(hd, mesh)) {
      vertices.insert(vd);
    }

    if(faces != nullptr)
      faces->push_back(fd);
  }
};

template <typename Mesh, typename Map>
struct Index_map_filler
{
  typedef typename boost::graph_traits<Mesh>::vertex_descriptor  vertex_descriptor;
  typedef typename boost::graph_traits<Mesh>::face_descriptor    face_descriptor;

  Index_map_filler(const Mesh& mesh, Map& map)
    : mesh(mesh), map(&map), index(0)
  { }

  void operator()(const face_descriptor& fd)
  {
    for(vertex_descriptor vd :
                  vertices_around_face(halfedge(fd, mesh), mesh)) {
      typename Map::iterator it;
      bool new_element;
      boost::tie(it,new_element) = map->insert(std::make_pair(vd,1));
      if(new_element) {
        it->second = index++;
      }
    }
  }

  const Mesh& mesh;
  mutable Map* map;
  int index;
};

} // namespace internal

} // namespace Surface_mesh_parameterization

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_CONTAINERS_FILLER_H
