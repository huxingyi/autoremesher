// Copyright (c) 2010 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/Ray_3_Ray_3.h $
// $Id: Ray_3_Ray_3.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//

#ifndef CGAL_INTERSECTIONS_3_RAY_3_RAY_3_H
#define CGAL_INTERSECTIONS_3_RAY_3_RAY_3_H

#include <CGAL/Ray_3.h>

#include <CGAL/Intersections_3/internal/intersection_3_1_impl.h>

namespace CGAL {
CGAL_INTERSECTION_FUNCTION_SELF(Ray_3, 3)
CGAL_DO_INTERSECT_FUNCTION_SELF(Ray_3, 3)
}

#endif // CGAL_INTERSECTIONS_3_RAY_3_RAY_3_H
