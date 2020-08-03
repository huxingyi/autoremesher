// Copyright (c) 2020 GeometryFactory (France) and Telecom Paris (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Tetrahedral_remeshing/include/CGAL/Tetrahedral_remeshing/Sizing_field.h $
// $Id: Sizing_field.h 825f77b 2020-04-10T14:14:30+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jane Tournois, Noura Faraj

#ifndef CGAL_SIZING_FIELD_H
#define CGAL_SIZING_FIELD_H

#include <CGAL/license/Tetrahedral_remeshing.h>

namespace CGAL
{
/*!
* Sizing field virtual class
*/
template <class Kernel>
class Sizing_field
{
public:
  typedef Kernel                      K;
  typedef typename Kernel::FT         FT;
  typedef typename Kernel::Point_3    Point_3;

public:
  virtual FT operator()(const Point_3& p) const = 0;
};

}//end namespace CGAL

#endif //CGAL_SIZING_FIELD_H
