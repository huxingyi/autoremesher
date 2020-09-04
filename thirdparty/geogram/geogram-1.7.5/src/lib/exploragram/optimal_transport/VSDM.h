/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
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
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
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
