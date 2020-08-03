// Copyright (c) 2009  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Alpha_shapes_3/include/CGAL/internal/Classification_type.h $
// $Id: Classification_type.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sebastien Loriot
//


#ifndef CGAL_INTERNAL_CLASSIFICATION_TYPE_H
#define CGAL_INTERNAL_CLASSIFICATION_TYPE_H

#include <CGAL/license/Alpha_shapes_3.h>


namespace CGAL{
  namespace internal{
    enum Classification_type {EXTERIOR,SINGULAR,REGULAR,INTERIOR};
  }
}
#endif //CGAL_INTERNAL_CLASSIFICATION_TYPE_H
