// Copyright (c) 2007  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/type_traits.h $
// $Id: type_traits.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Andreas Meyer

#ifndef CGAL_TYPE_TRAITS_H
#define CGAL_TYPE_TRAITS_H

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/mpl/or.hpp>

namespace CGAL {

template< class Base, class Derived >
struct is_same_or_derived :
  public ::boost::mpl::or_<
    ::boost::is_same< Base, Derived >,
    ::boost::is_base_and_derived< Base, Derived >
  >::type
{};

}

#endif // CGAL_TYPE_TRAITS_H
