// Copyright (c) 2014
// INRIA Saclay-Ile de France (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/NewKernel_d/include/CGAL/NewKernel_d/Cartesian_per_dimension.h $
// $Id: Cartesian_per_dimension.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Marc Glisse

#ifndef CGAL_KD_CARTESIAN_PER_DIM_H
#define CGAL_KD_CARTESIAN_PER_DIM_H
#include <CGAL/NewKernel_d/functor_tags.h>
#include <CGAL/Dimension.h>
#include <CGAL/predicates/sign_of_determinant.h>

// Should probably disappear.

namespace CGAL {
template <class Dim_, class R_, class Derived_>
struct Cartesian_per_dimension : public R_ {};
}

#endif
