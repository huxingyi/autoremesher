/*
 *  Copyright (c) 2012-2014, Bruno Levy
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

#ifndef GEOGRAM_MESH_MESH_DECIMATE
#define GEOGRAM_MESH_MESH_DECIMATE

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>

/**
 * \file geogram/mesh/mesh_decimate.h
 * \brief Functions for decimating a mesh
 */

namespace GEO {
    class Mesh;

    /**
     * \brief Determines the operating mode of
     * mesh_decimate_vertex_clustering().
     * The flags can be combined with the 'bitwise or' (|) operator.
     * MESH_REPAIR_DEFAULT fits most uses.
     */
    enum MeshDecimateMode {
        MESH_DECIMATE_FAST = 0,   /**< Gives a raw result quickly */
        MESH_DECIMATE_DUP_F = 1,  /**< Remove duplicated vertices */
        MESH_DECIMATE_DEG_3 = 2,  /**< Remove degree3 vertices    */
        MESH_DECIMATE_KEEP_B = 4, /**< Preserve borders           */
        MESH_DECIMATE_DEFAULT =
            MESH_DECIMATE_DUP_F |
            MESH_DECIMATE_DEG_3 |
            MESH_DECIMATE_KEEP_B
            /**< Fits most uses */
    };

    /**
     * \brief Generates a simplified representation of a mesh.
     * \param[in,out] M the mesh to decimate
     * \param[in] nb_bins the higher, the more detailed mesh.
     * \param[in] mode a combination of #MeshDecimateMode flags.
     *  Combine them with the 'bitwise or' (|) operator.
     * \param[in] vertices_flags an array of flags associated with
     *  each vertex of \p mesh_id, or nullptr if unspecified. Memory
     *  is managed by client code. If \p vertices_flags[v] is
     *  non-zero, then vertex v is preserved, else it can be discarded.
     */
    void GEOGRAM_API mesh_decimate_vertex_clustering(
        Mesh& M, index_t nb_bins,
        MeshDecimateMode mode = MESH_DECIMATE_DEFAULT,
        geo_index_t* vertices_flags = nullptr
    );
}

#endif

