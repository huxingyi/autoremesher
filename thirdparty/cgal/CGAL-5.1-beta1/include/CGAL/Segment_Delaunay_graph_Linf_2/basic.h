// Copyright (c) 2015  Università della Svizzera italiana.
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Segment_Delaunay_graph_Linf_2/include/CGAL/Segment_Delaunay_graph_Linf_2/basic.h $
// $Id: basic.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Panagiotis Cheilaris, Sandeep Kumar Dey, Evanthia Papadopoulou
//philaris@gmail.com, sandeep.kr.dey@gmail.com, evanthia.papadopoulou@usi.ch

#ifndef CGAL_SEGMENT_DELAUNAY_GRAPH_LINF_2_BASIC_H
#define CGAL_SEGMENT_DELAUNAY_GRAPH_LINF_2_BASIC_H

#include <CGAL/license/Segment_Delaunay_graph_Linf_2.h>


#include <CGAL/basic.h>

#define CGAL_SEGMENT_DELAUNAY_GRAPH_LINF_2_NS \
  CGAL::SegmentDelaunayGraphLinf_2

#ifndef CGAL_SDG_VERBOSE
#define CGAL_SDG_DEBUG(a)
#else
#define CGAL_SDG_DEBUG(a) { a }
#endif

#endif // CGAL_SEGMENT_DELAUNAY_GRAPH_LINF_2_BASIC_H
