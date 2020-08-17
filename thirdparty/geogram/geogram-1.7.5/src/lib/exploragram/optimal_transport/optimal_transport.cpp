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

#include <exploragram/optimal_transport/optimal_transport.h>
#include <exploragram/optimal_transport/linear_least_squares.h>

#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>

#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/delaunay/delaunay_3d.h>

#include <geogram/points/nn_search.h>
#include <geogram/numerics/optimizer.h>
#include <geogram/numerics/lbfgs_optimizers.h>

#include <geogram/basic/stopwatch.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/process.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/permutation.h>
#include <geogram/basic/command_line.h>

#include <geogram/NL/nl.h>

#include <geogram/bibliography/bibliography.h>


namespace GEO {

    OptimalTransportMap* OptimalTransportMap::instance_ = nullptr;

    OptimalTransportMap::Callback::~Callback() {
    }
    
    OptimalTransportMap::OptimalTransportMap(
	index_t dimension,
        Mesh* mesh, const std::string& delaunay, bool BRIO
    ) : mesh_(mesh) {

	geo_cite("DBLP:conf/compgeom/AurenhammerHA92");
	geo_cite("DBLP:journals/cgf/Merigot11");
	geo_cite("journals/M2AN/LevyNAL15");

	dimension_ = dimension;
	dimp1_ = dimension_+1;
	
        epsilon_regularization_ = 0.0;

        // Mesh is supposed to be embedded in d+1 dim (with
        // (d+1-th dimension set to zero).
        geo_assert(mesh->vertices.dimension() == dimp1_);

        // Note: we represent power diagrams as d+1 Voronoi diagrams
        delaunay_ = Delaunay::create(coord_index_t(dimp1_), delaunay);

        RVD_ = RestrictedVoronoiDiagram::create(delaunay_, mesh_);
        RVD_->set_volumetric(true);
        RVD_->set_check_SR(true);
        RVD_->create_threads();
        
        //   No need to reorder vertices if BRIO is activated since
        // vertices are then already reordered.
        if(BRIO) {
            RVD_->delaunay()->set_reorder(false);
        }

        newton_ = false;
	verbose_ = true;
        
        instance_ = nullptr;
        constant_nu_ = 0.0;
        total_mass_ = 0.0;
        current_call_iter_ = 0;
        epsilon_ = 0.01;
        level_ = 0;
        
        save_RVD_iter_ = false;
        save_RVD_last_iter_ = false;
        show_RVD_seed_ = false;
        current_iter_ = 0;
        
        pretty_log_ = false; // CmdLine::get_arg_bool("log:pretty");

        w_did_not_change_ = false;
        measure_of_smallest_cell_ = 0.0;
	callback_ = nullptr;
	Laguerre_centroids_ = nullptr;

	linsolve_epsilon_ = 0.001;
	linsolve_maxiter_ = 1000;

	linesearch_maxiter_ = 100;
	linesearch_init_iter_ = 0;

	linear_solver_ = OT_PRECG;

	nb_air_particles_ = 0;
	air_particles_ = nullptr;
	air_particles_stride_ = 0;
	air_fraction_ = 0.0;

	clip_by_balls_ = false;

	user_H_g_ = false;	
	user_H_ = nullptr;
    }

    OptimalTransportMap::~OptimalTransportMap() {
	delete callback_;
	callback_ = nullptr;
    }
    
    void OptimalTransportMap::set_points(
        index_t nb_points, const double* points, index_t stride
    ) {
	
	if(stride == 0) {
	    stride = dimension_;
	}

	index_t nb_total = nb_points + nb_air_particles_;
	
        // Note: we represent power diagrams as (d+1)-dim Voronoi diagrams.
        // The target points are lifted to (d+1)-dim.
        points_dimp1_.resize(nb_total * dimp1_);
        for(index_t i = 0; i < nb_points; ++i) {
	    double* p = &points_dimp1_[i*dimp1_];	    
	    for(index_t c=0; c<dimension_; ++c) {
		p[c] = points[i*stride+c];
	    }
            p[dimension()] = 0.0; // Yes, dimension() and not dimension()-1
	                          // (for instance, in 2d, x->0, y->1, W->2)
        }
	for(index_t i=0; i<nb_air_particles_; ++i) {
	    double* p = &points_dimp1_[(i + nb_points)*dimp1_];
	    for(index_t c=0; c<dimension_; ++c) {
		p[c] = air_particles_[i*air_particles_stride_+c];
	    }
            p[dimension()] = 0.0; // Yes, dimension() and not dimension()-1
	                          // (for instance, in 2d, x->0, y->1, W->2)
	}
        weights_.assign(nb_points, 0);
	constant_nu_ = (1.0 - air_fraction_) * total_mass_ / double(nb_points);
	if(air_fraction_ != 0.0 && nb_air_particles_ == 0) {
	    // constant_nu_ = pi*R^2 -> R^2 = constant_nu_ / pi
	    // TODO: volumetric case.
	    weights_.assign(nb_points, constant_nu_ / M_PI);
	}
    }

