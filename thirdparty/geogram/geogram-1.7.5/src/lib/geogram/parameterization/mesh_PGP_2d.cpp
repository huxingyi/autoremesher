/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#include <geogram/parameterization/mesh_PGP_2d.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/NL/nl.h>
#include <geogram/bibliography/bibliography.h>

#include <stack>

namespace {
    using namespace GEO;


    /**
     * \brief computes gradients in a triangle in 2D.
     */
    class ParamTrglGradient {
    public:
	ParamTrglGradient(
            const vec2& p1, const vec2& p2, const vec2& p3
	) {
	    vertex_[0] = p1 ;
	    vertex_[1] = p2 ;
	    vertex_[2] = p3 ;

	    double x1 = p1.x ;
	    double y1 = p1.y ;
	    double x2 = p2.x ;
	    double y2 = p2.y ;
	    double x3 = p3.x ;
	    double y3 = p3.y ;
	    
	    double d = x2*y3 - y2*x3 + x3*y1 - y3*x1 + x1*y2 - y1*x2 ;
	    
	    if(fabs(d) < 1e-10) {
		d = 1.0 ;
		is_flat_ = true ;
	    } else {
		is_flat_ = false ;
	    }

	    TX_[0] = (y2 - y3)/d ;
	    TX_[1] = (y3 - y1)/d ;
	    TX_[2] = (y1 - y2)/d ;
	    
	    TY_[0] = -(x2 - x3)/d ;
	    TY_[1] = -(x3 - x1)/d ;
	    TY_[2] = -(x1 - x2)/d ;
	}

        double TX(int i) const {
	    geo_debug_assert(i<3);
	    return TX_[i]; 
	}
	
        double TY(int i) const {
	    geo_debug_assert(i<3);
	    return TY_[i];
	}
	
        bool is_flat() const {
	    return is_flat_ ;
	}

    private:
        double TX_[3] ;
        double TY_[3] ;
        vec2 vertex_[3] ;
        bool is_flat_ ;
    } ;

    
    /** 
     * \brief Retrieves a coordinate from an angle computed by PGP.
     * \param[in] alpha the input variable.
     * \param[in] ref the reference variable.
     * \return a number congruent to \p alpha modulo 2 pi in the inverval 
     *  [ \p ref - M_PI, \p ref + M_PI]
     */
    double normalize_periodic_variable(
	double alpha, double ref
    ) {
	int count = 0;
	if(Numeric::is_nan(alpha)) {
	    return 0.0 ;
	}
	if(Numeric::is_nan(ref)) {
	    return 0.0 ;
	}
	double result = alpha ;
	count = 0 ;
	while(ref - result > M_PI) {
	    result += 2.0 * M_PI ;
	    count ++ ;
	    if(count > 100) {
		return 0.0 ;
	    }
	}
	count = 0 ;
	while(result - ref > M_PI) {
	    result -= 2.0 * M_PI ;
	    count ++ ;
	    if(count > 100) {
		return 0.0 ;
	    }
	}
	return result ;
    }
}

namespace GEO {

    namespace GlobalParam2d {
 
