// Copyright (c) 2009-2011  GeometryFactory Sarl (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Three/include/CGAL/Three/Scene_item_config.h $
// $Id: Scene_item_config.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent RINEAU

#ifndef SCENE_ITEM_CONFIG_H
#define SCENE_ITEM_CONFIG_H

#include <CGAL/license/Three.h>


#include <QtCore/qglobal.h>

#ifdef demo_framework_EXPORTS
#  define scene_item_EXPORTS
#endif

#ifdef scene_item_EXPORTS
#  define SCENE_ITEM_EXPORT Q_DECL_EXPORT
#else
#  define SCENE_ITEM_EXPORT Q_DECL_IMPORT
#endif

#endif // SCENE_ITEM_CONFIG_H
