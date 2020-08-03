// Copyright (c) 1997-2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Hash_map/include/CGAL/Handle_hash_function.h $
// $Id: Handle_hash_function.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel <seel@mpi-sb.mpg.de>
//                 Lutz Kettner <kettner@inf.ethz.ch>

#ifndef CGAL_HANDLE_HASH_FUNCTION_H
#define CGAL_HANDLE_HASH_FUNCTION_H

#include <CGAL/config.h>
#include <cstddef>

namespace CGAL {


//mechanism to abuse Handle_hash_function which is the default
//template parameter of Unique_hash_map
namespace internal{
  namespace handle {
    template <class H>
    struct Hash_functor{
      std::size_t
      operator()(const H& h)
      {
        return std::size_t(&*h) /
          sizeof( typename std::iterator_traits<H>::value_type);
      }
    };
  }
}

struct Handle_hash_function {
    typedef std::size_t result_type;
    template <class H>
    std::size_t operator() (const H& h) const {
      return ::CGAL::internal::handle::Hash_functor<H>()(h);
    }
};

} //namespace CGAL

#endif // CGAL_HANDLE_HASH_FUNCTION_H
// EOF
