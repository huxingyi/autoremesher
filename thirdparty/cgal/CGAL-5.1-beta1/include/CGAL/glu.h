// Copyright (c) 2008 GeometryFactory, Sophia Antipolis (France)
//  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/glu.h $
// $Id: glu.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial

#ifndef CGAL_GLU_H
#define CGAL_GLU_H

#ifdef _MSC_VER
#  include <wtypes.h>
#  include <wingdi.h>
#endif

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#endif // CGAL_GLU_H
