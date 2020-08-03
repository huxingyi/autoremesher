// Copyright (c) 2005  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesher/include/CGAL/Surface_mesher/Point_surface_indices_oracle_visitor.h $
// $Id: Point_surface_indices_oracle_visitor.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau


#ifndef CGAL_SURFACE_MESHER_POINT_SURFACE_INDICES_VISITOR_H
#define CGAL_SURFACE_MESHER_POINT_SURFACE_INDICES_VISITOR_H

#include <CGAL/license/Surface_mesher.h>


namespace CGAL {

  namespace Surface_mesher {

  /** Model of the OracleVisitor concept.
      This model of OracleVisitor sets the point "surface_index" to a
      constant \c int.
   */
  struct Point_surface_indices_visitor
  {
    int i;

    Point_surface_indices_visitor(const int index) : i(index)
    {
    }

    template <class P>
    void new_point(P& p) const
    {
      p.set_surface_index(i);
    }
  }; // end class Point_surface_indices_visitor

  }  // namespace Surface_mesher

} // namespace CGAL


#endif  // CGAL_SURFACE_MESHER_POINT_SURFACE_INDICES_VISITOR_H
