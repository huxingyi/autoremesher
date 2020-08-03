// Copyright (c) 2003,2004,2005,2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2/Traits_wrapper_2.h $
// $Id: Traits_wrapper_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#ifndef CGAL_SEGMENT_DELAUNAY_GRAPH_2_TRAITS_WRAPPER_2_H
#define CGAL_SEGMENT_DELAUNAY_GRAPH_2_TRAITS_WRAPPER_2_H

#include <CGAL/license/Segment_Delaunay_graph_2.h>


#include <CGAL/Segment_Delaunay_graph_2/basic.h>

namespace CGAL {

template<class Gt_base>
class Segment_Delaunay_graph_traits_wrapper_2 : public Gt_base
{
private:
  typedef Segment_Delaunay_graph_traits_wrapper_2<Gt_base> Self;

public:
  struct Triangle_2 {};

  Segment_Delaunay_graph_traits_wrapper_2() {}

  Segment_Delaunay_graph_traits_wrapper_2(const Gt_base&) {}
};

} //namespace CGAL

#endif // CGAL_SEGMENT_DELAUNAY_GRAPH_2_TRAITS_WRAPPER_2_H
