
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
 *  Contact for this Plugin: Bruno Levy - Bruno.Levy@inria.fr
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
	    bool do_brush=true, bool integer_constraints = true
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
		bool integer_constraints = true		
	    );
	}
    }
}

#endif

