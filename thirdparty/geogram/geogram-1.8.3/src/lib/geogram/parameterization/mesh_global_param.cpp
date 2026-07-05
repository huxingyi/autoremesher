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

#include <geogram/parameterization/mesh_global_param.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_frame_field.h>

#include <deque>
#include <stack>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/combinable.h>


namespace {
    using namespace GEO;

    /**
     * \brief Computes the angle between two vectors associated to two adjacent
     *  facets.
     * \param[in] mesh a pointer to a surface mesh
     * \param[in] f1 the first facet
     * \param[in] B1 the first 3d vector, in the plane of f1
     * \param[in] f2 the second facet
     * \param[in] B2 the second 3d vector, in the plane of f2
     * \return the angle in degrees between -180 and 180 required to 
     *  transform vector \p B2 to vector \p B1 expressed in the local frame 
     *  of facet f1 with the common edge of the two facets f1 and f2 as 
     *  the X axis
     */
    double angle(
	Mesh* mesh, index_t f1, const vec3& B1, index_t f2, const vec3& B2
    ) {
	index_t lf2 = mesh->facets.find_adjacent(f1,f2);
	geo_assert(lf2 != NO_FACET);
	index_t c1 = mesh->facets.corners_begin(f1) + lf2;
	index_t c2 = mesh->facets.next_corner_around_facet(f1,c1);
	index_t v1 = mesh->facet_corners.vertex(c1);
	index_t v2 = mesh->facet_corners.vertex(c2);
	if(v2 < v1) {
	    std::swap(v1,v2);
	}
	const vec3& p1 = Geom::mesh_vertex(*mesh, v1);
	const vec3& p2 = Geom::mesh_vertex(*mesh, v2);
	vec3 E = normalize(p2-p1);
	vec3 N1 = normalize(Geom::mesh_facet_normal(*mesh,f1));
	vec3 N2 = normalize(Geom::mesh_facet_normal(*mesh,f2));
	vec3 Y1 = cross(N1,E);
	vec3 Y2 = cross(N2,E);
	double x1 = dot(B1,E);
	double y1 = dot(B1,Y1);
	double x2 = dot(B2,E);
	double y2 = dot(B2,Y2);
	double a1 = atan2(y1,x1) * 180.0 / M_PI;
	double a2 = atan2(y2,x2) * 180.0 / M_PI;
	double result = a1-a2;
	
	while(result < -180.0) {
	    result += 360.0;
	}
	while(result > 180.0) {
	    result -= 360.0;
	}
	
	return result;
    }

    
    /**
     * \brief Gets the number of 90 degrees rotations required to minimize the
     *  angle between the vectors attached to two adjacent facets.
     * \param[in] mesh a pointer to a surface mesh
     * \param[in] f1 the first facet
     * \param[in] B1 the first 3D vector in the plane of f1
     * \param[in] f2 the second facet
     * \param[in] B2 the second 3D vector in the plane of f2
     * \return the number of times \p B2 should be rotated around the 
     *  normal vector of \p f2 to minimize its angle with \p B1 in angus 
     *  (in 0,1,2,3).
     */
    index_t Rij(
	Mesh* mesh, index_t f1, const vec3& B1, index_t f2, const vec3& B2
    ) {
	if(f1 > f2) {
	    index_t result = Rij(mesh, f2, B2, f1, B1);
	    return GlobalParam2d::Internal::inverse_R(result);
	}
	vec3 N2 = normalize(Geom::mesh_facet_normal(*mesh,f2));	
	vec3 cur_B2 = B2;
	double best_angle = ::fabs(angle(mesh, f1, B1, f2, cur_B2));
	index_t best_i = 0;
	for(index_t i=1; i<4; ++i) {
	    cur_B2 = cross(N2, cur_B2);	    
	    double cur_angle = ::fabs(angle(mesh, f1, B1, f2, cur_B2));

	    if(cur_angle < best_angle) {
		best_angle = cur_angle;
		best_i = i;
	    }
	}
	return best_i;
    }

