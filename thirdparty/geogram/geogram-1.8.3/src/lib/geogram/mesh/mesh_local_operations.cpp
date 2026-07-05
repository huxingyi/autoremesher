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

#include <geogram/mesh/mesh_local_operations.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh.h>

namespace {

    using namespace GEO;

    /**
     * \brief Finds the corner of a facet that is adjacent to a given facet.
     * \param[in] M a const reference to a Mesh
     * \param[in] f a facet index in \p M
     * \param[in] f_adj a facet index in \p M
     * \return the corner of facet \p f that is adjacent to facet \p f_adj
     *  or NO_FACET if no such corner exists
     */
    index_t find_corner_by_adjacent_facet(
        const Mesh& M, index_t f, index_t f_adj
    ) {
        for(index_t c: M.facets.corners(f)) {
            if(M.facet_corners.adjacent_facet(c) == f_adj) {
                return c;
            }
        }
        return NO_CORNER;
    }

    /**
     * \brief Copies a mesh vertex.
     * \details It is not correct to use:
     *  \code
     *    M.vertices.create_vertex(M.vertices.point_ptr(v));
     *  \endcode
     *  because Mesh::create_vertex() may realloc the points coordinates
     *  vector, thus invalidating M.vertices.point_ptr(v).
     * \param[in] M a reference to the mesh
     * \param[in] v the index of the vertex to be copied
     * \return the index of the new vertex
     */
    index_t copy_vertex(Mesh& M, index_t v) {
        index_t result = M.vertices.create_vertex();
        double* to = M.vertices.point_ptr(result);
        const double* from = M.vertices.point_ptr(v);
        for(index_t c=0; c<M.vertices.dimension(); ++c) {
            to[c] = from[c];
        }
        return result;
    }
}


/****************************************************************************/

namespace GEO {
    
    void glue_edges(
        Mesh& M,
        index_t f1, index_t c1,
        index_t f2, index_t c2
    ) {
        geo_assert(M.facet_corners.adjacent_facet(c1) == NO_FACET);
        geo_assert(M.facet_corners.adjacent_facet(c2) == NO_FACET);
        geo_debug_assert(c1 >= M.facets.corners_begin(f1));
        geo_debug_assert(c1 < M.facets.corners_end(f1));
        geo_debug_assert(c2 >= M.facets.corners_begin(f2));
        geo_debug_assert(c2 < M.facets.corners_end(f2));


        // Glue geometry
        {
            index_t v1 = M.facet_corners.vertex(c1);
            index_t v2 = M.facet_corners.vertex(
                M.facets.next_corner_around_facet(f1,c1)
            );

            double* p1 = M.vertices.point_ptr(v1);
            double* p2 = M.vertices.point_ptr(v2);

            index_t w1 = M.facet_corners.vertex(c2);
            index_t w2 = M.facet_corners.vertex(
                M.facets.next_corner_around_facet(f2,c2)
            );

            double* q1 = M.vertices.point_ptr(w1);
            double* q2 = M.vertices.point_ptr(w2);

            for(index_t c=0; c<M.vertices.dimension(); ++c) {
                p1[c] = 0.5*(p1[c] + q2[c]);
                q1[c] = 0.5*(q1[c] + p2[c]);
            }
        }

        
        index_t f[2];
        f[0] = f1;
        f[1] = f2;
        index_t c[2];
        c[0] = c1;
        c[1] = c2;

        vector<index_t> delete_vertex(M.vertices.nb(),0);

        //   We need to keep the corners to update in a temporary
        // vector, since updating them during the traversal would
        // break the traversal (don't saw the branch you are sitting
        // on !!)
        
        for(index_t k=0; k<2; ++k) {
            index_t v = M.facet_corners.vertex(c[k]);
            vector<index_t> corners_to_update;
            MeshHalfedges MH(M);
            {
                MeshHalfedges::Halfedge H(f[k],c[k]);
                do {
                    index_t w = M.facet_corners.vertex(H.corner);
                    if(w != v) {
                        delete_vertex[w] = 1;
                        corners_to_update.push_back(H.corner);
                    }
                } while(MH.move_to_prev_around_vertex(H));
            }
            {
                MeshHalfedges::Halfedge H(f[1-k], c[1-k]);
                MH.move_to_next_around_border(H);
                do {
                    index_t w = M.facet_corners.vertex(H.corner);
                    if(w != v) {
                        delete_vertex[w] = 1;
                        corners_to_update.push_back(H.corner);
                    }
                } while(MH.move_to_prev_around_vertex(H));

            }
            for(index_t i=0; i<corners_to_update.size(); ++i) {
                M.facet_corners.set_vertex(
                    corners_to_update[i],v
                );
            }
        }
        
        M.facet_corners.set_adjacent_facet(c[0],f[1]);
        M.facet_corners.set_adjacent_facet(c[1],f[0]);        


        
        M.vertices.delete_elements(delete_vertex);
    }

