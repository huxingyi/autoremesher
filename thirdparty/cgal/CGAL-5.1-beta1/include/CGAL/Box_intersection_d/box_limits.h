// Copyright (c) 2004  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Box_intersection_d/include/CGAL/Box_intersection_d/box_limits.h $
// $Id: box_limits.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>
//                 Andreas Meyer <ameyer@mpi-sb.mpg.de>

#ifndef CGAL_BOX_INTERSECTION_D_BOX_LIMITS_H
#define CGAL_BOX_INTERSECTION_D_BOX_LIMITS_H

#include <CGAL/license/Box_intersection_d.h>


#include <CGAL/basic.h>
#include <limits>

namespace CGAL {

namespace Box_intersection_d {


template<class T>
struct box_limits {};

template<>
struct box_limits<int> {
    static int inf() { return (std::numeric_limits<int>::min)(); }
    static int sup() { return (std::numeric_limits<int>::max)(); }
};

template<>
struct box_limits<unsigned int> {
    static int inf() { return 0; }
    static int sup() { return (std::numeric_limits<unsigned int>::max)(); }
};

template<>
struct box_limits<float> {
    static float inf() { return -sup(); }
    static float sup() { return (std::numeric_limits<float>::max)(); }
};

template<>
struct box_limits<double> {
    static double inf() { return -sup(); }
    static double sup() { return (std::numeric_limits<double>::max)(); }
};

} // end namespace Box_intersection_d


} //namespace CGAL


#endif