	void PGP(
	    Mesh* mesh,
	    Attribute<vec3>& B, Attribute<vec2>& U,
	    double scaling, bool constrain_hard_edges,
	    bool use_direct_solver,
	    double maximum_scaling_correction
	) {
	    geo_cite("DBLP:journals/tog/RayLLSA06");
	    
	    bool do_brush = true;
	    
	    // We use f = c/3
	    // (could be fixed in the future, using a c2f array...).
	    geo_assert(mesh->facets.are_simplices()); 
	    
	    // Step 0: Preparation

	    scaling *= 2.0;
	    scaling *= surface_average_edge_length(*mesh);
		
	    // Step 0.1: Preparation / Brushing
	    if(do_brush) {
		Internal::brush(mesh,B);
	    }

	    // Step 0.2: Preparation / Compute relative rotation of B between
	    //   pairs of adjacent facets
	    Attribute<index_t> R_ff(mesh->facet_corners.attributes(),"R");
	    Internal::compute_R_ff(mesh,B,R_ff);
	    Attribute<index_t> R_fv(mesh->facet_corners.attributes(),"R_fv");
	    Internal::compute_R_fv(mesh,R_ff,R_fv);
	    
	    Attribute<double> CC;
	    if(maximum_scaling_correction != 1.0) {
		Logger::out("PGP") << "Computing scaling correction"
				   << std::endl;
		CC.bind(mesh->vertices.attributes(), "CC");	    
		Attribute<vec3> Bv(mesh->vertices.attributes(), "B");
		Internal::transfer_B_to_vertices(mesh, B, Bv, R_fv);
		curl_correction(
		    mesh, Bv, R_fv, CC,
		    use_direct_solver, maximum_scaling_correction
		);
	    }


	    Logger::out("PGP") << "Solving for PGP" << std::endl;

	    
	    // Step 1: Determine the structure of the problem:
	    // - There are four variables per vertex (cu, su, cv, sv)
	    // - Each vertex has a reference corner (v2c[v])
	    // - Each corner c knows the number of rotations Rc[c] required
	    //    to make the B of its triangle match the B of the triangle
	    //    of the reference corner attached to its vertex (clear
	    //    enough ?)

	    // Rot[ R_ff[c] ][i][j] corresponds to the transform to
	    // be applied to the variables associated with a
	    // vertex to express their coordinates in the frame
	    // of corner c (it corresponds to the same Rot[][][]
	    // matrices as QuadCover, with the exception that each
	    // coefficient 1 is replaced with the 2x2 identity matrix
	    // and each coefficient -1 with Mat2x2(1,0,0,-1) (there is
	    // only one "-1" coefficient because cos(-x) = cos(x) !!).
	    // Note: in the PGP paper, the wrong coefficient is negated.
	    
	    static const double Rot[4][4][4] = {
		{{ 1, 0, 0, 0},
		 { 0, 1, 0, 0},
		 { 0, 0, 1, 0},
		 { 0, 0, 0, 1}},
		    
		{{ 0, 0, 1, 0},
		 { 0, 0, 0, 1},
		 { 1, 0, 0, 0},
		 { 0,-1, 0, 0}},
		
		{{ 1, 0, 0, 0},
		 { 0,-1, 0, 0},
		 { 0, 0, 1, 0},
		 { 0, 0, 0,-1}},
		
		{{ 0, 0, 1, 0},
		 { 0, 0, 0,-1},
		 { 1, 0, 0, 0},
		 { 0, 1, 0, 0}}
	    };

	    static double Rotc2[4][4];

	    // Step 2: setup and solve linear system
	    {

		nlNewContext();

		nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
		nlSolverParameteri(
		    NL_NB_VARIABLES, NLint(mesh->vertices.nb()*4)
		);

		if(use_direct_solver) {
		    if(nlInitExtension("CHOLMOD")) {
			nlSolverParameteri(NL_SOLVER, NL_CHOLMOD_EXT);
		    } else if(nlInitExtension("SUPERLU")) {
			nlSolverParameteri(NL_SOLVER, NL_PERM_SUPERLU_EXT);
		    } else {
			Logger::warn("PGP")
			    << "Could not initialize direct sovlver"
			    << std::endl;
			Logger::warn("PGP")
			    << "Falling back to Jacobi pre-CG"
			    << std::endl;
			use_direct_solver = false;
		    }
		}

		if(!use_direct_solver) {
		    // With the iterative solver, a very small threshold is
		    // needed, because of the very high scaling on the
		    // solution due to the varying modulus of the complex
		    // numbers far away from the constrained point.
		    nlSolverParameterd(NL_THRESHOLD, 1e-20);
		}
		
		nlBegin(NL_SYSTEM);

		if(constrain_hard_edges) {
		    for(index_t c: mesh->facet_corners) {
			index_t cnstr =
			    Internal::get_edge_constraints(mesh, c, B);
			index_t v = mesh->facet_corners.vertex(c);
                        // Inverse R, because when the axis turns
                        // clockwise, coordinates turn anticlockwise.
			index_t Rcc = Internal::inverse_R(R_fv[c]); 
			if(cnstr & Internal::CNSTR_U) {
			    if(Rcc == 0 || Rcc == 2) {
				nlLockVariable(4*v);
				nlSetVariable(4*v,1e4);
				nlLockVariable(4*v+1);
				nlSetVariable(4*v+1,0.0);
			    } else {
				nlLockVariable(4*v+2);
				nlSetVariable(4*v+2,1e4);
				nlLockVariable(4*v+3);
				nlSetVariable(4*v+3,0.0);
			    }
			}
			if(cnstr & Internal::CNSTR_V) {
			    if(Rcc == 0 || Rcc == 2) {
				nlLockVariable(4*v+2);
				nlSetVariable(4*v+2,1e4);
				nlLockVariable(4*v+3);
				nlSetVariable(4*v+3,0.0);
			    } else {
				nlLockVariable(4*v);
				nlSetVariable(4*v,1e4);
				nlLockVariable(4*v+1);
				nlSetVariable(4*v+1,0.0);
			    }
			}
		    }
		}

		// Lock at least one variable per connected component.
		{
		    std::vector<bool> f_visited(mesh->facets.nb(),false);
		    for(index_t f: mesh->facets) {
			if(!f_visited[f]) {
			    index_t nb_locked=0;
			    index_t first_v = mesh->facets.vertex(f,0);
			    std::stack<index_t> S;
			    f_visited[f] = true;
			    S.push(f);
			    while(!S.empty()) {
				index_t f_top = S.top();
				S.pop();
				FOR(le,mesh->facets.nb_vertices(f_top)) {
				    index_t v = mesh->facets.vertex(f_top,le);
				    if(
					nlVariableIsLocked(4*v) ||
					nlVariableIsLocked(4*v+2)) {
					++nb_locked;
				    }
				    index_t f_neigh =
					mesh->facets.adjacent(f_top,le);
				    
				    if(f_neigh != NO_FACET &&
				       !f_visited[f_neigh]
				    ) {
					f_visited[f_neigh] = true;
					S.push(f_neigh);
				    }
				}
			    }
			    
			    // Lock one of the points in each
			    // connected component to make sure that
			    // the minimum is well defined.
			    if(nb_locked == 0) {
				nlLockVariable(4*first_v);
				nlSetVariable(4*first_v,1e4);
		    
				nlLockVariable(4*first_v+1);
				nlSetVariable(4*first_v+1,0.0);
				
				nlLockVariable(4*first_v+2);
				nlSetVariable(4*first_v+2,1e4);
				
				nlLockVariable(4*first_v+3);
				nlSetVariable(4*first_v+3,0.0);
			    }
			}
		    }
		}
		
		nlBegin(NL_MATRIX);

		//  This one will be replaced in-place
		// with the rotation that encodes the
		// delta u and delta v along each edge.
		
		double RotDelta[4][4];
		FOR(i,4) {
		    FOR(j,4) {
			RotDelta[i][j] = ((i==j) ? 1.0 : 0.0);
		    }
		}
		
		for(index_t f: mesh->facets) {

		    vec3 Bf, BTf;

		    for(index_t c1=mesh->facets.corners_begin(f);
			c1 < mesh->facets.corners_end(f); ++c1) {

			Internal::get_B_on_edge(mesh, B, R_ff, f, c1, Bf, BTf);
			
			index_t c2 =
			    mesh->facets.next_corner_around_facet(f,c1);
			index_t v1 = mesh->facet_corners.vertex(c1);
			index_t v2 = mesh->facet_corners.vertex(c2);
			
			vec3 E     = vec3(mesh->vertices.point_ptr(v2)) -
			             vec3(mesh->vertices.point_ptr(v1));
			double delta_u = 2.0 * M_PI * dot(E,Bf)/scaling;
			double delta_v = 2.0 * M_PI * dot(E,BTf)/scaling;

			if(CC.is_bound()) {
			    double s = (0.5*(CC[v1] + CC[v2]));
			    delta_u /= s;
			    delta_v /= s;
			}
			
			double sdu = sin(delta_u);
			double cdu = cos(delta_u);
			double sdv = sin(delta_v);
			double cdv = cos(delta_v);

			RotDelta[0][0] =  cdu;
			RotDelta[0][1] = -sdu;
			RotDelta[1][0] =  sdu;
			RotDelta[1][1] =  cdu;

			RotDelta[2][2] =  cdv;
			RotDelta[2][3] = -sdv;
			RotDelta[3][2] =  sdv;
			RotDelta[3][3] =  cdv;

                        // Inverse R, because when the axis turns
                        // clockwise, coordinates turn anticlockwise.
			index_t Rc1 = Internal::inverse_R(R_fv[c1]);
			index_t Rc2 = Internal::inverse_R(R_fv[c2]);

			// Compute the product of the "delta u, delta v"
			// rotation with the Rc2 "90 degrees rotation" matrix,
			// exactly like in the PGP article.
			
			for(index_t i=0; i<4; ++i) {
			    for(index_t j=0; j<4; ++j) {
				Rotc2[i][j] = 0.0;
				for(index_t k=0; k<4; ++k) {
				    Rotc2[i][j] +=
					RotDelta[i][k] * Rot[Rc2][k][j];
				}
			    }
			}
			
			for(index_t i=0; i<4; ++i) {
			    nlBegin(NL_ROW);
			    for(index_t j=0; j<4; ++j) {
				double a1 = Rot[Rc1][i][j];
				if(a1 != 0.0) {
				    nlCoefficient(v1*4+j, a1);
				}
			    }
			    for(index_t j=0; j<4; ++j) {
				double a2 = Rotc2[i][j];
				if(a2 != 0.0) {
				    nlCoefficient(v2*4+j, -a2);
				}
			    }
			    nlEnd(NL_ROW);
			}
		    }
		}
		
		nlEnd(NL_MATRIX);
		nlEnd(NL_SYSTEM);

		nlSolve();

		Attribute<double> PGP;
		PGP.bind_if_is_defined(mesh->facet_corners.attributes(),"PGP");
		if(!PGP.is_bound()) {
		    PGP.create_vector_attribute(
			mesh->facet_corners.attributes(), "PGP", 4
		    );
		}
		
		for(index_t c: mesh->facet_corners) {
		    index_t v = mesh->facet_corners.vertex(c);
		    // Inverse R, because when the axis turns
		    // clockwise, coordinates turn anticlockwise.
		    index_t Rcc = Internal::inverse_R(R_fv[c]);
		    double vars[4];
		    for(index_t i=0; i<4; ++i) {
			vars[i] = 0.0;
			for(index_t j=0; j<4; ++j) {
			    vars[i] += Rot[Rcc][i][j] * nlGetVariable(4*v+j);
			}
		    }
		    double s = sqrt(vars[0]*vars[0]+vars[1]*vars[1]);
		    vars[0] /= s;
		    vars[1] /= s;

		    s = sqrt(vars[2]*vars[2]+vars[3]*vars[3]);
		    vars[2] /= s;
		    vars[3] /= s;
		    
		    U[c].x = atan2(vars[1], vars[0]); 
		    U[c].y = atan2(vars[3], vars[2]);

		    FOR(i,4) {
			PGP[4*c+i] = vars[i];
		    }
		}

		Attribute<index_t> singular(
		    mesh->facets.attributes(),"is_singular"
		);
		
		for(index_t f: mesh->facets) {
		    singular[f] = false;
		    for(index_t c1=mesh->facets.corners_begin(f);
			c1<mesh->facets.corners_end(f); ++c1) {
			index_t c2 =
			    mesh->facets.next_corner_around_facet(f,c1);

			index_t v1 = mesh->facet_corners.vertex(c1);
			index_t v2 = mesh->facet_corners.vertex(c2);
			vec3 E     = vec3(mesh->vertices.point_ptr(v2)) -
			             vec3(mesh->vertices.point_ptr(v1));
			
			vec3 Bf, BTf;
			Internal::get_B_on_edge(mesh, B, R_ff, f, c1, Bf, BTf);
			
			double delta_u = 2.0 * M_PI * dot(E,Bf)/scaling;
			double delta_v = 2.0 * M_PI * dot(E,BTf)/scaling;

			if(CC.is_bound()) {
			    double s = (0.5*(CC[v1] + CC[v2]));
			    delta_u /= s;
			    delta_v /= s;
			}
			
			double expected_u = U[c1].x-delta_u;
			double expected_v = U[c1].y-delta_v;
			vec2 uv(
			    normalize_periodic_variable(U[c2].x,expected_u),
			    normalize_periodic_variable(U[c2].y,expected_v)
			);
			if(c2 == mesh->facets.corners_begin(f)) {
			    singular[f] =
				 (length(uv - U[c2]) > 1e-20) ? 1 : 0;
			} else {
			    U[c2] = uv;
			}
			
		    }
		}

		for(index_t c: mesh->facet_corners) {
		    U[c].x /= M_PI;
		    U[c].y /= M_PI;
		    Internal::snap_tex_coord(U[c].x);
		    Internal::snap_tex_coord(U[c].y);
		}
		
		nlDeleteContext(nlGetCurrent());
	    }


	}