    /**
     * \brief Sets an attribute on both corners adjacent to the same edge.
     * \param[in] mesh a pointer to a surface mesh.
     * \param[out] attr a facet corner attribute
     * \param[in] f1 , f2 the two facets that share the edge
     * \param[in] val the new value of the attribute 
     */
    void set_edge_attr(
	Mesh* mesh, Attribute<index_t>& attr,
	index_t f1, index_t f2, index_t val
    ) {
	index_t e1 = mesh->facets.find_adjacent(f1,f2);
	index_t e2 = mesh->facets.find_adjacent(f2,f1);
	attr[mesh->facets.corners_begin(f1)+e1] = val;
	attr[mesh->facets.corners_begin(f2)+e2] = val;
    }

}

namespace GEO {
    namespace GlobalParam2d {
	namespace Internal {

	    void compute_R_ff(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff
	    ) {
		tbb::parallel_for(tbb::blocked_range<index_t>(0, mesh->facets.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t f1 = range.begin(); f1 != range.end(); ++f1) {
			    FOR(e1, mesh->facets.nb_vertices(f1)) {
				index_t f2 = mesh->facets.adjacent(f1,e1);
				index_t c = mesh->facets.corners_begin(f1) + e1;
				if(f2 != NO_FACET) {
				    index_t rij = Rij(mesh, f1, B[f1], f2, B[f2]);
				    index_t rji = Rij(mesh, f2, B[f2], f1, B[f1]);
				    geo_assert(rij == inverse_R(rji));
				    R_ff[c] = rij;
				}
			    }
			}
		    });
	    }


	    void compute_R_fv(
		Mesh* mesh, 
		Attribute<index_t>& R_ff, Attribute<index_t>& R_fv
	    ) {
		// - Each vertex has a reference corner (v2c[v])
		// - Each corner c knows the number of rotations Rc[c] required
		//    to make the B of its triangle match the B of the triangle
		//    of the reference corner attached to its vertex (clear
		//    enough ?)

		// Step 1: Compute v2c
		//   Note: if there exists a corner that has his previous corner
		//   around the facet that is on the surface border for a
		//   given v, then use this one.
		//   Later, when we turn around the vertices,
		//   it will be easier to start from such an halfedge for all
		//   vertices that are on the border.
		
		vector<index_t> v2c(mesh->vertices.nb(), NO_CORNER);
		{
		    for(index_t c: mesh->facet_corners) {
			index_t f = c/3;
			index_t c_prev =
			    mesh->facets.prev_corner_around_facet(f,c);
			if(mesh->facet_corners.adjacent_facet(c_prev) ==
			   NO_FACET
			) {
			    index_t v = mesh->facet_corners.vertex(c);
			    v2c[v] = c;
			}
		    }
		    for(index_t c: mesh->facet_corners) {
			index_t v = mesh->facet_corners.vertex(c);
			if(v2c[v] == NO_CORNER) {
			    v2c[v] = c;
			}
		    }
		}

		// Step 2: Compute Rv by turning around the facets that
		//  share a vertex.
		//   Yes, it is painful, I hate doing that, but R only works
		//   for pairs of *adjacent* facets (and I cannot think about
		//   a way of making work it *reliably* for any pair of facets).
		{
		    for(index_t v: mesh->vertices) {
			index_t prev_c = NO_CORNER;
			index_t c = v2c[v];
			
			// Isolated vertex, ignore
			if(c == NO_CORNER) {
			    continue;
			}
		    
			do {
			    index_t
				next_f = mesh->facet_corners.adjacent_facet(c);
			    index_t next_c = NO_CORNER;
			    if(next_f != NO_FACET) {
				for(index_t c2: mesh->facets.corners(next_f)) {
				    if(mesh->facet_corners.vertex(c2) == v) {
					next_c = c2;
				    break;
				    }
				}
				geo_assert(next_c != NO_CORNER);
			    }
			    if(prev_c != NO_CORNER) {
				R_fv[c] = (R_fv[prev_c] + R_ff[prev_c]) % 4; 
			}
			prev_c = c;
			c = next_c;
		    } while(c != NO_CORNER && c != v2c[v]);
		    }
		}
	    }
	    
