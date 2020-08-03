// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/ISnappable.h $
// $Id: ISnappable.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef ISNAPPABLE_H
#define ISNAPPABLE_H

class ISnappable
{
public:
  virtual ~ISnappable( ) { }
  virtual void setSnappingEnabled( bool b ) = 0;
  virtual void setSnapToGridEnabled( bool b ) = 0;
}; // class ISnappable


#endif // SNAPPABLE_H
