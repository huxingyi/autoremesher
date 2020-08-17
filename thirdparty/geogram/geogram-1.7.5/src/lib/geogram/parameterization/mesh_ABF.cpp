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

#include <geogram/parameterization/mesh_ABF.h>
#include <geogram/parameterization/mesh_LSCM.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/bibliography/bibliography.h>
#include <geogram/basic/memory.h>

#include <geogram/NL/nl.h>

// Uses OpenNL internal data structures and routines
// (NLSparseMatrix and associated functions).
extern "C" {
#include <geogram/NL/nl_matrix.h>    
}

namespace {
    using namespace GEO;

    class ABFPlusPlus {
    public:
	ABFPlusPlus(Mesh& mesh) :
	    mesh_(mesh),
	    epsilon_(1e-5),
	    newton_tolf_(1.0),
	    newton_tolx_(1.0),
	    max_newton_iter_(10),
	    positive_angle_ro_(1.2),
	    step_length_factor_(1.0) {
	    v_on_border_.assign(mesh.vertices.nb(),false);
	    v_to_c_.assign(mesh.vertices.nb(),NO_VERTEX);
	    next_c_around_v_.assign(mesh.facet_corners.nb(), NO_CORNER);
	    for(index_t c: mesh.facet_corners) {
		index_t v = mesh.facet_corners.vertex(c);
		if(mesh.facet_corners.adjacent_facet(c) == NO_FACET) {
		    v_on_border_[v] = true;
		}
		next_c_around_v_[c] = v_to_c_[v];
		v_to_c_[v] = c;
	    }
	    if(!mesh_.facets.are_simplices()) {
		c_to_f_.resize(mesh_.facet_corners.nb());
		for(index_t f: mesh_.facets) {
		    for(index_t c: mesh_.facets.corners(f)) {
			c_to_f_[c] = f;
		    }
		}
	    }
	    verbose_ = false;
	}

	~ABFPlusPlus() {
	    deallocate_variables();
	}

	void set_verbose(bool x) {
	    verbose_ = x;
	}
	
	bool parameterize() {
	    geo_cite("DBLP:journals/tog/ShefferLMB05");	    
	    allocate_variables();
	    compute_beta();
	    angle_.bind(mesh_.facet_corners.attributes(),"angle");
	    if(!solve_angles()) {
		if(verbose_) {
		    Logger::err("ABF++") << "Did not converge." << std::endl ; 
		    Logger::err("ABF++") << "Switching to LSCM" << std::endl ;
		}
		// Note: AnglesToUV with angles measured on the mesh
		//  (i.e. beta's) = LSCM !!!
		for(index_t c: mesh_.facet_corners) {
		    angle_[c] = beta_[c];
		}
	    } 
	    deallocate_variables() ;
	    angle_.unbind();
	    return true ;
	}

    protected:
	index_t c_to_f(index_t c) {
	    geo_debug_assert(c < mesh_.facet_corners.nb());
	    return mesh_.facets.are_simplices() ? (c/3) : c_to_f_[c];
	}
	
	index_t nb_interior_vertices(const Mesh& M) const {
	    index_t result=0;
	    for(index_t v: M.vertices) {
		if(!v_on_border_[v]) {
		    ++result;
		}
	    }
	    return result;
	}
	
	void allocate_variables() {
	    // ------- sizes & indexes ------
	    nf_ = mesh_.facets.nb();
	    nalpha_ = mesh_.facet_corners.nb();
	    nint_ = nb_interior_vertices(mesh_);
	    nlambda_ = nf_ + 2*nint_;
	    ntot_ = nalpha_ + nlambda_;

	    // ------- ABF variables --------
	    alpha_.resize(nalpha_) ;
	    lambda_.resize(nlambda_) ;
	    beta_.resize(nalpha_) ;
	    w_.resize(nalpha_) ;

	    // ------- Step vectors ---------
	    dalpha_.resize(nalpha_) ;
	    dlambda1_.resize(nf_) ;
	    dlambda2_.resize(2*nint_) ;

	    // ------- Gradients ------------
	    b1_.resize(nalpha_) ;
	    b2_.resize(nlambda_) ;

	    // ------- Jacobian -------------
	    nlSparseMatrixConstruct(
		&J2_, NLuint(2*nint_), NLuint(nalpha_), NL_MATRIX_STORE_COLUMNS
	    );

	    // ------- ABF++ ----------------
	    nlSparseMatrixConstruct(
		&J_star_, NLuint(2*nint_), NLuint(nf_), NL_MATRIX_STORE_COLUMNS
	    );
	    nlSparseMatrixConstruct(
		&M_, NLuint(2*nint_), NLuint(2*nint_), NL_MATRIX_STORE_ROWS
	    );
	}

