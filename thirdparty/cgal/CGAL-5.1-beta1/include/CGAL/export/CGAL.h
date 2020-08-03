// Copyright (c) 2011 GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/export/CGAL.h $
// $Id: CGAL.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_EXPORT_H
#define CGAL_EXPORT_H

#include <CGAL/config.h>
#include <CGAL/export/helpers.h>

#if defined(CGAL_BUILD_SHARED_LIBS) && ! defined(CGAL_HEADER_ONLY)

#  if defined(CGAL_EXPORTS) // defined by CMake or in cpp files of the dll

#    define CGAL_EXPORT CGAL_DLL_EXPORT
#    define CGAL_EXPIMP_TEMPLATE

#  else // not CGAL_EXPORTS

#    define CGAL_EXPORT CGAL_DLL_IMPORT
#    define CGAL_EXPIMP_TEMPLATE extern

#  endif // not CGAL_EXPORTS

#else // not CGAL_BUILD_SHARED_LIBS

#  define CGAL_EXPORT
#  define CGAL_EXPIMP_TEMPLATE

#endif // not CGAL_BUILD_SHARED_LIBS

#endif //  CGAL_EXPORT_H
