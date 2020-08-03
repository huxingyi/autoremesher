// Copyright (c) 2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Number_types/include/CGAL/constant.h $
// $Id: constant.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_CONSTANT_H
#define CGAL_CONSTANT_H

#include <CGAL/config.h>
#include <CGAL/tss.h>

namespace CGAL {

// The function constant<T, int i>() returns a const reference to T(i).
// TODO : is it worth documenting ?

template < typename T, int i >
inline
const T&
constant()
{
  CGAL_STATIC_THREAD_LOCAL_VARIABLE(T, t,i);
  return t;
}

} //namespace CGAL

#endif // CGAL_CONSTANT_H
