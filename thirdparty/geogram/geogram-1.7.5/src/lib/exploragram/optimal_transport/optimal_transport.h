
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
 * As an exception to the GPL, Graphite can be linked with 
 *     the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_H

#include <exploragram/basic/common.h>

struct NLMatrixStruct;
typedef NLMatrixStruct* NLMatrix;

namespace GEO {
    /**
     * \brief Specifies the linear solver to be used
     *  with OptimalTransport.
     */
    enum OTLinearSolver {
	OT_PRECG, OT_SUPERLU, OT_CHOLMOD
    };
}

#ifndef GOMGEN

#include <geogram/mesh/mesh.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/NL/nl.h>
#include <geogram/NL/nl_matrix.h>
#include <geogram/third_party/HLBFGS/HLBFGS.h>

/**
 * \file exploragram/optimal_transport/optimal_transport.h
 * \brief Base class for semi-discrete optimal transport.
 */

namespace GEO {
    class CentroidalVoronoiTesselation;


    /**
     * \brief Computes semi-discrete optimal transport maps.
     * \details Computes an optimal transport map between two
     *  distributions. The first distribution is represented
     *  by a simplicial mesh. The second distribution is a sum
     *  of Diracs.
     *  The algorithm is described in the following references:
     *   - 3D algorithm: http://arxiv.org/abs/1409.1279
     *   - Earlier 2D version by Quentin M\'erigot: 
     *    Q. Merigot. A multiscale approach to optimal transport.
     *    Computer Graphics Forum 30 (5) 1583--1592, 2011 (Proc SGP 2011).
     *   - Earlier article on OT and power diagrams: 
     *    F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
     *    and least-squares clustering. Algorithmica, 20:61-76, 1998.
     */
    class EXPLORAGRAM_API OptimalTransportMap {
    public:
        /**
         * \brief OptimalTransportMap constructor.
         * \param[in] mesh the source distribution, represented as a nd mesh.
         * \param[in] delaunay factory name of the Delaunay triangulation.
         * \param[in] BRIO true if vertices are already ordered using BRIO
         */
        OptimalTransportMap(
	    index_t dimension,
            Mesh* mesh,
            const std::string& delaunay = "default",
            bool BRIO = false
        );

	/**
	 * \brief OptimalTransportMap destructor.
	 */
	virtual ~OptimalTransportMap();

	/**
	 * \brief Gets the dimension.
	 * \return 2 for 2d, 3 for 3d.
	 */
	index_t dimension() const {
	    return dimension_;
	}
	
        /**
         * \brief Gets the mesh.
         * \return a reference to the mesh
         */
        Mesh& mesh() {
            return *mesh_;
        }

        /**
         * \brief Sets whether Newton algorithm should be used.
         * \details It is (for now) incompatible with multilevel.
         * \param[in] x if set, Newton algorithm is used instead
         *  of BFGS. 
         */
        void set_Newton(bool x) {
            newton_ = x;
        }
        
        /**
         * \brief Sets the points that define the target distribution.
	 * \details If air particles are used, then set_air_particles() needs
	 *  to be called before set_points().
         * \param[in] nb_points number of points in the target distribution
         * \param[in] points an array of size nb_points * dimension() with the
	 *  coordinates of the Diracs centers in the target
         *  distribution.
	 * \param[in] stride number of doubles between two consecutive points.
	 *  If 0 (default), then point coordinates are considered to be packed.
         */
        void set_points(
	    index_t nb_points, const double* points, index_t stride=0
	);

	/**
	 * \brief Sets the air particles that define the volume occupied by the
	 *  free space.
	 * \details If air particles are used, then set_air_particles() needs
	 *  to be called before set_points().
	 * \param[in] nb_air_particles number of air particles.
	 * \param[in] air_particles a pointer to the array of doubles with the
	 *  coordinates of the air particles.
	 * \param[in] stride number of doubles between two consecutive air
	 *  particles in the array, or 0 if tightly packed.
	 * \param[in] air_fraction the fraction of the total mass occupied by air.
	 */
	void set_air_particles(
	    index_t nb_air_particles, const double* air_particles, index_t stride,
	    double air_fraction
	) {
	    nb_air_particles_ = nb_air_particles;
	    air_particles_ = air_particles;
	    air_particles_stride_ = (stride == 0) ? dimension_ : stride;
	    air_fraction_ = air_fraction;
	    // "continuous air" mode (air fraction declared without any air
	    // particle).
	    clip_by_balls_ = (nb_air_particles == 0) && (air_fraction != 0.0);
	}

