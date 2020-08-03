// Copyright (c) 1997-2007  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Nef_3/include/CGAL/Nef_nary_union_3.h $
// $Id: Nef_nary_union_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     :     Peter Hachenberger  <hachenberger@mpi-sb.mpg.de>

#ifndef CGAL_NEF_NARY_UNION_3_H
#define CGAL_NEF_NARY_UNION_3_H

#include <CGAL/license/Nef_3.h>

#include <CGAL/disable_warnings.h>

#include <list>

namespace CGAL {

template<class Polyhedron>
class Nef_nary_union_3 {

  int inserted;
  std::list<Polyhedron> queue;
  typedef typename std::list<Polyhedron>::iterator pit;
  Polyhedron empty;

 public:
  Nef_nary_union_3() : inserted(0) {}

  void unite() {
    pit i1(queue.begin()), i2(i1);
    ++i2;

    Polyhedron tmp(*i1 + *i2);

    queue.pop_front();
    queue.pop_front();
    queue.push_front(tmp);
  }

  void add_polyhedron(const Polyhedron& P) {
    queue.push_front(P);
    ++inserted;
    for(int i=2;(inserted%i) == 0; i*=2) {
      unite();
    }
  }

  Polyhedron get_union() {

    while(queue.size() > 1)
      unite();
    inserted = 0;
    return queue.front();
  }
};

} //namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // CGAL_NEF_NARY_UNION_H
