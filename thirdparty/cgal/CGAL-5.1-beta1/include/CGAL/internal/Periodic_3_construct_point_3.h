// Copyright (c) 1999-2004,2006-2009,2014-2015   INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Periodic_3_triangulation_3/include/CGAL/internal/Periodic_3_construct_point_3.h $
// $Id: Periodic_3_construct_point_3.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Monique Teillaud <Monique.Teillaud@sophia.inria.fr>
//                 Sylvain Pion <Sylvain.Pion@sophia.inria.fr>
//                 Andreas Fabri <Andreas.Fabri@sophia.inria.fr>
//                 Nico Kruithof <Nico.Kruithof@sophia.inria.fr>
//                 Manuel Caroli <Manuel.Caroli@sophia.inria.fr>
//                 Aymeric Pellé <Aymeric.Pelle@sophia.inria.fr>
#ifndef CGAL_PERIODIC_3_CONSTRUCT_POINT_3_H
#define CGAL_PERIODIC_3_CONSTRUCT_POINT_3_H

#include <CGAL/license/Periodic_3_triangulation_3.h>

namespace CGAL {

template <typename K_, typename Construct_point_3_base_>
class Periodic_3_construct_point_3
  : public Construct_point_3_base_
{
  typedef Construct_point_3_base_            Base;

  typedef K_                                 Kernel;

  typedef typename Kernel::Point_3           Point;
  typedef typename Kernel::Weighted_point_3  Weighted_point;
  typedef typename Kernel::Offset            Offset;
  typedef typename Kernel::Iso_cuboid_3      Iso_cuboid_3;

public:
  Periodic_3_construct_point_3(const Iso_cuboid_3* dom,
                               const Base& cp)
    : Base(cp), _dom(dom)
  { }

  using Base::operator();

  Point operator() ( const Point& p, const Offset& o ) const {
    return operator()(p.x() + (_dom->xmax() - _dom->xmin()) * o.x(),
                      p.y() + (_dom->ymax() - _dom->ymin()) * o.y(),
                      p.z() + (_dom->zmax() - _dom->zmin()) * o.z());
  }

  Point operator() ( const Weighted_point& p, const Offset& o ) const {
    return operator()(p.x() + (_dom->xmax() - _dom->xmin()) * o.x(),
                      p.y() + (_dom->ymax() - _dom->ymin()) * o.y(),
                      p.z() + (_dom->zmax() - _dom->zmin()) * o.z());
  }

private:
  const Iso_cuboid_3* _dom;
};

} // namespace CGAL

#endif // CGAL_PERIODIC_3_CONSTRUCT_POINT_3_H
