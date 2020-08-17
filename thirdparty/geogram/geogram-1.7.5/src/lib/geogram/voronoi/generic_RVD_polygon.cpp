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

#include <geogram/voronoi/generic_RVD_polygon.h>
#include <geogram/numerics/predicates.h>
#include <algorithm>

namespace GEOGen {

    void Polygon::initialize_from_mesh_facet(
        const Mesh* mesh, index_t facet, bool symbolic,
        const GEO::Attribute<double>& vertex_weight
    ) {
        clear();
        if(symbolic) {
            // Copy facet
            for(index_t c = mesh->facets.corners_begin(facet);
                c < mesh->facets.corners_end(facet); c++
            ) {
                index_t v = mesh->facet_corners.vertex(c);
                index_t adjacent_facet = mesh->facet_corners.adjacent_facet(c);
                Vertex* vx = add_vertex(
                    Vertex(
                        mesh->vertices.point_ptr(v),
                        vertex_weight.is_bound() ? vertex_weight[v] : 1.0,
                        signed_index_t(adjacent_facet)
                    )
                );
                vx->sym().set_boundary_vertex(v);
            }

            // Initialize symbolic information
            for(index_t i1 = 0; i1 < nb_vertices(); i1++) {
                index_t i2 = next_vertex(i1);
                Vertex& v1 = vertex(i1);
                Vertex& v2 = vertex(i2);

                // Note: Here we compute v2.sym()
                // and we do not touch v1.sym()

                v2.sym().add_boundary_facet(facet);
                if(v1.adjacent_facet() >= 0) {
                    v2.sym().add_boundary_facet(
                        index_t(v1.adjacent_facet())
                    );
                } else {
                    // "Virtual" boundary facet:
                    //    indicates edge (i1,i2 = i1 \oplus 1)
                    // We no longer need them, except for
                    // indicating vertex sym. type.
                    v2.sym().add_boundary_facet(
                        mesh->facets.nb() + i1
                    );
                }

                // Note: we continue to compute v2.sym()
                // and we do not touch v1.sym() (it looks
                // like a copy-paste bug but it is correct !)

                if(v2.adjacent_facet() >= 0) {
                    v2.sym().add_boundary_facet(
                        index_t(v2.adjacent_facet())
                    );
                } else {
                    // "Virtual" boundary facet:
                    //    indicates edge (i1,i2 = i1 \oplus 1)
                    // We no longer need them, except for
                    //   indicating vertex sym. type.
                    v2.sym().add_boundary_facet(
                        mesh->facets.nb() + i2
                    );
                }
            }
#ifdef GEO_DEBUG
            // Sanity check: make sure that the facet is not
            // adjacent to the same facet twice.
            index_t n = mesh->facets.nb_vertices(facet);
            signed_index_t* adj = (signed_index_t*) alloca(
                sizeof(signed_index_t) * n
            );
            GEO::Memory::clear(adj, sizeof(signed_index_t) * n);
            index_t i = 0;
            for(index_t c = mesh->facets.corners_begin(facet);
                c < mesh->facets.corners_end(facet); ++c
            ) {
                adj[i] = signed_index_t(mesh->facet_corners.adjacent_facet(c));
                ++i;
            }
            std::sort(adj, adj + n);
            for(i = 0; i < n - 1; ++i) {
                // If this assertion fails, then the mesh probably has a degree2 vertex
                // (use remove_degree2_vertices() in mesh_preprocessing.h)
                geo_debug_assert(
                    adj[i] == -1 || adj[i] != adj[i + 1]
                );
            }
#endif
        } else {
            // We are not in symbolic mode,
            // we just gather the vertices, weights and adjacencies.
            for(index_t c = mesh->facets.corners_begin(facet);
                c < mesh->facets.corners_end(facet); c++
            ) {
                index_t v = mesh->facet_corners.vertex(c);
                index_t adjacent_facet = mesh->facet_corners.adjacent_facet(c);
                add_vertex(
                    Vertex(
                        mesh->vertices.point_ptr(v),
                        vertex_weight.is_bound() ? vertex_weight[v] : 1.0,
                        signed_index_t(adjacent_facet)
                    )
                );
            }
        }
    }

    Sign Polygon::side_exact(
        const Mesh* mesh, const Delaunay* delaunay,
        const Vertex& q, const double* pi, const double* pj, coord_index_t dim
    ) {

        switch(q.sym().nb_boundary_facets()) {
            case 0:
                // All the points that we manipulate are supposed to
                // belong to the restricted Voronoi diagram, therefore
                // they belong to the surface, and are at least on one
                // facet of the surface.
                geo_assert_not_reached;

            case 1:
            {
                // The point q is the intersection between
                //   a facet (f0,f1,f2) of the surface and two
                //   bisectors [pi b0] and [pi b1].
                index_t b0 = q.sym().bisector(0);
                index_t b1 = q.sym().bisector(1);
                index_t f = q.sym().boundary_facet(0);

                index_t if0 = mesh->facets.vertex(f,0);
                index_t if1 = mesh->facets.vertex(f,1);
                index_t if2 = mesh->facets.vertex(f,2);                
                const double* f0 = mesh->vertices.point_ptr(if0);
                const double* f1 = mesh->vertices.point_ptr(if1);
                const double* f2 = mesh->vertices.point_ptr(if2);
                return GEO::PCK::side3_SOS(
                    pi, delaunay->vertex_ptr(b0), delaunay->vertex_ptr(b1), pj,
                    f0, f1, f2, dim
                );
            } 

            case 2:
            {

                // The point q is the intersection between
                //   two facets of the surface (i.e. an edge [e0 e1])
                //   and one bisector [pi b0].
                // i.e. it's a vertex of the surface.
                index_t b0 = q.sym().bisector(0);
                index_t e0, e1;
                q.sym().get_boundary_edge(e0, e1);
                return GEO::PCK::side2_SOS(
                    pi, delaunay->vertex_ptr(b0), pj,
                    mesh->vertices.point_ptr(e0),
                    mesh->vertices.point_ptr(e1), dim
                );
            } 

            case 3:
            {
                // The point q is the intersection between
                //   three facets of the surface
                //   (i.e. a vertex v0 of the surface).
                index_t v0 = q.sym().get_boundary_vertex();
                return GEO::PCK::side1_SOS(
                    pi, pj, mesh->vertices.point_ptr(v0), dim
                );
            } 
        }
        geo_assert_not_reached;
    }
}