	void curl_correction(
	    Mesh* mesh, Attribute<vec3>& Bv,
	    Attribute<index_t>& R_fv, Attribute<double>& CC,
	    bool use_direct_solver, double max_scaling_correction
	) {
	    geo_assert(Bv.manager() == &mesh->vertices.attributes());
	    nlNewContext();
	    
	    nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	    nlSolverParameteri(
		NL_NB_VARIABLES, NLint(mesh->vertices.nb()*4)
	    );
	    
	    if(use_direct_solver) {
		if(nlInitExtension("CHOLMOD")) {
		    nlSolverParameteri(NL_SOLVER, NL_CHOLMOD_EXT);
		} else if(nlInitExtension("SUPERLU")) {
		    nlSolverParameteri(NL_SOLVER, NL_PERM_SUPERLU_EXT);
		} else {
		    Logger::warn("PGP")
			<< "Could not initialize direct solver"
			<< std::endl;
		    Logger::warn("PGP")
			<< "Falling back to Jacobi pre-CG"
			<< std::endl;
		    use_direct_solver = false;
		}
	    }
	    
	    if(!use_direct_solver) {
		// With the iterative solver, a very small threshold is
		// needed, because of the very high scaling on the
		// solution due to the varying modulus of the complex
		// numbers far away from the constrained point.
		nlSolverParameterd(NL_THRESHOLD, 1e-20);
	    }

	    const double locked_value = 1.0;
	    const double solver_scale = 1e3;
	    
	    nlBegin(NL_SYSTEM);
	    nlLockVariable(0u);
	    nlSetVariable(0u,locked_value);
	    nlBegin(NL_MATRIX);
	    for(index_t f: mesh->facets) {

		index_t va = mesh->facets.vertex(f,0);
		index_t vb = mesh->facets.vertex(f,1);
		index_t vc = mesh->facets.vertex(f,2);
		
		vec3 N = normalize(Geom::mesh_facet_normal(*mesh,f));

		vec3 fieldA3d = normalize(Bv[va]);
		vec3 fieldB3d = normalize(Bv[vb]);
		vec3 fieldC3d = normalize(Bv[vc]);

		FOR(i, Internal::inverse_R(R_fv[f*3])) {
		    fieldA3d = cross(N,fieldA3d);
		}

		FOR(i, Internal::inverse_R(R_fv[f*3+1])) {
		    fieldB3d = cross(N,fieldB3d);
		}

		FOR(i, Internal::inverse_R(R_fv[f*3+2])) {
		    fieldC3d = cross(N,fieldC3d);
		}
		
		vec3 U = normalize(cross(cross(N, fieldA3d),N));
		vec3 V = normalize(cross(N, U));


		// fieldx is the 2d field at point x. The fields are
		// rotated in a coherent way to take the modulus into
		// account notice that it will be better if the field
		// is rotated to the facet instead of being projected
		vec2 fieldA(1.0,0.0);
		vec2 fieldB(dot(fieldB3d, U), dot(fieldB3d, V)) ;
		fieldB = normalize(fieldB);
		
		vec2 fieldC (dot(fieldC3d, U), dot(fieldC3d, V)) ;
		fieldC = normalize(fieldC);

		vec3 A3d(mesh->vertices.point_ptr(va));
		vec3 B3d(mesh->vertices.point_ptr(vb));
		vec3 C3d(mesh->vertices.point_ptr(vc));		

		vec3 AB3d = B3d-A3d;
		vec3 AC3d = C3d-A3d;

		vec2 A(0.0,0.0);
		vec2 B(dot(AB3d,U), dot(AB3d,V));
		vec2 C(dot(AC3d,U), dot(AC3d,V));

		// hummm... flat triangles ?
		double a=0;
		double b=0;
		ParamTrglGradient trg(A,B,C);
		if (trg.is_flat() ||
		    length(cross(AB3d,AC3d))<1e-10 ||
		    ::fabs(fieldB.x) < 1e-20 ||
		    ::fabs(fieldC.x) < 1e-20
		) {
		    std::cerr<<"bad triangle ..." << std::endl ;
		
		    nlBegin(NL_ROW);
		    nlCoefficient(va,1e-2) ;
		    nlCoefficient(vb,-1e-2) ;
		    nlEnd(NL_ROW);

		    nlBegin(NL_ROW);
		    nlCoefficient(va,1e-2) ;
		    nlCoefficient(vc,-1e-2) ;
		    nlEnd(NL_ROW);

		    nlBegin(NL_ROW);
		    nlCoefficient(vc,1e-2) ;
		    nlCoefficient(vb,-1e-2) ;
		    nlEnd(NL_ROW);
                continue ;
            } else {

		    // the direction field is represented by angles
		    //  double alpha_A = 0;
		    //  double alpha_B = atan(fieldB.y/fieldB.x);
		    //  double alpha_C = atan(fieldC.y/fieldC.x);

		    // Order 1 Taylor Expansion ....
		    double alpha_A = 0;
		    double alpha_B = fieldB.y/fieldB.x;
		    double alpha_C = fieldC.y/fieldC.x;
                
		    vec2 grad_alpha (	
			trg.TX(0)*alpha_A +
			trg.TX(1)*alpha_B +
			trg.TX(2)*alpha_C ,
			trg.TY(0)*alpha_A +
			trg.TY(1)*alpha_B +
			trg.TY(2)*alpha_C
		    );
		    a = grad_alpha.x;
		    b = grad_alpha.y;
		}

		double sqrt_area =  ::sqrt(Geom::mesh_facet_area(*mesh,f)) ;

		if(
		    Numeric::is_nan(sqrt_area) ||
		    Numeric::is_nan(trg.TX(0)) ||
		    Numeric::is_nan(trg.TX(1)) ||                
		    Numeric::is_nan(trg.TX(2)) ||
		    Numeric::is_nan(trg.TY(0)) ||
		    Numeric::is_nan(trg.TY(1)) ||
		    Numeric::is_nan(trg.TY(2)) ||
		    Numeric::is_nan(a) ||
		    Numeric::is_nan(b) 
		) {
		    std::cerr << "Found NAN !!" << std::endl ;
		}


		nlRowScaling(sqrt_area) ;
		nlBegin(NL_ROW);
		nlCoefficient(va, trg.TX(0)) ;
		nlCoefficient(vb, trg.TX(1)) ;
		nlCoefficient(vc, trg.TX(2)) ;
		nlRightHandSide(solver_scale*b);
		nlEnd(NL_ROW);

		nlRowScaling(sqrt_area) ;
		nlBegin(NL_ROW);
		nlCoefficient(va, trg.TY(0)) ;
		nlCoefficient(vb, trg.TY(1)) ;
		nlCoefficient(vc, trg.TY(2)) ;
		nlRightHandSide(solver_scale*-a);
		nlEnd(NL_ROW);
	    }
	    nlEnd(NL_MATRIX);
	    nlEnd(NL_SYSTEM);

	    nlSolve();
	    double min_val = Numeric::max_float64();
	    double max_val = Numeric::min_float64();
	    for(index_t v: mesh->vertices) {
		CC[v] = ::exp(
		    (nlGetVariable(v)-locked_value)/solver_scale)
		;
		min_val = std::min(min_val,CC[v]);				
		max_val = std::max(max_val,CC[v]);
	    }
	    double avg_val = 0.5 * (min_val + max_val);
	    double min_limit = 1.0 / ::sqrt(max_scaling_correction);
	    double max_limit = ::sqrt(max_scaling_correction);
	    for(index_t v: mesh->vertices) {
		CC[v] = CC[v] / avg_val;
		geo_clamp(CC[v], min_limit, max_limit);
	    }
	    nlDeleteContext(nlGetCurrent());
	}
	
    } // namespace GlobalParam2d
} // namespace OGF
