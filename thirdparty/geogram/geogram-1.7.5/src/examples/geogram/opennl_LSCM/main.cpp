/*
 * GEOGRAM example program:
 * compute LSCM (Least Squares Conformal Maps) using OpenNL
 * Note: given as an example for using OpenNL, there is a
 * directly usable version of LSCM in geogram/parameterization/LSCM.h
 * that one may use instead.
 */

/*
 *  Copyright (c) 2004-2010, Bruno Levy
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
 *     levy@loria.fr
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */


#include <geogram/NL/nl.h>

#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

/******************************************************************************/
/* Basic geometric types */

/**
 * \brief A 2D vector
 */
class vec2 {
public:
    /**
     * \brief Constructs a 2D vector from its coordinates.
     * \param x_in , y_in the coordinates.
     */
    vec2(double x_in, double y_in) :
	x(x_in), y(y_in) {
    }

    /**
     * \brief Constructs the zero vector.
     */
    vec2() : x(0), y(0) {
    }
    
    double x;
    double y;
};

/**
 * \brief A 3D vector
 */
class vec3 {
public:

    /**
     * \brief Constructs a 3D vector from its coordinates.
     * \param x_in , y_in , z_in the coordinates.
     */
    vec3(double x_in, double y_in, double z_in) :
	x(x_in), y(y_in), z(z_in) {
    }

    /**
     * \brief Constructs the zero vector.
     */
    vec3() : x(0), y(0), z(0) {
    }

    /**
     * \brief Gets the length of this vector.
     * \return the length of this vector.
     */
    double length() const { 
        return sqrt(x*x + y*y + z*z);
    }

    /**
     * \brief Normalizes this vector.
     * \details This makes the norm equal to 1.0
     */
    void normalize() {
        double l = length();
        x /= l; y /= l; z /= l;
    }

    double x;
    double y;
    double z;
};

/**
 * \brief Outputs a 2D vector to a stream.
 * \param[out] out a reference to the stream
 * \param[in] v a const reference to the vector
 * \return the new state of the stream
 * \relates vec2
 */
inline std::ostream& operator<<(std::ostream& out, const vec2& v) {
    return out << v.x << " " << v.y;
}

/**
 * \brief Outputs a 3D vector to a stream.
 * \param[out] out a reference to the stream
 * \param[in] v a const reference to the vector
 * \return the new state of the stream
 * \relates vec3
 */
inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.x << " " << v.y << " " << v.z;
}

/**
 * \brief Reads a 2D vector from a stream
 * \param[in] in a reference to the stream
 * \param[out] v a reference to the vector
 * \return the new state of the stream
 * \relates vec2
 */
inline std::istream& operator>>(std::istream& in, vec2& v) {
    return in >> v.x >> v.y;
}

/**
 * \brief Reads a 3D vector from a stream
 * \param[in] in a reference to the stream
 * \param[out] v a reference to the vector
 * \return the new state of the stream
 * \relates vec3
 */
inline std::istream& operator>>(std::istream& in, vec3& v) {
    return in >> v.x >> v.y >> v.z;
}

/**
 * \brief Computes the dot product between two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the dot product between \p v1 and \p v2
 * \relates vec3
 */
inline double dot(const vec3& v1, const vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/**
 * \brief Computes the cross product between two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the cross product between \p v1 and \p v2
 * \relates vec3
 */
inline vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3(
        v1.y*v2.z - v2.y*v1.z,
        v1.z*v2.x - v2.z*v1.x,
        v1.x*v2.y - v2.x*v1.y
    );
}

/**
 * \brief Computes the sum of two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the sum of \p v1 and \p v2
 * \relates vec3
 */
inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z
    );
}

/**
 * \brief Computes the difference between two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the difference between \p v1 and \p v2
 * \relates vec3
 */
inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z
    );
}

/**
 * \brief Computes the sum of two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the sum of \p v1 and \p v2
 * \relates vec2
 */
inline vec2 operator+(const vec2& v1, const vec2& v2) {
    return vec2(
        v1.x + v2.x,
        v1.y + v2.y
    );
}

