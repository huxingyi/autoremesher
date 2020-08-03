// Copyright (c) 2005 Rijksuniversiteit Groningen (Netherlands)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Skin_surface_3/include/CGAL/mesh_skin_surface_3.h $
// $Id: mesh_skin_surface_3.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Nico Kruithof <Nico@cs.rug.nl>

#ifndef CGAL_MESH_SKIN_SURFACE_3_H
#define CGAL_MESH_SKIN_SURFACE_3_H

#include <CGAL/license/Skin_surface_3.h>

#include <CGAL/Cartesian_converter.h>
#include <CGAL/marching_tetrahedra_3.h>
#include <CGAL/Marching_tetrahedra_traits_skin_surface_3.h>
#include <CGAL/Marching_tetrahedra_observer_default_3.h>
#include <CGAL/Skin_surface_marching_tetrahedra_observer_3.h>
#include <CGAL/Skin_surface_polyhedral_items_3.h>

namespace CGAL {

template <class SkinSurface_3, class Polyhedron>
void mesh_skin_surface_3(SkinSurface_3 const &skin_surface, Polyhedron &p)
{
  skin_surface.mesh_surface_3(p);
}

//
//template <class SkinSurface_3, class P_Traits>
//void mesh_skin_surface_3
//(SkinSurface_3 const &skin_surface,
// Polyhedron_3<P_Traits, Skin_surface_polyhedral_items_3<SkinSurface_3> > &p)
//{
//  std::cout << "B" << std::endl;
//  skin_surface.mesh_skin_surface_3(p);
//}
//

} //namespace CGAL

#endif // CGAL_MESH_SKIN_SURFACE_3_H
