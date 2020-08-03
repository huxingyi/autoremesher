// Copyright (c) 2005 Rijksuniversiteit Groningen (Netherlands)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Skin_surface_3/include/CGAL/Skin_surface_polyhedral_items_with_face_information.h $
// $Id: Skin_surface_polyhedral_items_with_face_information.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Nico Kruithof <Nico@cs.rug.nl>

#ifndef CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_WITH_FACE_INFORMATION_H
#define CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_WITH_FACE_INFORMATION_H

#include <CGAL/license/Skin_surface_3.h>

#include <CGAL/Polyhedron_items_3.h>

namespace CGAL {

template <class Refs, class TriangulatedMixedComplex3>
struct Skin_Surface_polyhedral_face : public CGAL::HalfedgeDS_face_base<Refs>
{
  typedef typename TriangulatedMixedComplex3::Cell_handle Triang_Cell_handle;

  Triang_Cell_handle triang_ch;
};

template < class TriangulatedMixedComplex3 >
class Skin_surface_polyhedral_items_with_face_information_3
  : public Polyhedron_items_3
{
  template <class Refs, class Traits>
  struct Face_wrapper
  {
    typedef Skin_Surface_polyhedral_face<Refs, TriangulatedMixedComplex3> Face;
  };
};

} //namespace CGAL

#endif // CGAL_SKIN_SURFACE_POLYHEDRAL_ITEMS_WITH_FACE_INFORMATION_H