    void OptimalTransportMap::set_nu(index_t i, double nu) {
        geo_debug_assert(i < weights_.size());
        if(nu_.size() != weights_.size()) {
	    nu_.assign(weights_.size(), 0.0);
	}
	nu_[i] = nu;
    }
  
    void OptimalTransportMap::optimize_full_Newton(
        index_t max_iterations, index_t n
    ) {
        if(n == 0) {
            n = index_t(points_dimp1_.size() / dimp1_) - nb_air_particles_;
        }

        vector<double> pk(n);
        vector<double> xk(n);
        vector<double> gk(n);
        double fk=0.0;
        bool converged = false;

        double epsilon0 = 0.0;
        w_did_not_change_ = false;

	if(max_iterations == 0) {
	    if(Laguerre_centroids_ != nullptr) {
		callback_->set_Laguerre_centroids(Laguerre_centroids_);
	    }
	    funcgrad(n,weights_.data(),fk,gk.data());
	    if(Laguerre_centroids_ != nullptr) {
		callback_->set_Laguerre_centroids(nullptr);
	    }
	}

	// Inner iteration control for linesearch
	index_t first_inner_iter = linesearch_init_iter_;
	index_t inner_iter = first_inner_iter;
	bool use_inner_iter_prediction = (first_inner_iter != 0);
	
        for(index_t k=0; k<max_iterations; ++k) {
	    if(verbose_) {
		std::cerr << "======= k = " << k << std::endl;
	    }
            xk=weights_;

            new_linear_system(n,pk.data());
            eval_func_grad_Hessian(n,xk.data(),fk,gk.data());
            
            if(k == 0) {
                newiteration();
            }
            
            if(epsilon0 == 0.0) {
	        epsilon0 = measure_of_smallest_cell_;
	        FOR(i,n) {
		  epsilon0 = std::min(epsilon0, nu(i));
		}
		if(nb_air_particles_ != 0.0) {
		    epsilon0 = std::min(epsilon0, air_fraction_ * total_mass_);
		}
                epsilon0 = 0.5 * epsilon0;
            }

	    if(verbose_) {
		Logger::out("OTM") << "   Solving linear system" << std::endl;
	    }

	    if(nbZ_ != 0) {
		std::cerr << "There were empty cells !!!!!!" << std::endl;
		std::cerr << "FATAL error, exiting Newton" << std::endl;
		return;
	    }
            solve_linear_system();

	    if(verbose_) {
		std::cerr << "Line search ..." << std::endl;
	    }

            w_did_not_change_ = false;
            
            double alphak = 1.0;
            double gknorm = g_norm_;

	    if(first_inner_iter != 0) {
		alphak /= pow(2.0, double(first_inner_iter));
	    }
	    
            for(inner_iter=first_inner_iter;
		inner_iter < linesearch_maxiter_; ++inner_iter
	    ) {
		if(verbose_) {
		    std::cerr << "      inner iter = "
			      << inner_iter << std::endl;
		}

                // weights = xk + alphak pk
                for(index_t i=0; i<n; ++i) {
                    weights_[i] = xk[i] + alphak * pk[i];
                }

                // Compute cell measures and nbZ.
		if(Laguerre_centroids_ != nullptr) {
		    callback_->set_Laguerre_centroids(Laguerre_centroids_);
		}
		
		funcgrad(n,weights_.data(),fk,gk.data());

		if(Laguerre_centroids_ != nullptr) {
		    callback_->set_Laguerre_centroids(nullptr);
		}

		if(verbose_) {
		    std::cerr << "cell measure :"
			      << measure_of_smallest_cell_
			      << "(>=?)" << epsilon0 << std::endl;
		    std::cerr << "gradient norm:"
			      << g_norm_ << "(<=?)"
			      << (1.0 - 0.5*alphak) * gknorm << std::endl;
		}
		
		// Condition to exit linesearch loop
                if(
                    (measure_of_smallest_cell_ >= epsilon0)  && (
			(air_fraction_ != 0.0) || 
			(g_norm_ <= (1.0 - 0.5*alphak) * gknorm)
		    )
                ) {
		    // Condition for global convergence
                    if(g_norm_ < gradient_threshold(n)) {
                        converged = true;
                    }
                    break;
                } 
                // Else we halve the step.
                alphak /= 2.0;
            }

	    if(use_inner_iter_prediction) {
		if(inner_iter <= 2) {
		    first_inner_iter = 0;
		} else {
		    first_inner_iter = inner_iter / 2;
		}
	    } else {
		first_inner_iter = 0;
	    }
	    
            newiteration();
            if(converged) {
                break;
            }
            // No need to update the power diagram at next iteration,
            // since we will evaluate the Hessian for the same weight
            // vector.
            w_did_not_change_ = true;
        }
        if(save_RVD_last_iter_) {
            save_RVD(current_iter_);
        }
    }
    
