// Copyright (c) 2017 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/internal/Has_member_visited.h $
// $Id: Has_member_visited.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_HAS_MEMBER_VISITED_H
#define CGAL_HAS_MEMBER_VISITED_H

#include <boost/mpl/has_xxx.hpp>

namespace CGAL {
namespace internal {

  BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(Has_member_visited,
                                    Has_visited_for_vertex_extractor,
                                    false)

} // end namespace internal
} // end namespace CGAL

#endif // CGAL_HAS_NESTED_TYPE_BARE_POINT_H
