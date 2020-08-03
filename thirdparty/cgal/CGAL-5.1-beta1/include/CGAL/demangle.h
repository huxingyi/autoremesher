// Copyright (c) 2016  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/demangle.h $
// $Id: demangle.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Simon Giraudot

#ifndef CGAL_DEMANGLE_H
#define CGAL_DEMANGLE_H

#if BOOST_VERSION >= 105600
#include <boost/core/demangle.hpp>
#else
#include <boost/units/detail/utility.hpp>
#endif

namespace CGAL {


inline std::string demangle(const char* name)
{
#if BOOST_VERSION >= 105600
  return boost::core::demangle(name);
#else
  return boost::units::detail::demangle(name);
#endif
}


} //namespace CGAL

#endif // CGAL_DEMANGLE_H
