// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/Callback.h $
// $Id: Callback.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef CGAL_QT_CALLBACK_H
#define CGAL_QT_CALLBACK_H

#include <QObject>

#include "Utils.h"

class QRectF;
class QEvent;
class QKeyEvent;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;

namespace CGAL {
namespace Qt {

class Callback : public QObject, public QGraphicsSceneMixin
{
Q_OBJECT

public:
  Callback( QObject* parent );
  virtual void reset( );

public Q_SLOTS:
  virtual void slotModelChanged( );

Q_SIGNALS:
  void modelChanged( );

protected:
  virtual bool eventFilter( QObject* object, QEvent* event );
  virtual void mousePressEvent( QGraphicsSceneMouseEvent* event );
  virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
  virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
  virtual void keyPressEvent( QKeyEvent* event );
};

} // namespace Qt
} // namespace CGAL
#endif // CGAL_QT_CALLBACK_H
