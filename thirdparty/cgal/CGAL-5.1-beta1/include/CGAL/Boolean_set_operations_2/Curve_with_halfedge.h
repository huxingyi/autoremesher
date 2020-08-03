// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Boolean_set_operations_2/include/CGAL/Boolean_set_operations_2/Curve_with_halfedge.h $
// $Id: Curve_with_halfedge.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef CGAL_CURVE_WITH_HALFEDGE_H
#define CGAL_CURVE_WITH_HALFEDGE_H

#include <CGAL/license/Boolean_set_operations_2.h>


namespace CGAL {

template <class Arrangement_>
class Curve_with_halfedge
{
protected:
  typedef typename Arrangement_::Halfedge_handle        Halfedge_handle;
  typedef typename Arrangement_::Halfedge_const_handle  Halfedge_const_handle;

public:
  Halfedge_handle  m_he;

  Curve_with_halfedge()
  {};

  Curve_with_halfedge(Halfedge_handle he) : m_he(he)
  {}

  Halfedge_handle halfedge() const
  {
    return (m_he);
  }

  Halfedge_handle halfedge()
  {
    return (m_he);
  }

  void set_halfedge(Halfedge_handle he)
  {
    m_he = he;
  }
};

} //namespace CGAL
#endif
