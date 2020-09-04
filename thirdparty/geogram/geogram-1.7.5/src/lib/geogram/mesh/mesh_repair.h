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

#ifndef GEOGRAM_MESH_MESH_REPAIR
#define GEOGRAM_MESH_MESH_REPAIR

/**
 * \file mesh_repair.h
 */

#include <geogram/basic/common.h>
#include <geogram/basic/memory.h>

namespace GEO {

    class Mesh;

    /**
     * \brief Determines the operating mode of mesh_repair().
     * The flags can be combined with the 'bitwise or' (|) operator.
     * MESH_REPAIR_DEFAULT fits most uses. 
     */
    enum MeshRepairMode {
        MESH_REPAIR_TOPOLOGY = 0,     
                         /**< Dissociates non-manifold vertices (always done) */
        MESH_REPAIR_COLOCATE = 1,     /**< Merges identical vertices          */
        MESH_REPAIR_DUP_F = 2,        /**< Removes duplicated facets          */
        MESH_REPAIR_TRIANGULATE = 4,  /**< Triangulates mesh                  */
        MESH_REPAIR_RECONSTRUCT = 8,  /**< Post-process result of Co3Ne algo. */
	MESH_REPAIR_QUIET       = 16, /**< Do not display any message.        */
        MESH_REPAIR_DEFAULT =
            MESH_REPAIR_COLOCATE |
            MESH_REPAIR_DUP_F |
            MESH_REPAIR_TRIANGULATE
            /**< Fits most uses */
    };

    /**
     * \brief Fixes some defaults in a mesh.
     * \param[in,out] M the mesh to repair
     * \param[in] mode a combination of #MeshRepairMode flags.
     *  Combine them with the 'bitwise or' (|) operator.
     * \param[in] colocate_epsilon tolerance used to colocate vertices
     *  (if #MESH_REPAIR_COLOCATE is set in mode).
     */
    void GEOGRAM_API mesh_repair(
        Mesh& M,
        MeshRepairMode mode = MESH_REPAIR_DEFAULT,
        double colocate_epsilon = 0.0
    );

    /**
     * \brief Post-processes a Restricted
     *  Delaunay Triangulation.
     * \details Reconstructs the triangle-triangle connectivity and
     * removes some degeneracies (vertices with a unique triangle
     * incident to them).
     */
    void GEOGRAM_API mesh_postprocess_RDT(
        Mesh& M
    );



    /**
     * \brief Reorients the facets of a mesh coherently.
     * \details The input mesh may have facets that have 
     *  incoherent orientations, i.e. edges that do not
     *  respect the Moebius law (two facets that share an
     *  edge, one oriented clockwise and the other one
     *  anticlockwise). This function detects and repairs
     *  such configurations by flipping the incoherent facets.
     *  Facet-facet links (corner_adjacent_facet) need to be 
     *  initialized as follows:
     *  for two corners c1, c2, if we have:
     *   - v1 = corner_vertex_index(c1)
     *   - v2 = corner_vertex_index(c1,next_around_facet(c2f(c1),c1))
     *   - w1 = corner_vertex_index(c2)
     *   - w2 = corner_vertex_index(c2,next_around_facet(c2f(c2),c2))
     *  then c1 and c2 are adjacent if we have:
     *   - v1=w2 and v2=w1 (as usual) or:
     *   - v1=v2 and w1=w2 ('inverted' configuration)
     *  On exit, facets are flipped in such a way that only the first 
     *  configuration (v1=w2 and v2=w1) appears. Moebius strips, if 
     *  encountered, are cut.
     * \param[in,out] M the mesh to reorient
     * \param[out] moebius_facets a pointer to a vector. On exit,
     *  *moebius_facets[f] has a non-zero value if facet f is
     *  incident to an edge that could not be consistently oriented.
     *  If nullptr, then this information is not returned.
     */
    void GEOGRAM_API mesh_reorient(
        Mesh& M, vector<index_t>* moebius_facets=nullptr
    );

    /**
     * \brief Detects colocated vertices in a mesh.
     * \details Example of function to remove duplicated
     *  vertices in a pointset:
     *  \code
     *   mesh_detect_colocated_vertices(M, colocated, epsilon);
     *   for(index_t v=0; v<M.vertices.nb(); ++v) {
     *      if(colocated[v] == v) {
     *         // keep vertex if colocated with itself
     *         colocated[v] = 0; 
     *      } else {
     *         // delete vertex if colocated with other
     *         colocated[v] = 1; 
     *      }
     *   }
     *   // note: this code supposes that M is a pointset.
     *   // If the mesh has facets and cells, then
     *   // references to facet corners and cell corners
     *   // need to be updated here...
     *   M.vertices.delete_elements(colocated);
     *  \endcode
     * \param[in] M a const reference to the mesh
     * \param[out] v_colocated_index on exit, a vector
     *  of size M.vertices.nb(), such that for each vertex 
     *  index v, v_colocated_index[v] contains either v (if 
     *  v should be kept) or the index of the vertex that v 
     *  is colocated with.
     * \param[in] colocate_epsilon if the distance between two
     *  mesh vertices is smaller than colocate_epsilon, then they
     *  are colocated.
     */
    void GEOGRAM_API mesh_detect_colocated_vertices(
        const Mesh& M, vector<index_t>& v_colocated_index,
        double colocate_epsilon=0.0
    );

    /**
     * \brief Detects isolated vertices in a mesh.
     * \details A vertex is isolated if no mesh element
     *  (edge, facet or cell) is incident to it.
     * \param[in] M a const reference to the mesh
     * \param[out] v_is_isolated on exit, a vector of
     *  size M.vertices.nb(), such that v_is_isolated[v]
     *  is equal to 1 if v is isolated or 0 if v is 
     *  not isolated.
     */
    void GEOGRAM_API mesh_detect_isolated_vertices(
        const Mesh& M, vector<index_t>& v_is_isolated
    );

    /**
     * \brief Detects degenerate facets in a mesh.
     * \details A facet is degenerate if it is 
     *  incident to the same vertex several times.
     * \param[in] M a const reference to the mesh
     * \param[out] f_is_degenerate on exit, a vector of
     *  size M.facets.nb(), such that f_is_degenerate[f]
     *  is equal to 1 if f is degenerate or 0 if f is 
     *  not degenerate.
     */
    void GEOGRAM_API mesh_detect_degenerate_facets(
        const Mesh& M, vector<index_t>& f_is_degenerate
    );

    
}

#endif

