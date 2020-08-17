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

#include <geogram/parameterization/mesh_LSCM.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/bibliography/bibliography.h>
#include <geogram/NL/nl.h>
#include <algorithm>

namespace {
    using namespace GEO;


     /**
      * \brief Computes Least Squares Conformal Maps in least squares or
      *  spectral mode. 
      * \details The method is described in the following references:
      *  - Least Squares Conformal Maps, Levy, Petitjean, Ray, Maillot, ACM 
      *   SIGGRAPH, 2002
      *  - Spectral Conformal Parameterization, Mullen, Tong, Alliez, Desbrun,
      *   Computer Graphics Forum (SGP conf. proc.), 2008
      */
    class LSCM {
    public:
	
	/**
	 * \brief LSCM constructor
	 * \param[in] M a reference to the mesh. It needs to correspond to a 
	 *   topological disk (open surface with one border and no handle).
	 * \param[in] tex_coord a vector property of dimension 2 where to 
	 *   store the texture coordinates.
	 */
	LSCM(Mesh& M, Attribute<double>& tex_coord, Attribute<double>& angle) :
	    mesh_(M), tex_coord_(tex_coord), angle_(angle), eigen_(0) {
	    geo_assert(tex_coord.dimension() == 2);
	    locked_1_ = index_t(-1);
	    locked_2_ = index_t(-1);
	    verbose_ = false;
	}

	/**
	 * \brief Enables or disables messages.
	 * \param[in] x if true, messages are displayed on the console
	 *  with statistics. Default is non-verbose.
	 */
	void set_verbose(bool x) {
	    verbose_ = x;
	}
	
	/**
	 * \brief Sets whether spectral mode is used.
	 * \details In default mode, the trivial solution (all vertices to zero)
	 *  is avoided by locking two vertices (that are as "extremal" 
	 *  as possible). In spectral mode, the trivial solution is avoided by 
	 *  finding the first minimizer that is orthogonal to it (more elegant,
	 *  but more costly). 
	 */
	void set_spectral(bool x) {
	    spectral_ = x;
	}

	/**
	 * \brief Computes the least squares conformal map and stores it in
	 *  the texture coordinates of the mesh.
	 * \details Outline of the algorithm (steps 1,2,3 are not used 
	 *   in spectral mode):
	 *   - 1) Find an initial solution by projecting on a plane
	 *   - 2) Lock two vertices of the mesh
	 *   - 3) Copy the initial u,v coordinates to OpenNL
	 *   - 4) Construct the LSCM equation with OpenNL
	 *   - 5) Solve the equation with OpenNL
	 *   - 6) Copy OpenNL solution to the u,v coordinates
	 */
	
