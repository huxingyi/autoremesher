// Copyright (c) 1997  ETH Zurich (Switzerland).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Polyhedron_IO/include/CGAL/IO/scan_OFF.h $
// $Id: scan_OFF.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>

#ifndef CGAL_IO_SCAN_OFF_H
#define CGAL_IO_SCAN_OFF_H 1

#include <CGAL/license/Polyhedron.h>


#include <CGAL/IO/Polyhedron_scan_OFF.h>
#include <CGAL/Polyhedron_3.h>
#include <iostream>

namespace CGAL {

template < class Traits,
           class Items,
           template < class T, class I, class A>
           class HDS, class Alloc>
void scan_OFF( std::istream& in,
               Polyhedron_3<Traits,Items,HDS,Alloc>& P,
               File_header_OFF& header) {
    // reads a polyhedron from `in' and appends it to P.
    // Returns also the File_header_OFF structure of the object.
    typedef Polyhedron_3<Traits,Items,HDS,Alloc> Polyhedron;
    typedef typename Polyhedron::HalfedgeDS HalfedgeDS;
    typedef Polyhedron_scan_OFF<HalfedgeDS> Scanner;
    Scanner scanner( in, header.verbose());
    P.delegate(scanner);
    header = scanner.header();
}

template < class Traits,
           class Items,
           template < class T, class I, class A>
           class HDS, class Alloc>
void scan_OFF( std::istream& in, Polyhedron_3<Traits,Items,HDS,Alloc>& P,
               bool verbose = false) {
    // reads a polyhedron from `in' and appends it to P.
    typedef Polyhedron_3<Traits,Items,HDS,Alloc> Polyhedron;
    typedef typename Polyhedron::HalfedgeDS HalfedgeDS;
    typedef Polyhedron_scan_OFF<HalfedgeDS> Scanner;
    Scanner scanner( in, verbose);
    P.delegate(scanner);
}


} //namespace CGAL
#endif // CGAL_IO_SCAN_OFF_H //
// EOF //