	void deallocate_variables() {
	    // ------- ABF variables --------
	    alpha_.clear() ;
	    lambda_.clear() ;
	    beta_.clear() ;
	    w_.clear() ;

	    // ------- Step vectors ---------
	    dalpha_.clear() ;
	    dlambda1_.clear() ;
	    dlambda2_.clear() ;

	    // ------- Gradients ------------
	    b1_.clear() ;
	    b2_.clear() ;
	    nlSparseMatrixDestroy(&J2_);
	    
	    // ------- ABF++ ----------------
	    nlSparseMatrixDestroy(&J_star_);
	    nlSparseMatrixDestroy(&M_);
	}


	void compute_beta() {

	    for(index_t v: mesh_.vertices) {
		// Compute sum_angles
		double sum_angle = 0.0 ;
		{
		    index_t c = v_to_c_[v];
		    do {
			double angle = corner_angle(c) ;
			sum_angle += angle ;
			c = next_c_around_v_[c];
		    } while(c != NO_CORNER) ; 
		}

		double ratio = 1.0 ;
        
		if(!v_on_border_[v]) {
		    ratio =  2.0 * M_PI / sum_angle ;
		}
            
		{
		    index_t c = v_to_c_[v];
		    do {
			beta_[c] = corner_angle(c) * ratio ;
			beta_[c] = std::max(beta_[c], 3.0 * M_PI / 180.0);
			beta_[c] = std::min(beta_[c], 175.0 * M_PI / 180.0);
			c = next_c_around_v_[c];
		    } while(c != NO_CORNER) ; 
		}
	    }
	}

	double corner_angle(index_t c) {
	    index_t f = c_to_f(c);
	    index_t c_prev = mesh_.facets.prev_corner_around_facet(f,c);
	    index_t c_next = mesh_.facets.next_corner_around_facet(f,c);

	    const vec3& p1 =
		Geom::mesh_vertex(mesh_, mesh_.facet_corners.vertex(c));
	    const vec3& p2 =
		Geom::mesh_vertex(mesh_, mesh_.facet_corners.vertex(c_next));
	    const vec3& p3 =
		Geom::mesh_vertex(mesh_, mesh_.facet_corners.vertex(c_prev));
	    
	    double result = Geom::angle(p2-p1,p3-p1);
	    result = std::max(result, 2.0 * M_PI / 360.0) ;
	    return result ;
	}

	
	bool solve_angles() {

	    if(!nlInitExtension("SUPERLU")) {
		Logger::warn("ABF++")
		    << "Could not initialize SuperLU extension"
		    << std::endl;
		return false;
	    }
	    
	    // Initial values
	    lambda_.assign(lambda_.size(),0.0);
	    for(index_t i=0; i<nalpha_; i++) {
		alpha_[i] = beta_[i] ;
		w_[i] = 1.0 / (beta_[i] * beta_[i]) ;
	    }

	    for(index_t k=0; k<max_newton_iter_; k++) {

		// Compute Jacobian
		nlSparseMatrixZero(&J2_);
		add_JC2();
		add_JC3();

		// Compute rhs
		b1_.assign(b1_.size(), 0.0);
		b2_.assign(b2_.size(), 0.0);
		sub_grad_F();
		sub_grad_C1();
		sub_grad_C2();
		sub_grad_C3();

		double errf_k = errf() ;


		if(verbose_) {
		    Logger::out("ABF++")
			<< "iter= " << k << " errf= " << errf_k 
			<< std::endl ;
		}

		
		if(Numeric::is_nan(errf_k) || errf_k > 1e18) {
		    return false ;
		}


		if(errf_k <= newton_tolf_) {
		    if(verbose_) {
			Logger::out("ABF++") << "converged" << std::endl ;
		    }
		    return true ;
		}

		solve_current_iteration() ;

		double errx ;
		double s = compute_step_length_and_update_weights() ;
		s *= step_length_factor_ ;
		errx = compute_errx_and_update_x(s) ;

		// TODO: since errx is dependent on the size of the mesh,
		// weight the threshold by the size of the mesh.

		if(Numeric::is_nan(errx) || errx > 1e15) {
		    if(verbose_) {
			Logger::err("ABF++") << "errx: " << errx << std::endl ;
		    }
		    return false ;
		}

		if(verbose_) {
		    Logger::out("ABF++") << "iter= " << k << " errx= " << errx
					 << std::endl ;
		}

		if(errx <= newton_tolx_) {
		    if(verbose_) {
			Logger::out("ABF++") << "converged" << std::endl ;
		    }
		    return true ;
		}

		if(angle_.is_bound()) {
		    for(index_t c: mesh_.facet_corners) {
			angle_[c] = alpha_[c];
		    }
		}
	    }


	    for(index_t c: mesh_.facet_corners) {
		if(Numeric::is_nan(alpha_[c])) {
		    return false;
		}
	    }
	    
	    
	    if(verbose_) {
		Logger::out("ABF++") << "ran out of Newton iters" << std::endl ;
	    }
	    return true ;
	}

