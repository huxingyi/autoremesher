// Copyright (c) 1997  ETH Zurich (Switzerland).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Polyhedron_IO/include/CGAL/IO/Polyhedron_VRML_2_ostream.h $
// $Id: Polyhedron_VRML_2_ostream.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>

#ifndef CGAL_IO_POLYHEDRON_VRML_2_OSTREAM_H
#define CGAL_IO_POLYHEDRON_VRML_2_OSTREAM_H 1

#include <CGAL/license/Polyhedron.h>


#include <CGAL/basic.h>
#include <CGAL/IO/VRML_2_ostream.h>
#include <CGAL/IO/File_writer_VRML_2.h>
#include <CGAL/IO/generic_print_polyhedron.h>
#include <CGAL/Polyhedron_3.h>
#include <iostream>

namespace CGAL {

template < class Traits,
           class Items,
           template < class T, class I, class A>
           class HDS, class Alloc>
VRML_2_ostream&
operator<<( VRML_2_ostream& out,
            const Polyhedron_3<Traits,Items,HDS,Alloc>& P) {
    File_writer_VRML_2  writer;
    generic_print_polyhedron( out.os(), P, writer);
    return out;
}

} //namespace CGAL
#endif // CGAL_IO_POLYHEDRON_VRML_2_OSTREAM_H //
// EOF //