	void apply() {

	    geo_cite("DBLP:journals/tog/LevyPRM02");
	    if(spectral_) {
		geo_cite("DBLP:journals/cgf/MullenTAD08");
	    }
	    
	    const int nb_eigens = 10;
	    nlNewContext();
	    if(spectral_) {
		if(nlInitExtension("ARPACK")) {
		    if(verbose_) {
			Logger::out("LSCM") << "ARPACK extension initialized"
					    << std::endl;
		    }
		    nlEigenSolverParameteri(NL_EIGEN_SOLVER, NL_ARPACK_EXT);
		    nlEigenSolverParameteri(NL_NB_EIGENS, nb_eigens);
		    if(verbose_) {
			nlEnable(NL_VERBOSE);
		    }
		} else {
		    if(verbose_) {
			Logger::out("LSCM")
			    << "Could not initialize ARPACK extension"
			    << std::endl;
			Logger::out("LSCM")
			    << "Falling back to least squares mode"
			    << std::endl;
		    }
		    spectral_ = false;
		}
	    } else {
		/*
		  // Direct solver, commented-out for now, 
		  // causes problems with some configurations.
		if(nlInitExtension("CHOLMOD")) {
		    if(verbose_) {
			Logger::out("LSCM") << "using CHOLMOD"
					    << std::endl;
		    }
		    nlSolverParameteri(NL_SOLVER, NL_CHOLMOD_EXT);
		} else 
		*/

		{
		    if(verbose_) {
			Logger::out("LSCM") << "using JacobiCG"
					    << std::endl;
		    }
		}
	    }
	    NLuint nb_vertices = NLuint(mesh_.vertices.nb());
	    if(!spectral_) {
		project();
	    }
	    nlSolverParameteri(NL_NB_VARIABLES, NLint(2*nb_vertices));
	    nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	    nlSolverParameteri(NL_MAX_ITERATIONS, NLint(5*nb_vertices));
	    if(spectral_) {
		nlSolverParameterd(NL_THRESHOLD, 0.0);	    
	    } else {
		nlSolverParameterd(NL_THRESHOLD, 1e-10);
	    }
	    nlBegin(NL_SYSTEM);
	    mesh_to_solver();
	    nlBegin(NL_MATRIX);
	    setup_lscm();
	    nlEnd(NL_MATRIX);
	    nlEnd(NL_SYSTEM);
	    if(verbose_) {
		Logger::out("LSCM") << "Solving ..." << std::endl;
	    }
	    
	    if(spectral_) {
		nlEigenSolve();
		if(verbose_) {
		    for(NLuint i=0; i<nb_eigens; ++i) {
			Logger::out("LSCM") << "[" << i << "] "
					    << nlGetEigenValue(i) << std::endl;
		    }
		}
		
		// Find first "non-zero" eigenvalue
		double small_eigen = ::fabs(nlGetEigenValue(0)) ;
		eigen_ = 1;
		for(NLuint i=1; i<nb_eigens; ++i) {
		    if(::fabs(nlGetEigenValue(i)) / small_eigen > 1e3) {
			eigen_ = i ;
			break ;
		    }
		}
	    } else{
		nlSolve();
	    }
	    
	    solver_to_mesh();
	    normalize_uv();
	    
	    if(!spectral_) {
		if(verbose_) {
		    double time;
		    NLint iterations;	    
		    nlGetDoublev(NL_ELAPSED_TIME, &time);
		    nlGetIntegerv(NL_USED_ITERATIONS, &iterations);
		    Logger::out("LSCM") << "Solver time: " << time << std::endl;
		    Logger::out("LSCM") << "Used iterations: "
					<< iterations << std::endl;
		}
	    }
	    nlDeleteContext(nlGetCurrent());
	}

    protected:

	/**
	 * \brief Creates the LSCM equations in OpenNL.
	 */
	void setup_lscm() {
	    for(NLuint f=0; f<mesh_.facets.nb(); ++f) {
		setup_lscm(f);
	    }
	}

	/**
	 * \brief Creates the LSCM equations in OpenNL, related
	 *  with a given facet.
	 * \param[in] f the index of the facet.
	 * \details no-need to triangulate the facet,
	 *   we do that "virtually", by creating triangles
	 *   radiating around vertex 0 of the facet.
	 *   (however, this may be invalid for concave facets)
	 */
	void setup_lscm(NLuint f) {
	    NLuint nv = NLuint(mesh_.facets.nb_vertices(f));
	    if(angle_.is_bound()) {
		index_t c0 = mesh_.facets.corners_begin(f);
		double a0 = angle_[c0];
		for(NLuint i=1; i<nv-1; ++i) {
		    double ai = angle_[c0+i];
		    double aip1 = angle_[c0+1+1];
		    setup_conformal_map_relations(
			NLuint(mesh_.facets.vertex(f,0)),
			NLuint(mesh_.facets.vertex(f,i)),
			NLuint(mesh_.facets.vertex(f,i+1)),
			a0, ai, aip1
		    );
		}
	    } else {
		for(NLuint i=1; i<nv-1; ++i) {
		    setup_conformal_map_relations(
			NLuint(mesh_.facets.vertex(f,0)),
			NLuint(mesh_.facets.vertex(f,i)),
			NLuint(mesh_.facets.vertex(f,i+1))
		    );
		}
	    }
	}
	
