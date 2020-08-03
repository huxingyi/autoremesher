// Copyright (c) 2019  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Triangulation_2/include/CGAL/boost/graph/properties_Constrained_triangulation_plus_2.h $
// $Id: properties_Constrained_triangulation_plus_2.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H
#define CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H

#include <CGAL/Constrained_triangulation_plus_2.h>

#define CGAL_2D_TRIANGULATION_TEMPLATE_PARAMETERS typename Tr
#define CGAL_2D_TRIANGULATION CGAL::Constrained_triangulation_plus_2<Tr>

#include <CGAL/boost/graph/internal/properties_2D_triangulation.h>

#endif /* CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H */
