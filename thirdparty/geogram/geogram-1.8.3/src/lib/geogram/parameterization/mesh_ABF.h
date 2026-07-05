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

#ifndef GEOGRAM_MESH_MESH_ABF
#define GEOGRAM_MESH_MESH_ABF

/**
 * \file geogram/mesh/mesh_ABF.h
 */

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

namespace GEO {
    class Mesh;

    /**
     * \brief Computes texture coordinates using Least Squares Conformal Maps.
     * \details The method is described in the following reference:
     *   ABF++: fast and robust angle-based flattening, A. Sheffer, B. Levy,
     *   M. Mogilnitsky,  A. Bogomyakov, ACM Transactions on Graphics, 2005
     * \param[in,out] M a reference to a surface mesh. Facets need to be 
     *   triangulated.
     * \param[in] attribute_name the name of the vertex attribute where 
     *   texture coordinates are stored.
     * \param[in] verbose if true, messages with statistics are displayed 
     *   in the logger during computation.
     */
    void GEOGRAM_API mesh_compute_ABF_plus_plus(
	Mesh& M, const std::string& attribute_name="tex_coord",
	bool verbose = false
    );    
}

#endif