	void solve_current_iteration() {
        
	    Delta_inv_.resize(nalpha_) ;
            for(index_t i=0; i<nalpha_; i++) {
                Delta_inv_[i] = 1.0 / (2.0 * w_[i]) ;
            }

        
	    // 1) Create the pieces of J.Delta^-1.Jt
	    // 1.1) Diagonal part: Delta*^-1
	    Delta_star_inv_.resize(nf_) ;
	    for(index_t f=0; f<nf_; ++f) {
		double S = 0.0;
		for(index_t c: mesh_.facets.corners(f)) {
		    S += Delta_inv_[c];
		}
		Delta_star_inv_[f] =  1.0 / S;
	    }

	    // 1.2) J*  = J2.Delta^-1.J1^t
	    nlSparseMatrixZero(&J_star_);
	    for(index_t j=0; j<nalpha_; j++) {
		const NLRowColumn& Cj = J2_.column[j];
		for(NLuint ii=0; ii<Cj.size; ++ii) {
		    const NLCoeff& c = Cj.coeff[ii] ;
		    nlSparseMatrixAdd(
			&J_star_,
			NLuint(c.index), NLuint(c_to_f(j)),
			c.value * Delta_inv_[j]
		    );
		}
	    }
	    // Note: J** does not need to be built, it is directly added to M.

	    // 2) Right hand side: b1* and b2*

	    // 2.1) b1* = J1.Delta^-1.b1 - b2[1..nf]
	    b1_star_.resize(nf_);
	    
	    for(index_t f=0; f<nf_; ++f) {
		b1_star_[f] = -b2_[f];
		for(index_t c: mesh_.facets.corners(f)) {
		    b1_star_[f] += Delta_inv_[c] * b1_[c];
		}
	    }
	    
	    // 2.2) b2* = J2.Delta^-1.b1 - b2[nf+1 .. nf+2.nint-1]
	    b2_star_.assign(2*nint_,0.0);
	    add_J_D_x(b2_star_, J2_, Delta_inv_, b1_);
	    for(index_t i=0; i<2*nint_; i++) {
		b2_star_[i] -= b2_[nf_+i];
	    }


	    // 3) create final linear system 
        
	    // 3.1) M = J*.Delta*^-1.J*^t - J**
	    //       where J** = J2.Delta^-1.J2^t
	    nlSparseMatrixZero(&M_);
	    add_J_D_Jt(M_, J_star_, Delta_star_inv_) ;
	    sub_J_D_Jt(M_, J2_, Delta_inv_) ;

	    // 3.2) r = J*.Delta*^-1.b1* - b2*
	    r_.assign(2*nint_,0.0) ;
	    add_J_D_x(r_, J_star_, Delta_star_inv_, b1_star_);

	    geo_debug_assert(r_.size() == b2_star_.size());
	    for(index_t i=0; i<r_.size(); ++i) {
		r_[i] -= b2_star_[i];
	    }

	    if(verbose_) {
		Logger::out("ABF++") << "Solving linear system..." << std::endl;
	    }
	    NLMatrix Minv =
		nlMatrixFactorize((NLMatrix)&M_, NL_PERM_SUPERLU_EXT);
	    nlMultMatrixVector(Minv, r_.data(), dlambda2_.data());
	    nlDeleteMatrix(Minv);
	    if(verbose_) {
		Logger::out("ABF++") << "Solved" << std::endl;
	    }
	    
	    // 4) compute dlambda1 and dalpha in function of dlambda2
	    
	    // 4.1) dlambda1 = Delta*^-1 ( b1* - J*^t dlambda2 )
	    mult_transpose(J_star_, dlambda2_, dlambda1_) ;
	    for(index_t f=0; f<nf_; ++f) {
		dlambda1_[f] =
		    Delta_star_inv_[f] * (b1_star_[f] - dlambda1_[f]) ;
	    }
	    
	    // 4.2) Compute dalpha in function of dlambda:
	    // dalpha = Delta^-1( b1 -  J^t.dlambda                    )
	    //        = Delta^-1( b1 - (J1^t.dlambda1 + J2^t.dlambda2) )
	    mult_transpose(J2_, dlambda2_, dalpha_) ;

	    for(index_t f=0; f<nf_; ++f) {
		for(index_t c: mesh_.facets.corners(f)) {
		    dalpha_[c] += dlambda1_[f];
		}		
	    }

	    for(index_t i=0; i<nalpha_; i++) {
		dalpha_[i] = Delta_inv_[i] * (b1_[i] - dalpha_[i]) ;
	    }
	}


