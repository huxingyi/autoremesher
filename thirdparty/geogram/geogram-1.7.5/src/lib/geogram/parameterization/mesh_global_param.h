/*
 *  Copyright (c) 2012-2016, Bruno Levy
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
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef GEOGRAM_MESH_PARAM_MESH_GLOBAL_PARAM
#define GEOGRAM_MESH_PARAM_MESH_GLOBAL_PARAM

#include <geogram/basic/common.h>
#include <geogram/basic/attributes.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram/parameterization/mesh_global_param.h
 * \brief Basic functions used by global surface parameterization
 *  algorithms (PGP, QuadCover).
 */

namespace GEO {

    class Mesh;
    
    namespace GlobalParam2d {

	/**
	 * \brief Computes a guidance frame field for global parameterization.
	 * \details Creates the vector field in the "B" facet attribute,
	 *  of type vec3.
	 * \param[in] mesh a pointer to a surface mesh.
	 * \param[out] B a facet attribute with the computed frame field.
	 * \param[in] hard_angle_threshold edges with a facet angle larger than
	 *  this threshold are fixed as constraints for the frame field.
	 */
	void GEOGRAM_API frame_field(
	    Mesh* mesh, Attribute<vec3>& B,
	    double hard_angle_threshold=45.0
	);


	namespace Internal {
	
	    /**
	     * \brief Computes for each pair of adjacent facets the number of 
	     *  times the facet vector B should be rotated along the facet 
	     *  normal to minimize its angle with the vector B of the adjacent 
	     *  facet.
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[in] B a vec3 attribute attached to the facets
	     * \param[out] R_ff an index_t attribute attached to facet corners, 
	     *  in 0,1,2,3
	     */
	    void GEOGRAM_API compute_R_ff(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff
	    );

	    /**
	     * \brief Computes for each facet corner the number of 
	     *  times the facet vector B should be rotated along the facet 
	     *  normal to minimize its angle with the vector B of a reference
	     *  facet attached to each vertex.
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[in] R_ff an index_t attribute attached to facet corners,
	     *  computed by compute_R_ff().
	     * \param[out] R_fv an index_t attribute attached to facet corners, 
	     *  in 0,1,2,3
	     */
	    void GEOGRAM_API compute_R_fv(
		Mesh* mesh, 
		Attribute<index_t>& R_ff, Attribute<index_t>& R_fv
	     );

	    
	    /**
	     * \brief Marks the singular vertices of the direction field.
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[in] R_ff the Rij corner attribute indicating how many 
	     *  times the vector associated with facet j should be rotated by 
	     *  90 degrees around its facet normal to match the vector 
	     *  associated with facet i. It is computed by compute_R_ff().
	     * \param[out] v_is_singular a vertex attribute that indicates 
	     *  for each vertex whether it is singular.
	     */
	    void GEOGRAM_API mark_singular_vertices(
		Mesh* mesh,
		Attribute<index_t>& R_ff, Attribute<bool>& v_is_singular
	    );

	    /**
	     * \brief Brushes the direction field.
	     * \details Makes the field rotation between adjacent facets equal
	     *  to zero over a covering tree of the surface.
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[in,out] B a facet attribute with the guidance vector 
	     *  field one 3d vector per facet).
	     */
	    void GEOGRAM_API brush(Mesh* mesh, Attribute<vec3>& B);
	    
	    /**
	     * \brief Computes the border of the ball.
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[in] R_ff the Rij corner attribute indicating how many 
	     *  times the vector associated with facet j should be rotated by 
	     *  90 degrees around its facet normal to match the vector 
	     *  associated with facet i. It is computed by compute_R_ff().
	     * \param[out] c_on_border a facet corner attribute that contains 1 
	     *  if the halfede edge is on the border of the ball, 0 otherwise
	     */
	    void GEOGRAM_API do_the_ball(
		Mesh* mesh,
		Attribute<index_t>& R_ff, Attribute<index_t>& c_on_border
	    );

	    
	    /**
	     * \brief Computes the border of the ball.
	     * \details This version does not suppose that the B's are brushed 
	     *  and only computes the facet covering tree (no zipping).
	     * \param[in] mesh a pointer to a surface mesh
	     * \param[out] c_on_border a facet corner attribute that contains 1 
	     *  if the halfede edge is on the border of the ball, 0 otherwise
	     */
	    void GEOGRAM_API do_the_ball_no_brush_no_zip(
		Mesh* mesh, Attribute<index_t>& c_on_border
	    );


