// Copyright (c) 1997-2010  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Kernel_23/include/CGAL/Projection_traits_xy_3.h $
// $Id: Projection_traits_xy_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_PROJECTION_TRAITS_XY_3_H
#define CGAL_PROJECTION_TRAITS_XY_3_H

#include <CGAL/internal/Projection_traits_3.h>

namespace CGAL {

template < class R >
class Projection_traits_xy_3
  : public internal::Projection_traits_3<R,2>
{};

} //namespace CGAL

#endif // CGAL_PROJECTION_TRAITS_XY_3_H
