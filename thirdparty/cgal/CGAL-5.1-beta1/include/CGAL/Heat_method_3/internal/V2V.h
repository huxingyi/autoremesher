// Copyright (c) 2018  Carnegie Mellon University (USA), GeometryFactory (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Heat_method_3/include/CGAL/Heat_method_3/internal/V2V.h $
// $Id: V2V.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s) : Christina Vaz, Keenan Crane, Andreas Fabri

#ifndef CGAL_HEAT_METHOD_3_INTERNAL_V2V_H
#define CGAL_HEAT_METHOD_3_INTERNAL_V2V_H

#include <CGAL/license/Heat_method_3.h>

#ifndef DOXYGEN_RUNNING

namespace CGAL {
namespace Heat_method_3 {

  template <typename TM>
  struct V2V {

    V2V(const TM&)
    {}

    template <typename T>
    const T& operator()(const T& t) const
    {
      return t;
    }
  };
#endif
} // namespace Heat_method_3
} // namespace CGAL
#endif //CGAL_HEAT_METHOD_3_INTERNAL_V2V_H
