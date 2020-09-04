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

#ifndef GEOGRAM_MESH_MESH_MANIFOLD_HARMONICS
#define GEOGRAM_MESH_MESH_MANIFOLD_HARMONICS

/**
 * \file geogram/mesh/mesh_manifold_harmonics.h
 */

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>

namespace GEO {
    class Mesh;

    enum LaplaceBeltramiDiscretization {
	COMBINATORIAL,    // 1.0 everywhere
	UNIFORM,          // combinatorial divided by node degree
	FEM_P1,           // Linear finite elements
	FEM_P1_LUMPED     // Linear finite elements with lumped mass matrix
    };

    /**
     * \brief Computes the Manifold Harmonics basis functions
     * \details The computed eigenvectors are stored in a vertex attribute.
     * \param[in] M a reference to a surface mesh
     * \param[in] nb_eigens number of eigenfunctions to compute
     * \param[in] discretization the discretization of the Laplace-Beltrami 
     *   operator, one of:
     *	 - COMBINATORIAL: 1.0 everywhere
     *	 - UNIFORM: combinatorial divided by node degree
     *	 - FEM_P1: linear finite elements
     *	 - FEM_P1_LUMPED: linear finite elements with lumped mass matrix
     * \param[in] shift eigen shift applied to explore a certain part
     *  of the spectrum.
     * \param[in] print_spectrum if true, prints eigenvalues to the terminal.
     */
    void GEOGRAM_API mesh_compute_manifold_harmonics(
	Mesh& M, index_t nb_eigens,
	LaplaceBeltramiDiscretization discretization,
	const std::string& attribute_name = "eigen",
	double shift = 0.0,
	bool print_spectrum = false
    );


    /**
     * \brief A function pointer to be used with
     *  mesh_compute_manifold_harmonics_by_bands()
     */
    typedef void (*ManifoldHarmonicsCallback)(
	index_t eigen_index,
	double eigen_val, const double* eigen_vector,
	void* client_data
    );

    /**
     * \brief Computes Laplacian eigenfunctions band by band.
     * \details This function should be used when a large number
     *  of eigenfunctions should be computed.
     * \param[in] M a const reference to a surface mesh
     * \param[in] nb_eigens total number of eigenpairs to compute
     * \param[in] callback the client function to be called for 
     *  each computed eigenpair
     * \param[in] nb_eigens_per_band the number of eigenpairs to
     *  be computed in each band
     * \param[in] initial_shift the initial location in the spectrum
     * \param[in] client_data a pointer passed to the client callback
     */
    void GEOGRAM_API mesh_compute_manifold_harmonics_by_bands(
	Mesh& M, index_t nb_eigens,
	LaplaceBeltramiDiscretization discretization,
	ManifoldHarmonicsCallback callback,
	index_t nb_eigens_per_band = 30,
	double initial_shift = 0.0,
	void* client_data = nullptr
    );
    
}

#endif


