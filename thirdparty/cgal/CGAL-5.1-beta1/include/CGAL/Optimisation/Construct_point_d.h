// Copyright (c) 1997-2001
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Optimisation_basic/include/CGAL/Optimisation/Construct_point_d.h $
// $Id: Construct_point_d.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sven Schoenherr <sven@inf.ethz.ch>

#ifndef CGAL_OPTIMISATION_CONSTRUCT_POINT_D_H
#define CGAL_OPTIMISATION_CONSTRUCT_POINT_D_H

// includes
#  include <CGAL/Kernel_d/Interface_classes.h>
#  include <CGAL/Kernel_d/Point_d.h>

namespace CGAL {

// Class declaration
// =================
template < class K >
class _Construct_point_d;

// Class interface
// ===============
template < class K_ >
class _Construct_point_d {
  public:
    // self
    typedef  K_                         K;
    typedef  _Construct_point_d<K>      Self;

    // types
    typedef  typename K::Point_d        Point;

    // creation
    _Construct_point_d( ) { }

    // operations
    template < class InputIterator >
    Point
    operator() ( int d, InputIterator first, InputIterator last) const
    {
        // d-dim kernel has no functor to construct a point, use point's
        // constructor
        return Point( d, first, last);
    }
};

} //namespace CGAL

#endif // CGAL_OPTIMISATION_CONSTRUCT_POINT_D_H

// ===== EOF ==================================================================
