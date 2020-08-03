// Copyright (c) 2003  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/TDS_2/include/CGAL/Dummy_tds_2.h $
// $Id: Dummy_tds_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_TRIANGULATION_DUMMY_TDS_2_H
#define CGAL_TRIANGULATION_DUMMY_TDS_2_H

#include <CGAL/license/TDS_2.h>


#include <CGAL/config.h>

namespace CGAL {

// Dummy TDS which provides all types that a vertex_base or cell_base can use.
struct Dummy_tds_2 {
  struct Vertex {};
  struct Face {};
  struct Edge {};

  struct Vertex_handle {};
  struct Face_handle {};

  struct Vertex_iterator {};
  struct Face_iterator {};
  struct Edge_iterator {};

  struct Edge_circulator {};
  struct Facet_circulator {};
  struct Face_circulator {};
  struct Vertex_circulator {};

  typedef std::size_t size_type;
};

} //namespace CGAL

#endif // CGAL_TRIANGULATION_DUMMY_TDS_2_H