	/**
	 * \brief Computes the coordinates of the vertices of a triangle
	 * in a local 2D orthonormal basis of the triangle's plane.
	 * \param[in] p0 , p1 , p2 the 3D coordinates of the vertices of 
	 *   the triangle
	 * \param[out] z0 , z1 , z2 the 2D coordinates of the vertices of
	 *   the triangle
	 */
	static void project_triangle(
	    const vec3& p0, 
	    const vec3& p1, 
	    const vec3& p2,
	    vec2& z0,
	    vec2& z1,
	    vec2& z2
	) {
	    vec3 X = p1 - p0;
	    X = normalize(X);
	    vec3 Z = cross(X,(p2 - p0));
	    Z = normalize(Z);
	    vec3 Y = cross(Z,X);
	    const vec3& O = p0;
	    
	    double x0 = 0;
	    double y0 = 0;
	    double x1 = (p1 - O).length();
	    double y1 = 0;
	    double x2 = dot((p2 - O),X);
	    double y2 = dot((p2 - O),Y);        
            
	    z0 = vec2(x0,y0);
	    z1 = vec2(x1,y1);
	    z2 = vec2(x2,y2);        
	}

	/**
	 * \brief Creates the LSCM equation in OpenNL, related with
	 *   a given triangle, specified by vertex indices.
	 * \param[in] v0 , v1 , v2 the indices of the three vertices of
	 *   the triangle.
	 * \details Uses the geometric form of LSCM equation:
	 *  (Z1 - Z0)(U2 - U0) = (Z2 - Z0)(U1 - U0)
	 *  Where Uk = uk + i.vk is the complex number 
	 *                       corresponding to (u,v) coords
	 *       Zk = xk + i.yk is the complex number 
	 *                       corresponding to local (x,y) coords
	 * There is no divide with this expression,
	 *  this makes it more numerically stable in
	 * the presence of degenerate triangles.
	 */
	void setup_conformal_map_relations(
	    NLuint v0, NLuint v1, NLuint v2
	) {
            
	    const vec3& p0 = Geom::mesh_vertex(mesh_, v0);
	    const vec3& p1 = Geom::mesh_vertex(mesh_, v1);
	    const vec3& p2 = Geom::mesh_vertex(mesh_, v2);
            
	    vec2 z0,z1,z2;
	    project_triangle(p0,p1,p2,z0,z1,z2);
	    vec2 z01 = z1 - z0;
	    vec2 z02 = z2 - z0;
	    double a = z01.x;
	    double b = z01.y;
	    double c = z02.x;
	    double d = z02.y;
	    geo_assert(b == 0.0);

	    // Note  : 2*id + 0 --> u
	    //         2*id + 1 --> v
	    NLuint u0_id = 2*v0    ;
	    NLuint v0_id = 2*v0 + 1;
	    NLuint u1_id = 2*v1    ;
	    NLuint v1_id = 2*v1 + 1;
	    NLuint u2_id = 2*v2    ;
	    NLuint v2_id = 2*v2 + 1;
	    
	    // Note : rhs = 0
	    
	    // Real part
	    nlBegin(NL_ROW);
	    nlCoefficient(u0_id, -a+c) ;
	    nlCoefficient(v0_id,  b-d) ;
	    nlCoefficient(u1_id,   -c) ;
	    nlCoefficient(v1_id,    d) ;
	    nlCoefficient(u2_id,    a);
	    nlEnd(NL_ROW);
	    
	    // Imaginary part
	    nlBegin(NL_ROW);
	    nlCoefficient(u0_id, -b+d);
	    nlCoefficient(v0_id, -a+c);
	    nlCoefficient(u1_id,   -d);
	    nlCoefficient(v1_id,   -c);
	    nlCoefficient(v2_id,    a);
	    nlEnd(NL_ROW);
	}