	double compute_errx_and_update_x(double s) {
	    double result = 0 ;

	    // alpha += s * dalpha 
	    for(index_t i=0; i<nalpha_; i++) {
		double dai = s * dalpha_[i];
		alpha_[i] += dai ;
		result += ::fabs(dai) ;
	    }

	    // lambda += s * dlambda
	    for(index_t i=0; i<nf_; i++) {
		double dai = s * dlambda1_[i];
		lambda_[i] += dai ;
		result += ::fabs(dai) ;
	    }
	    
	    for(index_t i=0; i<2*nint_; i++) {
		double dai = s * dlambda2_[i];
		lambda_[nf_+i] += dai ;
		result += ::fabs(dai) ;
	    }
	    return result ;
	}

	
	// --------------------- Jacobian ----------------------------

	void add_JC2() {
	    index_t i = 0 ;
	    for(index_t v: mesh_.vertices) {
		if(v_on_border_[v]) {
		    continue ;
		}
		index_t c = v_to_c_[v];
		do {
		    nlSparseMatrixAdd(&J2_, NLuint(i), NLuint(c), 1.0);
		    c = next_c_around_v_[c];
		} while(c != NO_CORNER);
		i++ ;
	    }
	}
	
	void add_JC3() {
            index_t i = nint_ ;
	    for(index_t v: mesh_.vertices) {
                if(v_on_border_[v]) {
                    continue ;
                }
                double prod_prev_sin ;
                double prod_next_sin ;
                compute_product_sin_angles(v, prod_prev_sin, prod_next_sin) ;
		index_t c = v_to_c_[v];
                do {
		    index_t f = c_to_f(c);
		    index_t next_c = mesh_.facets.next_corner_around_facet(f,c);
		    nlSparseMatrixAdd(
			&J2_, NLuint(i), NLuint(next_c),
			prod_next_sin * cos(alpha_[next_c])/sin(alpha_[next_c])
		    );
		    index_t prev_c = mesh_.facets.prev_corner_around_facet(f,c);
		    nlSparseMatrixAdd(
			&J2_, NLuint(i), NLuint(prev_c),
		       -prod_prev_sin * cos(alpha_[prev_c])/sin(alpha_[prev_c])
		    );
		    c = next_c_around_v_[c];
                } while(c != NO_CORNER);
                i++ ;
            }
	}
	
