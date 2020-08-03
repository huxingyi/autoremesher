// Copyright (c) 2017  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/Hidden_point_memory_policy.h $
// $Id: Hidden_point_memory_policy.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_HIDDEN_POINT_MEMORY_POLICY_H
#define CGAL_HIDDEN_POINT_MEMORY_POLICY_H

#include <CGAL/tags.h>

namespace CGAL {

// A policy to select whether hidden points should be cached in cells or discarded
// during the construction of a regular triangulation.

template < typename Tag >
struct Hidden_points_memory_policy : public Tag { };

typedef Hidden_points_memory_policy<Tag_true> Keep_hidden_points;
typedef Hidden_points_memory_policy<Tag_false> Discard_hidden_points;

} // namespace CGAL

#endif // CGAL_HIDDEN_POINT_MEMORY_POLICY_H
