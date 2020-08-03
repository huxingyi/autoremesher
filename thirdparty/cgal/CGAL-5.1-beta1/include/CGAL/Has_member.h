// Copyright (c) 2017 Inria (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/Has_member.h $
// $Id: Has_member.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Clement Jamin

#ifndef CGAL_HAS_MEMBER_H
#define CGAL_HAS_MEMBER_H

// Macro used to check if a type T has a member named `X`
// It generates a class has_X<T> where has_X<T>::value is a boolean
// See example in Concurrent_compact_container.h
#define CGAL_GENERATE_MEMBER_DETECTOR(X)                                      \
template<typename T> class has_##X {                                          \
    struct Fallback { int X; };                                               \
    struct Derived : T, Fallback { };                                         \
                                                                              \
    template<typename U, U> struct Check;                                     \
                                                                              \
    typedef char ArrayOfOne[1];                                               \
    typedef char ArrayOfTwo[2];                                               \
                                                                              \
    template<typename U> static ArrayOfOne & func(                            \
                                            Check<int Fallback::*, &U::X> *); \
    template<typename U> static ArrayOfTwo & func(...);                       \
  public:                                                                     \
    typedef has_##X type;                                                     \
    enum { value = sizeof(func<Derived>(0)) == 2 };                           \
} // semicolon is after the macro call

#endif // CGAL_HAS_MEMBER_H