	    /**
	     * \brief Gets the field B evaluated at a given mesh edge.
	     * \details If the edge is incident to two facets, then the
	     *  field is averaged.
	     * \param[in] mesh a pointer to a surface mesh.
	     * \param[in,out] B a facet attribute with the guidance 
	     *  vector field, one 3d vector per facet).
	     * \param[in] R_ff the Rij corner attribute indicating how many 
	     *  times the vector associated with facet j should be rotated by 
	     *  90 degrees around its facet normal to match the vector 
	     *  associated with facet i. It is computed by compute_R_ff().
	     * \param[in] f a mesh facet.
	     * \param[in] c a corner of facet p f.
	     * \param[out] Bc the field along the edge originated 
	     *  from corner \p c of facet \p f.
	     * \param[out] BTc the orthogonal field along the edge originated 
	     *  from corner \p c of facet \p f.
	     */
	    void GEOGRAM_API get_B_on_edge(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff,
		index_t f, index_t c,
		vec3& Bc, vec3& BTc
	    );

	    
	    enum { CNSTR_NONE = 0, CNSTR_U = 1, CNSTR_V = 2 };
	    
	    /**
	     * \brief Determines the constraints for all edges of the mesh.
	     * \param[in] mesh a pointer to a surface mesh.
	     * \param[in,out] B a facet attribute with the guidance 
	     *  vector field, one 3d vector per facet).
	     * \param[in] R_ff the Rij corner attribute indicating how many 
	     *  times the vector associated with facet j should be rotated by 
	     *  90 degrees around its facet normal to match the vector 
	     *  associated with facet i. It is computed by compute_R_ff().
	     * \param[out] constraint for each corner, a binary-or 
	     *  combination of CNSTR_U and CNSTR_V.
	     */
	    void GEOGRAM_API get_constraints(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff,
		Attribute<index_t>& constraint
	    );

	    /**
	     * \brief Tests whether U and V are constrained for a given edge.
	     * \details An edge is constrained if it is a border edge or if it
	     *  is sharp. The coordinate that is constrained is determined from 
	     *  the dot product betwee the edge vector and \p B.
	     * \param[in] mesh a pointer to a surface mesh.
	     * \param[in] c a corner incident to the edge.
	     * \param[in,out] B a facet attribute with the guidance vector 
	     *  field one 3d vector per facet).
	     * \return One of CNSTR_U, CNSTR_V, CNSTR_U | CNSTR_V.
	     */
	    index_t GEOGRAM_API get_edge_constraints(
		Mesh* mesh, index_t c, Attribute<vec3>& B
	    );

	    
	    /**
	     * \brief Gets the inverse of a rotation.
	     * \param[in] R the rotation in angus (in 0,1,2,3)
	     * \return the inverse of R in angus (in 0,1,2,3)
	     */
	    index_t GEOGRAM_API inverse_R(index_t R);


	    /**
	     * \brief Snaps a texture coordinate.
	     * \details This is required by the mesh extraction algorithm.
	     * \param[in,out] coord the coordinate to be snapped.
	     */
	    void GEOGRAM_API snap_tex_coord(double& coord);

	    /**
	     * \brief Transfers a facet vector field to a vertex vector field.
	     * \param[in] mesh a pointer to a surface mesh.
	     * \param[in] B a facet attribute with the guidance vector 
	     *  field one 3d vector per facet).
	     * \param[out] Bv a vertex attribute with the guidance vector 
	     *  field one 3d vector per vertex).
	     * \param[in] R_fv the Rij corner attribute indicating how many 
	     *  times the vector associated with facet j should be rotated by 
	     *  90 degrees around its facet normal to match the vector attached
	     *  to the vertex. It is computed by compute_R_fv().
	     */
	    void GEOGRAM_API transfer_B_to_vertices(
		Mesh* mesh,
		Attribute<vec3>& B, Attribute<vec3>& Bv,
		Attribute<index_t>& R_fv
	    );
	    
	}
    }
}

#endif

