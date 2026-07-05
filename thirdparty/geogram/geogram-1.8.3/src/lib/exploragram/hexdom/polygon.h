/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
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
