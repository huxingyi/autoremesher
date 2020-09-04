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

#ifndef GEOGRAM_MESH_MESH_LSCM
#define GEOGRAM_MESH_MESH_LSCM


/**
 * \file geogram/mesh/mesh_LSCM.h
 * \brief Implementation of Least Squares Conformal Maps.
 */

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

namespace GEO {
    class Mesh;

    /**
     * \brief Computes texture coordinates using Least Squares Conformal Maps.
     * \details The method is described in the following references:
     *  - least squares mode: Least Squares Conformal Maps, 
     *    Levy, Petitjean, Ray, Maillot, ACM SIGGRAPH, 2002
     *  - spectral mode: Spectral Conformal Parameterization, 
     *    Mullen, Tong, Alliez, Desbrun, 
     *    Computer Graphics Forum (SGP conf. proc.), 2008
     * \param[in,out] M a reference to a surface mesh
     * \param[in] attribute_name the name of the vertex attribute where 
     *   texture coordinates are stored.
     * \param[in] spectral if true, use spectral conformal parameterization, 
     *   otherwise use least squares conformal maps. Spectral mode requires 
     *   support of the ARPACK OpenNL extension.
     * \param[in] angle_attribute_name if specified, the desired angles in 
     *   the 2D map. If unspecified, the desired angles are read on the 3D
     *   mesh.
     * \param[in] verbose if true, display statistics during computation.
     */
    void GEOGRAM_API mesh_compute_LSCM(
	Mesh& M, const std::string& attribute_name="tex_coord",
	bool spectral=false,
	const std::string& angle_attribute_name="",
	bool verbose=false
    );
}


#endif
