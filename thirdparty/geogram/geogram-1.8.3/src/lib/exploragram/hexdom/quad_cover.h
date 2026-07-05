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

#ifndef H_HEXDOM_ALGO_QUAD_COVER_H
#define H_HEXDOM_ALGO_QUAD_COVER_H

#include <exploragram/basic/common.h>
#include <geogram/parameterization/mesh_global_param.h>

namespace GEO {
    
    class Mesh;
    
    namespace GlobalParam2d {


	/**
	 * \brief Computes the quad-cover parameterization.
	 * \param[in] B a facet attribute with the frame field.
	 * \param[out] U a facet corner attribute with the computed 
	 *   quad-cover parameterization.
	 * \param[in] scaling the scaling of the parameterization, in function
	 *  of the average edge length.
	 * \param[in] constrain_hard_edges if true, constrain iso-u,v on hard 
	 *  edges.
	 * \param[in] do_brush if true, brush the input field
	 */
	void EXPLORAGRAM_API quad_cover(
	    Mesh* mesh, Attribute<vec3>& B, Attribute<vec2>& U,
	    double scaling=1.0, bool constrain_hard_edges = true,
	    bool do_brush=true, bool integer_constraints = true,
	    double hard_angle_threshold = 45.0
	);


	namespace Internal {
	    /**
	     * \brief Solves the quad-cover mixed real-integer problem.
	     * \param[in] mesh a pointer to a surfacic mesh.
	     * \param[in] B a facet attribute with the guidance vector field 
	     *  one 3d vector per facet).
	     * \param[in] R_ff a facet corner attribute that indicates how many 
	     *  times the B associated to the adjacent facet should be turned 
	     *  by 90 degrees  around the adjacent facet normal to match the B 
	     *  associated with the facet the corner is incident to, computed
	     *  by Internal::compute_R_ff().
	     * \param[in] on_border a facet corner attribute that indicates 
	     *  whether the halfedge associated with the corner is on the 
	     *  border of the ball (1) or not (0). It is computed by one of 
	     *  the two versions of do_the_ball().
	     * \param[in] constraints an attribute that indicates for each 
	     *  corner  whether the u and/or v coordinate is constrained to 
	     *  have an integer value.
	     * \param[out] U a vector attribute of dimension 2 associated with 
	     *  the  facet corners, that contains on exit the computed 
	     *  parameterization.
	     * \param[out] T a vector attribute of dimension 2 associated with 
	     *  the halfedges (facet corners), that contains on exit the 
	     *  translations between the pairs of adjacent triangles.
	     * \param[in] v_is_singular a vertex attribute that indicates 
	     *  whether each vertex is singular
	     * \param[in] scaling the scaling of the parameterization, in 
	     *  function of the average edge length.
	     * \param[in] constrain_hard_edges if true, 
	     *  constrain iso-u,v on hard edges.
	     */
	    void EXPLORAGRAM_API quad_cover_solve(
		Mesh* mesh,
		Attribute<vec3>& B, Attribute<index_t>& R_ff,
		Attribute<index_t>& on_border,
		Attribute<index_t>& constraints,
		Attribute<vec2>& U,
		Attribute<double>& T,
		Attribute<bool>& v_is_singular,
		double scaling = 1.0,
		bool constrain_hard_edges = true,
		bool integer_constraints = true,
	    double hard_angle_threshold = 45.0
	);
	}
    }
}

#endif

