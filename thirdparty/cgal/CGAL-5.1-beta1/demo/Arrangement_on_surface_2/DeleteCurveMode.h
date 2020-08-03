// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/DeleteCurveMode.h $
// $Id: DeleteCurveMode.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef DELETE_CURVE_MODE_H
#define DELETE_CURVE_MODE_H

#include <QMetaType>

class QString;

/**
An attribute describing the policy for deleting curves from the arrangement.
*/
class DeleteCurveMode
{
public:
  enum Mode {
    DELETE_CURVE,
    DELETE_EDGE
  };

  DeleteCurveMode( );
  DeleteCurveMode( const DeleteCurveMode& dcm );
  DeleteCurveMode( Mode mode );
  ~DeleteCurveMode( );

  Mode mode( ) const;
  void setMode( Mode mode );

  static QString ToString( const DeleteCurveMode& mode );

protected:
  Mode m_mode;
};

Q_DECLARE_METATYPE( DeleteCurveMode )

#endif // DELETE_CURVE_MODE_H
