// Copyright (c) 2009-2014 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Triangulation/include/CGAL/internal/Triangulation/Dummy_TDS.h $
// $Id: Dummy_TDS.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)    : Samuel Hornus

#ifndef CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_H
#define CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_H

#include <CGAL/license/Triangulation.h>


namespace CGAL {

namespace internal {
namespace Triangulation {

struct Dummy_TDS
{
    struct Vertex {};
    struct Vertex_handle {};
    struct Vertex_iterator {};
    struct Vertex_const_handle {};
    struct Vertex_const_iterator {};
    struct Full_cell {};
    struct Full_cell_handle {};
    struct Full_cell_iterator {};
    struct Full_cell_const_handle {};
    struct Full_cell_const_iterator {};
    struct Vertex_handle_const_iterator {};
    struct Full_cell_data {};
};

} // namespace Triangulation
} // namespace internal

} //namespace CGAL

#endif // CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_H
