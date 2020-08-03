// Copyright (c) 2012-2015  GeometryFactory Sarl (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Three/include/CGAL/Three/Viewer_config.h $
// $Id: Viewer_config.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent RINEAU, Maxime Gimeno

#ifndef VIEWER_CONFIG_H
#define VIEWER_CONFIG_H

#include <CGAL/license/Three.h>


#include <QtCore/qglobal.h>

#ifdef demo_framework_EXPORTS
#  define viewer_EXPORTS
#endif

#ifdef viewer_EXPORTS
#  define VIEWER_EXPORT Q_DECL_EXPORT
#else
#  define VIEWER_EXPORT Q_DECL_IMPORT
#endif

#endif // VIEWER_CONFIG_H
