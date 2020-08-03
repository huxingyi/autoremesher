// Copyright (c) 2010-2011  GeometryFactory Sarl (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Number_types/include/CGAL/FPU_gcc_i386_sse2.h $
// $Id: FPU_gcc_i386_sse2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

extern "C" {
#include <fenv.h>
}

namespace CGAL {

// replacement for C99

inline int
feclearexcept(int exceptions) {
    int mxcsr;
    asm volatile("stmxcsr %0" : "=m" (mxcsr) );
    mxcsr &= ~exceptions;
    asm volatile("ldmxcsr %0" : : "m" (mxcsr) );
    return 0;
}

inline int
fetestexcept(int exceptions) {
    int status;
    asm volatile("stmxcsr %0" : "=m" (status) );
    return status & exceptions;
}

} // end namespace CGAL
