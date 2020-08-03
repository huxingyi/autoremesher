// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/PointsGraphicsItem.h $
// $Id: PointsGraphicsItem.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef POINTS_GRAPHICS_ITEM_H
#define POINTS_GRAPHICS_ITEM_H

#include <vector>
#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/number_utils.h>
#include <QPen>

class QPainter;
class QPen;

/**
   Add a set of points to the QGraphicsScene.
*/
class PointsGraphicsItem: public CGAL::Qt::GraphicsItem
{
public:
  PointsGraphicsItem( );

  virtual void paint( QPainter* painter,
                      const QStyleOptionGraphicsItem* option, QWidget* widget );
  virtual QRectF boundingRect( ) const;

  template < class Point >
  void insert( const Point& point )
  {
    this->prepareGeometryChange( );

    double x = CGAL::to_double( point.x( ) );
    double y = CGAL::to_double( point.y( ) );
    this->points.push_back( QPointF( x, y ) );
  }

  void clear( );

  void setColor( QColor c );
  QColor getColor( ) const;

  void setPointRadius( double d );
  double getPointRadius( ) const;

public Q_SLOTS:
  virtual void modelChanged( );

protected:
  std::vector< QPointF > points;
  double pointRadius;
  QColor color;

}; // class PointsGraphicsItem

#endif // POINTS_GRAPHICS_ITEM_H