	/**
	 * \brief Gets the air fraction.
	 * \return the air fraction previously specified by set_air_particles()
	 */
	double air_fraction() const {
	    return air_fraction_;
	}

	/**
	 * \brief Gets the number of air particles.
	 * \return the air fraction previously specified by set_air_particles()
	 */
	index_t nb_air_particles() const {
	    return nb_air_particles_;
	}
	
	/**
	 * \brief Sets the desired mass at one of the Diracs.
	 * \details If unspecified, then default value is total mass
	 *  divided by number of point. Note that the sum of all specified
	 *  masses should match the total mass.
	 * \param[in] i the index of the dirac, in 0 .. nb_points-1
	 * \param[in] nu the desired mass at point i
	 */
	void set_nu(index_t i, double nu);
	
	
	/**
	 * \brief Specifies a user vector where the centroids of the Laguerre
	 *  cells will be stored after computing transport.
	 * \param[out] x a vector of nb points * dimension() doubles.
	 */
	void set_Laguerre_centroids(double* x) {
	    Laguerre_centroids_ = x;
	}
	
        /**
         * \brief Sets the maximum error.
         * \param eps acceptable relative deviation for the measure of a
         *   Voronoi cell.
         */
        void set_epsilon(double eps) {
            epsilon_ = eps;
        }


	/**
	 * \brief Sets the tolerance for linear solve.
	 * \param[in] eps the maximum value of 
	 *   \f$ \| Ax - b \| / \| b \| \f$b
	 */
	void set_linsolve_epsilon(double eps) {
	    linsolve_epsilon_ = eps;
	}

	/**
	 * \brief Sets the maximum number of iterations
	 *  for linear solve.
	 * \param[in] maxiter the maximum number of iterations.
	 */
	void set_linsolve_maxiter(index_t maxiter) {
	    linsolve_maxiter_ = maxiter;
	}

	/**
	 * \brief Sets the maximum number of line search iterations.
	 * \param[in] maxiter the maximum number of step length reduction
	 *  for line search.
	 */
	void set_linesearch_maxiter(index_t maxiter) {
	    linesearch_maxiter_ = maxiter;
	}

	/**
	 * \brief Sets the number of steplength reductions to be
	 *  done at the first iteration.
	 * \param[in] init_iter the number of steplength reductions to
	 *  be apllied at the first iteration. If left 0, start with
	 *  Newton step at each iteration, else do tentative steplength
	 *  prediction.
	 */
	void set_linesearch_init_iter(index_t init_iter) {
	    linesearch_init_iter_ = init_iter;
	}
	
        /**
         * \brief Sets the weight of the regularization term.
         * \details The regularization term (norm of the weight vector) cancels
         *  the translational degree of freedom of the weights.
         * \param[in] eps_reg the weight of the regularization term. Use 0.0 for
         *  no regularization.
         */
        void set_regularization(double eps_reg) {
            epsilon_regularization_ = eps_reg;
        }


	/**
	 * \brief Specifies whether a direct solver should be used.
	 * \param[in] solver one of OT_PRECG (default), OT_SUPERLU, OT_CHOLMOD.
	 * \details The direct solvers (OT_SUPERLU, OT_CHOLMOD) are recommended only for
	 *  surfacic data, since the sparse factors become not so sparse when
	 *  volumetric meshes are considered.
	 */
	void set_linear_solver(OTLinearSolver solver) {
	    linear_solver_ = solver;
	}
	
        /**
         * \brief Computes the weights that realize the optimal
         *  transport map between the source mesh and the target
         *  pointset.
         * \param[in] max_iterations maximum number of solver iterations.
         */
        void optimize(index_t max_iterations);

