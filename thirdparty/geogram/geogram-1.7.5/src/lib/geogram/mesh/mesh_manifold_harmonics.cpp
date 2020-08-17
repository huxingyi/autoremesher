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

#include <geogram/mesh/mesh_manifold_harmonics.h>
#include <geogram/basic/file_system.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/NL/nl.h>
#include <geogram/bibliography/bibliography.h>

namespace {
    using namespace GEO;

    /**
     * \brief Computes a coefficient of the P1 Laplacian.
     * \param[in] M a reference to a surface mesh
     * \param[in] f facet index
     * \param[in] v1 , v2 global indices of two vertices of \p f
     * \return the cotangent of the angle at the corner of \p f opposite to 
     *  \p v1 and \p v2
     */
    inline double P1_FEM_coefficient(
	const Mesh& M, index_t f, index_t v1, index_t v2
    ) {
	index_t v3 = NO_VERTEX;
	for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
	    index_t v = M.facets.vertex(f,lv);
	    if(v != v1 && v != v2) {
		v3 = v;
		break;
	    }
	}
	geo_assert(v3 != NO_VERTEX);
	const vec3& p1 = Geom::mesh_vertex(M,v1);
	const vec3& p2 = Geom::mesh_vertex(M,v2);
	const vec3& p3 = Geom::mesh_vertex(M,v3);
	vec3 V1=p1-p3;
	vec3 V2=p2-p3;
	return 1.0 / ::tan(Geom::angle(V1,V2));
    }


    /**
     * \brief Assemble the stiffness and mass matrices
     *  of the Laplacian in OpenNL.
     * \details This function is supposed to be called
     *  between nlBegin(NL_SYSTEM) and nlEnd().
     * \param[in] M a const reference to a surface mesh.
     * \param[in] discretization the discretization of the Laplace-Beltrami 
     *   operator, one of:
     *	 - COMBINATORIAL: 1.0 everywhere
     *	 - UNIFORM: combinatorial divided by node degree
     *	 - FEM_P1: linear finite elements
     *	 - FEM_P1_LUMPED: linear finite elements with lumped mass matrix
     */
    void assemble_Laplacian_matrices(
	const Mesh& M,
	LaplaceBeltramiDiscretization discretization
    ) {

	// Step 1: compute vertices degrees (used by
	// uniform weights).
	// **************************************************
	
	vector<index_t> v_degree;
	if(discretization == UNIFORM) {
	    v_degree.assign(M.vertices.nb(), 0);
	    for(index_t c: M.facet_corners) {
		index_t v = M.facet_corners.vertex(c);
		++v_degree[v];
	    }
	}

	// Sum of row coefficient associated with each vertex
	vector<double> v_row_sum(M.vertices.nb(), 0.0);
	
	// Step 2: compute stiffness matrix
	// **************************************************	
	
	nlMatrixMode(NL_STIFFNESS_MATRIX);	
	nlBegin(NL_MATRIX);

	for(index_t f: M.facets) {
	    index_t fnv = M.facets.nb_vertices(f);
	    for(index_t lv=0; lv<fnv; ++lv) {
		index_t v1 = M.facets.vertex(f,lv);
		index_t v2 = M.facets.vertex(f,(lv+1)%fnv);
		switch(discretization) {
		    case COMBINATORIAL: {
			double w = 1.0;
			nlAddIJCoefficient(v1,v2,w);
			v_row_sum[v1] += w;
		    } break;
		    case UNIFORM: {
			double w = 1.0 / double(v_degree[v1]);
			nlAddIJCoefficient(v1,v2,w);
			v_row_sum[v1] += w;
		    } break;
		    case FEM_P1:
		    case FEM_P1_LUMPED: {
			double w = 0.5 * P1_FEM_coefficient(M,f,v1,v2);
			nlAddIJCoefficient(v1,v2,w);
			nlAddIJCoefficient(v2,v1,w);
			v_row_sum[v1] += w;
			v_row_sum[v2] += w;
		    } break;
		}
	    }
	}
	for(index_t v: M.vertices) {
	    // Diagonal term is minus row sum
	    // plus small number to make M non-singular
	    nlAddIJCoefficient(v,v,-v_row_sum[v] + 1e-6);
	}
	nlEnd(NL_MATRIX);

	// Step 3: compute mass matrix
	// **************************************************	
	
	if(discretization == FEM_P1 || discretization == FEM_P1_LUMPED) {
	    nlMatrixMode(NL_MASS_MATRIX);
	    nlBegin(NL_MATRIX);
	    for(index_t f: M.facets) {
		index_t v1 = M.facets.vertex(f,0);
		index_t v2 = M.facets.vertex(f,1);
		index_t v3 = M.facets.vertex(f,2);
		const vec3& p1 = Geom::mesh_vertex(M,v1);
		const vec3& p2 = Geom::mesh_vertex(M,v2);
		const vec3& p3 = Geom::mesh_vertex(M,v3);
		double A = Geom::triangle_area(p1,p2,p3);
		    
		if(discretization == FEM_P1_LUMPED) {
		    
		    nlAddIJCoefficient(v1,v1,A/3.0);
		    nlAddIJCoefficient(v2,v2,A/3.0);
		    nlAddIJCoefficient(v3,v3,A/3.0);
		    
		} else if(discretization == FEM_P1) {
		    
		    nlAddIJCoefficient(v1,v2,A/12.0);
		    nlAddIJCoefficient(v1,v3,A/12.0);
		    nlAddIJCoefficient(v2,v3,A/12.0);
		    nlAddIJCoefficient(v2,v1,A/12.0);
		    nlAddIJCoefficient(v3,v1,A/12.0);
		    nlAddIJCoefficient(v3,v2,A/12.0);
		    
		    nlAddIJCoefficient(v1,v1,A/6.0);
		    nlAddIJCoefficient(v2,v2,A/6.0);
		    nlAddIJCoefficient(v3,v3,A/6.0);
		}
	    }
	    nlEnd(NL_MATRIX);
	}
    }
}


