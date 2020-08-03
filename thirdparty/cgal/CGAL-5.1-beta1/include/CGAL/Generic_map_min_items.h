// Copyright (c) 2016 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Combinatorial_map/include/CGAL/Generic_map_min_items.h $
// $Id: Generic_map_min_items.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_GENERIC_MAP_MIN_ITEMS_H
#define CGAL_GENERIC_MAP_MIN_ITEMS_H 1

namespace CGAL {

  /** @file Generic_map_min_items.h
   * Definition of min item class for dD generic map.
   */

  /** Minimal items for dD generic map.
   * Generic_map_min_items defines what is the minimal item class for a generic map.
   * No information associated with darts (i.e. the type Dart_info is
   * not defined), no enabled attribute (i.e. type Attributes not defined).
   */
  struct Generic_map_min_items
  {
    template < class Refs >
    struct Dart_wrapper
    {};
  };

} // namespace CGAL

#endif // CGAL_GENERIC_MAP_MIN_ITEMS_H
// EOF //
