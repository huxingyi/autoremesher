// Copyright (c) 2010-2011 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Combinatorial_map/include/CGAL/Combinatorial_map_fwd.h $
// $Id: Combinatorial_map_fwd.h 0308d1a 2020-03-27T18:35:15+01:00 Guillaume Damiand
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef COMBINATORIAL_MAP_FWD_H
#define COMBINATORIAL_MAP_FWD_H 1

#include <CGAL/memory.h>

namespace CGAL {

#if defined(CGAL_CMAP_DART_DEPRECATED) && !defined(CGAL_NO_DEPRECATED_CODE)
template <unsigned int d>
struct Combinatorial_map_min_items;
#else
struct Generic_map_min_items;
#endif

template<unsigned int d_, class Items_, class Alloc_ >
class Combinatorial_map_storage_1;

template < unsigned int d_, class Refs_,
#if defined(CGAL_CMAP_DART_DEPRECATED) && !defined(CGAL_NO_DEPRECATED_CODE)
           class Items_=Combinatorial_map_min_items<d_>,
#else
           class Items_=Generic_map_min_items,
#endif
           class Alloc_=CGAL_ALLOCATOR(int),
           class Storage_= Combinatorial_map_storage_1<d_, Items_, Alloc_> >
class Combinatorial_map_base;

template < unsigned int d_,
#if defined(CGAL_CMAP_DART_DEPRECATED) && !defined(CGAL_NO_DEPRECATED_CODE)
           class Items_=Combinatorial_map_min_items<d_>,
#else
           class Items_=Generic_map_min_items,
#endif
           class Alloc_=CGAL_ALLOCATOR(int),
           class Storage_= Combinatorial_map_storage_1<d_, Items_, Alloc_> >
class Combinatorial_map;

} // CGAL

#endif // COMBINATORIAL_MAP_FWD_H
