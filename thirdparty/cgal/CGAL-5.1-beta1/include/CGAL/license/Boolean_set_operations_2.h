// Copyright (c) 2016  GeometryFactory SARL (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/license/Boolean_set_operations_2.h $
// $Id: Boolean_set_operations_2.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Andreas Fabri
//
// Warning: this file is generated, see include/CGAL/licence/README.md

#ifndef CGAL_LICENSE_BOOLEAN_SET_OPERATIONS_2_H
#define CGAL_LICENSE_BOOLEAN_SET_OPERATIONS_2_H

#include <CGAL/config.h>
#include <CGAL/license.h>

#ifdef CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE

#  if CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE < CGAL_RELEASE_DATE

#    if defined(CGAL_LICENSE_WARNING)

       CGAL_pragma_warning("Your commercial license for CGAL does not cover "
                           "this release of the 2D Regularized Boolean Set-Operations package.")
#    endif

#    ifdef CGAL_LICENSE_ERROR
#      error "Your commercial license for CGAL does not cover this release \
              of the 2D Regularized Boolean Set-Operations package. \
              You get this error, as you defined CGAL_LICENSE_ERROR."
#    endif // CGAL_LICENSE_ERROR

#  endif // CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE < CGAL_RELEASE_DATE

#else // no CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE

#  if defined(CGAL_LICENSE_WARNING)
     CGAL_pragma_warning("\nThe macro CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE is not defined."
                          "\nYou use the CGAL 2D Regularized Boolean Set-Operations package under "
                          "the terms of the GPLv3+.")
#  endif // CGAL_LICENSE_WARNING

#  ifdef CGAL_LICENSE_ERROR
#    error "The macro CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE is not defined.\
            You use the CGAL 2D Regularized Boolean Set-Operations package under the terms of \
            the GPLv3+. You get this error, as you defined CGAL_LICENSE_ERROR."
#  endif // CGAL_LICENSE_ERROR

#endif // no CGAL_BOOLEAN_SET_OPERATIONS_2_COMMERCIAL_LICENSE

#endif // CGAL_LICENSE_BOOLEAN_SET_OPERATIONS_2_H
