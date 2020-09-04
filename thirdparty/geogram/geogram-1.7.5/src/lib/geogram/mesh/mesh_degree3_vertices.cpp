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

#include <geogram/mesh/mesh_degree3_vertices.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/index.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/logger.h>

namespace {

    using namespace GEO;

    // TODO: include in Doxygen documentation.
    /*
     * There are two different numerotation for triangles:
     *
     *  o GEO::Mesh numerotation:
     *     - Used everywhere in Vorpaline
     *     - Edge ei is right after vertex vi when turning
     *         around the facet.
     *
     *  o Standard triangle numerotation:
     *     - Used in this file, by t_xxx() functions
     *       and Degree3Vertex class.
     *     - Edge ei is opposite to vertex vi.
     *
     *  GEO::Mesh numerotation:
     *        v2
     *    e2 /  \e1
     *      /    \
     *     v0----v1
     *        e0
     *
     *  Standard triangle numerotation:
     *        v2
     *    e1 /  \e0
     *      /    \
     *     v0----v1
     *        e2
     */

    /**
     * \brief Gets the vertex of a triangle by its local index, using
     *  standard triangle numerotation (edge i is opposite to vertex i).
     * \param[in] M the mesh
     * \param[in] t the index of the triangle
     * \param[in] li the local index (0,1 or 2) of the vertex in \p t
     * \return the global index of the vertex in \p M
     * \note Everywhere else in Vorpaline, triangle edge numerotation is
     *  different from standard triangle numerotation used here.
     */
    inline index_t t_vertex(
        const Mesh& M, index_t t, index_t li
    ) {
        geo_debug_assert(M.facets.nb_vertices(t) == 3);
        geo_debug_assert(li < 3);
        return M.facet_corners.vertex(M.facets.corners_begin(t) + li);
    }

    /**
     * \brief Gets the index of an adjacent triangle, using
     * \param[in] M the mesh
     * \param[in] t the index of the triangle
     * \param[in] le the local index (0,1 or 2) of the edge in \p t, using
     *  standard triangle numerotation (edge i is opposite to vertex i).
     * \return the global index of the triangle adjacent to \p t accros
     *  edge \p le in \p M
     * \note Everywhere else in Vorpaline, triangle edge numerotation is
     *  different from standard triangle numerotation used here.
     */

    inline index_t t_adjacent(const Mesh& M, index_t t, index_t le) {
        geo_debug_assert(M.facets.nb_vertices(t) == 3);
        geo_debug_assert(le < 3);
        le = (le + 1) % 3; // convert corner numerotation
                           // to standard triangle numerotation
        return M.facet_corners.adjacent_facet(M.facets.corners_begin(t) + le);
    }

    /**
     * \brief Gets the local vertex index in a triangle by its global index,
     *  using standard triangle numerotation (edge i is opposite to vertex i).
     * \param[in] M the mesh
     * \param[in] t the index of the triangle
     * \param[in] v the global index of the vertex in \p M
     * \return the local index (0,1 or 2) of the vertex \p v in \p t
     * \note Everywhere else in Vorpaline, triangle edge numerotation is
     *  different from standard triangle numerotation used here.
     */
    inline index_t t_index(const Mesh& M, index_t t, index_t v) {
        geo_debug_assert(M.facets.nb_vertices(t) == 3);
        for(index_t li = 0; li < 3; li++) {
            if(M.facet_corners.vertex(M.facets.corners_begin(t) + li) == v) {
                return li;
            }
        }
        geo_assert_not_reached;
    }

    /**
     * \brief Sets a triangle vertices and neighbors
     *  using standard triangle numerotation (edge i is opposite to vertex i).
     * \param[in] M the mesh
     * \param[in] t the index of the triangle
     * \param[in] v1 the global index of the first vertex
     * \param[in] v2 the global index of the second vertex
     * \param[in] v3 the global index of the third vertex
     * \param[in] adj1 the index of the adjacent triangle opposite to \p v1
     * \param[in] adj2 the index of the adjacent triangle opposite to \p v2
     * \param[in] adj3 the index of the adjacent triangle opposite to \p v3
     * \note Everywhere else in Vorpaline, triangle edge numerotation is
     *  different from standard triangle numerotation used here.
     */
    inline void t_set(
        Mesh& M,
        index_t t,
        index_t v1, index_t v2, index_t v3,
        index_t adj1, index_t adj2, index_t adj3
    ) {
        geo_debug_assert(M.facets.nb_vertices(t) == 3);
        geo_debug_assert(adj1 != NO_FACET);
        geo_debug_assert(adj2 != NO_FACET);
        geo_debug_assert(adj3 != NO_FACET);
        index_t c1 = M.facets.corners_begin(t);
        index_t c2 = c1 + 1;
        index_t c3 = c1 + 2;
        M.facet_corners.set_vertex(c1, v1);
        M.facet_corners.set_vertex(c2, v2);
        M.facet_corners.set_vertex(c3, v3);
        M.facet_corners.set_adjacent_facet(c1, adj3);
        M.facet_corners.set_adjacent_facet(c2, adj1);
        M.facet_corners.set_adjacent_facet(c3, adj2);
    }