	/**
	 * \brief Enable/disable messages during optimization.
	 * \param[in] x true if messages should be displayed, false
	 *  otherwise.
	 */
	void set_verbose(bool x) {
	    verbose_ = x;
	}
	
        /**
         * \brief Computes the weights that realize the optimal
         *  transport map between the source mesh and the target
         *  pointset.
	 * \details The algorithm is described in http://arxiv.org/abs/1603.05579
	 *   Kitawaga, Merigot, Thibert, A Newton Algorithm for semi-discrete OT.
         * \param[in] max_iterations maximum number of solver iterations.
         * \param[in] n number of weights to optimize, used in hierarchical
         *  mode. If zero, optimizes all the weights.
         */
        void optimize_full_Newton(index_t max_iterations, index_t n=0);
        
        /**
         * \brief Optimizes one level of the multilevel algorithm.
         * \details The function supposes that the sequence [0,b)
         *  has been previously optimized. It is used to initialize
         *  the sequence [b,e). The whole sequence [0,e) is then
         *  optimized.
         * \param[in] b index fo the first point in the level
         * \param[in] e one position past the last index of the level
         * \param[in] max_iterations maximum number of iterations
         */
        void optimize_level(index_t b, index_t e, index_t max_iterations);

        /**
         * \brief Multi-level optimization.
         * \details The points specified by set_points() need to have
         *   a hierarchical structure. They can be constructed by
         *   compute_hierarchical_sampling().
         * \param[in] levels sample indices that correspond to level l are
         *   in the range levels[l] (included) ... levels[l+1] (excluded)
         * \param[in] max_iterations maximum number of iterations
         * \see compute_hierarchical_sampling()
         */
        void optimize_levels(
            const vector<index_t>& levels, index_t max_iterations
        );

        /**
         * \brief Gets the number of points.
         * \return The number of points, that was previously defined
         *  by set_points()
         */
        index_t nb_points() const {
            return weights_.size();
        }

        /**
         * \brief Gets a point.
         * \param[in] i index of the point
         * \return a const pointer to the coordinates of the 
	 *  (dimension()+1)d point \p i
         */
        const double* point_ptr(index_t i) const {
            geo_debug_assert(i < nb_points());
            return &(points_dimp1_[dimp1_ * i]);
        }

        /**
         * \brief Gets weight of a point.
         * \param[in] i index of the point
         * \return the weight that was computed for point \p i
         */
        double weight(index_t i) const {
            return weights_[i];
        }

        /**
         * \brief Sets a weight of a point.
         * \param[in] i index of the point
         * \param[in] val new value of the weight
         */
        void set_weight(index_t i, double val) {
            weights_[i] = val;
        }
        
        /**
         * \brief Gets the d+1-th coordinate of the embedding for a point.
         * \param[in] i index of the point
         * \return the d+1-th coordinate that was computed for point \p i
         */
        double potential(index_t i) const {
            return points_dimp1_[dimp1_*i + dimension_];
        }

        /**
         * \brief Callback for the numerical solver.
         * \details Evaluates the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        static void funcgrad_CB(
            index_t n, double* x, double& f, double* g
        );

        /**
         * \brief Callback for the numerical solver.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[in] f current value of the objective function
         * \param[in] g gradient of the objective function
         * \param[in] gnorm norm of the gradient of the objective function
         */
        static void newiteration_CB(
            index_t n, const double* x, double f, const double* g, double gnorm
        );

        /**
         * \brief Gets the restricted Voronoi diagram.
         * \return a pointer to the restricted Voronoi diagram
         */
        RestrictedVoronoiDiagram* RVD() {
            return RVD_;
        }

