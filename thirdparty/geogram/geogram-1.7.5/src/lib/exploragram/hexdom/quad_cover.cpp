
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Bruno Levy - Bruno.Levy@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#include <exploragram/hexdom/quad_cover.h>
#include <exploragram/hexdom/mixed_constrained_solver.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_frame_field.h>

namespace GEO {

    namespace GlobalParam2d {
	
	namespace Internal {

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
		bool integer_constraints
	    ) {
		scaling *= surface_average_edge_length(*mesh);
		index_t nb_U = mesh->facets.nb()*3*2;
		index_t nb_T = mesh->facets.nb()*3*2;
		MatrixMixedConstrainedSolver solver(nb_U+nb_T);
	
		// All Tijs are even integers (entiers pairs).
		FOR(t, nb_T) {
		    solver.set_multiplicity(nb_U+t,2);
		}

		// Constrained u,v coordinates
		if(constrain_hard_edges) {
		    FOR(c, mesh->facet_corners.nb()) {
			if((constraints[c] & CNSTR_U) != 0) {
			    solver.set_multiplicity(2*c,1);
			}
			if((constraints[c] & CNSTR_V) != 0) {
			    solver.set_multiplicity(2*c+1,1);
			}
		    }
		}
		
		// Compute vertex-to-corner map (exclude vertices on the border
		// and singular vertices)
		
		vector<index_t> v2c(mesh->vertices.nb(), NO_CORNER);
		FOR(c, mesh->facet_corners.nb()) {
		    index_t v = mesh->facet_corners.vertex(c);	    
		    v2c[v] = c;
		}
		{
		    FOR(c, mesh->facet_corners.nb()) {
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
		}
	

		FOR(pass, 4) {
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
		    
 	            //Wheel compatibility constraints (the Tij 1-form is closed)
		    FOR(v, mesh->vertices.nb()) {
			//   If the corner is on the border or incident
			// to a singular vertex then it is skipped.
			if(v2c[v] == NO_CORNER) {
			    continue;
			}
			//   Enforce the constraint on the wheel
			// neighborhood for each component of the Tijs.
			FOR(coord, 2) {
			    index_t c = v2c[v];
			    index_t r = 0;
			    solver.begin_constraint();		
			    do {
				solver.add_constraint_coeff(
				    nb_U+2*c,   Rot[r][coord][0]
				);
				solver.add_constraint_coeff(
				    nb_U+2*c+1, Rot[r][coord][1]
				);
				// Accumulate the rotation.
				r = (r + R[c]) % 4;
				// Find the next corner around the vertex.
				index_t f =
				    mesh->facet_corners.adjacent_facet(c);
				
				geo_assert(f != NO_FACET);
				index_t next_c = NO_CORNER;
				for(
				    next_c = mesh->facets.corners_begin(f);
				    next_c<mesh->facets.corners_end(f);
				    ++next_c
				) {
				    if(mesh->facet_corners.vertex(next_c)==v) {
					break;
				    }
				}
				geo_assert(
				    mesh->facet_corners.vertex(next_c) == v
				);
				c = next_c;
			    } while(c != v2c[v]);
			    // On non-singular vertices, by definition,
			    // compose of all rotations = identity.
			    geo_assert(r == 0);
			    solver.end_constraint();
			}
		    }

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
			    index_t cnstr = get_edge_constraints(mesh,c,B);
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
		}
		

		while (!solver.converged()) {
		    plop("MIQ iter");
		    solver.start_new_iter();

		    FOR(f, mesh->facets.nb()) {
			// setup objective function :
			// For each edge (pi,pj):
			//     ( B        * (pj-pi) - (uj-ui))^2 +
			//     ( rot90(B) * (pj-pi) - (vj-vi))^2
			vec3 N = normalize(Geom::mesh_facet_normal(*mesh,f));
			vec3 Bf  = normalize(B[f]);
			vec3 BTf = cross(N,Bf);

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
			    solver.add_energy_coeff(2*c2, scaling);
			    solver.add_energy_coeff(2*c1,-scaling);
			    solver.add_energy_rhs(dot(Bf,E));
			    solver.end_energy();
			    solver.begin_energy();
			    solver.add_energy_coeff(2*c2+1, scaling);
			    solver.add_energy_coeff(2*c1+1,-scaling);
			    solver.add_energy_rhs(dot(BTf,E));
			    solver.end_energy();
			}
		    }
		    solver.end_iter();
		    if(!integer_constraints) {
			break;
		    }
		}


		// Get the result
		FOR(u, nb_U) {
		    double coord = solver.value(u);
		    snap_tex_coord(coord); // Required by mesh extraction
		    U[u/2][u%2] = coord;
		}
		FOR(t, nb_T) {
		    T[t] = solver.value(nb_U+t);
		}
	    }
	    
	} // namespace Internal
	
	void quad_cover(
	    Mesh* mesh,
	    Attribute<vec3>& B, Attribute<vec2>& U,
	    double scaling, bool constrain_hard_edges, bool do_brush,
	    bool integer_constraints
	) {
	    {
		Attribute<index_t> R_ff(mesh->facet_corners.attributes(),"R");
		Attribute<index_t> c_on_border(
		    mesh->facet_corners.attributes(), "on_border"
		);

		if(do_brush) {
		    Internal::brush(mesh,B);
		}
		Internal::compute_R_ff(mesh,B,R_ff);
		Attribute<bool> v_is_singular(
		    mesh->vertices.attributes(), "is_singular"
		);
		Internal::mark_singular_vertices(mesh, R_ff, v_is_singular);
		
		if(do_brush) {
		    Internal::do_the_ball(mesh, R_ff, c_on_border);
		} else {
		    Internal::do_the_ball_no_brush_no_zip(mesh, c_on_border);
		}

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
		Internal::get_constraints(mesh, B, R_ff, constraint);
		Internal::quad_cover_solve(
		    mesh, B, R_ff, c_on_border,
		    constraint, U, T, v_is_singular,
		    scaling, constrain_hard_edges,
		    integer_constraints
		);
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
