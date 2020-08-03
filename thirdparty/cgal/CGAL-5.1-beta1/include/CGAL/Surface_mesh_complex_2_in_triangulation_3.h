// Copyright (c) 2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesher/include/CGAL/Surface_mesh_complex_2_in_triangulation_3.h $
// $Id: Surface_mesh_complex_2_in_triangulation_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_SURFACE_MESH_COMPLEX_2_IN_TRIANGULATION_3_H
#define CGAL_SURFACE_MESH_COMPLEX_2_IN_TRIANGULATION_3_H

#include <CGAL/license/Surface_mesher.h>

#include <CGAL/disable_warnings.h>

#include <CGAL/Complex_2_in_triangulation_3.h>

namespace CGAL {

/**  Simple forward class for CGAL::Complex_2_in_triangulation_3<Tr>.
 *
 *   CGAL::Surface_mesher_complex_2_in_triangulation_3<Tr> is an alias
 *   for CGAL::Complex_2_in_triangulation_3<Tr>.
 */
template <class Tr>
class Surface_mesh_complex_2_in_triangulation_3 :
    public Complex_2_in_triangulation_3<Tr>
{
public:
  Surface_mesh_complex_2_in_triangulation_3 (Tr& tr)
    : Complex_2_in_triangulation_3<Tr>(tr)
  {
  }
}; // end Surface_mesh_complex_2_in_triangulation_3

} // end namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // CGAL_SURFACE_MESH_COMPLEX_2_IN_TRIANGULATION_3_H
