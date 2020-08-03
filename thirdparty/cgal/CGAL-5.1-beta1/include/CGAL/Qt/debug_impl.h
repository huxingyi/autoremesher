// Copyright (c) 2008  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/GraphicsView/include/CGAL/Qt/debug_impl.h $
// $Id: debug_impl.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri <Andreas.Fabri@geometryfactory.com>
//                 Laurent Rineau <Laurent.Rineau@geometryfactory.com>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <CGAL/license/GraphicsView.h>

#else
#define CGAL_INLINE_FUNCTION
#endif

#include <CGAL/Qt/debug.h>
#include <QDir>
#include <iostream>
#include <QtOpenGL/qgl.h>
#include <qopenglfunctions.h>
namespace CGAL {
namespace Qt {


CGAL_INLINE_FUNCTION
void traverse_resources(const QString& name, const QString& dirname, int indent)
{
  std::cerr << qPrintable(QString(indent, ' '))
            << qPrintable(name);
  QString fullname =
    dirname.isEmpty() ?
    name :
    dirname + "/" + name;
  QDir dir(fullname);
  if(dir.exists()) {
    std::cerr << "/\n";
    Q_FOREACH(QString path, dir.entryList())
    {
      traverse_resources(path, fullname, indent + 2);
    }
  }
  else {
    std::cerr << "\n";
  }
}

} // namesapce Qt
} // namespace CGAL
