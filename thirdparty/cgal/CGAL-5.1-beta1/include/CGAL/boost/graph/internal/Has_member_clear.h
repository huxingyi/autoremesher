// Copyright (c) 2016 GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/BGL/include/CGAL/boost/graph/internal/Has_member_clear.h $
// $Id: Has_member_clear.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Philipp Moeller


#ifndef CGAL_HAS_MEMBER_CLEAR_H
#define CGAL_HAS_MEMBER_CLEAR_H

namespace CGAL {
namespace internal {

template<class T>
class Has_member_clear
{
private:
  template<class U, U>
  class check {};

  template<class C>
  static char f(check<void(C::*)(void), &C::clear>*);

  template<class C>
  static int f(...);
public:
  static const bool value = (sizeof(f<T>(0)) == sizeof(char));
};

}  // internal
}  // cgal

#endif /* CGAL_HAS_MEMBER_CLEAR_H */
