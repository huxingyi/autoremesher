// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Cartesian_kernel/include/CGAL/Cartesian/Iso_rectangle_2.h $
// $Id: Iso_rectangle_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Herve Bronnimann

#ifndef CGAL_CARTESIAN_ISO_RECTANGLE_2_H
#define CGAL_CARTESIAN_ISO_RECTANGLE_2_H

#include <CGAL/array.h>

namespace CGAL {

template <class R_>
class Iso_rectangleC2
{
  typedef typename R_::FT                   FT;
  typedef typename R_::Point_2              Point_2;
  typedef typename R_::Iso_rectangle_2      Iso_rectangle_2;
  typedef typename R_::Construct_point_2    Construct_point_2;

  typedef std::array<Point_2, 2>          Rep;
  typedef typename R_::template Handle<Rep>::type  Base;

  Base base;

public:
  typedef R_                                     R;

  Iso_rectangleC2() {}

  // Iso_rectangleC2(const Point_2 &p, const Point_2 &q)
  //  : base(p, q) {}

  Iso_rectangleC2(const Point_2 &p, const Point_2 &q, int)
    : base(CGAL::make_array(p, q))
  {
    // I have to remove the assertions, because of Cartesian_converter.
    // CGAL_kernel_assertion(p<=q);
  }

  const Point_2 & min BOOST_PREVENT_MACRO_SUBSTITUTION () const
  {
      return get_pointee_or_identity(base)[0];
  }
  const Point_2 & max BOOST_PREVENT_MACRO_SUBSTITUTION () const
  {
      return get_pointee_or_identity(base)[1];
  }

};

} //namespace CGAL

#endif // CGAL_CARTESIAN_ISO_RECTANGLE_2_H
