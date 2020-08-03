// Copyright (c) 2007   INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/use.h $
// $Id: use.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_USE_H
#define CGAL_USE_H

namespace CGAL { namespace internal {

template < typename T > inline
void use(const T&) {}

template<typename> void use_type() {}

} }

/// CGAL_USE() is a macro which aims at removing "variable is unused" warnings.
#define CGAL_USE(x) ::CGAL::internal::use(x)

/// CGAL_USE_TYPE() is a macro which aims at removing "typedef locally
/// defined but not used" warnings.
#define CGAL_USE_TYPE(T) ::CGAL::internal::use_type<T>()

#endif // CGAL_USE_H
