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

#include <geogram/mesh/mesh_frame_field.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/NL/nl.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/line_stream.h>
#include <geogram/basic/progress.h>
#include <geogram/bibliography/bibliography.h>

// Some member functions of NormalCycle are not used here.
// note: NormalCycle will be exported sometime, so for now
// we deactivate the warning.

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunused-member-function"
#endif

#ifdef __ICC
#pragma warning disable 177
#endif

namespace {
    using namespace GEO;

    /**
     * \brief We manipulate 4-symmetry direction fields [Ray et.al]
     */
    const double symd = 4.0;

    /**
     * \brief Represents a local orthonormal basis 
     *  of a mesh facet.
     */
    class MeshFacetBasis {
       public:

        /**
         * \brief Constructs a new MeshFacetBasis.
         * \param[in] M a const reference to the mesh
         * \param[in] f the index of the facet in \p M
         */
        MeshFacetBasis(
            const Mesh& M, index_t f 
        ) {
            X = normalize(
                Geom::mesh_corner_vector(M, M.facets.corners_begin(f))
            );
            N = normalize(Geom::mesh_facet_normal(M,f));
            Y = cross(N,X); 
        }

        /**
         * \brief Transforms a 3d vector into the local
         *  2d basis.
         * \param[in] v the input 3d vector
         * \return the representation of \p v in the local
         *  2d basis.
         */
        vec2 project(const vec3& v) const { 
            return vec2(dot(v,X),dot(v,Y));
        }
        
        /**
         * \brief Transforms a local 3d vector into the
         *  global 3d basis.
         * \param[in] v the input 2d vector in the local basis
         * \return the representation of \p v in the global
         *  3d basis.
         */
        vec3 unproject(const vec2& v) const { 
            return v.x*X + v.y*Y;
        }
        
        /**
         * \brief Computes the angles between a 3d vector and
         *  the first axis of the local basis.
         * \param[in] v the input vector in the global 3d basis
         * \return the angle between \p v and the first axis of
         *  the local basis
         */
        double angle(const vec3& v) const { 
            vec2 v2=project(v); 
            return atan2(v2.y,v2.x); 
        }

        /**
         * \brief Computes the rotation angle between the two reference vectors
         *  of two facets that share an edge.
         * \param[in] M a reference to the mesh
         * \param[in] c a corner index in \p M
         * \return the angle between the reference frames of the 
         *  two facets sharing the edge originating at \p c1
         */
        static double reference_rotation_accross_edge(
            const Mesh& M, index_t c
        ) {
            geo_debug_assert(M.facets.are_simplices());
            index_t f1 = c/3;
            index_t f2 = M.facet_corners.adjacent_facet(c);
            geo_debug_assert(f2 != NO_FACET);
            vec3 ref = Geom::mesh_corner_vector(M,c);
            MeshFacetBasis B1(M,f1);
            MeshFacetBasis B2(M,f2);
            return B2.angle(ref) - B1.angle(ref);
        }

    private:
        vec3 X; 
        vec3 Y;
        vec3 N; 
    };


