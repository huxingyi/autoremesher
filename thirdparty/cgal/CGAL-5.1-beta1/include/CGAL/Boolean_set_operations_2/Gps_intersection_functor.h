// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Boolean_set_operations_2/include/CGAL/Boolean_set_operations_2/Gps_intersection_functor.h $
// $Id: Gps_intersection_functor.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef CGAL_GPS_INTERSECTION_FUNCTOR_H
#define CGAL_GPS_INTERSECTION_FUNCTOR_H

#include <CGAL/license/Boolean_set_operations_2.h>


#include <CGAL/Boolean_set_operations_2/Gps_base_functor.h>

namespace CGAL {

template <class Arrangement_>
class Gps_intersection_functor : public Gps_base_functor<Arrangement_>
{
public:

  typedef Arrangement_                                    Arrangement_2;

  typedef typename Arrangement_2::Face_const_handle       Face_const_handle;
  typedef typename Arrangement_2::Face_handle             Face_handle;


  void create_face (Face_const_handle f1,
                    Face_const_handle f2,
                    Face_handle res_f)
  {
    if(f1->contained() && f2->contained())
    {
      res_f->set_contained(true);
    }
  }
};


} //namespace CGAL

#endif
