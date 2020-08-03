// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Partition_2/include/CGAL/Partition_2/Indirect_not_less_yx_2.h $
// $Id: Indirect_not_less_yx_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef CGAL_INDIRECT_NOT_LESS_YX_2_H
#define CGAL_INDIRECT_NOT_LESS_YX_2_H

#include <CGAL/license/Partition_2.h>


namespace CGAL {

template <class Traits>
class Indirect_not_less_yx_2
{
public:
     typedef typename Traits::Point_2 Point_2;
     typedef typename Traits::Less_yx_2     Less_yx_2;

     Indirect_not_less_yx_2(const Traits& traits) :
         less_yx_2(traits.less_yx_2_object()) {}

     template <class Iterator>
     bool
     operator()( const Iterator& p, const Iterator& q) const
     { return less_yx_2( Point_2(*q), Point_2(*p)); }

   private:
     Less_yx_2 less_yx_2;
};

}

#endif // CGAL_INDIRECT_NOT_LESS_YX_2_H