namespace GEO {



    void mesh_compute_manifold_harmonics(
	Mesh& M, index_t nb_eigens,
	LaplaceBeltramiDiscretization discretization,
	const std::string& attribute_name,
	double shift,
	bool print_spectrum
    ) {

	geo_cite("DBLP:conf/smi/Levy06");
	geo_cite("DBLP:journals/cgf/ValletL08");
	
	if(M.vertices.attributes().is_defined(attribute_name)) {
	    M.vertices.attributes().delete_attribute_store(attribute_name);
	}
	
	// Step 1: configure eigen solver
	// **************************************************
	
	
	if(!nlInitExtension("ARPACK")) {
	    Logger::err("MH")
		<< "Could not initialize OpenNL ARPACK extension"
		<< std::endl;
	    return;
	} 

	nlNewContext();
	
	nlEigenSolverParameteri(NL_EIGEN_SOLVER, NL_ARPACK_EXT);
	nlEigenSolverParameteri(NL_NB_VARIABLES, NLint(M.vertices.nb()));
	nlEigenSolverParameteri(NL_NB_EIGENS, (NLint)nb_eigens);
	nlEigenSolverParameterd(NL_EIGEN_SHIFT, shift);

	if(discretization == COMBINATORIAL) {
	    nlEigenSolverParameteri(NL_SYMMETRIC, NL_TRUE);
	}

	nlEnable(NL_VARIABLES_BUFFER);

	nlBegin(NL_SYSTEM);

	Attribute<double> eigen_vector;
	eigen_vector.create_vector_attribute(
	    M.vertices.attributes(), attribute_name, nb_eigens
	);
	
	for(index_t eigen=0; eigen<nb_eigens; ++eigen) {
	    // Bind directly the variables buffer to the attribute in
	    // the mesh, to avoid copying data.
	    nlBindBuffer(
		NL_VARIABLES_BUFFER,
		NLuint(eigen), 
		&eigen_vector[0] + eigen, // base address for eigenvector
		NLuint(sizeof(double)*nb_eigens) // number of bytes between two
		// consecutive components in current eigenvector
	    );
	}

	// Step 2: assemble matrices
	// *************************

	assemble_Laplacian_matrices(M, discretization);
	
	nlEnd(NL_SYSTEM);

	// Step 3: solve and cleanup
	// *************************
	
	nlEigenSolve();

	if(print_spectrum) {
	    for(index_t i=0; i<nb_eigens; ++i) {
		Logger::out("MH") << i << ":" << nlGetEigenValue(i)
				  << std::endl;
	    }
	}
	
	nlDeleteContext(nlGetCurrent());
    }


