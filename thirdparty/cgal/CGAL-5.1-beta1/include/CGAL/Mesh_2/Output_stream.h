// Copyright (c) 2006-2007  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Mesher_level/include/CGAL/Mesh_2/Output_stream.h $
// $Id: Output_stream.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_MESHES_OUTPUT_STREAM_H

#ifdef CGAL_MESHES_NO_OUTPUT
#  include <CGAL/IO/Verbose_ostream.h>
#  define CGAL_MESHES_OUTPUT_STREAM CGAL::Verbose_ostream()
#else
#  ifndef CGAL_MESHES_OUTPUT_ON_CERR
#    define CGAL_MESHES_OUTPUT_STREAM std::cout
#  else
#    define CGAL_MESHES_OUTPUT_STREAM std::cerr
#  endif
#endif

#endif
