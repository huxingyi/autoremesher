// Copyright (c) 2016 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Generalized_map/include/CGAL/Generalized_map_fwd.h $
// $Id: Generalized_map_fwd.h 0308d1a 2020-03-27T18:35:15+01:00 Guillaume Damiand
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef GENERALIZED_MAP_FWD_H
#define GENERALIZED_MAP_FWD_H 1

#include <CGAL/memory.h>

namespace CGAL {

template<unsigned int d_, class Items_, class Alloc_ >
class Generalized_map_storage_1;

struct Generic_map_min_items;

template < unsigned int d_, class Refs,
           class Items_=Generic_map_min_items,
           class Alloc_=CGAL_ALLOCATOR(int),
           class Storage_= Generalized_map_storage_1<d_, Items_, Alloc_> >
class Generalized_map_base;

template < unsigned int d_,
           class Items_=Generic_map_min_items,
           class Alloc_=CGAL_ALLOCATOR(int),
           class Storage_= Generalized_map_storage_1<d_, Items_, Alloc_> >
class Generalized_map;

} // CGAL

#endif // GENERALIZED_MAP_FWD_H
