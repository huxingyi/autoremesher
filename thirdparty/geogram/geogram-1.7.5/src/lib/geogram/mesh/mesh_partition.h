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

#ifndef GEOGRAM_MESH_MESH_PARTITION
#define GEOGRAM_MESH_MESH_PARTITION

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>

/**
 * \file geogram/mesh/mesh_partition.h
 * \brief Functions to split a mesh into multiple parts
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Strategy for mesh partitioning.
     */
    enum MeshPartitionMode {
        /** Mesh parts are index slices of equal sizes in Hilbert order */
        MESH_PARTITION_HILBERT = 1,

        /** Mesh parts are the connected components of the mesh */
        MESH_PARTITION_CONNECTED_COMPONENTS = 2
    };

    /**
     * \brief Partitions a mesh into a fixed number of parts.
     *
     * \details The mesh facets and tetrahedra are reordered in
     * such a way that each mesh part contains a contiguous set
     * of facet indices.
     *
     * \param[in,out] M the mesh to partition
     * \param[in] mode one of #MESH_PARTITION_HILBERT,
     *  #MESH_PARTITION_CONNECTED_COMPONENTS.
     * \param[out] facet_ptr
     *  on exit, part p's facets are facet_ptr[p] ... facet_ptr[p+1]-1
     * \param[out] tet_ptr
     *  on exit, part p's tetrahedra are tet_ptr[p] ... tet_ptr[p+1]-1
     * \param[in] nb_parts number of parts to create.
     *  Ignored if mode = #MESH_PARTITION_CONNECTED_COMPONENTS
     */
    void GEOGRAM_API mesh_partition(
        Mesh& M,
        MeshPartitionMode mode,
        vector<index_t>& facet_ptr,
        vector<index_t>& tet_ptr,
        index_t nb_parts = 0
    );

    /**
     * \brief Partitions a mesh into a fixed number of parts.
     *
     * \details The mesh facets are reordered in
     * such a way that each mesh part contains a contiguous set
     * of facet indices.
     *
     * \param[in,out] M the mesh to partition
     * \param[in] mode one of #MESH_PARTITION_HILBERT,
     *  #MESH_PARTITION_CONNECTED_COMPONENTS.
     * \param[out] facet_ptr
     *  on exit, part p's facets are facet_ptr[p] ... facet_ptr[p+1]-1
     * \param[in] nb_parts number of parts to create.
     *  Ignored if mode = #MESH_PARTITION_CONNECTED_COMPONENTS
     */
    void GEOGRAM_API mesh_partition(
        Mesh& M,
        MeshPartitionMode mode,
        vector<index_t>& facet_ptr,
        index_t nb_parts = 0
    );
}

#endif

