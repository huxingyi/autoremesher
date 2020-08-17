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

#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/numerics/predicates.h>

namespace GEOGen {

    index_t ConvexCell::plus1mod3_[3] = {1, 2, 0};
    index_t ConvexCell::minus1mod3_[3] = {2, 0, 1};

    std::ostream& ConvexCell::show_stats(std::ostream& os) const {
        unsigned int nb_free = 0;
        unsigned int nb_conflict = 0;
        unsigned int nb_used = 0;
        for(unsigned int t = 0; t < max_t(); t++) {
            switch(triangles_[t].status_) {
                case TRI_IS_FREE:
                    nb_free++;
                    break;
                case TRI_IS_USED:
                    nb_used++;
                    break;
                case TRI_IS_CONFLICT:
                    nb_conflict++;
                    break;
            }
        }
        return os << "Nb tot = " << max_t()
            << " free=" << nb_free
            << " used=" << nb_used
            << " conflict=" << nb_conflict
            << std::endl;
    }

    Sign ConvexCell::side_exact(
        const Mesh* mesh, const Delaunay* delaunay,
        const GEOGen::Vertex& q,
        const double* pi, const double* pj,
        coord_index_t dim,
        bool symbolic_is_surface
    ) const {

        switch(q.sym().nb_boundary_facets()) {
            case 0:
            {
                // The point q is the intersection between
                //   three bisectors [pi b0], [pi b1] and [pi b2]
                // (and a tet [q0 q1 q2 q3])

                index_t b0 = q.sym().bisector(0);
                index_t b1 = q.sym().bisector(1);
                index_t b2 = q.sym().bisector(2);

                if(dim == 3) {
                    // 3d is a special case for side4()
                    //   (intrinsic dim == ambient dim)
                    // therefore embedding tet q0,q1,q2,q3 is not needed.
                    return GEO::PCK::side4_3d_SOS(
                        pi,
                        delaunay->vertex_ptr(b0),
                        delaunay->vertex_ptr(b1),
                        delaunay->vertex_ptr(b2),
                        pj
                    );
                } else {
                    geo_debug_assert(cell_id() >= 0);
                    index_t t = index_t(cell_id());
                    return GEO::PCK::side4_SOS(
                        pi,
                        delaunay->vertex_ptr(b0),
                        delaunay->vertex_ptr(b1),
                        delaunay->vertex_ptr(b2),
                        pj,
                        mesh->vertices.point_ptr(mesh->cells.tet_vertex(t, 0)),
                        mesh->vertices.point_ptr(mesh->cells.tet_vertex(t, 1)),
                        mesh->vertices.point_ptr(mesh->cells.tet_vertex(t, 2)),
                        mesh->vertices.point_ptr(mesh->cells.tet_vertex(t, 3)),
                        dim
                    );
                }
            } 

            case 1:
            {
                // The point q is the intersection between
                //   a facet (f0,f1,f2) of the surface and two
                //   bisectors [pi b0] and [pi b1].

                index_t b0 = q.sym().bisector(0);
                index_t b1 = q.sym().bisector(1);
                index_t f = q.sym().boundary_facet(0);

                if(symbolic_is_surface) {
                    index_t c = mesh->facets.corners_begin(f);
                    const double* q0 = mesh->vertices.point_ptr(
                        mesh->facet_corners.vertex(c)
                    );
                    const double* q1 = mesh->vertices.point_ptr(
                        mesh->facet_corners.vertex(c+1)
                    );
                    const double* q2 = mesh->vertices.point_ptr(
                        mesh->facet_corners.vertex(c+2)
                    );
                                                                                
                    return GEO::PCK::side3_SOS(
                        pi,
                        delaunay->vertex_ptr(b0),
                        delaunay->vertex_ptr(b1),
                        pj,
                        q0, q1, q2, dim
                    );
                    
                } else {
                    index_t t = f / 4;
                    index_t lf = f % 4;
                    index_t j0 = mesh->cells.tet_vertex(
                        t, GEO::MeshCells::local_tet_facet_vertex_index(lf, 0)
                    );
                    index_t j1 = mesh->cells.tet_vertex(
                        t, GEO::MeshCells::local_tet_facet_vertex_index(lf, 1)
                    );
                    index_t j2 = mesh->cells.tet_vertex(
                        t, GEO::MeshCells::local_tet_facet_vertex_index(lf, 2)
                    );
                    
                    return GEO::PCK::side3_SOS(
                        pi,
                        delaunay->vertex_ptr(b0),
                        delaunay->vertex_ptr(b1),
                        pj,
                        mesh->vertices.point_ptr(j0),
                        mesh->vertices.point_ptr(j1),
                        mesh->vertices.point_ptr(j2),
                        dim
                    );
                }
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
                    mesh->vertices.point_ptr(e1),
                    dim
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

    void ConvexCell::initialize_from_mesh_tetrahedron(
        const Mesh* mesh, index_t t, bool symbolic,
        const GEO::Attribute<double>& vertex_weight
    ) {
        clear();

        index_t v0 = mesh->cells.tet_vertex(t, 0);
        index_t v1 = mesh->cells.tet_vertex(t, 1);
        index_t v2 = mesh->cells.tet_vertex(t, 2);
        index_t v3 = mesh->cells.tet_vertex(t, 3);

        signed_index_t t0 = signed_index_t(mesh->cells.tet_adjacent(t, 0));
        signed_index_t t1 = signed_index_t(mesh->cells.tet_adjacent(t, 1));
        signed_index_t t2 = signed_index_t(mesh->cells.tet_adjacent(t, 2));
        signed_index_t t3 = signed_index_t(mesh->cells.tet_adjacent(t, 3));

        create_vertex();
        create_vertex();
        create_vertex();
        create_vertex();

        set_cell_id(signed_index_t(t));

        set_vertex_id(0, (t0 == signed_index_t(GEO::NO_CELL)) ? 0 : -t0 - 1);
        set_vertex_id(1, (t1 == signed_index_t(GEO::NO_CELL)) ? 0 : -t1 - 1);
        set_vertex_id(2, (t2 == signed_index_t(GEO::NO_CELL)) ? 0 : -t2 - 1);
        set_vertex_id(3, (t3 == signed_index_t(GEO::NO_CELL)) ? 0 : -t3 - 1);

        double w0 = 1.0;
        double w1 = 1.0;
        double w2 = 1.0;
        double w3 = 1.0;
        
        if(vertex_weight.is_bound()) {
            w0 = vertex_weight[v0];
            w1 = vertex_weight[v1];
            w2 = vertex_weight[v2];
            w3 = vertex_weight[v3];            
        }
        
        create_triangle(mesh->vertices.point_ptr(v0), w0, 2, 1, 3, 2, 1, 3);
        create_triangle(mesh->vertices.point_ptr(v1), w1, 3, 0, 2, 3, 0, 2);
        create_triangle(mesh->vertices.point_ptr(v2), w2, 0, 3, 1, 0, 3, 1);
        create_triangle(mesh->vertices.point_ptr(v3), w3, 2, 0, 1, 2, 0, 1);

        if(symbolic) {

            index_t f0 = global_facet_id(mesh, t, 0);
            index_t f1 = global_facet_id(mesh, t, 1);
            index_t f2 = global_facet_id(mesh, t, 2);
            index_t f3 = global_facet_id(mesh, t, 3);

            triangle_dual(0).sym().set_boundary_vertex(v0);
            triangle_dual(0).sym().add_boundary_facet(f1);
            triangle_dual(0).sym().add_boundary_facet(f2);
            triangle_dual(0).sym().add_boundary_facet(f3);

            triangle_dual(1).sym().set_boundary_vertex(v1);
            triangle_dual(1).sym().add_boundary_facet(f2);
            triangle_dual(1).sym().add_boundary_facet(f3);
            triangle_dual(1).sym().add_boundary_facet(f0);

            triangle_dual(2).sym().set_boundary_vertex(v2);
            triangle_dual(2).sym().add_boundary_facet(f3);
            triangle_dual(2).sym().add_boundary_facet(f0);
            triangle_dual(2).sym().add_boundary_facet(f1);

            triangle_dual(3).sym().set_boundary_vertex(v3);
            triangle_dual(3).sym().add_boundary_facet(f0);
            triangle_dual(3).sym().add_boundary_facet(f1);
            triangle_dual(3).sym().add_boundary_facet(f2);
        }
    }

    void ConvexCell::initialize_from_surface_mesh(
        Mesh* mesh, bool symbolic
    ) {
        clear();
        
        for(index_t f = 0; f < mesh->facets.nb(); ++f) {
            index_t v = create_vertex();
            set_vertex_id(v,-1-signed_index_t(f));
        }
        GEO::vector<GEO::MeshHalfedges::Halfedge> v2h(mesh->vertices.nb());

        GEO::MeshHalfedges MH(*mesh);
        for(index_t f = 0; f < mesh->facets.nb(); ++f) {
            for(index_t c = mesh->facets.corners_begin(f);
                c < mesh->facets.corners_end(f); ++c
            ) {
                index_t v = mesh->facet_corners.vertex(c);
                v2h[v] = GEO::MeshHalfedges::Halfedge(f, c);
            }
        }

        for(index_t v = 0; v < mesh->vertices.nb(); ++v) {
            index_t fi[3];
            index_t va[3];
            index_t cur = 0;
            GEO::MeshHalfedges::Halfedge H = v2h[v];
            do {
                //   All the vertices of the input mesh should be
                // incident to three facets exactly (this is because
                // the ConvexCell is represented in dual form).
                geo_assert(cur < 3);
                fi[cur] = H.facet;
                index_t ca = mesh->facets.next_corner_around_facet(
                    H.facet, H.corner
                );
                va[cur] = mesh->facet_corners.vertex(ca);
                bool ok = MH.move_to_prev_around_vertex(H);
                geo_assert(ok);
                ++cur;
            } while(H != v2h[v]);
            
            // Note: va[] order is different, because of
            //   Mesh numbering -> Triangulation numbering
            // conversion !
            create_triangle(
                mesh->vertices.point_ptr(v), 1.0,
                fi[0], fi[1], fi[2], va[2], va[0], va[1]
            );
            if(symbolic) {
                triangle_dual(v).sym().add_boundary_facet(fi[0]);
                triangle_dual(v).sym().add_boundary_facet(fi[1]);
                triangle_dual(v).sym().add_boundary_facet(fi[2]);
                triangle_dual(v).sym().set_boundary_vertex(v);
            }
        }
        if(symbolic) {
            set_symbolic_is_surface(true);
        }
    }


    void ConvexCell::convert_to_mesh(Mesh* mesh, bool copy_symbolic_info) {
        GEO::vector<index_t> tri_to_v(max_t());
        mesh->clear();
        mesh->vertices.set_dimension(3);

        index_t cur_v = 0;
        for(index_t t = 0; t < max_t(); ++t) {
            if(triangle_is_valid(t)) {
                mesh->vertices.create_vertex(triangle_dual(t).point());
                tri_to_v[t] = cur_v;
                ++cur_v;
            }
        }
        GEO::Attribute<signed_index_t> facet_id;
        if(copy_symbolic_info) {
            facet_id.bind(mesh->facets.attributes(), "id");
        }
        GEO::vector<index_t> facet_vertices;
        for(index_t v = 0; v < max_v(); v++) {
            facet_vertices.resize(0);
            signed_index_t t = vertex_triangle(v);
            if(t != -1) {
                Corner first_c(
                    index_t(t), find_triangle_vertex(index_t(t), v)
                );
                Corner c = first_c;
                do {
                    facet_vertices.push_back(tri_to_v[c.t]);
                    move_to_next_around_vertex(c);
                } while(c != first_c);

                index_t f = mesh->facets.create_polygon(facet_vertices.size());
                for(index_t lv=0; lv<facet_vertices.size(); ++lv) {
                    mesh->facets.set_vertex(f,lv,facet_vertices[lv]);
                }
                if(facet_id.is_bound()) {
                    facet_id[f] = vertex_id(v);
                }
            }
        }
        mesh->facets.connect();
    }

    void ConvexCell::copy(const ConvexCell& rhs) {
	geo_debug_assert(
	    intersections_.dimension() == rhs.intersections_.dimension()
	);
	triangles_ = rhs.triangles_;
	vertices_ = rhs.vertices_;
	first_free_ = rhs.first_free_;
	v_to_t_dirty_ = rhs.v_to_t_dirty_;
	symbolic_is_surface_ = rhs.symbolic_is_surface_;
	cell_id_ = rhs.cell_id_;
    }
    
}

