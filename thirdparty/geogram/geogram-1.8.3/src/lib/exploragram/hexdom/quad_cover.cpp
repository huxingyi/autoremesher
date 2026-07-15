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

#include <exploragram/hexdom/quad_cover.h>
#include <exploragram/hexdom/mixed_constrained_solver.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_frame_field.h>
#include <geogram_report_progress.h>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

namespace GEO {

    namespace GlobalParam2d {
	
	namespace Internal {

	    // Progress reporting helpers for quad_cover stages.
	    // geogram_report_progress_round maps the pipeline into 8 buckets:
	    //   round=0: brush+R_ff, 1: singular, 2: ball,
	    //   round=3: constraints+setup, 4: passes0-1, 5: passes2-3,
	    //   round=6: MIQ, 7: result extraction.
	    #define QC_PROGRESS_BEGIN(rnd) do { \
		geogram_report_progress_round = (rnd); \
		if(geogram_report_progress_callback) \
		    geogram_report_progress_callback( \
			geogram_report_progress_tag, 0.0f); \
	    } while(false)

	    #define QC_PROGRESS_END(rnd) do { \
		geogram_report_progress_round = (rnd); \
		if(geogram_report_progress_callback) \
		    geogram_report_progress_callback( \
			geogram_report_progress_tag, 1.0f); \
	    } while(false)