	// --------------------- Right-hand side ---------------------

	void sub_grad_F() {
	    for(index_t i = 0; i < nalpha_; ++i) {
		b1_[i] -= 2.0 * w_[i] * ( alpha_[i] - beta_[i] );
	    }
	}
	
	// For each facet: sum angles - PI * (nb_vertices(f)-2)
	void sub_grad_C1() {
	    for(index_t f=0; f < nf_; ++f) {
		for(index_t c: mesh_.facets.corners(f)) {
		    b1_[c] -= lambda_[f];
		}
	    }
	    for(index_t f=0; f < nf_; ++f) {
		b2_[f] += M_PI * double(mesh_.facets.nb_vertices(f)-2);
		for(index_t c: mesh_.facets.corners(f)) {
		    b2_[f] -= alpha_[c];
		}
	    }
	}
	
	void sub_grad_C2() {
	    index_t i = nf_ ;
	    for(index_t v: mesh_.vertices) {
		if(v_on_border_[v]) {
		    continue ;
		}
		index_t c = v_to_c_[v];
		do {
		    b2_[i] -= alpha_[c];
		    b1_[c] -= lambda_[i];
		    c = next_c_around_v_[c];
		} while(c != NO_CORNER) ;
		b2_[i] += 2.0 * M_PI ;
		++i;
	    }
	}
	
	
	// For each vertex: prod sin(next angle) - prod sin(prev angle)
	void sub_grad_C3() {
            index_t i = nf_ + nint_ ;
	    for(index_t v: mesh_.vertices) {
		if(v_on_border_[v]) {
                    continue ;
                }
		
                double prod_prev_sin ;
                double prod_next_sin ;
                compute_product_sin_angles(v, prod_prev_sin, prod_next_sin) ;
                
                b2_[i] -= prod_next_sin - prod_prev_sin ;

		index_t c = v_to_c_[v];
                do {
		    index_t f = c_to_f(c);
		    index_t next_c = mesh_.facets.next_corner_around_facet(f,c);
                    b1_[next_c] -= 
                        lambda_[i] * prod_next_sin *
			cos(alpha_[next_c]) / sin(alpha_[next_c]) ;
                    
		    index_t prev_c = mesh_.facets.prev_corner_around_facet(f,c);
                    b1_[prev_c] += 
                        lambda_[i] * prod_prev_sin *
			cos(alpha_[prev_c]) / sin(alpha_[prev_c]) ;
                    
		    c = next_c_around_v_[c];
                } while(c != NO_CORNER);
                i++ ;
            }
        }

	// -------------------------------------------------------
	
	void compute_product_sin_angles(
	    index_t v, double& prod_prev_sin, double& prod_next_sin
	) {
	    prod_prev_sin = 1.0 ;
	    prod_next_sin = 1.0 ;
	    index_t c = v_to_c_[v];
	    do {
		index_t f = c_to_f(c);
		index_t prev_c = mesh_.facets.prev_corner_around_facet(f,c);
		index_t next_c = mesh_.facets.next_corner_around_facet(f,c);
		prod_prev_sin *= sin(alpha_[prev_c]);
		prod_next_sin *= sin(alpha_[next_c]);
		c = next_c_around_v_[c];
	    } while(c != NO_CORNER) ;
	}
	
	// --------------------- Convergence control -----------------