    /**
     * \brief Solves for Periodic Global Parameterization variables.
     * \details This function computes for each facet of the mesh \p M
     *  two variables, that correspond to the cosine and sine of an angle
     *  interpolated over the mesh. This angle is relative to the first
     *  edge of the facet, as defined in the MeshFacetBasis class.
     * \param[in] M a const reference to the surface mesh
     * \param[in,out] sincos_alpha a vector of 2*M.facets.nb() variables,
     *  that correspond to the interpolated variables. The initial value
     *  is taken into account in the fitting term if \p fitting is non-zero
     * \param[in] locked a vector of M.facets.nb() booleans, indicating
     *  whether each facet is locked. The variables that correspond to 
     *  a locked facet are unchanged.
     * \param[in] global_fitting importance of the fitting term with respect to 
     *  the initial value of \p sincos_alpha. If zero, no fitting term is
     *  installed
     * \param[in] local_fitting an optional vector of M.facets.nb() doubles 
     *  that specifies for each facet an individual factor that scales 
     *  global_fitting
     */
    void solve_PGP(
        const Mesh& M, 
        vector<double>& sincos_alpha, 
        const vector<bool>& locked,
        double global_fitting,
        const vector<double>& local_fitting = vector<double>() 
    ) {
        // Step 0: normalize variables
        for(index_t f: M.facets) {
            double c = sincos_alpha[2*f];
            double s = sincos_alpha[2*f+1];
            double scale = sqrt(s*s+c*c);
            if(scale > 1e-30) {
                sincos_alpha[2*f] = c/scale;
                sincos_alpha[2*f+1] = s/scale;
            }
        }

        // Step 1: Setup the OpenNL solver
        nlNewContext();
        nlSolverParameteri(NL_NB_VARIABLES, NLint(2*M.facets.nb()));
        nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
#ifdef GEO_DEBUG        
        nlEnable(NL_VERBOSE);
#endif        
        nlEnable(NL_NORMALIZE_ROWS);

        // Step 2: setup the variables
        nlBegin(NL_SYSTEM);
        for(index_t f: M.facets) {
            nlSetVariable(2*f, sincos_alpha[2*f]);
            nlSetVariable(2*f+1, sincos_alpha[2*f+1]);
            if(locked.size() != 0 && locked[f]) {
                nlLockVariable(2*f);
                nlLockVariable(2*f+1);
            }
        }

        nlBegin(NL_MATRIX);

        // Step 3: setup the PGP smoothness term
        for(index_t f1: M.facets) {
            for(index_t c1: M.facets.corners(f1)) {
                index_t f2 = M.facet_corners.adjacent_facet(c1);
                if(f2 == NO_FACET || f1 < f2) {
                    continue;
                }
                
                double angle = -symd*
                    MeshFacetBasis::reference_rotation_accross_edge(
                        M,c1
                    );

                double c = cos(angle); 
                double s = sin(angle);                    
                
                nlBegin(NL_ROW);
                nlCoefficient(2*f1,c);
                nlCoefficient(2*f1+1,s);
                nlCoefficient(2*f2,-1.0);                    
                nlEnd(NL_ROW);

                nlBegin(NL_ROW);
                nlCoefficient(2*f1,-s);
                nlCoefficient(2*f1+1,c);
                nlCoefficient(2*f2+1,-1.0);                    
                nlEnd(NL_ROW);
            }
        }  

        // Step 4: setup the data fitting term
        if(global_fitting != 0) {
            for(index_t f: M.facets) {

                double fitting = global_fitting;
                if(local_fitting.size() != 0) {
                    fitting *= local_fitting[f];
                }

                if(fitting == 0.0) {
                    continue;
                }

                nlRowScaling(fitting);
                nlBegin(NL_ROW);
                nlCoefficient(2*f,1.0);
                nlRightHandSide(sincos_alpha[2*f]);
                nlEnd(NL_ROW);

                nlRowScaling(fitting);
                nlBegin(NL_ROW);
                nlCoefficient(2*f+1,1.0);
                nlRightHandSide(sincos_alpha[2*f+1]);
                nlEnd(NL_ROW);
            }
        }

        nlEnd(NL_MATRIX);
        nlEnd(NL_SYSTEM);

        // Step 5: solve the linear system
        nlSolve() ;

        // Step 6: read the new values of the variables
        for(index_t f: M.facets) {
            sincos_alpha[2*f] = nlGetVariable(2*f);
            sincos_alpha[2*f+1] = nlGetVariable(2*f+1);
        }

        // Step 7: cleanup memory allocated by OpenNL 
        nlDeleteContext(nlGetCurrent());
    } 


    /**
     * \brief Estimates the curvature tensor using a set
     * of samples. Each sample is a vector and a dihedral angle.
     * \details The algorithm is detailed in the following reference:
     *    Restricted Delaunay Triangulation and Normal Cycle,
     *    D. Cohen-Steiner and J.M. Morvan,
     *    SOCG 2003
     */
    class NormalCycle {
    public:
        /**
         * \brief Constructs a new NormalCycle.
         */
        NormalCycle() {
            clear();
        }
        
        /**
         * \brief Clears the currently accumulated matrix.
         */
        void clear() {
            for(index_t i=0; i<6; ++i) {
                M_[i] = 0.0;
            }
        }