    void mesh_compute_manifold_harmonics_by_bands(
	Mesh& M, index_t nb_eigens,
	LaplaceBeltramiDiscretization discretization,
	ManifoldHarmonicsCallback callback,
	index_t nb_eigens_per_band,
	double initial_shift,
	void* client_data
    ) {
	
	// Step 1: configure eigen solver and assemble matrices
	// ****************************************************
	
	if(!nlInitExtension("ARPACK")) {
	    Logger::err("MH")
		<< "Could not initialize OpenNL ARPACK extension"
		<< std::endl;
	    return;
	} 

	nlNewContext();
	
	nlEigenSolverParameteri(NL_EIGEN_SOLVER, NL_ARPACK_EXT);
	nlEigenSolverParameteri(NL_NB_VARIABLES, NLint(M.vertices.nb()));
	nlEigenSolverParameteri(NL_NB_EIGENS, (NLint)nb_eigens_per_band);

	if(discretization == COMBINATORIAL) {
	    nlEigenSolverParameteri(NL_SYMMETRIC, NL_TRUE);
	}

	nlBegin(NL_SYSTEM);
	assemble_Laplacian_matrices(M, discretization);
	nlEnd(NL_SYSTEM);


	// Step 2: main loop
	// *****************

	double shift = initial_shift;
	index_t current_eigen = 0;
	index_t current_band = 0;
	double latest_eigen = 0.0;

	vector<double> eigen_vector(M.vertices.nb());
	
	for(;;) {
	    
	    bool compute_band = true;
	    while(compute_band) {
		Logger::out("MH")
		    << "Compute band, shift=" << shift << std::endl;
		nlEigenSolverParameterd(NL_EIGEN_SHIFT, shift);
		nlEigenSolve();
		compute_band = false;

		// Test whether the current band overlaps the previous one.
		// If this is not the case, go back (move shift towards zero)
		// a little bit.
		
		if(current_band != 0) {
		    if(::fabs(nlGetEigenValue(0)) > ::fabs(latest_eigen)) {
			Logger::out("MH")
			    << "Bands do no overlap (going back a little bit)"
			    << std::endl;
			shift -= 0.2*(
			    nlGetEigenValue(nb_eigens_per_band - 1) -
			    nlGetEigenValue(0)
			);
			compute_band = true;
		    }
		}
		
	    }
	    
	    for(index_t i=0; i<nb_eigens_per_band; ++i) {
		// Output all the eigenpairs with an eigenvalue that was
		// not previously seen (ignore the part of the current band
		// that overlaps the previous band).
		if(
		    current_eigen == 0 ||
		    ::fabs(nlGetEigenValue(i)) > ::fabs(latest_eigen)
		) {
		    latest_eigen = nlGetEigenValue(i);
		    for(index_t j: M.vertices) {
			eigen_vector[j] = nlMultiGetVariable(j,i);
		    }
		    callback(
			current_eigen,
			nlGetEigenValue(i), eigen_vector.data(), client_data
		    );
		    ++current_eigen;
		    if(current_eigen >= nb_eigens) {
			nlDeleteContext(nlGetCurrent());
			return;
		    }
		}
	    }

	    // Move to next band / next eigen shift.
	    ++current_band;
	    shift += 0.8 * (
		nlGetEigenValue(nb_eigens_per_band - 1) - nlGetEigenValue(0)
	    );
	}
    }

    
}

