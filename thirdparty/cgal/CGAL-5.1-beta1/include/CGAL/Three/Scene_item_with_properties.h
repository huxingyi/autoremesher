// Copyright (c) 2016  GeometryFactory Sarl (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Three/include/CGAL/Three/Scene_item_with_properties.h $
// $Id: Scene_item_with_properties.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Maxime GIMENO

#ifndef SCENE_ITEM_WITH_PROPERTIES_H
#define SCENE_ITEM_WITH_PROPERTIES_H

#include <CGAL/license/Three.h>

namespace CGAL
{
namespace Three {
  class Scene_item;

//! Base class to allow an item to copy properties from another.
//! Properties reprensent the current state of an item : its color,
//! the position of its manipulated frame, ...
class Scene_item_with_properties {
public:
  virtual ~Scene_item_with_properties(){}
 //!\brief Copies properties from another Scene_item.
 //!
 //! Override this function to specify what must be copied.
 virtual void copyProperties(Scene_item*)=0; //pure virtual method
};
}
}
#endif // SCENE_ITEM_WITH_PROPERTIES_H