    void OptimalTransportMap::optimize(index_t max_iterations) {

	index_t n = index_t(points_dimp1_.size() / dimp1_) - nb_air_particles_;
	
	// Sanity check
	if(nu_.size() != 0) {
	    double total_nu = 0.0;
	    FOR(i,n) {
		total_nu += nu(i);
	    }
	    if(verbose_) {
		std::cerr << "total nu=" << total_nu << std::endl;
		std::cerr << "total mass=" << total_mass_ << std::endl;
	    }
	    if(::fabs(total_nu - total_mass_)/total_mass_ > 0.01) {
		Logger::warn("OTM")
		    << "Specified nu do not sum to domain measure"
		    << std::endl;
		Logger::warn("OTM")
		    << "rescaling..."
		    << std::endl;
	    }
	    FOR(i,n) {
		set_nu(i, nu(i) * total_mass_ / total_nu);
	    }
	    
	    total_nu = 0.0;
	    FOR(i,n) {
		total_nu += nu(i);
	    }
	    if(::fabs(total_nu - total_mass_)/total_mass_ > 0.01) {
		Logger::warn("OTM") << "Specified nu do not sum to domain measure"
				    << std::endl;
		return;
	    }
	}
        
        if(newton_) {
            optimize_full_Newton(max_iterations);
            return;
        }

        level_ = 0;
        index_t m = 7;
        Optimizer_var optimizer = Optimizer::create("HLBFGS");
        
        optimizer->set_epsg(gradient_threshold(n));
        optimizer->set_epsf(0.0);
        optimizer->set_epsx(0.0);
        
        optimizer->set_newiteration_callback(newiteration_CB);
        optimizer->set_funcgrad_callback(funcgrad_CB);

        optimizer->set_N(n);
        optimizer->set_M(m);
        optimizer->set_max_iter(max_iterations);
        instance_ = this;
        current_call_iter_ = 0;
        current_iter_ = 0;
	
	callback_->set_eval_F(true);	
        optimizer->optimize(weights_.data());
	callback_->set_eval_F(false);
	
        instance_ = nullptr;
        // To make sure everything is reset properly
        double dummy = 0;
        funcgrad(n, weights_.data(), dummy, nullptr);
	if(verbose_) {
	    Logger::out("OTM")
		<< "Used " << current_call_iter_ << " iterations" << std::endl;
	}
        if(save_RVD_last_iter_) {
            save_RVD(current_iter_);
        }
    }