    // TODO: ascii-art needed here !!

    /**
     * \brief Stores the three facets incident to a degree 3 vertex
     *  and the three adjacent facets.
     * \details Implements operator< (according to distance between degree 3
     *  vertex and neighbor vertices supporting plane).
     * \note Uses standard triangle numerotation (edge i is opposite to
     *  vertex i). Everywhere else in Vorpaline, triangle edge numerotation is
     *  different from standard triangle numerotation used here.
     */
    struct Degree3Vertex {
        
        /**
         * \brief Constructs a new Degree3Vertex
         * \param[in] M the mesh
         * \param[in] v_in the index of the degree 3 vertex in \p M
         * \param[in] t_in the index of a triangle incident to \p v_in
         * \pre there are exactly three triangles incident to \p v_in
         */
        Degree3Vertex(const Mesh& M, index_t v_in, index_t t_in) {
            v[3] = v_in;
            t[0] = t_in;
            {
                index_t i = t_index(M, t[0], v[3]);
                index_t j = (i + 1) % 3;
                index_t k = (j + 1) % 3;

                t[1] = index_t(t_adjacent(M, t[0], j));
                geo_debug_assert(t[1] != index_t(-1));
                t[2] = index_t(t_adjacent(M, t[0], k));
                geo_debug_assert(t[2] != index_t(-1));
                v[1] = t_vertex(M, t[0], j);
                v[2] = t_vertex(M, t[0], k);
                adj[0] = t_adjacent(M, t[0], i);
            }

            {
                index_t i = t_index(M, t[1], v[3]);
                index_t j = (i + 1) % 3;
                index_t k = (j + 1) % 3;
                v[0] = t_vertex(M, t[1], k);
                geo_debug_assert(t_vertex(M, t[1], j) == v[2]);
                geo_debug_assert(t_adjacent(M, t[1], j) == t[2]);
                geo_debug_assert(t_adjacent(M, t[1], k) == t[0]);
                adj[1] = t_adjacent(M, t[1], i);
            }

            {
                index_t i = t_index(M, t[2], v[3]);
#ifdef GEO_DEBUG
                index_t j = (i + 1) % 3;
                index_t k = (j + 1) % 3;
                geo_debug_assert(t_vertex(M, t[2], j) == v[0]);
                geo_debug_assert(t_vertex(M, t[2], k) == v[1]);
                geo_debug_assert(t_adjacent(M, t[2], j) == t[0]);
                geo_debug_assert(t_adjacent(M, t[2], k) == t[1]);
#endif
                adj[2] = t_adjacent(M, t[2], i);
            }

            const vec3& p0 = Geom::mesh_vertex(M, v[0]);
            const vec3& p1 = Geom::mesh_vertex(M, v[1]);
            const vec3& p2 = Geom::mesh_vertex(M, v[2]);
            const vec3& p3 = Geom::mesh_vertex(M, v[3]);

            dist = ::sqrt(
                Geom::point_triangle_squared_distance(p3, p0, p1, p2)
            );
        }

        /**
         * \brief Compares two Degree3Vertex by the distance to the
         *  supporting planes of the neighbors.
         * \param[in] rhs the comparand
         * \return true if this Degree3Vertex can be removed before
         *  \p rhs, false otherwise
         */
        bool operator< (const Degree3Vertex& rhs) const {
            return dist < rhs.dist;
        }

        double dist;
        index_t v[4];
        index_t t[3];
        index_t adj[3];
    };
}

/****************************************************************************/

namespace GEO {

