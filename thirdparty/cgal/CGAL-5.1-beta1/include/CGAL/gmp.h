// Copyright (c) 2010 GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Number_types/include/CGAL/gmp.h $
// $Id: gmp.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author: Andreas Fabri

#ifndef CGAL_GMP_H
#define CGAL_GMP_H 1

#include <CGAL/config.h>
#include <CGAL/disable_warnings.h>
#if defined(BOOST_MSVC)
#  pragma warning(push)
#  pragma warning(disable: 4127 4244 4146) // conversion with loss of data
                                     // warning on - applied on unsigned number
#endif

#include <gmp.h>


#if defined(BOOST_MSVC)
#  pragma warning(pop)
#endif

#include <CGAL/enable_warnings.h>

#endif // CGAL_GMP_H
