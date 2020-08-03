// Copyright (c) 2016 Inria.
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arithmetic_kernel/include/CGAL/GMPXX_arithmetic_kernel.h $
// $Id: GMPXX_arithmetic_kernel.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author: Marc Glisse <marc.glisse@inria.fr>

#ifndef CGAL_GMPXX_ARITHMETIC_KERNEL_H
#define CGAL_GMPXX_ARITHMETIC_KERNEL_H

#include <CGAL/config.h>

#ifdef CGAL_USE_GMPXX

#include <CGAL/Arithmetic_kernel/Arithmetic_kernel_base.h>
#include <CGAL/Get_arithmetic_kernel.h>

#include <CGAL/gmpxx.h>

namespace CGAL {
/** \ingroup CGAL_Arithmetic_kernel
 *  \brief The GMPXX set of exact number types
 */
struct GMPXX_arithmetic_kernel : internal::Arithmetic_kernel_base {
  typedef mpz_class Integer;
  typedef mpq_class Rational;
};

template <class T, class U>
struct Get_arithmetic_kernel<__gmp_expr<T, U> > {
  typedef GMPXX_arithmetic_kernel Arithmetic_kernel;
};
} //namespace CGAL
#endif //CGAL_USE_GMPXX
#endif
