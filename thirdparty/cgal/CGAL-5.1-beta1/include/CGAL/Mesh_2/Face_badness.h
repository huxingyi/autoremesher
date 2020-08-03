// Copyright (c) 2004  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Mesh_2/include/CGAL/Mesh_2/Face_badness.h $
// $Id: Face_badness.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent RINEAU

#ifndef CGAL_MESH_2_FACE_BADNESS_H
#define CGAL_MESH_2_FACE_BADNESS_H

#include <CGAL/license/Mesh_2.h>


namespace CGAL
{
  namespace Mesh_2
  {
    enum Face_badness { NOT_BAD, BAD, IMPERATIVELY_BAD };
  }
}
#endif // CGAL_MESH_2_FACE_BADNESS_H
