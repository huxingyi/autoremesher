// Copyright (c) 2010 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_3/include/CGAL/Intersections_3/Sphere_3_Tetrahedron_3.h $
// $Id: Sphere_3_Tetrahedron_3.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//

#ifndef CGAL_INTERSECTIONS_3_SPHERE_3_TETRAHEDRON_3_H
#define CGAL_INTERSECTIONS_3_SPHERE_3_TETRAHEDRON_3_H

#include <CGAL/Sphere_3.h>
#include <CGAL/Tetrahedron_3.h>

#include <CGAL/Intersections_3/internal/Tetrahedron_3_Bounded_3_do_intersect.h>

namespace CGAL {
  CGAL_DO_INTERSECT_FUNCTION(Sphere_3, Tetrahedron_3, 3)
}

#endif // CGAL_INTERSECTIONS_3_SPHERE_3_TETRAHEDRON_3_H