	/**
	 * \brief Creates the LSCM equation in OpenNL, related with
	 *   a given triangle, specified by vertex indices, and with
	 *   specified desired angles.
	 * \details This version is used to recoved the u,v coordinates
	 *   from the angles computed by ABF++.
	 * \param[in] v0 , v1 , v2 the indices of the three vertices of
	 *   the triangle.
	 * \param[in] alpha0 , alpha1 , alpha2 the desired angles at the
	 *   three vertices of the triangle
	 */
	void setup_conformal_map_relations(
	    NLuint v0, NLuint v1, NLuint v2,
	    double alpha0, double alpha1, double alpha2
	) {
	    const vec3& p0 = Geom::mesh_vertex(mesh_, v0);
	    const vec3& p1 = Geom::mesh_vertex(mesh_, v1);
	    const vec3& p2 = Geom::mesh_vertex(mesh_, v2);
	    
            double scaling = ::sin(alpha1) / ::sin(alpha2) ;
	    double a = scaling * ::cos(alpha0);
	    double b = scaling * ::sin(alpha0);

	    double area = Geom::triangle_area(p0,p1,p2) ;
	    double s = ::sqrt(area) ;

	    // Note  : 2*id + 0 --> u
	    //         2*id + 1 --> v
	    NLuint u0_id = 2*v0    ;
	    NLuint v0_id = 2*v0 + 1;
	    NLuint u1_id = 2*v1    ;
	    NLuint v1_id = 2*v1 + 1;
	    NLuint u2_id = 2*v2    ;
	    NLuint v2_id = 2*v2 + 1;
	    
	    // Note : rhs = 0
	    
	    // Real part
	    nlRowScaling(s);
	    nlBegin(NL_ROW);
	    nlCoefficient(u0_id, 1.0 - a) ;
	    nlCoefficient(v0_id, b) ;
	    nlCoefficient(u1_id, a) ;
	    nlCoefficient(v1_id, -b) ;
	    nlCoefficient(u2_id, -1.0);
	    nlEnd(NL_ROW);
	    
	    // Imaginary part
	    nlRowScaling(s);	    
	    nlBegin(NL_ROW);
	    nlCoefficient(u0_id, -b);
	    nlCoefficient(v0_id, 1.0-a);
	    nlCoefficient(u1_id, b);
	    nlCoefficient(v1_id, a);
	    nlCoefficient(v2_id, -1.0);
	    nlEnd(NL_ROW);
	}
	
	/**
	 * \brief Copies u,v coordinates from OpenNL solver to the mesh.
	 */
	void solver_to_mesh() {
	    for(index_t i: mesh_.vertices) {
		double u = spectral_ ? nlMultiGetVariable(NLuint(2*i),eigen_)
		                     : nlGetVariable(2*i);
		double v = spectral_ ? nlMultiGetVariable(NLuint(2*i+1),eigen_)
		                     : nlGetVariable(2*i+1);
		tex_coord_[2*i] = u;
		tex_coord_[2*i+1] = v;
	    }
	}

	/**
	 * \brief Translates and scales tex coords in such a way that they fit
	 * within the unit square.
	 */
	void normalize_uv() {
	    double u_min=1e30, v_min=1e30, u_max=-1e30, v_max=-1e30;
	    for(NLuint i=0; i<mesh_.vertices.nb(); ++i) {
		double u = tex_coord_[2*i];
		double v = tex_coord_[2*i+1];
		
		u_min = std::min(u_min, u);
		v_min = std::min(v_min, v);
		u_max = std::max(u_max, u);
		v_max = std::max(v_max, v);
	    }
	    double l = std::max(u_max-u_min,v_max-v_min);
	    for(NLuint i=0; i<mesh_.vertices.nb(); ++i) {
		tex_coord_[2*i] -= u_min;
		tex_coord_[2*i] /= l;
		tex_coord_[2*i+1] -= v_min;
		tex_coord_[2*i+1] /= l;
	    }
	}


	/**
	 * \brief Tests whether a vertex is locked.
	 * \param[in] v the index of the vertex
	 * \retval true if the vertex is locked
	 * \retval false otherwise
	 */
        bool is_locked(index_t v) {
	    return (v==locked_1_ || v==locked_2_);
        }
    
    
	/**
	 * \brief Copies u,v coordinates from the mesh to OpenNL solver.
	 */
	void mesh_to_solver() {
	    for(NLuint i=0; i<mesh_.vertices.nb(); ++i) {
		double u = tex_coord_[2*i];
		double v = tex_coord_[2*i+1];
		nlSetVariable(2 * i    , u);
		nlSetVariable(2 * i + 1, v);
		if(!spectral_ && is_locked(i)) {
		    nlLockVariable(2 * i    );
		    nlLockVariable(2 * i + 1);
		} 
	    }
	}

