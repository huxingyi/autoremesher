// Copyright (c) 2018  GeometryFactory Sarl (France).
// All rights reserved.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Stream_support/include/CGAL/IO/traits_multipolygon.h $
// $Id: traits_multipolygon.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_IO_TRAITS_MULTIPOLYGON_H
#define CGAL_IO_TRAITS_MULTIPOLYGON_H
#if BOOST_VERSION >= 105600 && (! defined(BOOST_GCC) || BOOST_GCC >= 40500)
#include <CGAL/internal/Geometry_container.h>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/io/wkt/read.hpp>


namespace boost{
namespace geometry{
namespace traits{
// WKT traits for MultiPolygon
template< typename R >
struct tag<CGAL::internal::Geometry_container<R, multi_polygon_tag> >
{ typedef multi_polygon_tag type; };

}//end traits
}//end geometry
}//end boost

#endif
#endif

