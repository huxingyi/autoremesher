// Copyright (c) 2011 GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/CGAL_ImageIO/include/CGAL/export/ImageIO.h $
// $Id: ImageIO.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_IMAGEIO_EXPORT_H
#define CGAL_IMAGEIO_EXPORT_H

#include <CGAL/config.h>
#include <CGAL/export/helpers.h>

#if defined(CGAL_BUILD_SHARED_LIBS) && ! defined(CGAL_HEADER_ONLY)

#  if defined(CGAL_ImageIO_EXPORTS) // defined by CMake or in cpp files of the dll

#    define CGAL_IMAGEIO_EXPORT CGAL_DLL_EXPORT
#    define CGAL_IMAGEIO_EXPIMP_TEMPLATE

#  else // not CGAL_ImageIO_EXPORTS

#    define CGAL_IMAGEIO_EXPORT CGAL_DLL_IMPORT
#    define CGAL_IMAGEIO_EXPIMP_TEMPLATE extern

#  endif // not CGAL_IMAGEIO_EXPORTS

#else // not CGAL_BUILD_SHARED_LIBS

#  define CGAL_IMAGEIO_EXPORT
#  define CGAL_IMAGEIO_EXPIMP_TEMPLATE

#endif // not CGAL_BUILD_SHARED_LIBS

#endif //  CGAL_IMAGEIO_EXPORT_H


