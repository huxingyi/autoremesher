// Copyright (c) 2003,2004,2005,2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2/Kernel_wrapper_2.h $
// $Id: Kernel_wrapper_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>



#ifndef CGAL_SEGMENT_DELAUNAY_GRAPH_2_KERNEL_WRAPPER_2_H
#define CGAL_SEGMENT_DELAUNAY_GRAPH_2_KERNEL_WRAPPER_2_H

#include <CGAL/license/Segment_Delaunay_graph_2.h>


#include <CGAL/Segment_Delaunay_graph_2/basic.h>

#include <CGAL/Segment_Delaunay_graph_2/Constructions_C2.h>

#include <CGAL/Segment_Delaunay_graph_site_2.h>
#include <CGAL/Segment_Delaunay_graph_simple_site_2.h>



namespace CGAL {

namespace SegmentDelaunayGraph_2 {

namespace Internal {

  template<class K, class ITag> struct SDG_Which_site;

  // If the ITag is Tag_true we fully support intersections and
  // therefore we need the full-fletched site.
  template<class K>
  struct SDG_Which_site<K,Tag_true>
  {
    typedef K          Kernel;
    typedef Tag_true   Intersections_tag;

    typedef CGAL::Segment_Delaunay_graph_site_2<K> Site_2;

    typedef Construct_sdg_site_2<Site_2,Intersections_tag>
    Construct_site_2;
  };

  // If the ITag is Tag_false we are happy with the simple site.
  template<class K>
  struct SDG_Which_site<K,Tag_false>
  {
    typedef K          Kernel;
    typedef Tag_false  Intersections_tag;

    typedef CGAL::Segment_Delaunay_graph_simple_site_2<K> Site_2;

    typedef Construct_sdg_site_2<Site_2,Intersections_tag>
    Construct_site_2;
  };

} // namespace Internal



template<class Kernel_base_2, class ITag>
class Kernel_wrapper_2
  : public Kernel_base_2
{
public:
  typedef Kernel_base_2    Kernel_base;
  typedef ITag             Intersections_tag;

  typedef typename
  Internal::SDG_Which_site<Kernel_base,Intersections_tag>::Site_2  Site_2;

  typedef typename
  Internal::SDG_Which_site<Kernel_base,Intersections_tag>::Construct_site_2
  Construct_site_2;
};


} //namespace SegmentDelaunayGraph_2

} //namespace CGAL


#endif // CGAL_SEGMENT_DELAUNAY_GRAPH_2_KERNEL_WRAPPER_2_H
