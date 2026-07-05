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

#ifndef GEOGRAM_MESH_MESH_PARAM_PACKER
#define GEOGRAM_MESH_MESH_PARAM_PACKER

#include <geogram/basic/common.h>

/**
 * \file geogram/mesh/mesh_param_packer.h
 */

namespace GEO {

    class Mesh;
    
    /**
     * \brief Normalizes chart texture coordinates
     * \details Ensure that each chart has a texture area proportional
     *  to its 3D area.
     */
    void GEOGRAM_API pack_atlas_only_normalize_charts(Mesh& mesh);

    /**
     * \brief Packs an atlas using the xatlas library.
     * \details The mesh needs to have a parameterization
     *  stored in the tex_coord facet_corner attribute.
     * \param[in,out] mesh a reference to the mesh
     */
    void GEOGRAM_API pack_atlas_using_xatlas(Mesh& mesh);

    /**
     * \brief Packs an atlas using the tetris packer algorithm.
     * \details The mesh needs to have a parameterization
     *  stored in the tex_coord facet_corner attribute.
     *  The method is decribed in the following referene:
     *  - least squares mode: Least Squares Conformal Maps, 
     *    Levy, Petitjean, Ray, Maillot, ACM SIGGRAPH, 2002
     * \param[in,out] mesh a reference to the mesh
     */
    void GEOGRAM_API pack_atlas_using_tetris_packer(Mesh& mesh);
    
    /****************************************************************/

}
#endif

