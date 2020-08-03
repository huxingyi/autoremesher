// Copyright (c) 1999,2007
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Number_types/include/CGAL/sse2.h $
// $Id: sse2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     :  Andreas Fabri

#ifndef CGAL_SSE2_H
#define CGAL_SSE2_H

#include <emmintrin.h>

#if defined ( _MSC_VER )
#define CGAL_ALIGN_16  __declspec(align(16))
#elif defined( __GNUC__ )
#define  CGAL_ALIGN_16 __attribute__((aligned(16)))
#endif

#endif // CGAL_SSE2_H
