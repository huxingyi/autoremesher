// Copyright (c) 2006 Foundation for Research and Technology-Hellas (Greece).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Voronoi_diagram_2/include/CGAL/Voronoi_diagram_2/Site_accessors.h $
// $Id: Site_accessors.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#ifndef CGAL_VORONOI_DIAGRAM_2_SITE_ACCESSORS_H
#define CGAL_VORONOI_DIAGRAM_2_SITE_ACCESSORS_H 1

#include <CGAL/license/Voronoi_diagram_2.h>


#include <CGAL/Voronoi_diagram_2/basic.h>

namespace CGAL {

namespace VoronoiDiagram_2 { namespace Internal {

//=========================================================================
//=========================================================================

template<class S, class DG, class Use_const_ref> struct Site_accessor;
template<class S, class DG, class Use_const_ref> struct Point_accessor;

//=========================================================================

template<class T, class Use_const_ref> struct Const_ref_chooser;

template<class T>
struct Const_ref_chooser<T,Tag_true>
{
  typedef const T&  Type;
};

template<class T>
struct Const_ref_chooser<T,Tag_false>
{
  typedef T         Type;
};

//=========================================================================

template<class S, class DG, class Use_const_ref>
struct Site_accessor
{
  typedef typename Const_ref_chooser<S,Use_const_ref>::Type  result_type;
  typedef typename DG::Vertex_handle                         Vertex_handle;

  result_type operator()(const Vertex_handle& v) const {
    return v->site();
  }
};

//=========================================================================

template<class P, class DG, class Use_const_ref>
struct Point_accessor
{
  typedef typename Const_ref_chooser<P,Use_const_ref>::Type  result_type;
  typedef typename DG::Vertex_handle                         Vertex_handle;

  result_type operator()(const Vertex_handle& v) const {
    return v->point();
  }
};

//=========================================================================
//=========================================================================

} } //namespace VoronoiDiagram_2::Internal

} //namespace CGAL

#endif // CGAL_VORONOI_DIAGRAM_2_SITE_ACCESSORS_H
