// Copyright (c) 1997-2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Nef_3/include/CGAL/Nef_3/quotient_coordinates_to_homogeneous_point.h $
// $Id: quotient_coordinates_to_homogeneous_point.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Miguel Granados <granados@mpi-sb.mpg.de>

#ifndef CGAL_NEF_QUOTIENT_COORDINATES_TO_HOMOGENEOUS_POINT_H
#define CGAL_NEF_QUOTIENT_COORDINATES_TO_HOMOGENEOUS_POINT_H

#include <CGAL/license/Nef_3.h>


namespace CGAL {

template <typename Homogeneous>
typename Homogeneous::Point_3
quotient_coordinates_to_homogeneous_point(
                                  typename Homogeneous::FT x,
                                  typename Homogeneous::FT y,
                                  typename Homogeneous::FT z) {
  typedef typename Homogeneous::Point_3 Point_3;
  if( (x.denominator() == y.denominator()) &&
      (x.denominator() == z.denominator())) {
    Point_3 p( x.numerator(),
               y.numerator(),
               z.numerator(),
               x.denominator());
    return normalized(p);
  }
  else {
    Point_3 p( x.numerator()   * y.denominator() * z.denominator(),
               x.denominator() * y.numerator()   * z.denominator(),
               x.denominator() * y.denominator() * z.numerator(),
               x.denominator() * y.denominator() * z.denominator());
    return normalized(p);
  }
}

} //namespace CGAL

#endif // CGAL_NEF_QUOTIENT_COORDINATES_TO_HOMOGENEOUS_POINT_H
