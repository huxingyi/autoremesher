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

#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/index.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/stopwatch.h>

#include <stack>

namespace {

    using namespace GEO;

    /**
     * \brief
     * Compute the signed volume of the pyramid that connects
     * the origin to facet f.
     * \details Summing all the signed volumes
     * of the facets of a closed surface results in the signed
     * volume of the interior of the surface (volumes outside
     * the surface cancel-out).
     * \param[in] M the mesh
     * \param[in] f index of the facet
     * \return the signed volume of the pyramid that connects facet \p f to
     *  the origin
     */
    double signed_volume(const Mesh& M, index_t f) {
        double result = 0;
        index_t v0 = M.facet_corners.vertex(M.facets.corners_begin(f));
        const vec3& p0 = Geom::mesh_vertex(M, v0);
        for(index_t c =
            M.facets.corners_begin(f) + 1; c + 1 < M.facets.corners_end(f); c++
        ) {
            index_t v1 = M.facet_corners.vertex(c);
            const vec3& p1 = Geom::mesh_vertex(M, v1);
            index_t v2 = M.facet_corners.vertex(c + 1);
            const vec3& p2 = Geom::mesh_vertex(M, v2);
            result += dot(p0, cross(p1, p2));
        }
        return result;
    }
}

/****************************************************************************/

namespace GEO {

    void expand_border(Mesh& M, double epsilon) {
        if(epsilon == 0.0) {
            return;
        }
        vector<vec3> border_normal;
        border_normal.assign(M.vertices.nb(), vec3(0.0, 0.0, 0.0));
        for(index_t f: M.facets) {
            vec3 N = Geom::mesh_facet_normal(M, f);
            for(index_t c1: M.facets.corners(f)) {
                if(M.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = M.facets.next_corner_around_facet(f, c1);
                    index_t v1 = M.facet_corners.vertex(c1);
                    index_t v2 = M.facet_corners.vertex(c2);
                    const vec3& p1 = Geom::mesh_vertex(M, v1);
                    const vec3& p2 = Geom::mesh_vertex(M, v2);
                    vec3 Ne = cross(p2 - p1, N);
                    border_normal[v1] += Ne;
                    border_normal[v2] += Ne;
                }
            }
        }
        for(index_t v: M.vertices) {
            double s = length(border_normal[v]);
            if(s > 0.0) {
                Geom::mesh_vertex_ref(M, v) +=
                    epsilon * (1.0 / s) * border_normal[v];
            }
        }
    }

    // == connected components and small facets ================================

    void remove_small_facets(Mesh& M, double min_facet_area) {
        vector<index_t> remove_f(M.facets.nb(), 0);
        for(index_t f: M.facets) {
            if(Geom::mesh_facet_area(M, f, 3) < min_facet_area) {
                remove_f[f] = 1;
            }
        }
        M.facets.delete_elements(remove_f);
    }

    void remove_small_connected_components(
        Mesh& M, double min_area, index_t min_facets
    ) {
        vector<index_t> component;
        index_t nb_components = get_connected_components(M, component);
        vector<double> comp_area(nb_components, 0.0);
        vector<index_t> comp_facets(nb_components, 0);
        for(index_t f: M.facets) {
            comp_area[component[f]] += Geom::mesh_facet_area(M, f, 3);
            ++comp_facets[component[f]];
        }

        Logger::out("Components")
            << "Nb connected components=" << comp_area.size() << std::endl;
        index_t nb_remove = 0;
        for(index_t c = 0; c < comp_area.size(); c++) {
            if(comp_area[c] < min_area || comp_facets[c] < min_facets) {
                nb_remove++;
            }
        }

        if(nb_remove == 0) {
            Logger::out("Components")
                << "Mesh does not have small connected component (good)"
                << std::endl;
            return;
        }

        index_t nb_f_remove = 0;
        vector<index_t> remove_f(M.facets.nb(), 0);
        for(index_t f: M.facets) {
            if(
                comp_area[component[f]] < min_area || 
                comp_facets[component[f]] < min_facets
            ) {
                remove_f[f] = 1;
                nb_f_remove++;
            }
        }
        M.facets.delete_elements(remove_f);

        Logger::out("Components")
            << "Removed " << nb_remove << " connected components"
            << "(" << nb_f_remove << " facets)"
            << std::endl;
    }

    // ============== orient_normals ========================================

    void orient_normals(Mesh& M) {
        vector<index_t> component;
        index_t nb_components = get_connected_components(M, component);
        vector<double> comp_signed_volume(nb_components, 0.0);
        for(index_t f: M.facets) {
            comp_signed_volume[component[f]] += signed_volume(M, f);
        }
        for(index_t f: M.facets) {
            if(comp_signed_volume[component[f]] < 0.0) {
                M.facets.flip(f);
            }
        }
    }

    void invert_normals(Mesh& M) {
        for(index_t f: M.facets) {
            M.facets.flip(f);
        }
    }

    /************************************************************************/
    
    void remove_degree2_vertices(Mesh& M) {
        std::set<index_t> to_dissociate;
        for(index_t f: M.facets) {
            for(index_t i1:  M.facets.corners(f)) {
                index_t i2 = M.facets.next_corner_around_facet(f,i1);
                index_t f1 = M.facet_corners.adjacent_facet(i1);
                index_t f2 = M.facet_corners.adjacent_facet(i2);
                if(f1 != NO_FACET && f1 == f2) {
                    to_dissociate.insert(f);
                    to_dissociate.insert(f1);
                }
            }
        }
        if(!to_dissociate.empty()) {
            GEO::Logger::warn("Mesh")
                << to_dissociate.size()
                << " facets with degree 2 vertices (fixed)"
                << std::endl;
        }
        for(auto f : to_dissociate) {
            for(index_t c: M.facets.corners(f)) {
                M.facet_corners.set_adjacent_facet(c,NO_FACET);
            }
        }
    }

    /************************************************************************/
}

