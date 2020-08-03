// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/include/CGAL/Qt/GraphicsViewPointInput.h $
// $Id: GraphicsViewPointInput.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial

#ifndef CGAL_QT_GRAPHICS_VIEW_POINT_INPUT_H
#define CGAL_QT_GRAPHICS_VIEW_POINT_INPUT_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <CGAL/Qt/GraphicsViewInput.h>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <iostream>
namespace CGAL {
namespace Qt {
template < class K >
class GraphicsViewPointInput: public GraphicsViewInput
{
public:
    typedef typename K::Point_2 Point;

    GraphicsViewPointInput( QObject* parent );

protected:
    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
    bool eventFilter( QObject* obj, QEvent* event );

    Converter< K > convert;
    Point point;

}; // class GraphicsViewPointInput

template < class K >
GraphicsViewPointInput< K >::
GraphicsViewPointInput( QObject* parent ):
    GraphicsViewInput( parent )
{

}

template < class K >
void
GraphicsViewPointInput< K >::
mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    std::cout << event->pos( ).x( ) << std::endl;
    this->point = this->convert( event->scenePos( ) );
}

template < class K >
void
GraphicsViewPointInput< K >::
mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    emit generate( CGAL::make_object( this->point ) );
}

template < class K >
bool
GraphicsViewPointInput< K >::
eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type( ) == QEvent::GraphicsSceneMousePress )
    {
        QGraphicsSceneMouseEvent* mouseEvent =
            static_cast< QGraphicsSceneMouseEvent* >( event );
        this->mousePressEvent( mouseEvent );
    }
    else if ( event->type( ) == QEvent::GraphicsSceneMouseRelease )
    {
        QGraphicsSceneMouseEvent* mouseEvent =
            static_cast< QGraphicsSceneMouseEvent* >( event );
        this->mouseReleaseEvent( mouseEvent );
    }
    return QObject::eventFilter( obj, event );
}

} // namespace Qt
} // namespace CGAL
#endif // CGAL_QT_GRAPHICS_VIEW_POINT_INPUT_H
