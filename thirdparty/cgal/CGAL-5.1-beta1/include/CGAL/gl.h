// Copyright (c) 2008 GeometryFactory, Sophia Antipolis (France)
//  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/gl.h $
// $Id: gl.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial

#ifndef CGAL_GL_H
#define CGAL_GL_H

#ifdef _MSC_VER
#  include <wtypes.h>
#  include <wingdi.h>
#endif

#ifdef __APPLE__
#  if TARGET_OS_IPHONE
#    include <OpenGLES/ES2/gl.h>
#  else
#    include <OpenGL/gl.h>
#  endif
#else
#  ifdef __arm__
#    include <GLES3/gl3.h>
#  else
#    include <GL/gl.h>
#  endif
#endif

#endif // CGAL_GL_H
