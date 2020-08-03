// Copyright (c) 2018 GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/internal/disable_deprecation_warnings_and_errors.h $
// $Id: disable_deprecation_warnings_and_errors.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author: Mael Rouxel-Labbé

// Some tests are explicitely used to check the sanity of deprecated code and should not
// give warnings/errors on plateforms that defined CGAL_NO_DEPRECATED_CODE CGAL-wide
// (or did not disable deprecation warnings).

#if !defined(CGAL_NO_DEPRECATION_WARNINGS)
  #define CGAL_NO_DEPRECATION_WARNINGS
#endif

#if defined(CGAL_NO_DEPRECATED_CODE)
  #undef CGAL_NO_DEPRECATED_CODE
#endif