        /**
         * \brief Computes the eigenvalues
         *  and eigenvectors of the accumulated tensor.
         */
        void compute() {
            double trace = M_[0] + M_[2] + M_[5] ;
            double s = 1e-6 * trace ;
            if (trace==0.0) {
                s = 1e-6;
            }
            M_[0] += s ;
            M_[2] += s ;
            M_[5] += s ;
            
       
            double eigen_vectors[9] ;
            MatrixUtil::semi_definite_symmetric_eigen(
                M_, 3, eigen_vectors, eigen_value_
            ) ;
            
            axis_[0] = vec3(
                eigen_vectors[0], eigen_vectors[1], eigen_vectors[2]
            );
            
            axis_[1] = vec3(
                eigen_vectors[3], eigen_vectors[4], eigen_vectors[5]
            );
        
            axis_[2] = vec3(
                eigen_vectors[6], eigen_vectors[7], eigen_vectors[8]
            );
        
            // Normalize the eigen vectors
            
            for(index_t i=0; i<3; ++i) {
                axis_[i] = normalize(axis_[i]) ;
            }

            // Sort the eigen vectors
            i_[0] = 0 ;
            i_[1] = 1 ;
            i_[2] = 2 ;

            double l0 = ::fabs(eigen_value_[0]) ;
            double l1 = ::fabs(eigen_value_[1]) ;
            double l2 = ::fabs(eigen_value_[2]) ;
            
            if(l1 > l0) {
                std::swap(l0   , l1   ) ;
                std::swap(i_[0], i_[1]) ;
            }

            if(l2 > l1) {
                std::swap(l1   , l2   ) ;
                std::swap(i_[1], i_[2]) ;
            }
            if(l1 > l0) {
                std::swap(l0   , l1  ) ;
                std::swap(i_[0],i_[1]) ;
            }
        }

        /**
         * \brief Accumulates a dihedral angle to the current
         *  tensor. 
         * \details This function needs to be called between
         *  a begin() \ end() pair. If a geometric clipping 
         *  neighborhood is used, the specified edge vector 
         *  needs to be clipped by it.
         * \param[in] edge the supporting edge of the dihedron
         * \param[in] angle the angle of the dihedron
         * \param[in] neigh_area the area of the clipped
         *   neighborhood
         *  to store the accumulated tensor.
         */
        void accumulate_dihedral_angle(
            const vec3& edge, double angle, double neigh_area=1.0
        ) {
            vec3 e = normalize(edge) ;
            double s = length(edge) * angle * neigh_area ;
            M_[0] += s * e.x * e.x;
            M_[1] += s * e.x * e.y;
            M_[2] += s * e.y * e.y;
            M_[3] += s * e.x * e.z;
            M_[4] += s * e.y * e.z;
            M_[5] += s * e.z * e.z;        
        }

        /**
         * \brief Gets an eigenvector by index
         * \param[in] i the index of the eigenvector (0,1 or 2). 
         *  The eigenvectors are sorted by increasing eigenvalue
         *  magnitude.
         * \return the \p i%-th eigenvector
         */
        const vec3& eigen_vector(int i) const { 
            return axis_[i_[i]]; 
        }

        /**
         * \brief Gets an eigenvalue by index
         * \param[in] i the index of the eigenvalue (0,1 or 2). 
         *  The eigenvalues are sorted by increasing eigenvalue
         *  magnitude.
         * \return the \p i%-th eigenvalue
         */
        double eigen_value(int i) const { 
            return eigen_value_[i_[i]];  
        } 

        /**
         * \brief Gets the estimated normal vector.
         * \return the estimated normal vector
         */
        const vec3& N() const { 
            return eigen_vector(2); 
        }

        /**
         * \brief Gets the estimated direction of maximum curvature.
         * \return the estimated direction of maximum curvature
         */
        const vec3& Kmax() const { 
            return eigen_vector(0);
        }

        /**
         * \brief Gets the estimated direction of minimum curvature.
         * \return the estimated direction of minimum curvature
         */
        const vec3& Kmin() const { 
            return eigen_vector(1); 
        }

        /**
         * \brief Gets the estimated maximum curvature.
         * \return the estimated maximum curvature
         */
        double kmax() const { 
            return eigen_value(0);
        }

        /**
         * \brief Gets the estimated minimum curvature.
         * \return the estimated minimum curvature
         */
        double kmin() const { 
            return eigen_value(1); 
        }

        /**
         * \brief Adds the currently accumulated tensor to a
         *  matrix.
         * \param[out] M an array of 6 doubles that represents
         *  the tensor to which the current tensor will be added
         */
        void add_to_matrix(double* M) const {
            for(index_t i=0; i<6; ++i) {
                M[i] += M_[i];
            }
        }