/**
 * \brief Computes the difference between two vectors
 * \param[in] v1 , v2 const references to the two vectors
 * \return the difference between \p v1 and \p v2
 * \relates vec2
 */
inline vec2 operator-(const vec2& v1, const vec2& v2) {
    return vec2(
        v1.x - v2.x,
        v1.y - v2.y
    );
}

/******************************************************************************/
/* Mesh class */

/**
 * \brief A vertex in an IndexedMesh
 * \relates IndexedMesh
 */
class Vertex {
public:
    /**
     * \brief Vertex constructor.
     */
    Vertex() : locked(false) {
    }

    /**
     * \brief Vertex constructor from 3D point and texture
     *   coordinates.
     * \param[in] p the 3D coordinates of the vertex
     * \param[in] t the texture coordinates associated with the vertex
     */
    Vertex(
        const vec3& p, const vec2& t
    ) : point(p), tex_coord(t), locked(false) {
    }

    /**
     * \brief The 3D coordinates.
     */
    vec3 point;

    /**
     * \brief The texture coordinates (2D).
     */
    vec2 tex_coord;

    /**
     * \brief A boolean flag that indicates whether the vertex is
     *  locked, i.e. considered as constant in the optimizations.
     */
    bool locked;
};


/**
 * \brief A minimum mesh class.
 * \details It does not have facet adjacency information
 *   (we do not need it for LSCM), it just stores for each facet the indices
 *   of its vertices. It has load() and save() functions that use the 
 *   Alias Wavefront .obj file format.
 */
class IndexedMesh {
public:

    /**
     * \brief IndexedMesh constructor
     */
    IndexedMesh() : in_facet(false) {
	facet_ptr.push_back(0);
    }

    /**
     * \brief Gets the number of vertices.
     * \return the number of vertices in this mesh.
     */
    NLuint nb_vertices() const {
	return NLuint(vertex.size());
    }

    /**
     * \brief Gets the number of facets.
     * \return the number of facets in this mesh.
     */
    NLuint nb_facets() const {
	return NLuint(facet_ptr.size()-1);
    }

    /**
     * \brief Gets the number of vertices in a facet.
     * \param[in] f the facet, in 0..nb_facets()-1
     * \return the number of vertices in facet \p f
     * \pre f < nb_facets()
     */
    NLuint facet_nb_vertices(NLuint f) {
	assert(f < nb_facets());
	return facet_ptr[f+1]-facet_ptr[f];
    }

    /**
     * \brief Gets a facet vertex by facet index and
     *  local vertex index in facet.
     * \param[in] f the facet, in 0..nb_facets()-1
     * \param[in] lv the local vertex index in the facet,
     *  in 0..facet_nb_vertices(f)-1
     * \return the global vertex index, in 0..nb_vertices()-1
     * \pre f<nb_facets() && lv < facet_nb_vertices(f)
     */
    NLuint facet_vertex(NLuint f, NLuint lv) {
	assert(f < nb_facets());
	assert(lv < facet_nb_vertices(f));
	return corner[facet_ptr[f] + lv];
    }

    /**
     * \brief Adds a new vertex to the mesh.
     */
    void add_vertex() {
        vertex.push_back(Vertex());
    }

    /**
     * \brief Adds a new vertex to the mesh.
     * \param[in] p the 3D coordinates of the vertex
     * \param[in] t the texture coordinates of the vertex
     */
    void add_vertex(const vec3& p, const vec2& t) {
        vertex.push_back(Vertex(p,t));
    }

    /**
     * \brief Stats a new facet.
     */
    void begin_facet() {
        assert(!in_facet);
        in_facet = true;
    }

    /**
     * \brief Terminates the current facet.
     */
    void end_facet() {
        assert(in_facet);
        in_facet = false;
	facet_ptr.push_back(NLuint(corner.size()));
    }

    /**
     * \brief Adds a vertex to the current facet.
     * \param[in] v the index of the vertex 
     * \pre v < vertex.size()
     */
    void add_vertex_to_facet(NLuint v) {
        assert(in_facet);
        assert(v < vertex.size());
	corner.push_back(v);
    }

