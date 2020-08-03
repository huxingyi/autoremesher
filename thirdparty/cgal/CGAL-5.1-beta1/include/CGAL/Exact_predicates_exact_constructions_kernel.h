// Copyright (c) 2003
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Kernel_23/include/CGAL/Exact_predicates_exact_constructions_kernel.h $
// $Id: Exact_predicates_exact_constructions_kernel.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas, Sylvain Pion

#ifndef CGAL_EXACT_PREDICATES_EXACT_CONSTRUCTIONS_KERNEL_H
#define CGAL_EXACT_PREDICATES_EXACT_CONSTRUCTIONS_KERNEL_H

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Triangulation_structural_filtering_traits.h>
#include <CGAL/internal/Exact_type_selector.h>

#ifndef CGAL_DONT_USE_LAZY_KERNEL
#  include <CGAL/Lazy_kernel.h>
#endif

namespace CGAL {

// Epeck_ft is either Gmpq, or leda_rational, or Quotient<MP_float>
typedef internal::Exact_field_selector<double>::Type Epeck_ft;

// The following are redefined kernels instead of simple typedefs in order to shorten
// template name length (for error messages, mangling...).

#ifdef CGAL_DONT_USE_LAZY_KERNEL

// Equivalent to Filtered_kernel<Simple_cartesian<Lazy_exact_nt<Epeck_ft> > >
class Epeck
  : public Filtered_kernel_adaptor<
               Type_equality_wrapper< Simple_cartesian<Lazy_exact_nt<Epeck_ft> >::Base<Epeck>::Type, Epeck >,
#ifdef CGAL_NO_STATIC_FILTERS
               false >
#else
               true >
#endif
{}; // end class Epeck

#else // no CGAL_DONT_USE_LAZY_KERNEL

// Equivalent to Lazy_kernel<Simple_cartesian<Epeck_ft> >
class Epeck
  : public Type_equality_wrapper<
             Lazy_kernel_base< Simple_cartesian<Epeck_ft>,
                               Simple_cartesian<Interval_nt_advanced>,
                               Cartesian_converter< Simple_cartesian<Epeck_ft>,
                                                    Simple_cartesian<Interval_nt_advanced> >,
                               Epeck>,
             Epeck >
{};

#endif // no CGAL_DONT_USE_LAZY_KERNEL

typedef Epeck Exact_predicates_exact_constructions_kernel;

template <>
struct Triangulation_structural_filtering_traits<Epeck> {
  typedef Tag_true Use_structural_filtering_tag;
};

} //namespace CGAL

#endif // CGAL_EXACT_PREDICATES_EXACT_CONSTRUCTIONS_KERNEL_H
