// Copyright (c) 2006  GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesh_simplification/include/CGAL/Null_matrix.h $
// $Id: Null_matrix.h ff09c5d 2019-10-25T16:35:53+02:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//
#ifndef CGAL_NULL_MATRIX_H
#define CGAL_NULL_MATRIX_H

#include <CGAL/license/Surface_mesh_simplification.h>

namespace CGAL {

class Null_matrix {};

static const Null_matrix NULL_MATRIX = Null_matrix();

} // namespace CGAL

#endif // CGAL_NULL_MATRIX_H