    /**
     * \brief Removes all vertices and all facets from
     *  this mesh.
     */
    void clear() {
        vertex.clear();
	corner.clear();
	facet_ptr.clear();
	facet_ptr.push_back(0);
    }

    /**
     * \brief Loads a file in Alias Wavefront OFF format.
     * \param[in] file_name the name of the file.
     */
    void load(const std::string& file_name) {
        std::ifstream input(file_name.c_str());
        clear();
        while(input) {
	    std::string line;
	    std::getline(input, line);
            std::stringstream line_input(line);
            std::string keyword;
            line_input >> keyword;
            if(keyword == "v") {
                vec3 p;
                line_input >> p;
                add_vertex(p,vec2(0.0,0.0));
            } else if(keyword == "vt") {
                // Ignore tex vertices
            } else if(keyword == "f") {
                begin_facet();
                while(line_input) {
                    std::string s;
                    line_input >> s;
                    if(s.length() > 0) {
                        std::stringstream v_input(s.c_str());
                        NLuint index;
                        v_input >> index;
                        add_vertex_to_facet(index - 1);
                        char c;
                        v_input >> c;
                        if(c == '/') {
                            v_input >> index;
                            // Ignore tex vertex index
                        }
                    }
                }
                end_facet();
            }
        } 
        std::cout << "Loaded " << vertex.size() << " vertices and " 
                  << nb_facets() << " facets" << std::endl;
    }

    /**
     * \brief Saves a file in Alias Wavefront OFF format.
     * \param[in] file_name the name of the file.
     */
    void save(const std::string& file_name) {
        std::ofstream out(file_name.c_str());
        for(NLuint v=0; v<nb_vertices(); ++v) {
            out << "v " << vertex[v].point << std::endl;
        }
        for(NLuint v=0; v<nb_vertices(); ++v) {	
	    out << "vt " << vertex[v].tex_coord << std::endl;
        }
        for(NLuint f=0; f<nb_facets(); ++f) {
	    NLuint nv = facet_nb_vertices(f);
            out << "f ";
	    for(NLuint lv=0; lv<nv; ++lv) {
		NLuint v = facet_vertex(f,lv);
                out << (v + 1) << "/" << (v + 1) << " ";
            }
            out << std::endl;
        }
        for(NLuint v=0; v<nb_vertices(); ++v) {
            if(vertex[v].locked) {
                out << "# anchor " << v+1 << std::endl;
            }
        }
    }

    std::vector<Vertex> vertex;
    bool in_facet;
    
    /**
     * \brief All the vertices associated with the facet corners.
     */
    std::vector<NLuint> corner;

