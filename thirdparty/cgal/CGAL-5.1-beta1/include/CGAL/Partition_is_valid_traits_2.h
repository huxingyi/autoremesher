// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Partition_2/include/CGAL/Partition_is_valid_traits_2.h $
// $Id: Partition_is_valid_traits_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef CGAL_PARTITION_2_IS_VALID_TRAITS_H
#define CGAL_PARTITION_2_IS_VALID_TRAITS_H

#include <CGAL/license/Partition_2.h>


namespace CGAL {

template <class Traits, class PolygonIsValid>
class Partition_is_valid_traits_2 : public Traits
{
public:
   typedef typename Traits::Point_2         Point_2;
   typedef typename Traits::Polygon_2       Polygon_2;
   typedef typename Traits::Less_xy_2       Less_xy_2;
   typedef typename Traits::Left_turn_2      Left_turn_2;
   typedef typename Traits::Orientation_2   Orientation_2;

   typedef PolygonIsValid                   Is_valid;

  Partition_is_valid_traits_2()
  {}

  Partition_is_valid_traits_2(const Traits& traits)
    : Traits(traits)
  {}
public:
   Is_valid
   is_valid_object(const Traits& traits) const
   {  return Is_valid(traits); }
};

}

#endif // CGAL_PARTITION_2_IS_VALID_TRAITS_H
