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

#ifndef GEOGRAM_MESH_PARAM_MESH_PGP_2D
#define GEOGRAM_MESH_PARAM_MESH_PGP_2D

#include <geogram/basic/common.h>
#include <geogram/parameterization/mesh_global_param.h>

namespace GEO {

    class Mesh;
    
    namespace GlobalParam2d {

	/**
	 * \brief Computes the PGP parameterization.
	 * \param[in] mesh a pointer to a surface mesh.
	 * \param[in] B a facet attribute with the frame field.
	 * \param[out] U a facet corner attribute with the computed 
	 *   PGP parameterization.
	 * \param[in] scaling the scaling of the parameterization, in function
	 *  of the average edge length.
	 * \param[in] constrain_hard_edges if true, align the parameterization
	 *  with the hard edges (not implemented yet).
	 * \param[in] use_direct_solver if true, use CHOLMOD or SUPERLU, else
	 *  use Jacobi-preconditioned conjugate gradient.
	 * \param[in] max_scaling_correction maximum multiplicative and
	 *  dividing factor for the scaling, used to generate a smaller 
	 *  number of singularities. Use 1.0 to disable scaling correction.
	 */
	void GEOGRAM_API PGP(
	    Mesh* mesh, Attribute<vec3>& B, Attribute<vec2>& U,
	    double scaling=1.0, bool constrain_hard_edges=false,
	    bool use_direct_solver=false,
	    double max_scaling_correction=2.0
	);

	/**
	 * \brief Computes the curl-correction.
	 * \note Bv is a vertex attribute (not a facet attribute). Can be
	 *  computed using Internal::transfer_B_to_vertices().
	 * \param[in] mesh a pointer to a surface mesh.
	 * \param[in] Bv a vertex attribute with the guidance vector field.
	 * \param[in] R_fv the Rij corner attribute indicating how many 
	 *  times the vector associated with facet j should be rotated by 
	 *  90 degrees around its facet normal to match the vector attached
	 *  to the vertex. It is computed by compute_R_fv().
	 * \param[out] CC a vertex attribute with the curl-correction.
	 * \param[in] use_direct_solver if true, use CHOLMOD or SUPERLU, else
	 *  use Jacobi-preconditioned conjugate gradient.
	 * \param[in] max_scaling_correction maximum multiplicative and
	 *  dividing factor for the scaling. It is used to clamp the result.
	 */
	void GEOGRAM_API curl_correction(
	    Mesh* mesh, Attribute<vec3>& Bv,
	    Attribute<index_t>& R_fv, Attribute<double>& CC,
	    bool use_direct_solver=false,
	    double max_scaling_correction=2.0
	);
    }
}

#endif

