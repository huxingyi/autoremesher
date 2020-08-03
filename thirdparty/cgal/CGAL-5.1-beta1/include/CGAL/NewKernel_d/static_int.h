// Copyright (c) 2014
// INRIA Saclay-Ile de France (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/NewKernel_d/include/CGAL/NewKernel_d/static_int.h $
// $Id: static_int.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Marc Glisse

#ifndef CGAL_STATIC_INT_H
#define CGAL_STATIC_INT_H
#include <CGAL/constant.h>

namespace CGAL {
template <class NT> struct static_zero {
        operator NT() const { return constant<NT,0>(); }
};
template <class NT> struct static_one {
        operator NT() const { return constant<NT,1>(); }
};

template <class NT> static_zero<NT> operator-(static_zero<NT>) { return static_zero<NT>(); }

template <class NT> NT operator+(NT const& x, static_zero<NT>) { return x; }
template <class NT> NT operator+(static_zero<NT>, NT const& x) { return x; }
template <class NT> static_zero<NT> operator+(static_zero<NT>, static_zero<NT>) { return static_zero<NT>(); }
template <class NT> static_one<NT> operator+(static_zero<NT>, static_one<NT>) { return static_one<NT>(); }
template <class NT> static_one<NT> operator+(static_one<NT>, static_zero<NT>) { return static_one<NT>(); }

template <class NT> NT operator-(NT const& x, static_zero<NT>) { return x; }
template <class NT> NT operator-(static_zero<NT>, NT const& x) { return -x; }
template <class NT> static_zero<NT> operator-(static_zero<NT>, static_zero<NT>) { return static_zero<NT>(); }
template <class NT> static_zero<NT> operator-(static_one<NT>, static_one<NT>) { return static_zero<NT>(); }
template <class NT> static_one<NT> operator-(static_one<NT>, static_zero<NT>) { return static_one<NT>(); }

template <class NT> NT operator*(NT const& x, static_one<NT>) { return x; }
template <class NT> NT operator*(static_one<NT>, NT const& x) { return x; }
template <class NT> static_zero<NT> operator*(NT const&, static_zero<NT>) { return static_zero<NT>(); }
template <class NT> static_zero<NT> operator*(static_zero<NT>, NT const&) { return static_zero<NT>(); }
template <class NT> static_zero<NT> operator*(static_zero<NT>, static_zero<NT>) { return static_zero<NT>(); }
template <class NT> static_one<NT> operator*(static_one<NT>, static_one<NT>) { return static_one<NT>(); }
template <class NT> static_zero<NT> operator*(static_zero<NT>, static_one<NT>) { return static_zero<NT>(); }
template <class NT> static_zero<NT> operator*(static_one<NT>, static_zero<NT>) { return static_zero<NT>(); }

template <class NT> NT operator/(NT const& x, static_one<NT>) { return x; }
template <class NT> static_zero<NT> operator/(static_zero<NT>, NT const&) { return static_zero<NT>(); }
template <class NT> static_zero<NT> operator/(static_zero<NT>, static_one<NT>) { return static_zero<NT>(); }
template <class NT> static_one<NT> operator/(static_one<NT>, static_one<NT>) { return static_one<NT>(); }

}
#endif // CGAL_STATIC_INT_H