    /**
     * \brief Indicates where facets start and end within the corner 
     *  array (facet indicence matrix is stored in the compressed row 
     *  storage format).
     * \details The corners associated with facet f are in the range
     *  facet_ptr[f] ... facet_ptr[f+1]-1
     */
    std::vector<NLuint> facet_ptr;
};

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
     */
    LSCM(IndexedMesh& M) : mesh_(&M) {
	spectral_ = false;
    }

    /**
     * \brief Sets whether spectral mode is used.
     * \details In default mode, the trivial solution (all vertices to zero)
     *  is avoided by locking two vertices (that are as "extremal" as possible).
     *  In spectral mode, the trivial solution is avoided by finding the first
     *  minimizer that is orthogonal to it (more elegant, but more costly). 
     */
    void set_spectral(bool x) {
	spectral_ = x;
    }

    /**
     * \brief Computes the least squares conformal map and stores it in
     *  the texture coordinates of the mesh.
     * \details Outline of the algorithm (steps 1,2,3 are not used 
     *   in spetral mode):
     *   - 1) Find an initial solution by projecting on a plane
     *   - 2) Lock two vertices of the mesh
     *   - 3) Copy the initial u,v coordinates to OpenNL
     *   - 4) Construct the LSCM equation with OpenNL
     *   - 5) Solve the equation with OpenNL
     *   - 6) Copy OpenNL solution to the u,v coordinates
     */

    void apply() {
	const int nb_eigens = 10;
	nlNewContext();
	if(spectral_) {
	    if(nlInitExtension("ARPACK")) {
		std::cout << "ARPACK extension initialized"
			  << std::endl;
	    } else {
		std::cout << "Could not initialize ARPACK extension"
			  << std::endl;
		exit(-1);
	    }
	    nlEigenSolverParameteri(NL_EIGEN_SOLVER, NL_ARPACK_EXT);
	    nlEigenSolverParameteri(NL_NB_EIGENS, nb_eigens);
	    nlEnable(NL_VERBOSE);
	} 
        NLuint nb_vertices = NLuint(mesh_->vertex.size());
	if(!spectral_) {
	    project();
	}
        nlSolverParameteri(NL_NB_VARIABLES, NLint(2*nb_vertices));
        nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
        nlSolverParameteri(NL_MAX_ITERATIONS, NLint(5*nb_vertices));
	if(spectral_) {
	    nlSolverParameterd(NL_THRESHOLD, 0.0);	    
	} else {
	    nlSolverParameterd(NL_THRESHOLD, 1e-6);
	}
        nlBegin(NL_SYSTEM);
        mesh_to_solver();
        nlBegin(NL_MATRIX);
        setup_lscm();
        nlEnd(NL_MATRIX);
        nlEnd(NL_SYSTEM);
        std::cout << "Solving ..." << std::endl;

	if(spectral_) {
	    nlEigenSolve();
	    for(NLuint i=0; i<nb_eigens; ++i) {
		std::cerr << "[" << i << "] "
			  << nlGetEigenValue(i) << std::endl;
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
	    double time;
	    NLint iterations;	    
	    nlGetDoublev(NL_ELAPSED_TIME, &time);
	    nlGetIntegerv(NL_USED_ITERATIONS, &iterations);
	    std::cout << "Solver time: " << time << std::endl;
	    std::cout << "Used iterations: " << iterations << std::endl;
	}
	
        nlDeleteContext(nlGetCurrent());
    }

protected:

    /**
     * \brief Creates the LSCM equations in OpenNL.
     */
    void setup_lscm() {
        for(NLuint f=0; f<mesh_->nb_facets(); ++f) {
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
	NLuint nv = mesh_->facet_nb_vertices(f);
	for(NLuint i=1; i<nv-1; ++i) {
            setup_conformal_map_relations(
		mesh_->facet_vertex(f,0),
		mesh_->facet_vertex(f,i),
		mesh_->facet_vertex(f,i+1)
            );
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
        X.normalize();
        vec3 Z = cross(X,(p2 - p0));
        Z.normalize();
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
            
        const vec3& p0 = mesh_->vertex[v0].point; 
        const vec3& p1 = mesh_->vertex[v1].point;
        const vec3& p2 = mesh_->vertex[v2].point;
            
        vec2 z0,z1,z2;
        project_triangle(p0,p1,p2,z0,z1,z2);
        vec2 z01 = z1 - z0;
        vec2 z02 = z2 - z0;
        double a = z01.x;
        double b = z01.y;
        double c = z02.x;
        double d = z02.y;
        assert(b == 0.0);

        // Note  : 2*id + 0 --> u
        //         2*id + 1 --> v
        NLuint u0_id = 2*v0    ;
        NLuint v0_id = 2*v0 + 1;
        NLuint u1_id = 2*v1    ;
        NLuint v1_id = 2*v1 + 1;
        NLuint u2_id = 2*v2    ;
        NLuint v2_id = 2*v2 + 1;
        
        // Note : b = 0

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
     * \brief Copies u,v coordinates from OpenNL solver to the mesh.
     */
    void solver_to_mesh() {
        for(NLuint i=0; i<mesh_->vertex.size(); ++i) {
            Vertex& it = mesh_->vertex[i];
            double u = spectral_ ? nlMultiGetVariable(2 * i    ,eigen_)
		                 : nlGetVariable(2 * i    );
            double v = spectral_ ? nlMultiGetVariable(2 * i + 1,eigen_)
		                 : nlGetVariable(2 * i + 1);
            it.tex_coord = vec2(u,v);
        }
    }

    /**
     * \brief Translates and scales tex coords in such a way that they fit
     * within the unit square.
     */
    void normalize_uv() {
	double u_min=1e30, v_min=1e30, u_max=-1e30, v_max=-1e30;
        for(NLuint i=0; i<mesh_->vertex.size(); ++i) {
	    u_min = std::min(u_min, mesh_->vertex[i].tex_coord.x);
	    v_min = std::min(v_min, mesh_->vertex[i].tex_coord.y);
	    u_max = std::max(u_max, mesh_->vertex[i].tex_coord.x);
	    v_max = std::max(v_max, mesh_->vertex[i].tex_coord.y);	    
	}
	double l = std::max(u_max-u_min,v_max-v_min);
        for(NLuint i=0; i<mesh_->vertex.size(); ++i) {
	    mesh_->vertex[i].tex_coord.x -= u_min;
	    mesh_->vertex[i].tex_coord.x /= l;
	    mesh_->vertex[i].tex_coord.y -= v_min;
	    mesh_->vertex[i].tex_coord.y /= l;
	}
    }
    
    /**
     * \brief Copies u,v coordinates from the mesh to OpenNL solver.
     */
    void mesh_to_solver() {
        for(NLuint i=0; i<mesh_->vertex.size(); ++i) {
            Vertex& it = mesh_->vertex[i];
            double u = it.tex_coord.x;
            double v = it.tex_coord.y;
            nlSetVariable(2 * i    , u);
            nlSetVariable(2 * i + 1, v);
            if(!spectral_ && it.locked) {
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
        unsigned int i;

        double xmin =  1e30;
        double ymin =  1e30;
        double zmin =  1e30;
        double xmax = -1e30;
        double ymax = -1e30;
        double zmax = -1e30;

        for(i=0; i<mesh_->vertex.size(); i++) {
            const Vertex& v = mesh_->vertex[i];
            xmin = std::min(v.point.x, xmin);
            ymin = std::min(v.point.y, ymin);
            zmin = std::min(v.point.z, zmin);

            xmax = std::max(v.point.x, xmax);
            ymax = std::max(v.point.y, ymax);
            zmax = std::max(v.point.z, zmax);
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

        Vertex* vxmin = nullptr;
        double  umin = 1e30;
        Vertex* vxmax = nullptr;
        double  umax = -1e30;

        for(i=0; i<mesh_->vertex.size(); i++) {
            Vertex& V = mesh_->vertex[i];
            double u = dot(V.point,V1);
            double v = dot(V.point,V2);
            V.tex_coord = vec2(u,v);
            if(u < umin) {
                vxmin = &V;
                umin = u;
            } 
            if(u > umax) {
                vxmax = &V;
                umax = u;
            } 
        }

        vxmin->locked = true;
        vxmax->locked = true;
    }

    IndexedMesh* mesh_;
    
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
};


int main(int argc, char** argv) {
    bool spectral = false;
    bool OK = true;
    std::vector<std::string> filenames;

    nlInitialize(argc, argv);
    
    for(int i=1; i<argc; ++i) {
	if(!strcmp(argv[i],"spectral=true")) {
	    spectral = true;
	} else if(!strcmp(argv[i],"spectral=false")) {
	    spectral = false;
	} else if(strchr(argv[i],'=') == nullptr) {
	    filenames.push_back(argv[i]);
	}
    }

    OK = OK && (filenames.size() >= 1) && (filenames.size() <= 2);
    
    if(!OK) {
        std::cerr << "usage: " << argv[0]
		  << " infile.obj <outfile.obj> <spectral=true|false>" 
		  << std::endl;
        return -1;
    }

    if(filenames.size() == 1) {
	filenames.push_back("out.obj");
    }

    IndexedMesh mesh;
    std::cout << "Loading " << filenames[0] << "   ..." << std::endl;
    mesh.load(filenames[0]);

    LSCM lscm(mesh);
    lscm.set_spectral(spectral);
    lscm.apply();

    std::cout << "Saving " << filenames[1] << "   ..." << std::endl;
    mesh.save(filenames[1]);
}