	    #define QC_PROGRESS(pct) do { \
		if(geogram_report_progress_callback) \
		    geogram_report_progress_callback( \
			geogram_report_progress_tag, (pct)); \
	    } while(false)

	    /**
	     * \brief the four 2x2 rotation matrices associated with the 
	     *  values of R that transform coordinates between two triangles.
	     * \details First index is Rij (in 0..3, number of 90 degrees 
	     *  rotations), then  row and column index of the 2x2 rotation 
	     *  matrix, where Rij is the number of times coordinates axes are 
	     *  rotated by 90 degrees. 
	     * \note The rotation is inversed as compared with the 
	     *  computation in Rij() since when the axes rotate clockwise, 
	     *  the coordinates rotate anticlockwise (and conversely).
	     */
	    static double Rot[4][2][2] = {
		{{1, 0},
		 {0, 1}},
		
		{{ 0, 1},
		 {-1, 0}},
		
		{{-1, 0},
		 { 0,-1}},

		{{0 ,-1},
		 {1 , 0}}
	    };

	    void quad_cover_solve(
		Mesh* mesh,
		Attribute<vec3>& B, Attribute<index_t>& R,
		Attribute<index_t>& on_border,
		Attribute<index_t>& constraints,
		Attribute<vec2>& U,
		Attribute<double>& T,
		Attribute<bool>& v_is_singular,
		double scaling,
		bool constrain_hard_edges,
		bool integer_constraints,
	    double hard_angle_threshold
	    ) {
		scaling *= surface_average_edge_length(*mesh);

		// Optional curvature-adaptive density: when the caller attaches a
		// per-facet "adaptive_scaling" attribute, it multiplies the global
		// scaling locally (values < 1 produce finer quads, > 1 coarser ones).
		Attribute<double> adaptive_scaling;
		adaptive_scaling.bind_if_is_defined(
		    mesh->facets.attributes(), "adaptive_scaling"
		);

		index_t nb_U = mesh->facets.nb()*3*2;
		index_t nb_T = mesh->facets.nb()*3*2;
		MatrixMixedConstrainedSolver solver(nb_U+nb_T);
	
		// All Tijs are even integers (entiers pairs).
		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, nb_T),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t t = range.begin(); t != range.end(); ++t)
			    solver.set_multiplicity(nb_U+t, 2);
		    });

		// Constrained u,v coordinates
		if(constrain_hard_edges) {
		    tbb::parallel_for(
			tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
			[&](const tbb::blocked_range<index_t> &range) {
			    for(index_t c = range.begin(); c != range.end(); ++c) {
				if((constraints[c] & CNSTR_U) != 0) {
				    solver.set_multiplicity(2*c, 1);
				}
				if((constraints[c] & CNSTR_V) != 0) {
				    solver.set_multiplicity(2*c+1, 1);
				}
			    }
			});
		}
		
		QC_PROGRESS(0.3f);

		// Compute vertex-to-corner map (exclude vertices on the border
		// and singular vertices)
		
		vector<index_t> v2c(mesh->vertices.nb(), NO_CORNER);
		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t c = range.begin(); c != range.end(); ++c) {
			    index_t v = mesh->facet_corners.vertex(c);
			    v2c[v] = c;
			}
		    });
		{
		    tbb::parallel_for(
			tbb::blocked_range<index_t>(0, mesh->facet_corners.nb()),
			[&](const tbb::blocked_range<index_t> &range) {
			    for(index_t c = range.begin(); c != range.end(); ++c) {
				index_t v = mesh->facet_corners.vertex(c);
				if(
				    (
					mesh->facet_corners.adjacent_facet(c) ==
					NO_FACET
				    ) || v_is_singular[v]
				) {
				    v2c[v] = NO_CORNER;
				}
			    }
			});
		}
	

		QC_PROGRESS(0.15f);
		FOR(pass, 4) {
		    QC_PROGRESS(0.15f + 0.55f * pass / 4.0f);
		    FOR(c, mesh->facet_corners.nb()) {
			if(mesh->facet_corners.adjacent_facet(c) == NO_FACET) {
			    continue;
			}

			index_t f2 = mesh->facet_corners.adjacent_facet(c);
			index_t e2 = mesh->facets.find_adjacent(f2,c/3);
			index_t c2 = mesh->facets.corners_begin(f2)+e2;
			index_t c3 =
			    mesh->facets.next_corner_around_facet(f2,c2);
			
			geo_assert(
			    mesh->facet_corners.vertex(c) ==
			    mesh->facet_corners.vertex(c3)
			);
			index_t Rij = R[c];

			// Chart transform for each pair of adjacent triangles
			
			// On the border of the ball, Tij + Rij*Tji = 0
			// (the Tij 1-form is ... a 1-form)
			if(on_border[c]) {
			    solver.begin_constraint();
			    solver.add_constraint_coeff(nb_U+2*c,    1.0);
			    solver.add_constraint_coeff(nb_U+2*c2,   Rot[Rij][0][0]);
			    solver.add_constraint_coeff(nb_U+2*c2+1, Rot[Rij][0][1]);
			    solver.end_constraint();
			    solver.begin_constraint();
			    solver.add_constraint_coeff(nb_U+2*c+1,  1.0);
			    solver.add_constraint_coeff(nb_U+2*c2,   Rot[Rij][1][0]);
			    solver.add_constraint_coeff(nb_U+2*c2+1, Rot[Rij][1][1]);
			    solver.end_constraint();
			} else {
			    // Inside the ball, Tij = 0
			    solver.begin_constraint();
			    solver.add_constraint_coeff(nb_U+2*c, 1.0);
			    solver.end_constraint();
			    solver.begin_constraint();
			    solver.add_constraint_coeff(nb_U+2*c+1, 1.0);
			    solver.end_constraint();
			} 

			// Setup relation between Ui - Rij*Uj - Tij = 0
			// (dU = T)
			solver.begin_constraint();
			solver.add_constraint_coeff(2*c   ,  1.0);
			solver.add_constraint_coeff(2*c3  , -Rot[Rij][0][0]);
			solver.add_constraint_coeff(2*c3+1, -Rot[Rij][0][1]);
			solver.add_constraint_coeff(nb_U+2*c,-1.0);
			solver.end_constraint();
			
			solver.begin_constraint();
			solver.add_constraint_coeff(2*c+1 ,  1.0);
			solver.add_constraint_coeff(2*c3  , -Rot[Rij][1][0]);
			solver.add_constraint_coeff(2*c3+1, -Rot[Rij][1][1]);
			solver.add_constraint_coeff(nb_U+2*c+1,-1.0);
			solver.end_constraint();
		    }
		    
            QC_PROGRESS(0.15f + 0.55f * (pass + 0.33f) / 4.0f);
 	            //Wheel compatibility constraints (the Tij 1-form is closed)
		    FOR(v, mesh->vertices.nb()) {
			//   If the corner is on the border or incident
			// to a singular vertex then it is skipped.
			if(v2c[v] == NO_CORNER) {
			    continue;
			}
			//   Walk around the vertex, collecting the corner/rotation
			// pairs. Only apply the constraints if the walk succeeds
			// and the accumulated rotation is zero.
			struct WheelEntry { index_t c; index_t r; };
			vector<WheelEntry> wheel;
			{
			    index_t c = v2c[v];
			    index_t r = 0;
			    bool ok = true;
			    do {
				wheel.push_back({c, r});
				r = (r + R[c]) % 4;
				index_t f =
				    mesh->facet_corners.adjacent_facet(c);
				if(f == NO_FACET) {
				    ok = false;
				    break;
				}
				index_t next_c = NO_CORNER;
				for(
				    next_c = mesh->facets.corners_begin(f);
				    next_c < mesh->facets.corners_end(f);
				    ++next_c
				) {
				    if(mesh->facet_corners.vertex(next_c) == v) {
					break;
				    }
				}
				if(
				    next_c >= mesh->facets.corners_end(f) ||
				    mesh->facet_corners.vertex(next_c) != v
				) {
				    ok = false;
				    break;
				}
				c = next_c;
			    } while(c != v2c[v]);
			    if(!ok || r != 0) {
				v_is_singular[v] = true;
				v2c[v] = NO_CORNER;
				continue;
			    }
			}
			//   Apply the wheel constraints from the collected data.
			FOR(coord, 2) {
			    solver.begin_constraint();
			    for(const auto& entry : wheel) {
				solver.add_constraint_coeff(
				    nb_U+2*entry.c,   Rot[entry.r][coord][0]
				);
				solver.add_constraint_coeff(
				    nb_U+2*entry.c+1, Rot[entry.r][coord][1]
				);
			    }
			    solver.end_constraint();
			}
		    }
		    QC_PROGRESS(0.15f + 0.55f * (pass + 0.67f) / 4.0f);

		    // Constrained edges - equality between coordinates
		    //   Note: sometimes, setting this constraint causes
		    // an assertion failure in Nico's mixed integer solver:
		    // Assertion failed: pass != 3 || cM0M1M2.empty().
		    //   Note2: seems to be OK now that the wheel compat. cnstr.
		    // is there (to be checked).
		    if(constrain_hard_edges) {
			FOR(c, mesh->facet_corners.nb()) {
			    index_t f=c/3;
			    index_t c2 =
				mesh->facets.next_corner_around_facet(f,c);
			    index_t cnstr = get_edge_constraints(mesh,c,B,hard_angle_threshold);
			    if(cnstr != 0) { 
				if(cnstr == CNSTR_U) {
				    solver.begin_constraint();
				    solver.add_constraint_coeff(2*c,   1.0);
				    solver.add_constraint_coeff(2*c2, -1.0);
				    solver.end_constraint();
				} else if(cnstr == CNSTR_V) {
				    solver.begin_constraint();
				    solver.add_constraint_coeff(2*c+1,   1.0);
				    solver.add_constraint_coeff(2*c2+1, -1.0);
				    solver.end_constraint();
				} else {
				    geo_assert_not_reached;
				}
			    }
			}
		    }
		    
		    solver.end_pass(pass);
		    QC_PROGRESS(0.15f + 0.55f * (pass + 1.0f) / 4.0f);
		}
		

		int miq_iter = 0;
		QC_PROGRESS_BEGIN(6);
		QC_PROGRESS(0.70f);
		while (!solver.converged() && miq_iter < 100) {
		    geogram_report_miq_iter = miq_iter + 1;
			QC_PROGRESS(0.70f + 0.25f * (miq_iter++) / 10.0f);
		    plop("MIQ iter");
		    solver.start_new_iter();

		    int qc_miq_facet_count = (int)mesh->facets.nb();
		    int qc_miq_progress_threshold = 0;
		    FOR(f, mesh->facets.nb()) {
			if (qc_miq_facet_count > 0) {
			    int step = (int)(5.0f * f / qc_miq_facet_count);
			    if (step > qc_miq_progress_threshold) {
				qc_miq_progress_threshold = step;
				QC_PROGRESS(0.70f + 0.25f * miq_iter / 10.0f + 0.025f * step);
			    }
			}
			// setup objective function :
			// For each edge (pi,pj):
			//     ( B        * (pj-pi) - (uj-ui))^2 +
			//     ( rot90(B) * (pj-pi) - (vj-vi))^2
			vec3 N = normalize(Geom::mesh_facet_normal(*mesh,f));
			vec3 Bf  = normalize(B[f]);
			vec3 BTf = cross(N,Bf);

			double s = scaling;
			if(adaptive_scaling.is_bound()) {
			    s *= adaptive_scaling[f];
			}

			for(index_t c1 = mesh->facets.corners_begin(f);
			    c1 < mesh->facets.corners_end(f); ++c1) {
			    index_t c2 =
				mesh->facets.next_corner_around_facet(f,c1);
			    index_t v1 = mesh->facet_corners.vertex(c1);
			    index_t v2 = mesh->facet_corners.vertex(c2);
			    vec3 E     =
				vec3(mesh->vertices.point_ptr(v2)) -
				vec3(mesh->vertices.point_ptr(v1));
			    solver.begin_energy();
			    solver.add_energy_coeff(2*c2, s);
			    solver.add_energy_coeff(2*c1,-s);
			    solver.add_energy_rhs(dot(Bf,E));
			    solver.end_energy();
			    solver.begin_energy();
			    solver.add_energy_coeff(2*c2+1, s);
			    solver.add_energy_coeff(2*c1+1,-s);
			    solver.add_energy_rhs(dot(BTf,E));
			    solver.end_energy();
			}
		    }
		    solver.end_iter();
		    if(!integer_constraints) {
			break;
		    }
		}


		QC_PROGRESS_END(6);
		QC_PROGRESS(0.95f);

		// Get the result
		tbb::parallel_for(
		    tbb::blocked_range<index_t>(0, nb_U),
		    [&](const tbb::blocked_range<index_t> &range) {
			for(index_t u = range.begin(); u != range.end(); ++u) {
			    double coord = solver.value(u);
			    //snap_tex_coord(coord); // Required by mesh extraction
			    U[u/2][u%2] = coord;
			}
		    });
		// tbb::parallel_for(
		//     tbb::blocked_range<index_t>(0, nb_T),
		//     [&](const tbb::blocked_range<index_t> &range) {
		// 	for(index_t t = range.begin(); t != range.end(); ++t)
		// 	    T[t] = solver.value(nb_U+t);
		//     });
	    }
	    
	} // namespace Internal
	
	void quad_cover(
	    Mesh* mesh,
	    Attribute<vec3>& B, Attribute<vec2>& U,
	    double scaling, bool constrain_hard_edges, bool do_brush,
	    bool integer_constraints,
	    double hard_angle_threshold
	) {
	    {
		Attribute<index_t> R_ff(mesh->facet_corners.attributes(),"R");
		Attribute<index_t> c_on_border(
		    mesh->facet_corners.attributes(), "on_border"
		);

		if(do_brush) {
		    QC_PROGRESS_BEGIN(0);
		    Internal::brush(mesh,B);
		    QC_PROGRESS(1.0f);
		}
		Internal::compute_R_ff(mesh,B,R_ff);
		QC_PROGRESS(1.0f);
		if(!do_brush) {
		    QC_PROGRESS_END(0);
		} else {
		    QC_PROGRESS_END(1);
		}
		Attribute<bool> v_is_singular(
		    mesh->vertices.attributes(), "is_singular"
		);
		QC_PROGRESS_BEGIN(do_brush ? 2 : 1);
		Internal::mark_singular_vertices(mesh, R_ff, v_is_singular);
		QC_PROGRESS_END(do_brush ? 2 : 1);

		QC_PROGRESS_BEGIN(do_brush ? 3 : 2);
		if(do_brush) {
		    Internal::do_the_ball(mesh, R_ff, c_on_border);
		} else {
		    Internal::do_the_ball_no_brush_no_zip(mesh, c_on_border);
		}
		QC_PROGRESS_END(do_brush ? 3 : 2);

		Attribute<double> T;
		T.bind_if_is_defined(mesh->facet_corners.attributes(),"T");
		if(!T.is_bound()) {
		    T.create_vector_attribute(
			mesh->facet_corners.attributes(), "T", 2
		    );
		}
		Attribute<index_t> constraint(
		    mesh->facet_corners.attributes(), "cnstr"
		);
		QC_PROGRESS_BEGIN(do_brush ? 4 : 4);
		Internal::get_constraints(mesh, B, R_ff, constraint, hard_angle_threshold);
		QC_PROGRESS(1.0f);
		Internal::quad_cover_solve(
		    mesh, B, R_ff, c_on_border,
		    constraint, U, T, v_is_singular,
		    scaling, constrain_hard_edges,
		    integer_constraints,
		    hard_angle_threshold
		);
		QC_PROGRESS_END(do_brush ? 4 : 4);
	    }
	    
	    // Destroy the temporary attributes
	    // I keep them for now, for debugging...
	    //   
	    //   mesh->facet_corners.attributes().delete_attribute_store("cnstr");
	    //   mesh->facet_corners.attributes().delete_attribute_store("on_border");
	    //   mesh->facet_corners.attributes().delete_attribute_store("T");
	    //   mesh->facet_corners.attributes().delete_attribute_store("R");
	    //   mesh->facet_corners.attributes().delete_attribute_store("UU");
	    //   mesh->vertices.attributes().delete_attribute_store("is_singular");
	    
	}

    } // namespace GlobalParam2d
    
} // namespace GEO