	double compute_step_length_and_update_weights() {
	    double ratio = 1.0 ;
	    for(index_t i=0; i<nalpha_; i++) {
		if(alpha_[i] + dalpha_[i] < 10.0 * epsilon_) {
		    double r1 = -.5 * (alpha_[i] - 10.0 * epsilon_)/dalpha_[i];
		    ratio = std::min(ratio, r1) ;
		    w_[i] *= positive_angle_ro_ ;
		} else if(alpha_[i] + dalpha_[i] > M_PI - 10.0 * epsilon_) {
		    // double r1 =
		    //	.5*(M_PI - alpha_[i]+10.0 * epsilon_)/dalpha_[i];
                    // ratio = ogf_min(ratio, r1) ;
		    // two previous lines commented-out, I'm unsure why, to be
		    // tested.
		    w_[i] *= positive_angle_ro_ ;
		}
	    }
	    return ratio ;
	}
	
	double errf() const {
	    double result = 0 ;
	    for(index_t i=0; i<nalpha_; ++i) {
		result += ::fabs(b1_[i]);
	    }
	    for(index_t i=0; i<nlambda_; ++i) {
		result += ::fabs(b2_[i]);
	    }
	    return result ;
	}

	
	// -------------------- Matrix utilities ---------------------

	static void mult_transpose(
	    const NLSparseMatrix& M,
	    const vector<double>& x,
	    vector<double>& y
	) {
	    geo_debug_assert(y.size() == M.n);
	    geo_debug_assert(x.size() == M.m);
	    if((M.storage & NL_MATRIX_STORE_COLUMNS) != 0) {
		for(NLuint j=0; j<M.n; ++j) {
		    y[j] = 0.0;
		    const NLRowColumn& Cj = M.column[j];
		    for(NLuint ii=0; ii<Cj.size; ++ii) {
			double a = Cj.coeff[ii].value;
			index_t i = Cj.coeff[ii].index;
			y[j] += a * x[i];
		    }
		}
	    } else {
		geo_assert((M.storage & NL_MATRIX_STORE_ROWS) != 0);
		y.assign(y.size(), 0.0);
		for(NLuint i=0; i<M.m; ++i) {
		    const NLRowColumn& Ci = M.row[i];
		    for(NLuint jj=0; jj<Ci.size; ++jj) {		    
			double a = Ci.coeff[jj].value;
			index_t j = Ci.coeff[jj].index;
			y[j] += a * x[i];
		    }
		}
	    }
	}
	
	static void add_J_D_x(
	    vector<double>& y, 
	    const NLSparseMatrix& J,
	    const vector<double>& D,
	    const vector<double>& x
	) {
	    geo_debug_assert(y.size() == J.m) ;
	    geo_debug_assert(D.size() == J.n) ;
	    geo_debug_assert(x.size() == J.n) ;
	    
	    for(NLuint j=0; j<D.size(); ++j) {
		const NLRowColumn& Cj = J.column[j] ;
		for(NLuint ii=0; ii<Cj.size; ++ii) {
		    const NLCoeff& c = Cj.coeff[ii] ;
		    y[c.index] += c.value * D[j] * x[j] ;
		}
	    }
	}
    
	static void add_J_D_Jt(
	    NLSparseMatrix& M,
	    const NLSparseMatrix& J,
	    const vector<double>& D
	) {
	    geo_debug_assert(M.m == J.m) ;
	    geo_debug_assert(M.n == J.m) ;
	    geo_debug_assert(D.size() == J.n) ;
	    
	    for(NLuint j=0; j<D.size(); j++) {
		const NLRowColumn& Cj = J.column[j];
		for(NLuint ii1=0; ii1<Cj.size; ii1++) {        
		    for(NLuint ii2=0; ii2<Cj.size; ii2++) {
			nlSparseMatrixAdd(
			    &M,
			    NLuint(Cj.coeff[ii1].index),
			    NLuint(Cj.coeff[ii2].index),
			    Cj.coeff[ii1].value * Cj.coeff[ii2].value * D[j]
			);
		    }
		}
	    }
	}