    void OptimalTransportMap::optimize_level(
        index_t b, index_t e, index_t max_iterations
    ) {

        // If this is not the first level, propagate the weights from
        // the lower levels.
        if(b != 0) {
            
            //   Create a nearest neighbor search data structure
            // and insert the [0..b) samples into it (they were
            // initialized at previous calls).
            NearestNeighborSearch_var NN =
		NearestNeighborSearch::create(coord_index_t(dimension()));
	    
            NN->set_points(b, points_dimp1_.data(), dimp1_);
            index_t degree = 2; // CmdLine::get_arg_uint("fitting_degree");
            
            // If degree \notin {1,2}, use weight of nearest sample
            if(degree < 1 || degree > 2) {
                for(index_t i = b; i < e; ++i) {
                    weights_[i] =
                        weights_[
                            NN->get_nearest_neighbor(&points_dimp1_[dimp1_ * i])
                        ];
                }
            } else {
                
                //   If degree \in {1,2} use linear least squares to
                // compute an estimate of the weight function.
                LinearLeastSquares LLS(degree);
                for(index_t i = b; i < e; ++i) {
                    const index_t nb = 10 * degree;
                    index_t neighbor[100];
                    double dist[100];
                    NN->get_nearest_neighbors(
                        nb, &points_dimp1_[dimp1_ * i], neighbor, dist
                    );
                    LLS.begin();
                    for(index_t jj = 0; jj < nb; ++jj) {
                        if(dist[jj] != 0.0) {
                            index_t j = neighbor[jj];
                            LLS.add_point(
				 &points_dimp1_[dimp1_ * j], weights_[j]
			    );
                        }
                    }
                    LLS.end();
                    weights_[i] = LLS.eval(&points_dimp1_[dimp1_ * i]);
                }
            }
        }
        
        // Optimize the weights associated with the sequence [0,e)
        index_t n = e;
        
        // Important! constant_nu_ (target measure of a cell) needs
        // to be updated, since it depends on the number of samples
        // (that varies at each level).
        constant_nu_ = total_mass_ / double(n);

        if(newton_) {
            optimize_full_Newton(max_iterations, n);
            return;
        }
        
        index_t m = 7;
        Optimizer_var optimizer = Optimizer::create("HLBFGS");

        optimizer->set_epsg(gradient_threshold(n));
        optimizer->set_epsf(0.0);
        optimizer->set_epsx(0.0);
        optimizer->set_newiteration_callback(newiteration_CB);
        optimizer->set_funcgrad_callback(funcgrad_CB);

        optimizer->set_N(n);
        optimizer->set_M(m);
        optimizer->set_max_iter(max_iterations);
        instance_ = this;
        current_call_iter_ = 0;
	callback_->set_eval_F(true);		
        optimizer->optimize(weights_.data());
	callback_->set_eval_F(false);			
        instance_ = nullptr;
        
        // To make sure everything is reset properly
        double dummy = 0;
        funcgrad(n, weights_.data(), dummy, nullptr);
    }

    void OptimalTransportMap::optimize_levels(
        const vector<index_t>& levels, index_t max_iterations
    ) {
	if(verbose_) {
	    if(levels.size() > 2) {
		Logger::out("OTM") << "Using " << levels.size()-1
				   << " levels" << std::endl;
	    } else {
		Logger::out("OTM") << "Using 1 level" << std::endl;
	    }
	}
        for(index_t l = 0; l + 1 < levels.size(); ++l) {
            level_ = l+1;
            index_t b = levels[l];
            index_t e = levels[l + 1];
            vector<index_t> brio_levels;
            for(index_t i=0; i<=l+1; ++i) {
                brio_levels.push_back(levels[i]);
            }
            RVD_->delaunay()->set_BRIO_levels(brio_levels);
            optimize_level(b, e, max_iterations);
        }
        if(save_RVD_last_iter_) {
            save_RVD(current_iter_);
        }
    }
    
    void OptimalTransportMap::funcgrad_CB(
        index_t n, double* x, double& f, double* g
    ) {
        instance_->funcgrad(n, x, f, g);
    }

    void OptimalTransportMap::newiteration_CB(
        index_t n, const double* x, double f, const double* g, double gnorm
    ) {
        geo_argused(n);
        geo_argused(x);
        geo_argused(f);
        geo_argused(g);
        geo_argused(gnorm);
        instance_->newiteration();
    }

    void OptimalTransportMap::newiteration() {
        //xxx std::cerr << "newiteration" << std::endl;
        if(save_RVD_iter_) {
	    if(verbose_) {
		std::cerr << "  save iter" << std::endl;
	    }
            save_RVD(current_iter_);
        }
        ++current_iter_;
    }