    void unglue_edges(
        Mesh& M,
        index_t f1, index_t c1
    ) {
        geo_debug_assert(c1 >= M.facets.corners_begin(f1));
        geo_debug_assert(c1 < M.facets.corners_end(f1));
        index_t f2 = M.facet_corners.adjacent_facet(c1);
        geo_assert(f2 != NO_FACET);
        index_t c2 = find_corner_by_adjacent_facet(M,f2,f1);
        geo_assert(c2 != NO_CORNER);

        M.facet_corners.set_adjacent_facet(c1,NO_FACET);
        M.facet_corners.set_adjacent_facet(c2,NO_FACET);

        // Now, we need to determine whether edge extremities were dissociated.
        
        MeshHalfedges MH(M);

        MeshHalfedges::Halfedge H1(f1,c1);
        MeshHalfedges::Halfedge H2(f2,c2);

        MeshHalfedges::Halfedge H1_prev(H1);
        MH.move_to_prev_around_border(H1_prev);
        MeshHalfedges::Halfedge H1_next(H1);
        MH.move_to_next_around_border(H1_next);

        // If the predecessor of H1 around the border is not H2, then
        // H1's origin was splitted into two vertices. Thus we
        // create the new vertex and assign it to H1's origin (c1).
        if(H1_prev != H2) {
            vector<index_t> corners_to_update;
            index_t v = M.facet_corners.vertex(c1);
            index_t new_v = copy_vertex(M,v);
            
            //   Note: we cannot set the corner vertices while we
            // are traversing, since traversal relies on corner-vertex
            // relations (do not saw the branch you are sitting on...),
            // thus we store the corners to be updated in a temporary
            // vector (a bit ugly, but not a big drama).
            MeshHalfedges::Halfedge H(H1);
            do {
                corners_to_update.push_back(H.corner);
            } while(MH.move_to_prev_around_vertex(H));
            for(index_t i=0; i<corners_to_update.size(); ++i) {
                M.facet_corners.set_vertex(
                    corners_to_update[i],new_v
                );
            }
        }

        // If the successor of H1 around the border is not H2, then
        // H2's origin was splitted into two vertices. Thus we
        // create the new vertex and assign it to H2's origin (c2).
        if(H1_next != H2) {
            vector<index_t> corners_to_update;            
            index_t v = M.facet_corners.vertex(c2);
            index_t new_v = copy_vertex(M,v);
            //   Note: we cannot set the corner vertices while we
            // are traversing, since traversal relies on corner-vertex
            // relations (do not saw the branch you are sitting on...),
            // thus we store the corners to be updated in a temporary
            // vector (a bit ugly, but not a big drama).
            MeshHalfedges::Halfedge H(H2);
            do {
                corners_to_update.push_back(H.corner);                
            } while(MH.move_to_prev_around_vertex(H));
            for(index_t i=0; i<corners_to_update.size(); ++i) {
                M.facet_corners.set_vertex(
                    corners_to_update[i],new_v
                );
            }
        }
    }
}


