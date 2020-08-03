// Copyright (c) 2019  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Triangulation_2/include/CGAL/boost/graph/properties_Regular_triangulation_2.h $
// $Id: properties_Regular_triangulation_2.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_PROPERTIES_REGULAR_TRIANGULATION_2_H
#define CGAL_PROPERTIES_REGULAR_TRIANGULATION_2_H

#include <CGAL/Regular_triangulation_2.h>

#define CGAL_2D_TRIANGULATION_TEMPLATE_PARAMETERS typename GT, typename TDS
#define CGAL_2D_TRIANGULATION CGAL::Regular_triangulation_2<GT, TDS>

#include <CGAL/boost/graph/internal/properties_2D_triangulation.h>

#endif /* CGAL_PROPERTIES_REGULAR_TRIANGULATION_2_H */