        /**
         * \brief Adds a matrix to the currently accumulated tensor.
         * \param[in] M an array of 6 doubles that represents the
         *  matrix that should be added to the currently accumulated
         *  tensor.
         */
        void add_matrix(const double* M) {
            for(index_t i=0; i<6; ++i) {
                M_[i] += M[i];
            }
        }

    private:
        vec3 axis_[3] ;
        double eigen_value_[3] ;
        double M_[6] ;
        int i_[3] ;
    };

    /**
     * \brief Estimates the direction of the maximum principal curvature.
     * \details The direction of maximum principal curvature is encoded 
     *  as the cosine and sine of the angle it makes relative to the first
     *  edge of teach triangle, as defined in the MeshFacetBasis class.
     * \param[in] M a const reference to the surface mesh
     * \param[in,out] sincos_alpha a vector of 2*M.facets.nb() doubles, 
     *  that contains the cosines and sines of the angle between the estimated
     *  directions and the first edge of each facet.
     * \param[in] locked a vector of M.facets.nb() booleans, that indicates
     *  for each facet whether it is locked. Directions of locked facets are
     *  kept unchanged
     * \param[out] magnitude a vector of M.facets.nb() doubles that indicates
     *  for each facet the magnitude of the principal direction of curvature
     */
    void estimate_max_curvature_direction(
        const Mesh& M, vector<double>& sincos_alpha, const vector<bool>& locked,
        vector<double>& magnitude
    ) {
        NormalCycle NC;
        vector<double> matrices(M.vertices.nb()*6,0.0);

        // Compute tensors of vertex neighborhoods
        for(index_t f1: M.facets) {
            for(index_t c: M.facets.corners(f1)) {
                index_t f2 = M.facet_corners.adjacent_facet(c);
                if(f2 == NO_FACET || f2 < f1) {
                    continue;
                }

                index_t v1 = M.facet_corners.vertex(c);
                index_t v2 = M.facet_corners.vertex(c+1);

                vec3 e = Geom::mesh_corner_vector(M,c);
                double alpha = Geom::mesh_normal_angle(M,c);
                
                NC.clear();
                NC.accumulate_dihedral_angle(e,alpha);
                NC.add_to_matrix(&matrices[6*v1]);
                NC.add_to_matrix(&matrices[6*v2]);
            } 
        }


        //  For each facet, accumulate the tensors of all its
        // vertices.
        for(index_t f: M.facets) {
            if(locked.size() != 0 && locked[f]) {
                continue;
            }
            NC.clear();
            for(index_t c: M.facets.corners(f)) {
                index_t v=M.facet_corners.vertex(c);
                NC.add_matrix(&matrices[6*v]);
            }
            NC.compute();

            // Compute eigenvector and encode it as PGP
            // representation (cosine and sine of the
            // angle relative to the first edge of the triangle).
            vec2 K = MeshFacetBasis(M,f).project(
                NC.Kmax()
            );
            double angle = atan2(K.y,K.x)*symd;
            sincos_alpha[2*f] = cos(angle);
            sincos_alpha[2*f+1] = sin(angle);

            magnitude[f] = fabs(NC.kmax());
        }
    }


}

namespace GEO {
    
