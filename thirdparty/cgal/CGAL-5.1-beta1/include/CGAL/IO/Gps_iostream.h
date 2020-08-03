// Copyright (c) 2009  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Boolean_set_operations_2/include/CGAL/IO/Gps_iostream.h $
// $Id: Gps_iostream.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Efi Fogel <efif@post.tau.ac.il>

#ifndef CGAL_GPS_IOSTREAM_H
#define CGAL_GPS_IOSTREAM_H

#include <CGAL/license/Boolean_set_operations_2.h>

#include <CGAL/disable_warnings.h>

#include <iostream>
#include <list>

#include <CGAL/basic.h>
#include <CGAL/General_polygon_set_2.h>

namespace CGAL {

template <typename Traits>
std::ostream & operator<< (std::ostream& os,
                           const CGAL::General_polygon_set_2<Traits> & pgn_set)
{
  typedef typename CGAL::General_polygon_set_2<Traits>::Polygon_with_holes_2
                                                        Polygon_with_holes_2;
  typedef std::list<Polygon_with_holes_2>               Pgn_with_holes_container;

  Pgn_with_holes_container res;
  pgn_set.polygons_with_holes (std::back_inserter (res));

  std::cout << pgn_set.number_of_polygons_with_holes() << std::endl;
  std::copy(res.begin(), res.end(),
            std::ostream_iterator<Polygon_with_holes_2>(std::cout, "\n"));

  return os;
}

} //namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
