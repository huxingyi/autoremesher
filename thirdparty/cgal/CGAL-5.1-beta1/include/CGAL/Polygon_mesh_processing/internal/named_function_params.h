// Copyright (c) 2015 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Polygon_mesh_processing/include/CGAL/Polygon_mesh_processing/internal/named_function_params.h $
// $Id: named_function_params.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jane Tournois

#ifndef CGAL_PMP_BGL_NAMED_FUNCTION_PARAMS_H
#define CGAL_PMP_BGL_NAMED_FUNCTION_PARAMS_H

#include <CGAL/license/Polygon_mesh_processing/core.h>


#include <CGAL/boost/graph/Named_function_parameters.h>

#define CGAL_PMP_NP_TEMPLATE_PARAMETERS CGAL_BGL_NP_TEMPLATE_PARAMETERS
#define CGAL_PMP_NP_CLASS CGAL_BGL_NP_CLASS

namespace CGAL { namespace Polygon_mesh_processing { namespace parameters = CGAL::parameters; } }

#endif //CGAL_PMP_BGL_NAMED_FUNCTION_PARAMS_H
