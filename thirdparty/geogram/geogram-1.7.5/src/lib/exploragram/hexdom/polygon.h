/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#ifndef H_HEXDOM_ALGO_POLYGON_H
#define H_HEXDOM_ALGO_POLYGON_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/geometry.h>
#include <exploragram/hexdom/mesh_utils.h>
#include <geogram/mesh/mesh_io.h>

#include <assert.h>
#include <cmath>

namespace GEO {
    
    struct EXPLORAGRAM_API Poly2d {
	
        Poly2d(vector<vec2> &p_pts) : pts(p_pts) {
        }

        vec2 barycenter();

        void dump_contour();

        // returns 1024. if concave angle is encountered or if proposed triangle contains one of pts
        // otherwise returns max angle of the proposed triangle
        double cost(index_t i, index_t j, index_t k) ;

        bool try_triangulate_minweight(vector<index_t>& triangles);

        // find parity of original points
        index_t parity_of_original_points() ;

        bool middle_point_quadrangulate(vector<index_t>& quads);

        bool quads_are_valid(vector<index_t>& quads);

		bool try_quad_cover(vector<index_t>& quads);
		bool try_quadrangulate(vector<index_t>& quads);

        vector<vec2> &pts;
    };


    struct EXPLORAGRAM_API Poly3d {
        Poly3d(vector<vec3> &p_pts) : pts(p_pts) {
        }
		void dump_contour();
        vec3 barycenter();

        vec3 normal();

        bool try_triangulate_minweight(vector<index_t>& triangles);

        /**
        * WARNING: it may introduce new vertices in pts
        */
        bool try_quadrangulate(vector<index_t>& quads);

        vector<vec3> &pts;
    };

}
#endif
