// Copyright (c) 2003,2004  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Apollonius_graph_2/include/CGAL/Apollonius_graph_hierarchy_vertex_base_2.h $
// $Id: Apollonius_graph_hierarchy_vertex_base_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#ifndef CGAL_APOLLONIUS_GRAPH_HIERARCHY_VERTEX_BASE_2_H
#define CGAL_APOLLONIUS_GRAPH_HIERARCHY_VERTEX_BASE_2_H

#include <CGAL/license/Apollonius_graph_2.h>


#include <CGAL/basic.h>

namespace CGAL {

template < class Vbb >
class Apollonius_graph_hierarchy_vertex_base_2
 : public Vbb
{
  typedef Vbb                                                Base;
  typedef typename Base::Apollonius_graph_data_structure_2   Agds;

public:
  typedef typename Base::Site_2             Site_2;
  typedef Agds                              Apollonius_graph_data_structure_2;
  typedef typename Agds::Vertex_handle      Vertex_handle;
  typedef typename Agds::Face_handle        Face_handle;

  template < typename AGDS2 >
  struct Rebind_TDS {
    typedef typename Vbb::template Rebind_TDS<AGDS2>::Other   Vb2;
    typedef Apollonius_graph_hierarchy_vertex_base_2<Vb2>     Other;
  };

  Apollonius_graph_hierarchy_vertex_base_2()
    : Base(), _up(), _down()
    {}
  Apollonius_graph_hierarchy_vertex_base_2(const Site_2& p,
                                           Face_handle f)
    : Base(p,f), _up(), _down()
    {}
  Apollonius_graph_hierarchy_vertex_base_2(const Site_2& p)
    : Base(p), _up(), _down()
    {}

  Vertex_handle up() {return _up;}
  Vertex_handle down() {return _down;}
  void set_up(Vertex_handle u) {_up=u;}
  void set_down(Vertex_handle d) {_down=d;}


 private:
  Vertex_handle  _up;    // same vertex one level above
  Vertex_handle  _down;  // same vertex one level below
};

} //namespace CGAL

#endif // CGAL_APOLLONIUS_GRAPH_HIERARCHY_VERTEX_BASE_2_H
