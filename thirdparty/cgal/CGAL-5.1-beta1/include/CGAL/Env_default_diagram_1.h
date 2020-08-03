// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Envelope_2/include/CGAL/Env_default_diagram_1.h $
// $Id: Env_default_diagram_1.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Ron Wein   <wein@post.tau.ac.il>

#include <CGAL/basic.h>

#ifndef CGAL_ENV_DEFAULT_DIAGRAM_1_H
#define CGAL_ENV_DEFAULT_DIAGRAM_1_H

#include <CGAL/license/Envelope_2.h>

#define CGAL_DEPRECATED_HEADER "<CGAL/Env_default_diagram_1.h>"
#define CGAL_REPLACEMENT_HEADER "<CGAL/Envelope_diagram_1>"
#include <CGAL/internal/deprecation_warning.h>

#if (defined __GNUC__)
  #if !(defined __STRICT_ANSI__)
  #warning "Env_default_diagram.h is DEPRECATED, please include Envelope_diagram_1.h instead."
  #endif
#elif (defined _MSC_VER)
  #pragma message("Env_default_diagram.h is DEPRECATED, please include Envelope_diagram_1.h instead")
#endif

#include <CGAL/Envelope_diagram_1.h>

namespace CGAL {

template <class Traits_, class Allocator = CGAL_ALLOCATOR(int)>
class Env_default_diagram_1 : public Envelope_diagram_1<Traits_, Allocator>{};

} //namespace CGAL

#endif