	/**
	 * \brief Chooses an initial solution, and locks two vertices.
	 */
	void project() {
	    // Get bbox
	    double xmin =  1e30;
	    double ymin =  1e30;
	    double zmin =  1e30;
	    double xmax = -1e30;
	    double ymax = -1e30;
	    double zmax = -1e30;
	    
	    for(index_t i: mesh_.vertices) {
		const vec3& p = Geom::mesh_vertex(mesh_,i);
		xmin = std::min(p.x, xmin);
		ymin = std::min(p.y, ymin);
		zmin = std::min(p.z, zmin);
		
		xmax = std::max(p.x, xmax);
		ymax = std::max(p.y, ymax);
		zmax = std::max(p.z, zmax);
	    }
	    
	    double dx = xmax - xmin;
	    double dy = ymax - ymin;
	    double dz = zmax - zmin;
	    
	    vec3 V1,V2;
	    
	    // Find shortest bbox axis
	    if(dx <= dy && dx <= dz) {
		if(dy > dz) {
		    V1 = vec3(0,1,0);
		    V2 = vec3(0,0,1);
		} else {
		    V2 = vec3(0,1,0);
		    V1 = vec3(0,0,1);
		}
	    } else if(dy <= dx && dy <= dz) {
		if(dx > dz) {
		    V1 = vec3(1,0,0);
		    V2 = vec3(0,0,1);
		} else {
		    V2 = vec3(1,0,0);
		    V1 = vec3(0,0,1);
		}
	    } else if(dz <= dx && dz <= dy) {
		if(dx > dy) {
		    V1 = vec3(1,0,0);
		    V2 = vec3(0,1,0);
		} else {
		    V2 = vec3(1,0,0);
		    V1 = vec3(0,1,0);
		}
	    }

	    // Project onto shortest bbox axis,
	    // and lock extrema vertices
	    
	    double  umin = 1e30;
	    double  umax = -1e30;
	    
	    for(index_t i: mesh_.vertices) {
		const vec3& p = Geom::mesh_vertex(mesh_,i);
		double u = dot(p,V1);
		double v = dot(p,V2);
		tex_coord_[2*i]   = u;
		tex_coord_[2*i+1] = v;
		if(u < umin) {
		    locked_1_ = i;
		    umin = u;
		} 
		if(u > umax) {
		    locked_2_ = i;
		    umax = u;
		} 
	    }
	}
	
	Mesh& mesh_;

	Attribute<double>& tex_coord_;

	Attribute<double>& angle_;
	/**
	 * \brief true if spectral mode is used,
	 *  false if locked least squares mode is used.
	 */  
	bool spectral_;

	/**
	 * \brief In spectral mode, the index of the first
	 *  non-zero eigenvalue.
	 */
	NLuint eigen_;

        /**
	 * \brief The indices of the two locked vertices.
	 */
        index_t locked_1_, locked_2_;

	bool verbose_;
    };
    
}

namespace GEO {

    void mesh_compute_LSCM(
	Mesh& M, const std::string& attribute_name, bool spectral,
	const std::string& angle_attribute_name,
	bool verbose
    ) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(M.vertices.attributes(), attribute_name);
	if(tex_coord.is_bound() && tex_coord.dimension() != 2) {
	    Logger::err("LSCM") << "Attribute " << attribute_name
				<< " already exists in mesh with dimension "
				<< tex_coord.dimension()
				<< " (expected 2)"
				<< std::endl;
	    return;
	}
	if(!tex_coord.is_bound()) {
	    tex_coord.create_vector_attribute(
		M.vertices.attributes(),attribute_name,2
	    );
	}
	Attribute<double> angle;
	angle.bind_if_is_defined(
	    M.facet_corners.attributes(), angle_attribute_name
	);
	LSCM lscm(M,tex_coord,angle);
	lscm.set_spectral(spectral);
	lscm.set_verbose(verbose);
	lscm.apply();
    }
}