	    void mark_singular_vertices(
		Mesh* mesh,
		Attribute<index_t>& R_ff, Attribute<bool>& v_is_singular
	    ) {
		vector<index_t> Rsum(mesh->vertices.nb(), 0);

		// Accumulate R_ff sums per vertex using per-thread accumulators
		// to avoid write contention on shared vertices.
		tbb::combinable<vector<index_t>> perThreadRsum(
		    [mesh]() { return vector<index_t>(mesh->vertices.nb(), 0); });

		tbb::parallel_for(tbb::blocked_range<index_t>(0, mesh->facets.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			auto &local = perThreadRsum.local();
			for(index_t f = range.begin(); f != range.end(); ++f) {
			    for(index_t c: mesh->facets.corners(f)) {
				if(
				    mesh->facet_corners.adjacent_facet(c) !=
				    index_t(-1)
				) {
				    index_t v = mesh->facet_corners.vertex(c);
				    local[v] += R_ff[c];
				}
			    }
			}
		    });

		perThreadRsum.combine_each([&](const vector<index_t> &local) {
		    for(index_t v = 0; v < mesh->vertices.nb(); ++v)
			Rsum[v] += local[v];
		});

		tbb::parallel_for(tbb::blocked_range<index_t>(0, mesh->vertices.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t v = range.begin(); v != range.end(); ++v)
			    v_is_singular[v] = ((Rsum[v] % 4) != 0);
		    });

