// Copyright (c) 2010 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Mesh_3/include/CGAL/Mesh_3/Null_exuder_visitor.h $
// $Id: Null_exuder_visitor.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description :
//******************************************************************************

#ifndef CGAL_MESH_3_NULL_EXUDER_VISITOR_H
#define CGAL_MESH_3_NULL_EXUDER_VISITOR_H

#include <CGAL/license/Mesh_3.h>


#include <cstddef>

namespace CGAL {
namespace Mesh_3 {

template < typename C3T3 >
class Null_exuder_visitor
{
  typedef typename C3T3::Triangulation    Tr;
  typedef typename Tr::Geom_traits::FT    FT;

public:
  void after_cell_pumped(std::size_t) {}
};

} // end namespace Mesh_3
} // end namespace CGAL

#endif // CGAL_MESH_3_NULL_EXUDER_VISITOR_H