    index_t remove_degree3_vertices(Mesh& M, double max_dist) {

        if(max_dist == 0.0) {
            return 0;
        }

        // Step 1: detect degree3 vertices

        vector<signed_index_t> vertex_degree(M.vertices.nb(), 0);
        //   or -1 if v is on border or if v has an incident facet that
        // is not a triangle.

        for(index_t f: M.facets) {
            bool f_is_triangle = (M.facets.nb_vertices(f) == 3);
            for(index_t c: M.facets.corners(f)) {
                index_t v = M.facet_corners.vertex(c);
                if(
                    !f_is_triangle ||
                    (M.facet_corners.adjacent_facet(c) == NO_FACET)
                ) {
                    vertex_degree[v] = -1;
                } else {
                    if(vertex_degree[v] != -1) {
                        vertex_degree[v]++;
                    }
                }
            }
        }

        // Step 2: count degree3 vertices

        index_t nb_degree3_vertices = 0;
        for(index_t v: M.vertices) {
            if(vertex_degree[v] == 3) {
                nb_degree3_vertices++;
            }
        }

        if(nb_degree3_vertices == 0) {
            Logger::out("Degree3")
                << "Does not have any degree 3 vertex (good)" << std::endl;
            return 0;
        }

        // Step 3: v2f[v] is one of the facets adjacent to v

        vector<index_t> v2f(M.vertices.nb());
        for(index_t f: M.facets) {
            for(index_t c: M.facets.corners(f)) {
                index_t v = M.facet_corners.vertex(c);
                v2f[v] = f;
            }
        }

        // Step 4: compute and sort degree3 vertices configurations

        vector<Degree3Vertex> degree3vertices;
        degree3vertices.reserve(nb_degree3_vertices);
        for(index_t v: M.vertices) {
            if(vertex_degree[v] == 3) {
                Degree3Vertex V(M, v, v2f[v]);
                if(V.dist < max_dist) {
                    degree3vertices.push_back(V);
                }
            }
        }

        Logger::out("Degree3")
            << "Removing " << degree3vertices.size()
            << "/" << nb_degree3_vertices
            << " degree 3 vertices (within max_deg3_dist)"
            << std::endl;

        std::sort(degree3vertices.begin(), degree3vertices.end());

        // Step 5: remove degree3 vertices in order

        enum FacetStatus {
            F_UNTOUCHED = 0,
            F_TO_REMOVE = 1,
            F_MODIFIED = 2
        };

        vector<index_t> facet_status(M.facets.nb(), F_UNTOUCHED);
        index_t nb_removed = 0;

        for(index_t i = 0; i < degree3vertices.size(); i++) {
            const Degree3Vertex& V = degree3vertices[i];
            index_t t1 = V.t[0];
            index_t t2 = V.t[1];
            index_t t3 = V.t[2];

            // Skip vertex if one of its incident facet was already modified
            // when removing a neighboring degree 3 vertex.
            if(
                facet_status[t1] != F_UNTOUCHED ||
                facet_status[t2] != F_UNTOUCHED ||
                facet_status[t3] != F_UNTOUCHED
            ) {
                continue;
            }

            // Skip vertex if one of its neighboring facet
            // was already modified...
            index_t adj1 = V.adj[0];
            index_t adj2 = V.adj[1];
            index_t adj3 = V.adj[2];
            if(adj1 != NO_FACET && facet_status[adj1] != F_UNTOUCHED) {
                continue;
            }
            if(adj2 != NO_FACET && facet_status[adj2] != F_UNTOUCHED) {
                continue;
            }
            if(adj3 != NO_FACET && facet_status[adj3] != F_UNTOUCHED) {
                continue;
            }

            facet_status[t2] = F_TO_REMOVE;  // t2 will be deleted.
            facet_status[t3] = F_TO_REMOVE;  // t3 will be deleted.
            facet_status[t1] = F_MODIFIED;   // t1 is recycled and
                                             // used by the new facet.
            t_set(
                M, t1, V.v[0], V.v[1], V.v[2],
                index_t(V.adj[0]), index_t(V.adj[1]), index_t(V.adj[2])
            );

            // connect the neighbors of t2 and t3 to t1
            for(index_t j = 1; j <= 2; j++) {
                if(V.adj[j] != NO_FACET) {
                    index_t f = index_t(V.adj[j]);
                    for(index_t c: M.facets.corners(f)) {
                        if(M.facet_corners.adjacent_facet(c) == V.t[j]) {
                            M.facet_corners.set_adjacent_facet(c, V.t[0]);
                        }
                    }
                }
            }
            nb_removed++;
        }

        // Step 6: delete dangling triangles

        // Do not delete the facets that were recycled.
        for(index_t i = 0; i < facet_status.size(); i++) {
            if(facet_status[i] == F_MODIFIED) {
                facet_status[i] = 0;
            }
        }

        // Note: remove_facets() calls remove_isolated_vertices()
        M.facets.delete_elements(facet_status);

        return nb_removed;
    }
}

