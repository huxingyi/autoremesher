// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Cartesian_kernel/include/CGAL/Cartesian/Data_accessor_2.h $
// $Id: Data_accessor_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_DATA_ACCESSOR_2_H
#define CGAL_CARTESIAN_DATA_ACCESSOR_2_H

namespace CGAL {

// 2D Cartesian point data accessor
template < class R_ >
class Data_accessorC2
{
public:
    // Min_ellipse_2 wants FT public...
    typedef typename R_::FT                   FT;
    typedef typename R_::Point_2              Point;

    typedef R_                           R;

    FT  get_x( const Point &p) const { return( p.x()); }
    FT  get_y( const Point &p) const { return( p.y()); }

    void get( const Point &p, FT &x, FT &y) const { x=get_x(p); y=get_y(p); }
    void set( Point& p, const FT &x, const FT &y) const { p=Point(x,y); }
};

} //namespace CGAL

#endif // CGAL_CARTESIAN_DATA_ACCESSOR_2_H
