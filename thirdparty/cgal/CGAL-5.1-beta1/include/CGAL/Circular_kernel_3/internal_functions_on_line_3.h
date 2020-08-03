// Copyright (c) 2008  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Circular_kernel_3/include/CGAL/Circular_kernel_3/internal_functions_on_line_3.h $
// $Id: internal_functions_on_line_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Monique Teillaud, Sylvain Pion, Pedro Machado

// Partially supported by the IST Programme of the EU as a
// STREP (FET Open) Project under Contract No  IST-006413
// (ACS -- Algorithms for Complex Shapes)

#ifndef CGAL_SPHERICAL_KERNEL_PREDICATES_ON_LINE_3_H
#define CGAL_SPHERICAL_KERNEL_PREDICATES_ON_LINE_3_H

#include <CGAL/license/Circular_kernel_3.h>


namespace CGAL {
  namespace SphericalFunctors {

    template < class SK >
    typename SK::Line_3
    construct_line_3(const typename SK::Polynomials_for_line_3 &eq)
    {
      typedef typename SK::Line_3 Line_3;
      typedef typename SK::Point_3 Point_3;
      typedef typename SK::Vector_3 Vector_3;
      return Line_3(Point_3(eq.b1(),eq.b2(),eq.b3()),
                    Vector_3(eq.a1(),eq.a2(),eq.a3()));
    }

  }//SphericalFunctors
}//CGAL

#endif //CGAL_SPHERICAL_KERNEL_PREDICATES_ON_LINE_3_H
