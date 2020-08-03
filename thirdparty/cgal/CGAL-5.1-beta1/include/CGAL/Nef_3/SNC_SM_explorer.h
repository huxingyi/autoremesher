// Copyright (c) 1997-2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Nef_3/include/CGAL/Nef_3/SNC_SM_explorer.h $
// $Id: SNC_SM_explorer.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Peter Hachenberger <hachenberger@mpi-sb.mpg.de>

#ifndef CGAL_SNC_SM_EXPLORER_H
#define CGAL_SNC_SM_EXPLORER_H

#include <CGAL/license/Nef_3.h>


#include <CGAL/basic.h>
#include <CGAL/Nef_S2/SM_const_decorator.h>

namespace CGAL {

template <typename SMCDEC>
class SNC_SM_explorer : public SMCDEC {
  typedef SMCDEC                            Base;
  typedef SNC_SM_explorer<SMCDEC>           Self;
  //  typedef typename Base::Kernel    Kernel;
  //  typedef typename Kernel::Point_3 Point_3;

 public:
  SNC_SM_explorer(const Base& E) : Base(E) {}
  Self& operator=(const Self& E) {
    Base::operator=(E);
    return *this;
  }
};

} //namespace CGAL

#endif  // CGAL_SNC_SM_EXPLORER_H
