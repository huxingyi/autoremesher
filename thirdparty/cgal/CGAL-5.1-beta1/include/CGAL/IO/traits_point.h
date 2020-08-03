// Copyright (c) 2018  GeometryFactory Sarl (France).
// All rights reserved.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Stream_support/include/CGAL/IO/traits_point.h $
// $Id: traits_point.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_IO_TRAITS_POINT_H
#define CGAL_IO_TRAITS_POINT_H
#if BOOST_VERSION >= 105600 && (! defined(BOOST_GCC) || BOOST_GCC >= 40500)
#include <CGAL/number_utils.h>
#include <CGAL/Point_2.h>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/io/wkt/read.hpp>

namespace boost{
namespace geometry{
namespace traits{

//WKT traits for Points
template< typename K > struct tag<CGAL::Point_2<K> >
{ typedef point_tag type; };

template< typename K > struct coordinate_type<CGAL::Point_2<K> >
{ typedef double type; };

template< typename K > struct coordinate_system<CGAL::Point_2<K> >
{ typedef cs::cartesian type; };

template< typename K > struct dimension<CGAL::Point_2<K> > : boost::mpl::int_<2> {};

template< typename K >
struct access<CGAL::Point_2<K> , 0>
{
  static double get(CGAL::Point_2<K>  const& p)
  {
    return CGAL::to_double(p.x());
  }

  static void set(CGAL::Point_2<K> & p, typename K::FT c)
  {
    p = CGAL::Point_2<K> (c, p.y());
  }

};

template< typename K >
struct access<CGAL::Point_2<K> , 1>
{
  static double get(CGAL::Point_2<K>  const& p)
  {
    return CGAL::to_double(p.y());
  }

  static void set(CGAL::Point_2<K> & p, typename K::FT c)
  {
    p = CGAL::Point_2<K> (p.x(), c);
  }

};

}}}//end namespaces
#endif
#endif
