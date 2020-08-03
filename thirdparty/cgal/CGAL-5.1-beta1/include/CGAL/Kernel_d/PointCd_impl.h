// Copyright (c) 2001
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Kernel_d/include/CGAL/Kernel_d/PointCd_impl.h $
// $Id: PointCd_impl.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel <seel@mpi-sb.mpg.de>

#ifndef CGAL_POINTCDXXX_C
#define CGAL_POINTCDXXX_C
namespace CGAL {
#define PointCd PointCd2

template <class FT, class LA>
PointCd<FT,LA> PointCd<FT,LA>::
transform(const Aff_transformationCd<FT,LA>& t) const
{ typename LA::Vector res = t(vector_rep());
  return PointCd<FT,LA>(dimension(),res.begin(),res.end()); }

template <class FT, class LA>
VectorCd<FT,LA> PointCd<FT,LA>::operator-(const Origin&) const
{ return VectorCd<FT,LA>(Base(*this)); }

template <class FT, class LA>
PointCd<FT,LA> PointCd<FT,LA>::operator+(const VectorCd<FT,LA> &v) const
{ PointCd<FT,LA> res(dimension());
  res.ptr()->cartesian_add(ptr(), v.ptr());
  return res;
}

template <class FT, class LA>
PointCd<FT,LA> PointCd<FT,LA>::operator-(const VectorCd<FT,LA> &v) const
{ PointCd<FT,LA> res(dimension());
  res.ptr()->cartesian_sub(ptr(), v.ptr());
  return res;
}

template <class FT, class LA>
PointCd<FT,LA>& PointCd<FT,LA>::operator+= (const VectorCd<FT,LA>& v)
{ int d = dimension();
  PointCd<FT,LA> old(*this);
  *this = PointCd<FT,LA>(d);
  ptr()->cartesian_add(old.ptr(), v.ptr());
  return *this;
}

template <class FT, class LA>
PointCd<FT,LA>& PointCd<FT,LA>::operator-= (const VectorCd<FT,LA>& v)
{ int d = dimension();
  PointCd<FT,LA> old(*this);
  *this = PointCd<FT,LA>(d);
  ptr()->cartesian_sub(old.ptr(), v.ptr());
  return *this;
}

template <class FT, class LA>
std::istream& operator>>(std::istream& I, PointCd<FT,LA>& p)
{ p.copy_on_write(); p.ptr()->read(I);
  return I;
}

template <class FT, class LA>
std::ostream& operator<<(std::ostream& O, const PointCd<FT,LA>& p)
{ p.ptr()->print(O,"PointCd"); return O; }

#undef PointCd
} //namespace CGAL
#endif // CGAL_POINTCDXXX_C
