// Copyright (c) 2019 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_topology/include/CGAL/Polygonal_schema_fwd.h $
// $Id: Polygonal_schema_fwd.h 0308d1a 2020-03-27T18:35:15+01:00 Guillaume Damiand
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_POLYGONAL_SCHEMA_FWD_H
#define CGAL_POLYGONAL_SCHEMA_FWD_H 1

#include <CGAL/license/Surface_mesh_topology.h>

#include <CGAL/memory.h>
#include <CGAL/Combinatorial_map_fwd.h>
#include <CGAL/Generalized_map_fwd.h>

namespace CGAL {
namespace Surface_mesh_topology {

struct Polygonal_schema_min_items;

template <class Items_=Polygonal_schema_min_items,
          class Alloc_=CGAL_ALLOCATOR(int),
          class Storage_= Combinatorial_map_storage_1<2, Items_, Alloc_> >
class Polygonal_schema_with_combinatorial_map;

template <class Items_=Polygonal_schema_min_items,
          class Alloc_=CGAL_ALLOCATOR(int),
          class Storage_= Generalized_map_storage_1<2, Items_, Alloc_> >
class Polygonal_schema_with_generalized_map;

} // Surface_mesh_topology
} // CGAL

#endif // CGAL_POLYGONAL_SCHEMA_FWD_H
