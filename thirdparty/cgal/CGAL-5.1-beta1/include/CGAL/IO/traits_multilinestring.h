// Copyright (c) 2018  GeometryFactory Sarl (France).
// All rights reserved.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Stream_support/include/CGAL/IO/traits_multilinestring.h $
// $Id: traits_multilinestring.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_IO_TRAITS_MULTILINESTRING_H
#define CGAL_IO_TRAITS_MULTILINESTRING_H
#if BOOST_VERSION >= 105600 && (! defined(BOOST_GCC) || BOOST_GCC >= 40500)
#include <CGAL/internal/Geometry_container.h>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/io/wkt/read.hpp>

namespace boost{
namespace geometry{
namespace traits{
// WKT traits for MultiLinestring
template< typename R >
struct tag<CGAL::internal::Geometry_container<R, multi_linestring_tag> >
{ typedef multi_linestring_tag type; };

}//end traits
}//end geometry
}//end boost
#endif // TRAITS_MULTILINESTRING_H
#endif
