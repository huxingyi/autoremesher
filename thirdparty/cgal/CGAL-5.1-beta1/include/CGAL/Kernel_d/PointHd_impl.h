// Copyright (c) 2001
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Kernel_d/include/CGAL/Kernel_d/PointHd_impl.h $
// $Id: PointHd_impl.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel <seel@mpi-sb.mpg.de>

#ifndef CGAL_POINTHD_C
#define CGAL_POINTHD_C
namespace CGAL {
#define PointHd PointHd2

template <class RT, class LA>
PointHd<RT,LA> PointHd<RT,LA>::
transform(const Aff_transformationHd<RT,LA>& t) const
{ typename LA::Vector res = t(vector_rep());
  return PointHd<RT,LA>(dimension(),res.begin(),res.end()); }

template <class RT, class LA>
VectorHd<RT,LA> PointHd<RT,LA>::operator-(const Origin&) const
{ return VectorHd<RT,LA>(Base(*this)); }

template <class RT, class LA>
PointHd<RT,LA> PointHd<RT,LA>::operator+(const VectorHd<RT,LA> &v) const
{ PointHd<RT,LA> res(dimension());
  res.ptr()->homogeneous_add(ptr(), v.ptr());
  return res;
}

template <class RT, class LA>
PointHd<RT,LA> PointHd<RT,LA>::operator-(const VectorHd<RT,LA> &v) const
{ PointHd<RT,LA> res(dimension());
  res.ptr()->homogeneous_sub(ptr(), v.ptr());
  return res;
}

template <class RT, class LA>
PointHd<RT,LA>& PointHd<RT,LA>::operator+= (const VectorHd<RT,LA>& v)
{ int d = dimension();
  PointHd<RT,LA> old(*this);
  *this = PointHd<RT,LA>(d);
  ptr()->homogeneous_add(old.ptr(), v.ptr());
  return *this;
}

template <class RT, class LA>
PointHd<RT,LA>& PointHd<RT,LA>::operator-= (const VectorHd<RT,LA>& v)
{ int d = dimension();
  PointHd<RT,LA> old(*this);
  *this = PointHd<RT,LA>(d);
  ptr()->homogeneous_sub(old.ptr(), v.ptr());
  return *this;
}

template <class RT, class LA>
std::istream& operator>>(std::istream& I, PointHd<RT,LA>& p)
{ p.copy_on_write(); p.ptr()->read(I);
  CGAL_assertion_msg((p.homogeneous(p.dimension()) > 0),
  "operator>>: denominator of point must be larger than zero.");
  return I;
}

template <class RT, class LA>
std::ostream& operator<<(std::ostream& O, const PointHd<RT,LA>& p)
{ p.ptr()->print(O,"PointHd"); return O; }

#undef PointHd
} //namespace CGAL
#endif // CGAL_POINTHD_C