        /**
         * \brief Sets whether the restricted Voronoi diagram at
         *  each iteration should be saved.
         * \details If flag is set, then each iteration is saved 
         *  in file "RVD_nnn.geogram".
         * \param[in] x true if each iteration should be saved, 
         *  false otherwise.
         * \param[in] show_RVD_seed if true, the seed associated
         *  with each restricted Voronoi cell is connected to it
         * \param[in] last_iter_only if true, only the last iteration
         *  is saved
         */
        void set_save_RVD_iter(
            bool x, 
            bool show_RVD_seed = false,
            bool last_iter_only = false
        ) {
            if(last_iter_only) {
                save_RVD_iter_ = false;
                save_RVD_last_iter_ = true;
            } else {
                save_RVD_iter_ = x;
            }
            show_RVD_seed_ = show_RVD_seed;
        }

	/**
	 * \brief Computes a mesh with the restricted Voronoi diagram.
	 * \param[out] M a reference to the computed restricted Voronoi diagram.
	 */
        virtual void get_RVD(Mesh& M) = 0;

        /**
         * \brief Computes the centroids of the Laguerre cells.
         * \param[out] centroids a pointer to the dimension()*nb_points 
	 *  coordinates of the centroids.
         */
        virtual void compute_Laguerre_centroids(double* centroids) = 0;

        /**
         * \brief Updates the sparsity pattern of the Hessian right after
         *  a new Laguerre diagram was computed.
         */
        void update_sparsity_pattern();
        
        /**
         * \brief Starts a new linear system.
         * \param[in] n the dimension of the system
	 * \param[in] x pointer to a contiguous array of \p n doubles,
	 *  where the solution will be stored.
         */
        void new_linear_system(index_t n, double* x);

        /**
         * \brief Adds a coefficient to the matrix of the system.
         * \param[in] i , j the indices of the coefficient
         * \param[in] a the value to be added to the coefficient
         */
        void add_ij_coefficient(index_t i, index_t j, double a) {
	    if(!user_H_g_) {
		nlAddIJCoefficient(i,j,a);
	    } else {
		if(user_H_ != nullptr) {
		    geo_debug_assert(user_H_->type == NL_MATRIX_SPARSE_DYNAMIC);
		    nlSparseMatrixAdd((NLSparseMatrix*)user_H_, i, j, a);
		}
	    }
	}

        /**
         * \brief Adds a coefficient to the right hand side.
         * \param[in] i the index of the coefficient
         * \param[in] a the value to be added to the coefficient
         */
        void add_i_right_hand_side(index_t i, double a) {
	    if(!user_H_g_) {
		nlAddIRightHandSide(i,a);
	    } 
	}
        
        /**
         * \brief Solves a linear system.
         * \details The solution is stored in the vector that 
	 *  was previously specified to new_linear_system().
         */
        void solve_linear_system();

	/**
	 * \brief Sets the initial value of the weight associated
	 *  with one of the points.
	 * \param[in] i index of the point, in 0..nb_points-1, where
	 *  np_points corresponds to the parameter of set_points.
	 * \param[in] w the value of the weight.
	 */
	void set_initial_weight(index_t i, double w) {
	    weights_[i] = w;
	}

	/**
	 * \brief Gets the total mass of the domain.
	 * \return the total mass. 
	 */
	double total_mass() const {
	    return total_mass_;
	}

	/**
	 * \brief Computes the P1 Laplacian of the Laguerre cells.
	 * \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[in] weights the weights of the Laguerre diagram.
	 * \param[out] Laplacian P1 Laplacian of the Laguerre diagram or nullptr if 
	 *  not needed.
	 * \param[out] measures optional measures the measures of 
	 *  all Laguerre cells, or nullptr if not needed.
	 */
	void compute_P1_Laplacian(
	    const double* weights, NLMatrix Laplacian, double* measures
	);
	
    protected:

	/**
	 * \brief Gets the mass of the Dirac associated with point p.
	 * \return the desired mass at point p.
	 */
	double nu(index_t p) const {
	  return nu_.size() == 0 ? constant_nu_ : nu_[p];
	}

	
        /**
         * \brief Callback for the numerical solver.
         */
        virtual void newiteration();
        
        /**
         * \brief Saves the RVD at each iteration if
         *   specified on command line (just for debugging/
         *   explaining the algorithm).
         * \param[in] id index to be used for the file, that
         *   will be named RVD_id.meshb
         */
        void save_RVD(index_t id);