    bool FrameField::load(
        const Mesh& M, bool volumetric, const std::string& filename
    ) {
        Logger::out("Frames") << "Loading frames from "
                              << filename << std::endl;
        frames_.clear();
        frames_.reserve(M.cells.nb() * 9);
        centers_.clear();
        bool result = true;
        bool with_centers = false;
        try {
            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
            bool first_line = true;
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                if(first_line) {
                    if(in.nb_fields() == 12) {
                        with_centers = true;
                    }
                    first_line=false;
                }
                if(!with_centers && in.nb_fields() != 3) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << ": invalid number of fields (expected 3, got"
                        << in.nb_fields()
                        << ")"
                        << std::endl;
                    result = false;
                    break;
                }
                if(with_centers && in.nb_fields() != 12) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << ": invalid number of fields (expected 12, got"
                        << in.nb_fields()
                        << ")"
                        << std::endl;
                    result = false;
                    break;
                }
                if(with_centers) {
                    for(index_t i=0; i<3; ++i) {
                        centers_.push_back(in.field_as_double(i));
                    }
                    for(index_t i=3; i<12; ++i) {
                        frames_.push_back(in.field_as_double(i));
                    }
                } else {
                    frames_.push_back(in.field_as_double(0));
                    frames_.push_back(in.field_as_double(1));
                    frames_.push_back(in.field_as_double(2));
                }
            }
        }
        catch(const std::exception& ex) {
            Logger::err("I/O") << ex.what() << std::endl;
            result = false;
        }
        if(!result) {
            Logger::err("I/O")
                << "Could not load file: " << filename
                << std::endl;
            return false;
        }
        if(!with_centers) {
            if(volumetric) {
                if(frames_.size() != M.cells.nb() * 9) {
                    Logger::err("I/O")
                        << "Invalid number of elements in frame for: "
                        << filename << std::endl;
                    return false;
                }
                centers_.resize(M.cells.nb() * 3);
                for(index_t t : M.cells) {
                    vec3 g = Geom::mesh_tet_center(M, t);
                    centers_[3 * t] = g.x;
                    centers_[3 * t + 1] = g.y;
                    centers_[3 * t + 2] = g.z;
                }
            } else {
                if(frames_.size() != M.facets.nb() * 9) {
                    Logger::err("I/O")
                        << "Invalid number of elements in frame for: "
                        << filename << std::endl;
                    return false;
                }
                centers_.resize(M.facets.nb() * 3);
                for(index_t f: M.facets) {
                    vec3 g = Geom::mesh_facet_center(M, f);
                    centers_[3 * f] = g.x;
                    centers_[3 * f + 1] = g.y;
                    centers_[3 * f + 2] = g.z;
                }
            }
        }
        geo_assert((centers_.size()/3)*3 == centers_.size());
        geo_assert((frames_.size()/9)*9 == frames_.size());

        index_t nb_vectors = frames_.size()/3;
        for(index_t i=0; i<nb_vectors; ++i) {
            double s = 0.0;
            for(index_t c=0; c<3; ++c) {
                s += frames_[3*i+c]*frames_[3*i+c];
            }
            s = ::sqrt(s);
            if(s == 0.0) {
                Logger::warn("Frames")
                    << "Zero-length vector in frame" << std::endl;
            } else {
                for(index_t c=0; c<3; ++c) {
                    frames_[3*i+c] /= s;
                }
            }
        }

        
        Logger::out("Frames") << "Loaded " << centers_.size()/3
                              << " frames" << std::endl;
        Logger::out("Frames") << "Creating NN search" << std::endl;
        NN_ = NearestNeighborSearch::create(3, "default");
        NN_->set_points(centers_.size()/3, centers_.data());
        return true;
    }

    void FrameField::create_from_surface_mesh(
        const Mesh& M, bool volumetric, double sharp_angle_threshold
    ) {

	geo_cite("DBLP:journals/tog/RayVLL08");
	geo_cite("DBLP:journals/tog/RayVAL09");
	
        sharp_angle_threshold *= M_PI/180.0 ;

        vector<double> alpha_sincos(2*M.facets.nb(),0.0);
        vector<bool> locked(M.facets.nb());

        // Step 1: setup the fixed variables
        index_t nb_constrained = 0;

        for(index_t f1: M.facets) {
            for(index_t c1: M.facets.corners(f1)) {
                index_t f2 = M.facet_corners.adjacent_facet(c1);
                if(
                    f2 == NO_FACET || (
			::fabs(Geom::mesh_normal_angle(M,c1)) > 
			sharp_angle_threshold
                    )
                ) {
                    vec2 v = MeshFacetBasis(M,f1).project(
                                Geom::mesh_corner_vector(M,c1)
                             );
                    double angle = atan2(v.y,v.x)*symd;

		    locked[f1]=true;
                    alpha_sincos[2*f1] = cos(angle);
                    alpha_sincos[2*f1+1] = sin(angle);
                    
                    ++nb_constrained;
                }
            }
        }

        Logger::out("Frames") 
            << nb_constrained << " constrained edges" << std::endl;

        vector<double> certainty(M.facets.nb());
        estimate_max_curvature_direction(M,alpha_sincos,locked,certainty);
        double max_certainty = 0.0;
        for(index_t f: M.facets) {
            max_certainty = std::max(max_certainty,certainty[f]);
        }
        for(index_t f: M.facets) {
            certainty[f] /= max_certainty;
            if(Numeric::is_nan(certainty[f])) {
                certainty[f] = 0.0;
            }
        }

        // Step 2: solve for sines and cosines 
        // (Periodic Global Parameterization)
        try {
            ProgressTask progress("Frames Smth.",4);
            for(index_t k=0; k<5; ++k) {
                solve_PGP(M,alpha_sincos,locked,1.0,certainty);
                progress.progress(k);
            }
        } catch(const TaskCanceled&) {
        }


        // Step 3: deduce the frame field from
        //  the solution of the linear system
        frames_.resize(M.facets.nb()*9);
        centers_.resize(M.facets.nb()*3);
        for(index_t f: M.facets) {
            double angle = atan2(
                             alpha_sincos[2*f+1],
                             alpha_sincos[2*f]
                           ) / symd;
            vec3 U = MeshFacetBasis(M,f).unproject(vec2(cos(angle),sin(angle)));
            vec3 W = normalize(Geom::mesh_facet_normal(M,f));
            vec3 V = cross(W,U);

            frames_[9*f+0] = U.x;
            frames_[9*f+1] = U.y;
            frames_[9*f+2] = U.z;
            frames_[9*f+3] = V.x;
            frames_[9*f+4] = V.y;
            frames_[9*f+5] = V.z;
            frames_[9*f+6] = W.x;
            frames_[9*f+7] = W.y;
            frames_[9*f+8] = W.z;

            vec3 g = Geom::mesh_facet_center(M,f);
            centers_[3*f+0] = g.x;
            centers_[3*f+1] = g.y;
            centers_[3*f+2] = g.z;
        }

	if(use_NN_ || volumetric) {
	    NN_ = NearestNeighborSearch::create(3, "default");
	    NN_->set_points(centers_.size()/3, centers_.data());
	}

        // Step 4: In volumetric mode, for each tet we find the nearest
        // facet and lookup the frame field from it.
        if(volumetric) {
            vector<double> new_frames(9*M.cells.nb());
            vector<double> new_centers(3*M.cells.nb());
            for(index_t t: M.cells) {
                vec3 g = Geom::mesh_tet_center(M,t);
                get_nearest_frame(g.data(), &new_frames[9*t]);
                new_centers[3*t+0] = g.x;
                new_centers[3*t+1] = g.y;
                new_centers[3*t+2] = g.z;
            }
            frames_.swap(new_frames);
            centers_.swap(new_centers);
            NN_->set_points(centers_.size()/3, centers_.data());
        }
    }




    void FrameField::scale_frame_vector(
        double* frame, const vec3& N, double s
    ) {
        index_t max_index = 0;
        double max_prod = -1e30;
        for(index_t i = 0; i < 3; ++i) {
            double cur_prod =
                ::fabs(
                    N.x * frame[3 * i] + 
                    N.y * frame[3 * i + 1] + 
                    N.z * frame[3 * i + 2]
                );
            if(cur_prod > max_prod) {
                max_prod = cur_prod;
                max_index = i;
            }
        }
        vec3 W = s*normalize(vec3(&frame[3*max_index]));
        frame[3*max_index  ]=W.x;
        frame[3*max_index+1]=W.y;
        frame[3*max_index+2]=W.z;
    }

    void FrameField::fix_frame(double* frame, const vec3& N) {
        index_t w_index=0;
        double max_prod = -1e30;
        for(index_t i=0; i<3; ++i) {
            double cur_prod = 
                ::fabs(N.x*frame[3*i]+N.y*frame[3*i+1]+N.z*frame[3*i+2]);
            if(cur_prod > max_prod) {
                max_prod = cur_prod;
                w_index = i;
            }
        }
        index_t u_index = (w_index + 1)%3;
        index_t v_index = (u_index + 1)%3;
        vec3 U = normalize(vec3(frame+3*u_index));
        vec3 V = normalize(vec3(frame+3*v_index));
        vec3 W = normalize(N);

        if(dot(cross(U,V),W) < 0.0) {
            U = -U;
        }

        U -= dot(U,W)*W;
        V -= dot(V,W)*W;

        U = normalize(U);
        V = normalize(V);


        vec3 X = normalize(U+V);
        vec3 Y = normalize(U-V);
        U = normalize(X+Y);
        V = normalize(X-Y);

        frame[0] = U.x;
        frame[1] = U.y;
        frame[2] = U.z;
        frame[3] = V.x;
        frame[4] = V.y;
        frame[5] = V.z;
        frame[6] = W.x;
        frame[7] = W.y;
        frame[8] = W.z;
    }
    

}

