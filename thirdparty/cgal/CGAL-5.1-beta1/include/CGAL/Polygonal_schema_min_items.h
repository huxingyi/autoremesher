// Copyright (c) 2019 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_topology/include/CGAL/Polygonal_schema_min_items.h $
// $Id: Polygonal_schema_min_items.h 8bb22d5 2020-03-26T14:23:37+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_POLYGONAL_SCHEMA_MIN_ITEMS_H
#define CGAL_POLYGONAL_SCHEMA_MIN_ITEMS_H 1

#include <CGAL/license/Surface_mesh_topology.h>
#include <string>

namespace CGAL {
namespace Surface_mesh_topology {

  /** @file Polygonal_schema_min_items.h
   * Definition of min item class for Polygonal_schema.
   */

  /** Minimal items for polygonal schema.
   * Generic_map_min_items defines what is the minimal item class for a generic map.
   * One struct associated with darts, having one std::string named m_label..
   */
  struct Polygonal_schema_min_items
  {
    template < class Refs >
    struct Dart_wrapper
    {
      struct Info_for_darts
      { std::string m_label; };

      typedef Info_for_darts Dart_info;
    };
  };

} // namespace Surface_mesh_topology
} // namespace CGAL

#endif // CGAL_POLYGONAL_SCHEMA_MIN_ITEMS_H
// EOF //
