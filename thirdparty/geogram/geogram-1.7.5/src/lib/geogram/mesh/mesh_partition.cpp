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

#include <geogram/mesh/mesh_partition.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/basic/permutation.h>
#include <stack>

namespace {

    using namespace GEO;

    /**
     * \brief Partitions a surface using Hilbert ordering.
     * \details Reorders the surface facets using Hilber order then
     *  extracts index slices of equal sizes.
     * \param[in,out] M the mesh to be partitioned
     * \param[out] facet_ptr the facet pointers of the parts.
     *  Facets indices of part \p p are: facet_ptr[p],...,facet_ptr[p+1].
     * \param[in] nb_parts number of parts to generate
     */
    void partition_Hilbert_surface(
        Mesh& M,
        vector<index_t>& facet_ptr,
        index_t nb_parts
    ) {
        mesh_reorder(M, MESH_ORDER_HILBERT);
        index_t part_size = M.facets.nb() / nb_parts;
        facet_ptr.resize(nb_parts + 1);
        facet_ptr[0] = 0;
        for(index_t i = 1; i < nb_parts; i++) {
            facet_ptr[i] = facet_ptr[i - 1] + part_size;
        }
        facet_ptr[nb_parts] = M.facets.nb();
    }

    /**
     * \brief Partitions a surface and a volume using Hilbert ordering.
     * \details Reorders the surface facets and tetrahedra
     *  using Hilber order then extracts index slices of equal sizes.
     * \param[in,out] M the mesh to be partitioned
     * \param[out] facet_ptr the facet pointers of the parts.
     *  Facets indices of part \p p are: facet_ptr[p],...,facet_ptr[p+1].
     * \param[out] tet_ptr the tetrahedra pointers of the parts.
     *  Tets indices of part \p p are: tet_ptr[p],...,tet_ptr[p+1].
     * \param[in] nb_parts number of parts to generate
     */
    void partition_Hilbert_surface_and_volume(
        Mesh& M,
        vector<index_t>& facet_ptr,
        vector<index_t>& tet_ptr,
        index_t nb_parts
    ) {
        partition_Hilbert_surface(M, facet_ptr, nb_parts);
        if(M.cells.nb() != 0) {
            index_t part_size = M.cells.nb() / nb_parts;
            tet_ptr.resize(nb_parts + 1);
            tet_ptr[0] = 0;
            for(index_t i = 1; i < nb_parts; i++) {
                tet_ptr[i] = tet_ptr[i - 1] + part_size;
            }
            tet_ptr[nb_parts] = M.cells.nb();
        }
    }

    /**
     * \brief Partitions a surface into its connected components.
     * \param[in,out] M the mesh to be partitioned. Its facets are
     *  reorder in such a way that the facets that correspond to
     *  the same connected component have contiguous indices
     * \param[out] facet_ptr the facet pointers of the parts.
     *  Facets indices of part \p p are: facet_ptr[p],...,facet_ptr[p+1].
     */
    void partition_surface_connected_components(
        Mesh& M,
        vector<index_t>& facet_ptr
    ) {
        const index_t UNVISITED = index_t(-1);

        vector<index_t> new_index(M.facets.nb(), UNVISITED);
        std::stack<index_t> S;
        index_t new_cur_index = 0;
        for(index_t f: M.facets) {
            if(new_index[f] == UNVISITED) {
                facet_ptr.push_back(new_cur_index);
                new_index[f] = new_cur_index;
                new_cur_index++;
                S.push(f);
            }
            while(!S.empty()) {
                index_t ftop = S.top();
                S.pop();
                for(index_t c: M.facets.corners(ftop)) {
                    index_t g = M.facet_corners.adjacent_facet(c);
                    if(g != NO_FACET && new_index[g] == UNVISITED) {
                        new_index[g] = new_cur_index;
                        new_cur_index++;
                        S.push(index_t(g));
                    }
                }
            }
        }
        geo_assert(new_cur_index == M.facets.nb());
        facet_ptr.push_back(new_cur_index);
        Permutation::invert(new_index);
        M.facets.permute_elements(new_index);
    }

    /**
     * \brief Partitions a volume into its connected components.
     * \param[in,out] M the mesh to be partitioned. Its tets are
     *  reorder in such a way that the tets that correspond to
     *  the same connected component have contiguous indices
     * \param[out] tet_ptr the tetrahedra pointers of the parts.
     *  Tets indices of part \p p are: tet_ptr[p],...,tet_ptr[p+1].
     */
    void partition_volume_connected_components(
        Mesh& M,
        vector<index_t>& tet_ptr
    ) {
        const index_t UNVISITED = index_t(-1);

        vector<index_t> new_index(M.cells.nb(), UNVISITED);
        std::stack<index_t> S;
        index_t new_cur_index = 0;
        for(index_t t: M.cells) {
            if(new_index[t] == UNVISITED) {
                tet_ptr.push_back(new_cur_index);
                new_index[t] = new_cur_index;
                new_cur_index++;
                S.push(t);
            }
            while(!S.empty()) {
                index_t t1 = S.top();
                S.pop();
                for(index_t lf = 0; lf < 4; lf++) {
                    index_t t2 = M.cells.adjacent(t1, lf);
                    if(t2 != NO_CELL && new_index[t2] == UNVISITED) {
                        new_index[t2] = new_cur_index;
                        new_cur_index++;
                        S.push(index_t(t2));
                    }
                }
            }
        }
        geo_assert(new_cur_index == M.cells.nb());
        tet_ptr.push_back(new_cur_index);
        Permutation::invert(new_index);
        M.cells.permute_elements(new_index);
    }
}

/****************************************************************************/

namespace GEO {

    void mesh_partition(
        Mesh& M,
        MeshPartitionMode mode,
        vector<index_t>& facet_ptr,
        index_t nb_parts
    ) {
        switch(mode) {
            case MESH_PARTITION_HILBERT:
                partition_Hilbert_surface(M, facet_ptr, nb_parts);
                break;
            case MESH_PARTITION_CONNECTED_COMPONENTS:
                partition_surface_connected_components(M, facet_ptr);
                break;
        }
    }

    void mesh_partition(
        Mesh& M,
        MeshPartitionMode mode,
        vector<index_t>& facet_ptr,
        vector<index_t>& tet_ptr,
        index_t nb_parts
    ) {
        switch(mode) {
            case MESH_PARTITION_HILBERT:
                partition_Hilbert_surface_and_volume(
                    M, facet_ptr, tet_ptr, nb_parts
                );
                break;
            case MESH_PARTITION_CONNECTED_COMPONENTS:
                partition_surface_connected_components(M, facet_ptr);
                if(M.cells.nb() != 0) {
                    partition_volume_connected_components(M, tet_ptr);
                }
                break;
        }
    }
}

