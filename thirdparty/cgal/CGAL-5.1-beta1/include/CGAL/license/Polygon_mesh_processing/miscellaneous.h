// Copyright (c) 2016  GeometryFactory SARL (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/license/Polygon_mesh_processing/miscellaneous.h $
// $Id: miscellaneous.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Andreas Fabri
//
// Warning: this file is generated, see include/CGAL/licence/README.md

#ifndef CGAL_LICENSE_POLYGON_MESH_PROCESSING_MISCELLANEOUS_H
#define CGAL_LICENSE_POLYGON_MESH_PROCESSING_MISCELLANEOUS_H

#include <CGAL/config.h>
#include <CGAL/license.h>

#ifdef CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE

#  if CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE < CGAL_RELEASE_DATE

#    if defined(CGAL_LICENSE_WARNING)

       CGAL_pragma_warning("Your commercial license for CGAL does not cover "
                           "this release of the Polygon Mesh Processing - Miscellaneous package.")
#    endif

#    ifdef CGAL_LICENSE_ERROR
#      error "Your commercial license for CGAL does not cover this release \
              of the Polygon Mesh Processing - Miscellaneous package. \
              You get this error, as you defined CGAL_LICENSE_ERROR."
#    endif // CGAL_LICENSE_ERROR

#  endif // CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE < CGAL_RELEASE_DATE

#else // no CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE

#  if defined(CGAL_LICENSE_WARNING)
     CGAL_pragma_warning("\nThe macro CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE is not defined."
                          "\nYou use the CGAL Polygon Mesh Processing - Miscellaneous package under "
                          "the terms of the GPLv3+.")
#  endif // CGAL_LICENSE_WARNING

#  ifdef CGAL_LICENSE_ERROR
#    error "The macro CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE is not defined.\
            You use the CGAL Polygon Mesh Processing - Miscellaneous package under the terms of \
            the GPLv3+. You get this error, as you defined CGAL_LICENSE_ERROR."
#  endif // CGAL_LICENSE_ERROR

#endif // no CGAL_POLYGON_MESH_PROCESSING_MISCELLANEOUS_COMMERCIAL_LICENSE

#endif // CGAL_LICENSE_POLYGON_MESH_PROCESSING_MISCELLANEOUS_H