    void OptimalTransportMap::save_RVD(index_t id) {
	Mesh RVD_mesh;
	get_RVD(RVD_mesh);
	MeshIOFlags flags;
	flags.set_attribute(MESH_CELL_REGION);
	flags.set_attribute(MESH_FACET_REGION);            
	mesh_save(
	    RVD_mesh,
	    "RVD_" + String::to_string(id) + ".geogram",
	    flags
        );
    }

    void OptimalTransportMap::funcgrad(
        index_t n, double* w, double& f, double* g
    ) {

        bool is_Newton_step = callback_->is_Newton_step();	    
        
        // For now, always compute function and gradient
        bool update_fg = true;
        
        // Delaunay triangulation is only updated if function and
        // gradient is evaluated. If only Hessian needs to be evaluated,
        // then it is at the same point as the latest function and gradient
        // evaluation (see Yang Liu's CVT-Newton code).
        if(update_fg && !w_did_not_change_) {
            // Step 1: determine the (dim+1)d embedding from the weights
            double W = 0.0;
            for(index_t p = 0; p < n; ++p) {
                W = std::max(W, w[p]);
            }
	    for(index_t p = 0; p < n; ++p) {
		// Yes, dimension_ and not dimension_ -1,
		// for instance in 2d, x->0, y->1, W->2
		points_dimp1_[dimp1_ * p + dimension_] = ::sqrt(W - w[p]);
	    }
	    if(nb_air_particles_ != 0) {
		for(index_t p = 0; p < nb_air_particles_; ++p) {
		    points_dimp1_[dimp1_ * (n + p) + dimension_] = ::sqrt(W - 0.0);
		}		
	    }
		
            // Step 2: compute function and gradient
            {
		Stopwatch* SW = nullptr;
		if(newton_) {
		    if(verbose_) {
			SW = new Stopwatch("Power diagram");
			Logger::out("OTM") << "In power diagram..." << std::endl;
		    }
		}
                delaunay_->set_vertices((n + nb_air_particles_), points_dimp1_.data());
		if(verbose_ && newton_) {
		    delete SW;
		}
            }
        }

        if(is_Newton_step) {
            update_sparsity_pattern();
        }
        
        if(g == nullptr) {
            if(pretty_log_) {
                CmdLine::ui_clear_line();
                CmdLine::ui_message(last_stats_ + "\n");
            }
            return;
        }
        
        if(update_fg) {
            f = 0.0;
            for(index_t p = 0; p < n; ++p) {
                g[p] = 0.0;
            }
        }

	callback_->set_w(w,n);
	callback_->set_g(g);
	callback_->set_nb_threads(Process::maximum_concurrent_threads());

	if(callback_->has_Laguerre_centroids()) {
	    Memory::clear(callback_->Laguerre_centroids(), nb_points()*sizeof(double)*dimension());	    
	}
	
	{
	    Stopwatch* W = nullptr;
	    if(verbose_ && newton_) {
		W = new Stopwatch("RVD");
		Logger::out("OTM") << "In RVD (funcgrad)..." << std::endl;
	    }
	    call_callback_on_RVD();
	    if(verbose_ && newton_) {
		delete W;
	    }
	}
	f = callback_->funcval(); 

	if(callback_->has_Laguerre_centroids()) {
	    for(index_t v=0; v<nb_points(); ++v) {
		for(index_t c=0; c<dimension_; ++c) {
		    callback_->Laguerre_centroids()[dimension_*v+c] /= g[v];
		}
	    }
	}

        index_t nb_empty_cells = 0;       
        if(update_fg) {
            measure_of_smallest_cell_ = Numeric::max_float64();
            for(index_t i=0; i<n; ++i) {
                measure_of_smallest_cell_ =
                    std::min(measure_of_smallest_cell_, g[i]);
	        if(g[i] == 0.0) {
		    ++nb_empty_cells;
		}
            }
	    if(air_fraction_ != 0.0) {
		double air_mass = total_mass_;
		for(index_t i=0; i<n; ++i) {
		    air_mass -= g[i];
		}
		if(fabs(air_mass) < 1e-30) {
		    ++nb_empty_cells;
		}
		measure_of_smallest_cell_ =
		    std::min(measure_of_smallest_cell_, air_mass);
	    }
        }
        
        if(update_fg) {
            for(index_t p = 0; p < n; ++p) {
	      f += nu(p) * w[p];
                // Note: we minimize -f instead of maximizing f,
                // therefore, in the paper:
                //    g[p] = lambda_p - mesure(power cell associated with p)
                //
                // What is programmed:
                //    g[p] = mesure(power cell associated with p) - lambda_p
	      g[p] -= nu(p);

                if(is_Newton_step) {
                    // Newton step: solve H deltax = -g
                    // (note the minus sign on the right hand side)
                    // g[p] -= lamnda_p_ -> RHS[p] += constant_nu_
		  add_i_right_hand_side(p, nu(p));
                }
            }
        }

        double max_diff = 0.0;
        double avg_diff = 0.0;

        for(index_t p = 0; p < n; ++p) {
            double cur_diff = ::fabs(g[p]);
            max_diff = std::max(max_diff, cur_diff);
            avg_diff += cur_diff / double(n);
        }

        
        // Regularisation: minimize the squared norm of the weight
        // vector to remove a translational degree of freedom.
        // It seems to make the overall convergence slower
        // (but this may be due to a wrong
        // scaling between the different levels, to be investigated...)
        if(epsilon_regularization_ != 0.0) {
            if(update_fg) {                                    
                for(index_t p = 0; p < n; ++p) {
		  f += 0.5 * epsilon_regularization_ * nu(p) * w[p]*w[p];
		  g[p] += epsilon_regularization_ * nu(p) * w[p];
                }
            }
            if(is_Newton_step) {
                for(index_t p = 0; p < n; ++p) {
                    add_ij_coefficient(
  		        p,p,epsilon_regularization_*nu(p)
                    );
                    add_i_right_hand_side(
 		        p,-epsilon_regularization_*nu(p)*w[p]
                    );
                }
            }
        }

        
        double gNorm = 0.0;
        for(index_t i = 0; i < n; ++i) {
            gNorm += geo_sqr(g[i]);
        }
        gNorm = ::sqrt(gNorm);

        nbZ_ = nb_empty_cells;
        
        std::ostringstream str;
        if(pretty_log_) {
            if(level_ == 0) {
                str << "o-[OTM         ] " ;
            } else {
                str << "o-[OTM Lvl." << level_ << "   ] " ;
            }
        } else {
            if(level_ == 0) {
                str << "   OTM      : " ;
            } else {
                str << "   OTM Lvl." << level_ << ": " ;
            }
        }

	double scl = 100.0 / constant_nu_;
	
        str << "iter=" << current_call_iter_
            << " nbZ=" << nb_empty_cells
            //                << " f=" << f
	    << " avg_diff=" << (avg_diff * scl) << "%"
	    << " max_diff=" << (max_diff * scl) << "%"
            << " g=" << gNorm
//          << " f=" << f 
            << " threshold=" << gradient_threshold(n);
        last_stats_ = str.str();

        g_norm_ = gNorm;
        
        // "custom task progress" (clears the standard message
        // and replaces it with another one).
	if(verbose_) {
	    if(pretty_log_) {
		if(current_call_iter_ != 0) {
		    CmdLine::ui_clear_line();
		}
		CmdLine::ui_message(str.str());
	    } else {
//		str << " f=" << f;
		CmdLine::ui_message(str.str() + "\n");
	    }
	}
        ++current_call_iter_;
    }