	static void sub_J_D_Jt(
	    NLSparseMatrix& M,
	    const NLSparseMatrix& J,
	    const vector<double>& D
	) {
	    geo_debug_assert(M.m == J.m) ;
	    geo_debug_assert(M.n == J.m) ;
	    geo_debug_assert(D.size() == J.n) ;
	    
	    for(NLuint j=0; j<D.size(); j++) {
		const NLRowColumn& Cj = J.column[j];
		for(NLuint ii1=0; ii1<Cj.size; ii1++) {        
		    for(NLuint ii2=0; ii2<Cj.size; ii2++) {
			nlSparseMatrixAdd(
			    &M,
			    NLuint(Cj.coeff[ii1].index),
			    NLuint(Cj.coeff[ii2].index),
			    -Cj.coeff[ii1].value * Cj.coeff[ii2].value * D[j]
			);
		    }
		}
	    }
	}
	
    private:
	Mesh& mesh_;
	Attribute<double> angle_;
	vector<bool> v_on_border_;
	vector<index_t> v_to_c_;
	vector<index_t> next_c_around_v_;
	vector<index_t> c_to_f_;
	
        // ------ Solver parameters -----------------------------------
	double epsilon_; // Threshold for small angles
	double newton_tolf_; // threshold for gradient norm (rhs)
	double newton_tolx_;
	index_t max_newton_iter_;
	double positive_angle_ro_;
	double step_length_factor_;

        // ------ Sizes -----------------------------------------------
        index_t nf_ ;      // Number of facets 
        index_t nalpha_ ;  // Number of angles 
        index_t nint_ ;    // Number of interior nodes 
        index_t nlambda_ ; // Number of constraints (= nf+2.nint) 
        index_t ntot_ ;    // Total number of unknowns (= nalpha + nlamda) 

        // ------ ABF variables & Lagrange multipliers ----------------
        vector<double> alpha_ ;    // Unknown angles. size = nalpha
        vector<double> lambda_ ;   // Lagrange multipliers. size = nlambda
        vector<double> beta_ ;     // Optimum angles.  size = nalpha
        vector<double> w_ ;        // Weights.         size = nalpha

        // ------ Step vectors ----------------------------------------
        vector<double> dalpha_   ; // size = nalpha ; angles
        vector<double> dlambda1_ ; // size = nf     ; C1 part
        vector<double> dlambda2_ ; // size = 2.nint ; C2 and C3 part

        // ------ Right-hand side ( - gradients ) ---------------------
        vector<double> b1_ ;      // size = nalpha
        vector<double> b2_ ;      // size = nlambda

        // ------ Jacobian of the constraints -------------------------
        // J1 (Jacobian of constraint 1) is not stored, it is implicit
        NLSparseMatrix J2_ ; // size = 2.nint * nalpha


        // ------ ABF++ variables -------------------------------------
        vector<double> Delta_inv_      ; // size = nalpha
        vector<double> Delta_star_inv_ ; // size = nf ; 
        NLSparseMatrix J_star_         ; // size = 2.nint * nf
        vector<double> b1_star_        ; // size = nf
        vector<double> b2_star_        ; // size = 2.nint

        // ------ Final linear system ---------------------------------
        NLSparseMatrix M_        ; // size = 2.nint * 2.nint
        vector<double> r_        ; // size = 2.nint

	bool verbose_;
    };
    
}

namespace GEO {
    
    void mesh_compute_ABF_plus_plus(
	Mesh& M, const std::string& attribute_name, bool verbose
    ) {
	// Normally, the ABFPlusPlus class can handle non-triangulated
	// surfaces, but:
	// 1) it does not seem to converge (to be checked, I may have a bug)
	// 2) the angle-to-uv algorithm in LSCM needs to be
	//    adapted.
	// (for now, we still require triangulated surfaces)
	geo_assert(M.facets.are_simplices());
	
	ABFPlusPlus ABF(M);
	ABF.set_verbose(verbose);
	ABF.parameterize(); // This computes the "angle" attribute.
	// Now use LSCM to retrieve (u,v) coordinates from the angles.
	mesh_compute_LSCM(M, attribute_name, false, "angle");
	M.facet_corners.attributes().delete_attribute_store("angle");
    }
    
}
