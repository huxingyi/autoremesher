// Copyright (c) 1997  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Alpha_shapes_3/include/CGAL/Weighted_alpha_shape_euclidean_traits_3.h $
// $Id: Weighted_alpha_shape_euclidean_traits_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Tran Kai Frank DA <Frank.Da@sophia.inria.fr>

#ifndef CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H
#define CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H

#include <CGAL/license/Alpha_shapes_3.h>

#define CGAL_DEPRECATED_HEADER "<CGAL/Weighted_alpha_shape_euclidean_traits_3.h>"
#define CGAL_DEPRECATED_MESSAGE_DETAILS \
  "The kernel K can be used directly as traits since weighted points and "\
  "the associated function objects are now part of the concept Kernel."
#include <CGAL/internal/deprecation_warning.h>

namespace CGAL {

template < class K_ >
class Weighted_alpha_shape_euclidean_traits_3
  : public K_
{
public:
  Weighted_alpha_shape_euclidean_traits_3() { }
  Weighted_alpha_shape_euclidean_traits_3(const K_& k) : K_(k) { }
};

} // namespace CGAL

#endif // CGAL_WEIGHTED_ALPHA_SHAPE_EUCLIDEAN_TRAITS_3_H
