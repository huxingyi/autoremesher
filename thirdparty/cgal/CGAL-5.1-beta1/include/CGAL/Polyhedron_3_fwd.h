// Copyright (C) 2018  GeometryFactory Sarl
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/Polyhedron_3_fwd.h $
// $Id: Polyhedron_3_fwd.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//

#ifndef CGAL_POLYHEDRON_3_FWD_H
#define CGAL_POLYHEDRON_3_FWD_H

#include <CGAL/memory.h>

/// \file Polyhedron_3_fwd.h
/// Forward declarations of the Polyhedron_3 package.

#ifndef DOXYGEN_RUNNING
namespace CGAL {

  class Polyhedron_items_3;

  template < class T, class I, class A>
  class HalfedgeDS_default;

  template < class PolyhedronTraits_3,
             class PolyhedronItems_3 = Polyhedron_items_3,
             template < class T, class I, class A>
             class T_HDS = HalfedgeDS_default,
             class Alloc = CGAL_ALLOCATOR(int)
             >
  class Polyhedron_3;

} // CGAL
#endif

#endif /* CGAL_POLYHEDRON_3_FWD_H */


