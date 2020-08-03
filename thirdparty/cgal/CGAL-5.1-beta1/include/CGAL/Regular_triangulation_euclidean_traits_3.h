// Copyright (c) 1999,2004   INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Triangulation_3/include/CGAL/Regular_triangulation_euclidean_traits_3.h $
// $Id: Regular_triangulation_euclidean_traits_3.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Sylvain Pion
//                 Monique Teillaud <Monique.Teillaud@sophia.inria.fr>
//                 Mariette Yvinec <Mariette.Yvinec@sophia.inria.fr>

#ifndef CGAL_REGULAR_TRIANGULATION_EUCLIDEAN_TRAITS_3_H
#define CGAL_REGULAR_TRIANGULATION_EUCLIDEAN_TRAITS_3_H

#include <CGAL/license/Triangulation_3.h>

#define CGAL_DEPRECATED_HEADER "<CGAL/Regular_triangulation_euclidean_traits_3.h>"
#define CGAL_DEPRECATED_MESSAGE_DETAILS \
  "The kernel K can be used directly as traits since weighted points and "\
  "the associated function objects are now part of the concept Kernel."
#include <CGAL/internal/deprecation_warning.h>

namespace CGAL {

template < class K_, class Weight = typename K_::RT >
class Regular_triangulation_euclidean_traits_3
  : public K_
{
public:
  Regular_triangulation_euclidean_traits_3() {}
  Regular_triangulation_euclidean_traits_3(const K_& k): K_(k) {}
};

} //namespace CGAL

#endif // CGAL_REGULAR_TRIANGULATION_EUCLIDEAN_TRAITS_3_H
