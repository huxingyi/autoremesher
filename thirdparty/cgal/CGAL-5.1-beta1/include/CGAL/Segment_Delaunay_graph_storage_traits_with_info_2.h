// Copyright (c) 2006 Foundation for Research and Technology-Hellas (Greece).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h $
// $Id: Segment_Delaunay_graph_storage_traits_with_info_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#ifndef CGAL_SEGMENT_DELAUNAY_GRAPH_STORAGE_TRAITS_WITH_INFO_2_H
#define CGAL_SEGMENT_DELAUNAY_GRAPH_STORAGE_TRAITS_WITH_INFO_2_H 1

#include <CGAL/license/Segment_Delaunay_graph_2.h>


#include <CGAL/basic.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_site_with_info_2.h>
#include <CGAL/Segment_Delaunay_graph_2/Construct_storage_site_with_info_2.h>

namespace CGAL {

template<class Gt, typename Info_, class Converter, class Merger>
class Segment_Delaunay_graph_storage_traits_with_info_2
  : public Segment_Delaunay_graph_storage_traits_2<Gt>
{
public:
  typedef Info_                                        Info;
  typedef Converter                                    Convert_info;
  typedef Merger                                       Merge_info;

private:
  typedef Segment_Delaunay_graph_storage_traits_2<Gt>  Base;
  typedef typename Base::Storage_site_2                Base_storage_site_2;

  typedef Segment_Delaunay_graph_storage_traits_with_info_2<Gt,
                                                            Info,
                                                            Convert_info,
                                                            Merge_info>
  Self;

public:
  typedef typename Base::Geom_traits               Geom_traits;

  typedef
  Segment_Delaunay_graph_storage_site_with_info_2<Self,
                                                  Info,
                                                  Base_storage_site_2>
  Storage_site_2;

  typedef
  CGAL_SEGMENT_DELAUNAY_GRAPH_2_NS::Construct_storage_site_with_info_2<Self>
  Construct_storage_site_2;

  // MK::FIGURE OUT HOW TO PASS A REFERENCE TO GEOM_TRAITS AND HAVE
  // DEFAULT CONSTRUCTOR AS WELL IF POSSIBLE
  Segment_Delaunay_graph_storage_traits_with_info_2
  (const Geom_traits& gt = Geom_traits())
    : Base(gt) {}

  inline Construct_storage_site_2
  construct_storage_site_2_object() const {
    return Construct_storage_site_2();
  }

  inline Convert_info
  convert_info_object() const {
    return Convert_info();
  }

  inline Merge_info
  merge_info_object() const {
    return Merge_info();
  }
};



} //namespace CGAL

#endif // CGAL_SEGMENT_DELAUNAY_GRAPH_STORAGE_TRAITS_WITH_INFO_2_H
