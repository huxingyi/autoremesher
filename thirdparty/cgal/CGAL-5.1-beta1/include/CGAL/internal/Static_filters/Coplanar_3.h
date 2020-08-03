// Copyright (c) 2011 GeometryFactory (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Filtered_kernel/include/CGAL/internal/Static_filters/Coplanar_3.h $
// $Id: Coplanar_3.h 8bb22d5 2020-03-26T14:23:37+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Laurent Rineau


#ifndef CGAL_INTERNAL_STATIC_FILTERS_COPLANAR_3_H
#define CGAL_INTERNAL_STATIC_FILTERS_COPLANAR_3_H

namespace CGAL {

namespace internal {

namespace Static_filters_predicates {


template < typename K_base, typename SFK >
class Coplanar_3
  : public K_base::Coplanar_3
{
  typedef typename K_base::Point_3 Point_3;
  typedef typename K_base::Coplanar_3 Base;
  typedef typename SFK::Orientation_3 Orientation_3;

public:

  typedef typename Base::result_type  result_type;



  result_type
  operator()(const Point_3& p,const Point_3& q, const Point_3& r, const Point_3& s) const
  {
    return Orientation_3()(p,q,r,s) == COPLANAR;
  }



}; // end class Coplanar_3

} // end namespace Static_filters_predicates

} // end namespace internal

} // end namespace CGAL

#endif  // CGAL_INTERNAL_STATIC_FILTERS_COPLANAR_3_H