		// Vertices on border can have non-zero Rsum without being
		// singular.
		tbb::parallel_for(tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c) {
			    if(mesh->facet_corners.adjacent_facet(c) == NO_FACET) {
				v_is_singular[mesh->facet_corners.vertex(c)] = false;
			    }
			}
		    });
	    }

	    void brush(Mesh* mesh, Attribute<vec3>& B) {
		std::vector<bool> visited(mesh->facets.nb(),false);
		std::deque<index_t> S;
		S.push_back(0);
		visited[0] = true;
		while(!S.empty()) {
		    index_t f1 = S.front();
		    S.pop_front();
		    FOR(e1, mesh->facets.nb_vertices(f1)) {
			index_t f2 = mesh->facets.adjacent(f1,e1);
			if(f2 != NO_FACET && !visited[f2]) {
			    vec3 N2 = normalize(
				Geom::mesh_facet_normal(*mesh,f2)
			    );
			    index_t Rc1 = Rij(mesh,f1,B[f1],f2,B[f2]);
			    FOR(i, Rc1) {
				B[f2] = cross(N2,B[f2]);
			    }
			    visited[f2] = true;
			    S.push_back(f2);
			}
		    }
		}
	    }


	    void do_the_ball(
		Mesh* mesh,
		Attribute<index_t>& R_ff, Attribute<index_t>& c_on_border
	    ) {
		// Initialize all corners as border (parallel)
		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c)
			    c_on_border[c] = 1;
		    });
		
		// Covering tree (BFS - inherently sequential)
		
		std::vector<bool> visited(mesh->facets.nb(),false);
		std::deque<index_t> S;
		S.push_back(0);
		visited[0] = true;
		while(!S.empty()) {
		    index_t f1 = S.front();
		    S.pop_front();
		    for(index_t c1: mesh->facets.corners(f1)) {
			index_t f2 = mesh->facet_corners.adjacent_facet(c1);
			if(f2 != NO_FACET && !visited[f2] && R_ff[c1] == 0) {
			    set_edge_attr(mesh, c_on_border, f1, f2, 0);
			    visited[f2] = true;
			    S.push_back(f2);
			}
		    }
		}
		
		// Zipping: count border edges per vertex (parallel accumulation)

		vector<index_t> v_nb_borders(mesh->vertices.nb(), 0);
		{
		    tbb::combinable<vector<index_t>> perThreadBorders(
			[mesh]() { return vector<index_t>(mesh->vertices.nb(), 0); });
		    tbb::parallel_for(
			tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
			[&](const tbb::blocked_range<index_t> &range) {
			    auto &local = perThreadBorders.local();
			    for(index_t c = range.begin(); c != range.end(); ++c) {
				if(c_on_border[c]) {
				    ++local[mesh->facet_corners.vertex(c)];
				}
			    }
			});
		    perThreadBorders.combine_each([&](const vector<index_t> &local) {
			for(index_t v = 0; v < mesh->vertices.nb(); ++v)
			    v_nb_borders[v] += local[v];
		    });
		}

		// Zipping: sequential (iterative degree-1 vertex removal)
		bool there_are_degree1_vertices = true;
		while(there_are_degree1_vertices) {
		    there_are_degree1_vertices = false;
		    for(index_t c1: mesh->facet_corners) {
			index_t v1 = mesh->facet_corners.vertex(c1);
			if(v_nb_borders[v1] == 1 &&
			   (c_on_border[c1] != 0) && R_ff[c1] == 0
			) {
			    index_t f2 = mesh->facet_corners.adjacent_facet(c1);
			    if(f2 != NO_FACET) {
				there_are_degree1_vertices = true;
				index_t f1 = c1/3;
				index_t c2 = mesh->facets.corners_begin(f2) +
				    mesh->facets.find_adjacent(f2,f1);
				index_t v2 = mesh->facet_corners.vertex(c2);
				c_on_border[c1] = 0;
				c_on_border[c2] = 0;
				--v_nb_borders[v1];
				--v_nb_borders[v2];
			    }
			}
		    }
		}
	    }

    void do_the_ball_no_brush_no_zip(
		Mesh* mesh, Attribute<index_t>& c_on_border
	    ) {
		// Initialize all corners as border (parallel)
		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c)
			    c_on_border[c] = 1;
		    });

		// Covering tree (BFS - inherently sequential)
		
		std::vector<bool> visited(mesh->facets.nb(),false);
		std::deque<index_t> S;
		S.push_back(0);
		visited[0] = true;
		while(!S.empty()) {
		    index_t f1 = S.front();
		    S.pop_front();
		    for(index_t c1: mesh->facets.corners(f1)) {
			index_t f2 = mesh->facet_corners.adjacent_facet(c1);
			if(f2 != NO_FACET && !visited[f2]) {
			    set_edge_attr(mesh, c_on_border, f1, f2, 0);
			    visited[f2] = true;
			    S.push_back(f2);
			}
		    }
		}
	    }
	    
	    void get_B_on_edge(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff,
		index_t f, index_t c,
		vec3& Bc, vec3& BTc
	    ) {
		vec3 Nf = normalize(Geom::mesh_facet_normal(*mesh,f));
		Bc = B[f];
		BTc = cross(Nf,Bc);
		index_t f2 = mesh->facet_corners.adjacent_facet(c);
		if(f2 != NO_FACET) {
		    vec3 N2 = normalize(Geom::mesh_facet_normal(*mesh,f2));
		    vec3 B2 = B[f2];
		    vec3 BT2 = cross(N2,B2);
		    FOR(i,R_ff[c]) {
			B2 = cross(N2,B2);
			BT2 = cross(N2,BT2);
		    }
		    Bc += B2;
		    BTc += BT2;
		}
		Bc = normalize(Bc);
		BTc = normalize(BTc);
	    }
	    
	    void get_constraints(
		Mesh* mesh, Attribute<vec3>& B, Attribute<index_t>& R_ff,
		Attribute<index_t>& constraint,
		double hard_angle_threshold
	    ) {

		geo_argused(R_ff);

		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c)
			    constraint[c] = GlobalParam2d::Internal::CNSTR_NONE;
		    });

		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c) {
			    index_t edge_constraints =
				GlobalParam2d::Internal::get_edge_constraints(mesh,c,B,hard_angle_threshold);
			    index_t f = c/3;
			    index_t c2 =
				mesh->facets.next_corner_around_facet(f,c);
			    constraint[c] |= edge_constraints;
			    constraint[c2] |= edge_constraints;
			}
		    });
	    }
	    void snap_tex_coord(double& coord) {
		double snapped = GEO::round(coord);
		if(std::fabs(coord - snapped) < 0.05) {
		    coord = snapped;
		}
	    }
	    
	    index_t inverse_R(index_t R) {
		geo_assert(R < 4);
		static index_t inverse[4] = {
		    0,
		    3,
		    2,
		    1
		};
		return inverse[R];
	    }


	    void transfer_B_to_vertices(
		Mesh* mesh,
		Attribute<vec3>& B, Attribute<vec3>& Bv,
		Attribute<index_t>& R_fv
	    ) {
		for(index_t v: mesh->vertices) {
		    Bv[v] = vec3(0.0, 0.0, 0.0);
		}
		for(index_t c: mesh->facet_corners) {
		    index_t v = mesh->facet_corners.vertex(c);	    
		    index_t f = c/3;
		    vec3 Bf = normalize(B[f]);
		    vec3 N = normalize(Geom::mesh_facet_normal(*mesh, f));
		    FOR(k, R_fv[c]) {
			Bf = cross(N,Bf);
		    }
		    Bv[v] += Bf;
		}
		for(index_t v: mesh->vertices) {
		    Bv[v] = normalize(Bv[v]);
		}
	    }
	    index_t get_edge_constraints(
		Mesh* mesh, index_t c, Attribute<vec3>& B,
			double hard_angle_threshold
	    ) {
		index_t result = 0;
		
		index_t f = c/3;	
		vec3 N = normalize(Geom::mesh_facet_normal(*mesh,f));
		
		index_t f2 = mesh->facet_corners.adjacent_facet(c);
		if(f2 != NO_FACET) {
		    if(
			::fabs(Geom::mesh_normal_angle(*mesh,c)) * 180.0 / M_PI
			< hard_angle_threshold
		    ) {
			return 0;
		    }
		}
		
		index_t v1 = mesh->facet_corners.vertex(c);
		index_t c2 = mesh->facets.next_corner_around_facet(c/3,c);
		index_t v2 = mesh->facet_corners.vertex(c2);
		vec3 E =
		    vec3(mesh->vertices.point_ptr(v2)) -
		    vec3(mesh->vertices.point_ptr(v1));
		vec3 Bf = normalize(B[f]);
		vec3 Bfrot = cross(N,Bf);
		
		double a1 = (Geom::angle(E,Bf)) * 180.0 / M_PI;
		a1 = std::min(a1, 180.0-a1);
		if(a1 < 10.0) {
		    result |= GlobalParam2d::Internal::CNSTR_V;
		}
		
		double a2 = (Geom::angle(E,Bfrot)) * 180.0 / M_PI;
		a2 = std::min(a2, 180.0-a2);	
		if(a2 < 10.0) {
		    result |= GlobalParam2d::Internal::CNSTR_U;
		}
		
		// geo_assert!(a1 < 10.0 && a2 < 10.0));
		// Should not occur...
		if(a1 < 10.0 && a2 < 10.0) {
		    result = 0;
		}
		
		return result;
	    }

	} // namespace Internal
	
	void frame_field(
	    Mesh* mesh, Attribute<vec3>& B,
	    double hard_angle_threshold
	) {
	    FrameField FF;
	    // We will directly query the field on the facets, no need
	    // for the KD-tree.
	    FF.set_use_spatial_search(false);
	    FF.create_from_surface_mesh(*mesh,false,hard_angle_threshold);
	    const vector<double>& frames = FF.frames();
	    for(index_t f: mesh->facets) {
		B[f] = vec3(
		    frames[9*f+0],
		    frames[9*f+1],
		    frames[9*f+2]		
		);
	    }
	}



    } // namespace GlobalParam2d
} // namespace OGF
