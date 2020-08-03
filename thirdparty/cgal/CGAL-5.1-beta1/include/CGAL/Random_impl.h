// Copyright (c) 1997-2001
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Random_numbers/include/CGAL/Random_impl.h $
// $Id: Random_impl.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sven Schönherr <sven@inf.ethz.ch>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline
#else
#define CGAL_INLINE_FUNCTION
#endif

#include <ctime>
#include <iostream>
#include <sstream>

namespace CGAL {

// Class implementation (continued)
// ================================

// constructors
CGAL_INLINE_FUNCTION
Random::
Random()
    :  val(0)
{
    // get system's time
    std::time_t s;
    std::time( &s);
    seed = (unsigned int)s;
    // initialize random numbers generator
    rng.seed(static_cast<boost::int32_t>(seed));
    random_value = get_int(0, 1<<15);
}

CGAL_INLINE_FUNCTION
Random::
Random(internal::Random_print_seed)
    :  val(0)
{
    // get system's time
    std::time_t s;
    std::time( &s);
    seed = (unsigned int)s;
    std::cerr << "CGAL::Random()::get_seed() = " << seed << std::endl;
    // initialize random numbers generator
    rng.seed(static_cast<boost::int32_t>(seed));
    random_value = get_int(0, 1<<15);
}

CGAL_INLINE_FUNCTION
Random::
Random( unsigned int  seed)
    : val(0), seed(seed)
{
    // initialize random numbers generator
    rng.seed(static_cast<boost::int32_t>(seed));
    random_value = get_int(0, 1<<15);
}


// seed
CGAL_INLINE_FUNCTION
unsigned int
Random::get_seed () const
{
  return seed;
}

// state
CGAL_INLINE_FUNCTION
void
Random::save_state( Random::State& state) const
{
  std::ostringstream os;
  os << rng;
  state = Random::State(os.str(),random_value, val, seed);
}

CGAL_INLINE_FUNCTION
void
Random::restore_state( const Random::State& state)
{
  std::istringstream is(state.rng);
  is >> rng;
  random_value = state.random_value;
  val = state.val;
  seed = state.seed;
}

} //namespace CGAL
