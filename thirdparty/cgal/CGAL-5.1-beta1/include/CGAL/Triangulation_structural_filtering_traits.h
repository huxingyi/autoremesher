// Copyright (c) 2010  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Kernel_23/include/CGAL/Triangulation_structural_filtering_traits.h $
// $Id: Triangulation_structural_filtering_traits.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Laurent Rineau <Laurent.Rineau__CGAL@normalesup.org>
//

#ifndef CGAL_TRIANGULATION_STRUCTURAL_FILTERING_TRAITS_H
#define CGAL_TRIANGULATION_STRUCTURAL_FILTERING_TRAITS_H

#include <CGAL/tags.h>

namespace CGAL {

template <typename Geom_traits>
struct Triangulation_structural_filtering_traits {
  typedef Tag_false Use_structural_filtering_tag;
};

} // namespace CGAL

#endif // CGAL_TRIANGULATION_STRUCTURAL_FILTERING_TRAITS_H
