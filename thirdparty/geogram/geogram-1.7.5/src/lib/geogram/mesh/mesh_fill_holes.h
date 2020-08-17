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

#ifndef GEOGRAM_MESH_MESH_FILL_HOLES
#define GEOGRAM_MESH_MESH_FILL_HOLES

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/mesh/mesh_fill_holes.h
 * \brief Functions for filling the holes in a mesh
 */

namespace GEO {
    class Mesh;

    /**
     * \brief Fills holes in mesh by generating new triangles
     * \details This fills specific holes in mesh \p M with new triangles.
     * This only fills the holes for which the total area of the
     *  generated triangles is smaller than \p max_area and for which
     *  the number of edges is smaller than \p max_edges.
     *  This does not generate new vertices.
     *
     * \param[in,out] M the mesh to modify
     * \param[in] max_area maximum area of a hole to be filled, larger holes
     *  are ignored.
     * \param[in] max_edges maximum number of edges around a hole to be filled,
     *  larger holes are ignored.
     * \param[in] repair if true (default), then the newly generated triangles 
     *  are connected to their neighbors and the zero-length edges are 
     *  discarted. 
     */
    void GEOGRAM_API fill_holes(
        Mesh& M, 
        double max_area = 0.0,
        index_t max_edges = max_index_t(),
        bool repair = true
    );


    /**
     * \brief Subdivides the facets with more than nb_vertices.
     * \param[in] M a reference to a surface mesh.
     * \param[in] max_nb_vertices maximum number of vertices in
     *  a facet.
     */
    void GEOGRAM_API tessellate_facets(
	Mesh& M, index_t max_nb_vertices
    );
}

#endif

