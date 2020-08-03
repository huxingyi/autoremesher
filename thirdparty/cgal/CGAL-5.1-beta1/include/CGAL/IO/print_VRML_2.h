// Copyright (c) 1997  ETH Zurich (Switzerland).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Polyhedron_IO/include/CGAL/IO/print_VRML_2.h $
// $Id: print_VRML_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>

#ifndef CGAL_IO_PRINT_VRML_2_H
#define CGAL_IO_PRINT_VRML_2_H 1

#include <CGAL/license/Polyhedron.h>


#include <CGAL/IO/Polyhedron_VRML_2_ostream.h>

namespace CGAL {

template <class Polyhedron>
void print_polyhedron_VRML_2( std::ostream& out, const Polyhedron& P) {
    VRML_2_ostream os( out);
    os << P;
}

// Deprecated global functions, replaced with functions above

template < class Traits,
           class Items,
           template < class T, class I, class A>
           class HDS, class Alloc>
void
print_VRML_2( std::ostream& out,
              const Polyhedron_3<Traits,Items,HDS,Alloc>& P) {
    VRML_2_ostream os( out);
    os << P;
}

} //namespace CGAL
#endif // CGAL_IO_PRINT_VRML_2_H //
// EOF //
