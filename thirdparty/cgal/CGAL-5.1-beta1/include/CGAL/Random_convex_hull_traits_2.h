// Copyright (c) 2014
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Generator/include/CGAL/Random_convex_hull_traits_2.h $
// $Id: Random_convex_hull_traits_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Remy Thomasse  <remy.thomasse@inria.fr>

#ifndef CGAL_RANDOM_CONVEX_HULL_TRAITS_2_H
#define CGAL_RANDOM_CONVEX_HULL_TRAITS_2_H

namespace CGAL{
        template<class R_>
class Random_convex_hull_traits_2
{
  public:
    typedef R_                            R;
    typedef typename R::FT                FT;
    typedef typename R::Point_2           Point_2;
    typedef typename R::Segment_2                  Segment_2;
           typedef typename R::Compare_x_2                  Compare_x_2;
           typedef typename R::Compare_y_2                  Compare_y_2;
           typedef typename R::Orientation_2          Orientation_2;


    Compare_x_2
    compare_x_2_object() const
    {
            return Compare_x_2();
    }

    Compare_y_2
    compare_y_2_object() const
    {
            return Compare_y_2();
    }

    Orientation_2
    orientation_2_object() const
    {
            return Orientation_2();
    }

};

} //namespace CGAL
#endif //CGAL_RANDOM_CONVEX_HULL_TRAITS_2_H