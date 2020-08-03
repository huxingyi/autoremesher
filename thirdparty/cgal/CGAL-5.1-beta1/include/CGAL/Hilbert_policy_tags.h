// Copyright (c) 2011  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Spatial_sorting/include/CGAL/Hilbert_policy_tags.h $
// $Id: Hilbert_policy_tags.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Olivier Devillers

#ifndef CGAL_HILBERT_POLICY_H
#define CGAL_HILBERT_POLICY_H


namespace CGAL {

struct Middle {};
struct Median {};


// A policy to select the sorting strategy.

template < typename Tag >
struct Hilbert_policy {};

typedef Hilbert_policy<Middle>      Hilbert_sort_middle_policy;
typedef Hilbert_policy<Median>      Hilbert_sort_median_policy;

} // namespace CGAL

#endif // CGAL_HILBERT_POLICY_H
