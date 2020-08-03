// Copyright (c) 1999
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Homogeneous_kernel/include/CGAL/Homogeneous.h $
// $Id: Homogeneous.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Stefan Schirra

#ifndef CGAL_HOMOGENEOUS_H
#define CGAL_HOMOGENEOUS_H

#include <CGAL/Homogeneous/Homogeneous_base.h>
#include <CGAL/Handle_for.h>
#include <CGAL/Kernel/Type_equality_wrapper.h>
#include <CGAL/Quotient.h>

namespace CGAL {

template < typename RT_, typename FT_, typename Kernel >
struct Homogeneous_base_ref_count
  : public Homogeneous_base<RT_, FT_, Kernel >
{
    typedef RT_                                           RT;
    typedef FT_                                           FT;

    // The mechanism that allows to specify reference-counting or not.
    template < typename T >
    struct Handle { typedef Handle_for<T>    type; };

    template < typename Kernel2 >
    struct Base {
        typedef Homogeneous_base_ref_count<RT_,FT_,Kernel2> Type;
    };
};

template < typename RT_, typename FT_ = Quotient<RT_> >
struct Homogeneous
  : public Type_equality_wrapper<
                Homogeneous_base_ref_count<RT_, FT_, Homogeneous<RT_, FT_> >,
                Homogeneous<RT_, FT_> >
{};

} //namespace CGAL

#endif // CGAL_HOMOGENEOUS_H