        /**
         * \brief Computes the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] w current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        void funcgrad(index_t n, double* w, double& f, double* g);

	/**
	 * \brief Calls the callback for each intersection between a 
	 *  Laguerre cell and a simplex of the background mesh.
	 */
	virtual void call_callback_on_RVD() = 0;
	
        /**
         * \brief Computes the objective function, its gradient and its Hessian.
         * \details Gradient and Hessian are used to solve a Newton
         *  step H p = -g
         * \param[in] n number of variables
         * \param[in] w current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        void eval_func_grad_Hessian(
            index_t n, const double* w,
            double& f, double* g
        );

        /**
         * \brief Computes the stopping criterion of the solver.
         * \details The stopping criterion is determined from
         *  the user-specified epsilon, number of samples and
         *  target measure of a cell (constant_nu_).
         * \param n number of samples
         * \return the gradient threshold
         * \see set_epsilon()
         */
        double gradient_threshold(index_t n) const {
            return ::sqrt(double(n) * geo_sqr(epsilon_ * constant_nu_));
        }

      public:

	/**
	 * \brief Base class for the callbacks executed for each intersection
	 *  between a Laguerre cell and a simplex of the background mesh.
	 */
	 class Callback {
	 public:
	    /**
	     * \brief Callback constructor.
	     * \param[in] OTM a pointer to the OptimalTransportMap
	     */
	     Callback(
	      OptimalTransportMap* OTM
	     ) : OTM_(OTM),
	        Newton_step_(false),
		eval_F_(false),
	        n_(0),
	        w_(nullptr),
	        g_(nullptr),
		mg_(nullptr) {
		weighted_ =
		    OTM->mesh().vertices.attributes().is_defined("weight");
	    }

	   /**
	    * \brief Callback destructor.
	    */
	    virtual ~Callback();

	   /**
	    * \brief Sets where centroids should be output.
	    * \details This computes mass times centroid. The mass can
	    *  be retreived (and used to divide) from the gradient.
	    * \param[in] mg a pointer to the dimension()*nb_points coordinates
	    *  of the centroids times the mass of the Laguerre cells
	    */
	    void set_Laguerre_centroids(double* mg) {
		mg_ = mg;
	    }

	   /**
	    * \brief Tests whether Laguerre centroids should be computed.
	    * \retval true if Laguerre centroids should be computed.
	    * \retval false otherwise.
	    */
	    bool has_Laguerre_centroids() const {
		return (mg_ != nullptr);
	    }

	    /**
	     * \brief Gets a pointer to the Laguerre centroids.
	     * \return a pointer to nb vertices * dimension doubles
	     *  with the centroids of the Laguerre cells times the mass
	     *  of the Laguerre cells.
	     */
	    double* Laguerre_centroids() {
		return mg_;
	    }
	
	   /**
	    * \brief Sets the weight vector
	    * \param[in] w a const pointer to the weight vector.
	    * \param[in] n the number of weights in the weight vector.
	    */
	    void set_w(const double* w, index_t n) { 
		w_ = w;
		n_ = n;
	    }

	    /**
	     * \brief Specifies whether current step is a Newton step.
	     * \details If it is a Newton step, then the Hessian is
	     *  computed.
	     * \param[in] Newton true if the current step is a Newton
	     *  step, false otherwise.
	     */
	    void set_Newton_step(bool Newton) {
		Newton_step_ = Newton;
	    }

	    /**
	     * \brief Tests whether the current step is a Newton step.
	     * \retval true if the current step is a Newton step.
	     * \retval false otherwise.
	     */
	    bool is_Newton_step() const {
		return Newton_step_;
	    }

	    /**
	     * \brief Specifies the number of threads.
	     * \details This allocates one function value per thread.
	     * \param[in] nb the number of threads.
	     */
	    void set_nb_threads(index_t nb) {
		funcval_.assign(nb, 0.0);	    
	    }

