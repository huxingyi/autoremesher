// Copyright (c) 2011  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/GraphicsView/include/CGAL/Qt/resources_impl.h $
// $Id: resources_impl.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau <Laurent.Rineau@geometryfactory.com>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <CGAL/license/GraphicsView.h>

#else
#define CGAL_INLINE_FUNCTION
#endif

#include <QDir>
#include <CGAL/Qt/resources.h>

// cannot use namespaces because of the Q_INIT_RESOURCE macro
CGAL_INLINE_FUNCTION
void CGAL_Qt_init_resources() {
  Q_INIT_RESOURCE(File);
  Q_INIT_RESOURCE(Triangulation_2);
  Q_INIT_RESOURCE(Input);
  Q_INIT_RESOURCE(CGAL);
}
