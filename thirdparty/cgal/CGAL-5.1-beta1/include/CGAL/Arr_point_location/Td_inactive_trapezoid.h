// Copyright (c) 2005,2006,2007,2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/include/CGAL/Arr_point_location/Td_inactive_trapezoid.h $
// $Id: Td_inactive_trapezoid.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)         : Oren Nechushtan <theoren@math.tau.ac.il>
//               updated by: Michal Balas <balasmic@post.tau.ac.il>

#ifndef CGAL_TD_INACTIVE_TRAPEZOID_H
#define CGAL_TD_INACTIVE_TRAPEZOID_H

#include <CGAL/license/Arrangement_on_surface_2.h>


/*! \file
 * Defintion of the Td_inactive_trapezoid class.
 */

#include <CGAL/Arr_point_location/Trapezoidal_decomposition_2.h>
#include <boost/variant.hpp>


#ifdef CGAL_TD_DEBUG
#define CGAL_TD_INLINE
#else
#define CGAL_TD_INLINE inline
#endif

namespace CGAL {

/*! \class
 */
class Td_inactive_trapezoid
{
public:
  /*! Operator==. */
  inline bool operator== (const Td_inactive_trapezoid& t2) const
  {
    return (this == &t2);
  }
};

} //namespace CGAL

#endif
