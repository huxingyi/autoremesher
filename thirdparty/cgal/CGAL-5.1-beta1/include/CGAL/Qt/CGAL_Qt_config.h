// Copyright (c) 2011 GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/GraphicsView/include/CGAL/Qt/CGAL_Qt_config.h $
// $Id: CGAL_Qt_config.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_QT_CONFIG_H
#define CGAL_QT_CONFIG_H

#include <QtCore/qglobal.h>

#if defined(CGAL_Qt5_DLL)
#  if defined(CGAL_Qt5_EXPORTS)
#    define CGAL_QT_EXPORT Q_DECL_EXPORT
#  else
#    define CGAL_QT_EXPORT Q_DECL_IMPORT
#  endif
#else
// empty definition
#  define CGAL_QT_EXPORT
#endif

#endif // CGAL_QT_CONFIG_H