	    /**
	     * \brief Specifies where the gradient should be stored.
	     * \param[in] g a pointer to an array of nb points doubles.
	     */
	    void set_g(double* g) {
		g_ = g;
	    }

	    /**
	     * \brief Specifies whether the objective function should 
	     *  be evaluated.
	     * \details The Newton solver does not need evaluating the
	     *  objective function, only the BFGS solver needs it.
	     * \param[in] x true if the objective function should be
	     *  evaluated, false otherwise. Default is false.
	     */
	    void set_eval_F(bool x) {
		eval_F_ = x;
	    }
	    
	    /**
	     * \brief Gets the computed value of the objective function.
	     * \details This sums the contributions of all threads.
	     * \retval the value of the objective function.
	     */
	    double funcval() const {
		double result = 0.0;
		FOR(i,funcval_.size()) {
		    result += funcval_[i];
		}
		return result;
	    }

	  protected:
	    OptimalTransportMap* OTM_;
	    bool weighted_;
	    bool Newton_step_;
	    bool eval_F_;
	    vector<double> funcval_;
	    index_t n_;
	    const double* w_;
	    double* g_;
	    double* mg_;
	};
	
      protected:
        static OptimalTransportMap* instance_;
	index_t dimension_;
	index_t dimp1_; /**< \brief dimension_ + 1 */
        Mesh* mesh_;
        Delaunay_var delaunay_;
        RestrictedVoronoiDiagram_var RVD_;
        vector<double> points_dimp1_;
        vector<double> weights_;
        double total_mass_;
        double constant_nu_; /**< \brief Value of one of the Diracs if cte. */
	vector<double> nu_;  /**< \brief Value of all the Diracs. */
        double epsilon_;
        /**< \brief Acceptable relative deviation for the measure of a cell */
        index_t current_call_iter_;

	Callback* callback_;
	
        std::string last_stats_;
        bool pretty_log_;
        index_t level_;

        bool save_RVD_iter_;
        bool save_RVD_last_iter_;
        bool show_RVD_seed_;
        index_t current_iter_;
        bool newton_;
	bool verbose_;

        /**
         * \brief Add a regularization term to remove 
         *  translational degree of freedom for the
         *  weights.
         */
        double epsilon_regularization_;

        /**
         * \brief Number of empty cells in last iteration.
         */
        index_t nbZ_;

        /**
         * \brief Norm of the gradient in last iteration.
         */
        double g_norm_;

        /**
         * \brief Measure of the smallest Laguerre cell.
         */
        double measure_of_smallest_cell_;

        /**
         * \brief True if w did not change, thus there is 
         *  no need to recompute the power diagram.
         */
        bool w_did_not_change_;

	/** \brief If user-specified, then Laguerre centroids are output here */
	double* Laguerre_centroids_;

	/** \brief maximum value of \f$ \| Ax - b \| / \| b \| \f$ */
	double linsolve_epsilon_;
	
	/** \brief maximum number of iterations for linear solve */
	index_t linsolve_maxiter_;

	/** \brief maximum number of steplength divisions */
	index_t linesearch_maxiter_;

	/** \brief starting number of steplength divisions */
	index_t linesearch_init_iter_;

	/** \brief one of OT_PRECG, OT_SUPERLU, OT_CHOLMOD. */
	OTLinearSolver linear_solver_;

	/** \brief if set, pointer to the air particles. */
	const double* air_particles_;
	
	/** \brief if non-zero, number of air particles. */
	index_t nb_air_particles_;

	/** 
	 * \brief Number of doubles between two consecutive
	 *  air particles in air_particles_.
	 */
	index_t air_particles_stride_;

	/**
	 * \brief The fraction of the total mass occupied by air.
	 */
	double air_fraction_;

	/**
	 * \brief Enabled if air fraction is specified without any
	 *  air particles.
	 */
	bool clip_by_balls_;


	/**
	 * \brief True if class is just used by user to compute
	 *  Hessian and gradient instead of doing full computation.
	 */
	bool user_H_g_;
	
	/**
	 * \brief User-defined Hessian matrix.
	 */
	NLMatrix user_H_;
    };

}

#endif

#endif