    void OptimalTransportMap::eval_func_grad_Hessian(
        index_t n, const double* w, double& f, double* g
    ) {
	callback_->set_Newton_step(true);
	funcgrad(n,const_cast<double*>(w),f,g);
	callback_->set_Newton_step(false);
    }
    
/************************************************************/

    // TODO: in the Euler code, see if we do not have duplicated computations, i.e.
    //    - Power diagrams when leaving and entering iteration ?
    //    - Centroids: do we restart a RVD computation ?
    // TODO: are we obliged to create/destroy OpenNL context for each system ?
    // TODO: we could have an OpenNL buffer for the RHS of the Newton solve ?
    //   Not really, because this is *minus* the gradient.
    
    void OptimalTransportMap::update_sparsity_pattern() {
        // Does nothing for now,
	// (we let OpenNL discover the sparsity pattern)
	// Tryed smarter things, but was not faster...
    }

    void OptimalTransportMap::new_linear_system(index_t n, double* x) {
        nlNewContext();
            
        if(linear_solver_ != OT_PRECG) {
	    if(linear_solver_ == OT_SUPERLU) {
		if(nlInitExtension("SUPERLU") != NL_TRUE) {
		    linear_solver_ = OT_PRECG;
		}
	    } else if(linear_solver_ == OT_CHOLMOD) {
		if(nlInitExtension("CHOLMOD") != NL_TRUE) {
		    linear_solver_ = OT_PRECG;
		}
	    }
	    if(linear_solver_ == OT_PRECG) {
		Logger::warn("OTM") << "Could not initialize OpenNL extension"
				    << std::endl;
		Logger::warn("OTM") << "Falling back to conjugate gradient"
				    << std::endl;
	    }
        }

	if(verbose_) {
	    nlEnable(NL_VERBOSE);
	}
	
        nlSolverParameteri(NL_NB_VARIABLES, NLint(n));
	switch(linear_solver_) {
	    case OT_PRECG:
		nlSolverParameteri(NL_SOLVER, NL_CG);
		nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_JACOBI);
		nlSolverParameteri(NL_SYMMETRIC, NL_TRUE);
		nlSolverParameterd(NL_THRESHOLD, linsolve_epsilon_);
		nlSolverParameteri(NL_MAX_ITERATIONS, NLint(linsolve_maxiter_));

		break;
	    case OT_SUPERLU:
		nlSolverParameteri(NL_SOLVER, NL_PERM_SUPERLU_EXT);		
		break;
	    case OT_CHOLMOD:
		nlSolverParameteri(NL_SOLVER, NL_CHOLMOD_EXT);		
		break;
	}
	nlEnable(NL_VARIABLES_BUFFER);
	nlEnable(NL_NO_VARIABLES_INDIRECTION); 
        nlBegin(NL_SYSTEM);
	nlBindBuffer(NL_VARIABLES_BUFFER, 0, x, NLuint(sizeof(double)));
        nlBegin(NL_MATRIX);
    }

    void OptimalTransportMap::solve_linear_system() {
        nlEnd(NL_MATRIX);
        nlEnd(NL_SYSTEM);
        nlSolve();
        if(verbose_) {
            int used_iters;
            double elapsed_time;
            double gflops;
            double error;
            nlGetIntegerv(NL_USED_ITERATIONS, &used_iters);
            nlGetDoublev(NL_ELAPSED_TIME, &elapsed_time);
            nlGetDoublev(NL_GFLOPS, &gflops);
            nlGetDoublev(NL_ERROR, &error);
	    std::cerr << "   "
		      << used_iters << " iters in "
		      << elapsed_time << " seconds "
		      << gflops << " GFlop/s"
		      << "  ||Ax-b||/||b||="
		      << error
		      << std::endl;
	}
        nlDeleteContext(nlGetCurrent());
    }

    void OptimalTransportMap::compute_P1_Laplacian(
	const double* w, NLMatrix Laplacian, double* measures
    ) {
	index_t n = index_t(points_dimp1_.size() / dimp1_) - nb_air_particles_;

	if(measures != nullptr) {
	    Memory::clear(measures, n*sizeof(double));
	}

	user_H_g_ = true;
	user_H_ = Laplacian;
	callback_->set_Newton_step(true);

	// Step 1: determine the (dim+1)d embedding from the weights
	double W = 0.0;
	for(index_t p = 0; p < n; ++p) {
	    W = std::max(W, w[p]);
	}
	for(index_t p = 0; p < n; ++p) {
	    // Yes, dimension_ and not dimension_ -1,
	    // for instance in 2d, x->0, y->1, W->2
	    points_dimp1_[dimp1_ * p + dimension_] = ::sqrt(W - w[p]);
	}

	if(nb_air_particles_ != 0) {
	    for(index_t p = 0; p < nb_air_particles_; ++p) {
		points_dimp1_[dimp1_ * (n + p) + dimension_] = ::sqrt(W - 0.0);
	    }		
	}
		
	// Step 2: compute Laplacian and cell measures.
	delaunay_->set_vertices((n + nb_air_particles_), points_dimp1_.data());
	callback_->set_w(w,n);
	callback_->set_g(measures);
	callback_->set_nb_threads(Process::maximum_concurrent_threads());
	call_callback_on_RVD();

	callback_->set_Newton_step(false);
	user_H_g_ = false;
	user_H_ = nullptr;
    }


    
    /**********************************************************************/
}
