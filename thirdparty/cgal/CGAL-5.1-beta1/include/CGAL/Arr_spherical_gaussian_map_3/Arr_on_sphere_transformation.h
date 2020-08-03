// Copyright (c) 2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/include/CGAL/Arr_spherical_gaussian_map_3/Arr_on_sphere_transformation.h $
// $Id: Arr_on_sphere_transformation.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Naama mayer         <naamamay@post.tau.ac.il>

#ifndef CGAL_ARR_ON_SPHERE_TRANSFORMATION_H
#define CGAL_ARR_ON_SPHERE_TRANSFORMATION_H

#include <CGAL/license/Arrangement_on_surface_2.h>


namespace CGAL {

/* This function rotates the face when the arrangement is sgm */

template <class Arrangement, class Transformation_3>
class Arr_on_sphere_transformation
{
        void rotate_face(Arrangement & arr, Face_Handle f1, Face_handle f2)
        {

        }
};

} //namespace CGAL

#endif
