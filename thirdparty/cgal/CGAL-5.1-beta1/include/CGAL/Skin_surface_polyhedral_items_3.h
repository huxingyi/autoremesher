// Copyright (c) 2005 Rijksuniversiteit Groningen (Netherlands)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Skin_surface_3/include/CGAL/Skin_surface_polyhedral_items_3.h $
// $Id: Skin_surface_polyhedral_items_3.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Nico Kruithof <Nico@cs.rug.nl>

#ifndef CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_3_H
#define CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_3_H

#include <CGAL/license/Skin_surface_3.h>

#include <CGAL/HalfedgeDS_face_base.h>
#include <CGAL/Polyhedron_items_3.h>
#include <CGAL/assertions.h>

namespace CGAL {

template<class Refs, class SkinSurface3>
struct Skin_Surface_polyhedral_face: public CGAL::HalfedgeDS_face_base<Refs>
{
  typedef SkinSurface3 Skin_surface;
  typedef typename SkinSurface3::TMC::Cell_handle TMC_Cell_handle;
  typedef typename SkinSurface3::Simplex Simplex;

  typename SkinSurface3::Simplex containing_simplex()
  {
    CGAL_assertion(tmc_ch != nullptr);
    return tmc_ch->info().first;
  }

  TMC_Cell_handle tmc_ch;
};

template<class SkinSurface3>
struct Skin_surface_polyhedral_items_3: public Polyhedron_items_3
{
  template<class Refs, class Traits>
  struct Face_wrapper
  {
    typedef Skin_Surface_polyhedral_face<Refs, SkinSurface3> Face;
  };
};

} //namespace CGAL

#endif // CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_3_H
