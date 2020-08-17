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

#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/logger.h>
#include <stack>

namespace {

    using namespace GEO;

    /**
     * \brief Computes the number of surface vertices that are not isolated.
     * \param[in] M the mesh
     * \return the number of vertices of a mesh with at least
     *  one incident surface facet
     */
    index_t nb_non_isolated_surface_vertices(const Mesh& M) {
        index_t result = 0;
        std::vector<bool> visited(M.vertices.nb(), false);
        for(index_t c: M.facet_corners) {
            visited[M.facet_corners.vertex(c)] = true;
        }
        for(index_t v: M.vertices) {
            if(visited[v]) {
                ++result;
            }
        }
        return result;
    }
}

namespace GEO {

    index_t get_connected_components(
        const Mesh& M, vector<index_t>& component
    ) {
        static const index_t NO_COMPONENT = index_t(-1);
        index_t nb_components = 0;
        component.assign(M.facets.nb(), NO_COMPONENT);
        for(index_t f: M.facets) {
            if(component[f] == NO_COMPONENT) {
                std::stack<index_t> S;
                S.push(f);
                component[f] = nb_components;
                do {
                    index_t cur_f = S.top();
                    S.pop();
                    for(index_t c: M.facets.corners(cur_f)) {
                        index_t adj_f = M.facet_corners.adjacent_facet(c);
                        if(adj_f != NO_FACET &&
                           component[adj_f] == NO_COMPONENT
                        ) {
                            S.push(index_t(adj_f));
                            component[adj_f] = nb_components;
                        }
                    }
                } while(!S.empty());
                nb_components++;
            }
        }
        return nb_components;
    }

    index_t mesh_nb_connected_components(const Mesh& M) {
        vector<index_t> component;
        return get_connected_components(M, component);
    }

    signed_index_t mesh_Xi(const Mesh& M) {
        index_t nb_v = nb_non_isolated_surface_vertices(M);
        if(nb_v != M.vertices.nb()) {
            if(M.cells.nb() == 0) {
                Logger::warn("Topology")
                    << "Surface mesh has " << M.vertices.nb() - nb_v
                    << " isolated vertices"
                    << std::endl;
            } else {
                Logger::out("Topology")
                    << "Surface mesh has " << M.vertices.nb() - nb_v
                    << " isolated vertices "
                    << " (but they may be attached to tetrahedra)"
                    << std::endl;
            }
        }
        signed_index_t result = signed_index_t(nb_v + M.facets.nb());
        for(index_t f: M.facets) {
            for(index_t c: M.facets.corners(f)) {
                index_t f2 = M.facet_corners.adjacent_facet(c);
                if(f2 == NO_FACET || f > f2) {
                    --result;
                }  // We count each edge once,
            }
        }
        return result;
    }

    signed_index_t mesh_nb_borders(const Mesh& M) {
        // Step 1: chain vertices around borders
        std::vector<index_t> next_around_border(M.vertices.nb(),NO_VERTEX);
        for(index_t f: M.facets) {
            for(index_t c1: M.facets.corners(f)) {
                if(M.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = M.facets.next_corner_around_facet(f, c1);
                    index_t v1 = M.facet_corners.vertex(c1);
                    index_t v2 = M.facet_corners.vertex(c2);
                    if(next_around_border[v1] != NO_VERTEX) {
                        // If this happens, then the same vertex
                        // is incident to more than two edges on
                        // the border (non-manifold configuration,
                        // return "error value" -1)
                        return -1;
                    }
                    // May happen with non-manifold configuration,
                    // where several connected component of the
                    // border can touch the same vertex several
                    // times.
                    next_around_border[v1] = v2;
                }
            }
        }
        // Step 2: count connected components of the borders
        index_t result = 0;
        for(index_t v: M.vertices) {
            if(next_around_border[v] != NO_VERTEX) {
                result++;
                index_t cur = v;
                while(next_around_border[cur] != NO_VERTEX) {
                    index_t next = next_around_border[cur];
                    next_around_border[cur] = NO_VERTEX;
                    cur = next;
                }
            }
        }
        return signed_index_t(result);
    }

    bool meshes_have_same_topology(
        const Mesh& M1, const Mesh& M2, bool verbose
    ) {
        signed_index_t Xi1 = mesh_Xi(M1);
        signed_index_t Xi2 = mesh_Xi(M2);
        if(!verbose && Xi1 != Xi2) {
            return false;
        }

        signed_index_t nbB1 = mesh_nb_borders(M1);
        signed_index_t nbB2 = mesh_nb_borders(M2);
        if(!verbose && nbB1 != nbB2) {
            return false;
        }

        index_t nb_conn1 = mesh_nb_connected_components(M1);
        index_t nb_conn2 = mesh_nb_connected_components(M2);
        if(!verbose && nb_conn1 != nb_conn2) {
            return false;
        }
        if(!verbose) {
            return true;
        }

        bool result = (Xi1 == Xi2 && nbB1 == nbB2 && nb_conn1 == nb_conn2);
        Logger::out("Topology")
            << "M1: Xi=" << Xi1 << " nbB=" << nbB1
            << " nbConn=" << nb_conn1 << std::endl;

        Logger::out("Topology")
            << "M2: Xi=" << Xi2 << " nbB=" << nbB2
            << " nbConn=" << nb_conn2 << std::endl;

        Logger::out("Topology") << (result ? "match." : "mismatch.")
            << std::endl;
        return result;
    }
}

