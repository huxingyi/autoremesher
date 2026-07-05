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

#ifndef EXPLORAGRAM_OPTIMAL_TRANSPORT_VSDM
#define EXPLORAGRAM_OPTIMAL_TRANSPORT_VSDM

#include <exploragram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/numerics/optimizer.h>
#include <geogram/NL/nl_matrix.h>

namespace GEO {
    class Mesh;
    class ProgressTask;
    
    /**
     * \brief Voronoi Square Distance Minimization.
     * \details Fits a surface mesh to a target surface
     *  mesh. The algorithm is described in the following
     *  references:
     *  - Fitting Polynomial Surfaces to Triangular Meshes with
     *    Voronoi Squared Distance Minimization, Vindent Nivolers,
     *    Dong-Ming Yan and Bruno Levy, Intl. Meshing Roundtable 
     *    Conf. Proc., 2011
     *  - Centroidal Voronoi Tessellation of Line Segments and Graphs,
     *    Lin Lu, Bruno Levy and Wenping Wang, Eurographics conf. Proc.,
     *    2012
     */
    class EXPLORAGRAM_API VSDM {
      public:
	/**
	 * \brief VSDM constructor.
	 * \param[in] S the surface mesh to be fitted.
	 * \param[in] T the target surface mesh.
	 */
	VSDM(Mesh* S, Mesh* T);

	/**
	 * \brief VSDM destructor.
	 */
	virtual ~VSDM();

	/**
	 * \brief Sets the affinity.
	 * \param[in] x the affinity, the higher the smoother.
	 *  Default value is 1.0.
	 */
	void set_affinity(double x) {
	   affinity_ = x;
	}

	/**
	 * \brief Optimizes the fitting.
	 * \param[in] nb_iter maximum number of iterations.
	 */
	void optimize(index_t nb_iter); 

	/**
	 * \brief Sets an optional progress bar to track progress
	 *  during calls of optimize()
	 * \param[in] progress a pointer to a ProgressTask.
	 */
	void set_progress(ProgressTask* progress) {
	    progress_ = progress;
	}

	/**
	 * \brief Sets a subdivision surface. 
	 * \details If this function is called, then the S mesh passed to the
	 *  constructor is considered as the control mesh of a subdivision
	 *  surface.
	 * \param[in] mesh a pointer to the mesh that will store the
	 *  subdivision surface. 
	 * \param[in] nb_subdiv the number of Catmull-Clark subdivisions.
	 */
        void set_subdivision_surface(Mesh* mesh, index_t nb_subdiv);

	/**
	 * \brief The callback to evaluate the objective function, used by
	 *  the optimizer.
	 * \param[in] n dimension of the problem (here, three times the number
	 *  of vertices of the surface S).
	 * \param[in] x a pointer to the current vector of variables, of
	 *  dimension n.
	 * \param[out] f the value of the objective function.
	 * \param[out] g a pointer to a vector of dimension n with the 
	 *  gradient of the objective function.
	 */
        static void funcgrad_CB(index_t n, double* x, double& f, double* g);

	/**
	 * \brief The callback for each iteration of the optimizer.
	 * \param[in] n dimension of the problem (here, three times the number
	 *  of vertices of the surface S).
	 * \param[in] x a pointer to the current vector of variables, of
	 *  dimension n.
	 * \param[in] f the current value of the objective function.
	 * \param[in] g a pointer to a vector of dimension n with the 
	 *  gradient of the objective function.
	 * \param[in] gnorm the norm of the gradient.
	 */
	static void newiteration_CB(
	    index_t n, const double* x, double f, const double* g, double gnorm
	);

      protected:
	/**
	 * \brief Gets the instance.
	 * \return a pointer to the current VSDM instance.
	 */
	static VSDM* instance() {
	    return instance_;
	}

	/**
	 * \brief Evaluates the objective function and its gradient.
	 * \param[in] n dimension of the problem (here, three times the number
	 *  of vertices of the surface S).
	 * \param[in] x a pointer to the current vector of variables, of
	 *  dimension n.
	 * \param[out] f the value of the objective function.
	 * \param[out] g a pointer to a vector of dimension n with the 
	 *  gradient of the objective function.
	 */
        virtual void funcgrad(index_t n, double* x, double& f, double* g);

	/**
	 * \brief Updates the progress bar (if specified) and graphics.
	 * \details This function is called after each iteration of the 
	 *  optimizer.
	 */
	virtual void newiteration();

	/**
	 * \brief Adds the value and gradient of the affinity to the currently
	 *  evaluated objective function.
	 * \param[in] n dimension of the problem (here, three times the number
	 *  of vertices of the surface S).
	 * \param[in] x a pointer to the current vector of variables, of
	 *  dimension n.
	 * \param[in,out] f the value of the objective function.
	 * \param[in,out] g a pointer to a vector of dimension n with the 
	 *  gradient of the objective function.
	 */
	void add_funcgrad_affinity(index_t n, double* x, double& f, double* g);

	/**
	 * \brief Computes the graph Laplacian of a surface mesh.
	 * \param[in] S a pointer to a surface mesh.
	 * \param[out] L the graph Laplacian of S. 
	 */
	static void compute_graph_Laplacian(Mesh* S, NLSparseMatrix* L);
	
      protected:
	Delaunay_var delaunay_;
	RestrictedVoronoiDiagram_var RVD_;
	Mesh* S_;
	Mesh* T_;
	double affinity_;
	double affinity_scaling_;
	ProgressTask* progress_;
	static VSDM* instance_;
	Optimizer_var optimizer_;
	index_t nb_iter_;
	index_t cur_iter_;
	
	NLSparseMatrix L_;
	vector<double> temp_V1_;
	vector<double> temp_V2_;

	NLMatrix subd_matrix_;
	Mesh* subd_;
	vector<double> subd_g_;
    };
    
}

#endif
