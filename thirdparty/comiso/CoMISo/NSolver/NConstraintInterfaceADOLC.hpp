//=============================================================================
//
//  CLASS NConstraintInterfaceADOLC
//
//=============================================================================

#ifndef COMISO_NCONSTRAINTINTERFACEADOLC_HH
#define COMISO_NCONSTRAINTINTERFACEADOLC_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_ADOLC_AVAILABLE
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include "SuperSparseMatrixT.hh"

#include <adolc/adolc.h>
#include <adolc/adouble.h>
#include <adolc/drivers/drivers.h>
#include <adolc/sparse/sparsedrivers.h>
#include <adolc/taping.h>

#include "NConstraintInterface.hh"

#include "TapeIDSingleton.hh"

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

/** \class NProblemInterfaceADOLC NProblemInterfaceADOLC.hpp

    The problem interface using automatic differentiation.
 */
class COMISODLLEXPORT NConstraintInterfaceADOLC : public NConstraintInterface {
public:

    // Define Sparse Datatypes
    typedef NConstraintInterface::SVectorNC SVectorNC;
    typedef NConstraintInterface::SMatrixNC SMatrixNC;

    typedef NConstraintInterface::ConstraintType ConstraintType;

    /// Default constructor
    NConstraintInterfaceADOLC(int _n_unknowns,
    		const ConstraintType _type = NC_EQUAL,
    		double _eps = 1e-6) :
        NConstraintInterface(_type, _eps),
        n_unknowns_(_n_unknowns),
        tape_available_(false),
        use_tape_(true),
        tape_(static_cast<short int>(TapeIDSingleton::Instance()->requestId())),
        dense_hessian_(NULL),
        sparse_nz_(0),
		sparse_r_ind_p_(NULL),
		sparse_c_ind_p_(NULL),
		sparse_val_p_(NULL) {

    }

    /// Destructor
    virtual ~NConstraintInterfaceADOLC() {

    	cleanup_sparse_hessian();

        TapeIDSingleton::Instance()->releaseId(static_cast<size_t>(tape_));
    }

    /**
     * \brief Only override this function
     */
    virtual adouble eval_c_adouble(const adouble* _x) = 0;

public:

    virtual int n_unknowns() {
        return n_unknowns_;
    }

    virtual double eval_constraint(const double* _x) {

        double y = 0.0;

        if(!tape_available_ || !use_tape_) {

        	adouble y_d = 0.0;

//        	if(active_vars_.get() == NULL)
//        		active_vars_.reset(new adouble[n_unknowns_]);

        	adouble* xa = new adouble[n_unknowns_];

			trace_on(tape_); // Start taping

			// Fill data vector
			for(int i = 0; i < n_unknowns_; ++i)
				xa[i] <<= _x[i];

			// Call virtual function to compute
			// functional value
			y_d = eval_c_adouble(xa);

			y_d >>= y;

			trace_off();

#ifdef ADOLC_STATS
			print_stats();
#endif

			delete[] xa;

			tape_available_ = true;

        } else {

        	int ec = function(tape_, 1, n_unknowns_, const_cast<double*>(_x), &y);

#ifdef ADOLC_RET_CODES
			std::cout << "Info: function() returned code " << ec << std::endl;
#endif

			// tape not valid anymore? retape and evaluate again
			if(ec < 0)
			{
			  tape_available_ = false;
			  return eval_constraint(_x);
			}
        }

        return y;
    }

    virtual void eval_gradient(const double* _x, SVectorNC& _g) {

        if(!tape_available_ || !use_tape_) {

        	// Evaluate original functional
            eval_constraint(_x);
        }

//        if(gradient_.get() == NULL)
//        	gradient_.reset(new double[n_unknowns_]);

        double* grad_p = new double[n_unknowns_];

        _g.resize(n_unknowns_);
        _g.setZero();

        // Evaluate gradient
		int ec = gradient(tape_, n_unknowns_, _x, grad_p);

		// Check if retaping is required
		if(ec < 0)
		{
#ifdef ADOLC_RET_CODES
			std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
#endif

			tape_available_ = false;
			eval_gradient(_x,_g);
		}

#ifdef ADOLC_RET_CODES
        std::cout << "Info: gradient() returned code " << ec << std::endl;
#endif

        for(int i = 0; i < n_unknowns_; ++i) {
            _g.coeffRef(i) += grad_p[i];
        }

        delete[] grad_p;
    }

    virtual void eval_hessian(const double* _x, SMatrixNC& _H) {

        _H.resize(n_unknowns_, n_unknowns_);

        // tape update required?
		if (!tape_available_ || !use_tape_)
			eval_constraint(_x);

		/*
		 * Hessian matrix is sparse
		 */
		if(sparse_hessian()) {

			int opt[2] = {0, 0};

			bool sparsity_pattern_available = bool(sparse_r_ind_p_);

			int ec = sparse_hess(tape_, n_unknowns_,
					sparsity_pattern_available, _x, &sparse_nz_,
					&sparse_r_ind_p_, &sparse_c_ind_p_, &sparse_val_p_, opt);

			if (ec < 0) {
#ifdef ADOLC_RET_CODES
				std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
#endif
				// Retape function if return code indicates discontinuity
				tape_available_ = false;
				eval_constraint(_x);
				ec = sparse_hess(tape_, n_unknowns_,
						sparsity_pattern_available, _x, &sparse_nz_,
						&sparse_r_ind_p_, &sparse_c_ind_p_, &sparse_val_p_,	opt);
			}

			// data should be available now
			assert(sparse_r_ind_p_ != NULL);
			assert(sparse_c_ind_p_ != NULL);
			assert(sparse_val_p_ != NULL);

#ifdef ADOLC_RET_CODES
			std::cout << "Info: sparse_hessian() returned code " << ec << std::endl;
#endif

			// Store data
			for (int i = 0; i < sparse_nz_; ++i) {

				_H(sparse_r_ind_p_[i], sparse_c_ind_p_[i]) = sparse_val_p_[i];

				// Provide also upper diagonal part?
				if (sparse_r_ind_p_[i] != sparse_c_ind_p_[i])
					_H(sparse_c_ind_p_[i], sparse_r_ind_p_[i]) = sparse_val_p_[i];
			}

		} else {
			/*
			 * Hessian matrix is dense
			 */
			allocate_dense_hessian();

			int ec = hessian(tape_, n_unknowns_, const_cast<double*>(_x),
					dense_hessian_);

			if (ec < 0) {
#ifdef ADOLC_RET_CODES
				std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
#endif
				// Retape function if return code indicates discontinuity
				tape_available_ = false;
				eval_constraint(_x);
				ec = hessian(tape_, n_unknowns_, const_cast<double*>(_x),
						dense_hessian_);
			}

#ifdef ADOLC_RET_CODES
			std::cout << "Info: hessian() returned code " << ec << std::endl;
#endif

			for (int i = 0; i < n_unknowns_; ++i) {
				// Diagonal
				_H(i, i) = dense_hessian_[i][i];
				for (int j = 0; j < i; ++j) {
					_H(i, j) = dense_hessian_[i][j];
					// Also store upper diagonal part
					_H(j, i) = dense_hessian_[i][j];
				}
			}
		}
    }

    /** \brief Use tape
     * Set this to false if the energy functional
     * is discontinuous (so that the operator tree
     * has to be re-established at each evaluation)
     */
    bool use_tape() const {
        return use_tape_;
    }

    void use_tape(bool _b) {
        use_tape_ = _b;
    }

    /**
     * \brief Provide information to make computations
     * as efficient as possible.
     */
    virtual bool is_linear()         const { return false; }
    virtual bool constant_gradient() const { return false; }

    /**
     * \brief Indicate whether the hessian is sparse.
     * If so, the computations (as well as the memory
     * consumption) can be performed more efficiently.
     */
    virtual bool constant_hessian () const { return false;}

private:

	void allocate_dense_hessian() {
		if (!dense_hessian_) {
			dense_hessian_ = new double*[n_unknowns_];
			for (int i = 0; i < n_unknowns_; ++i)
				dense_hessian_[i] = new double[i + 1];
		}

		// store #unknowns to access in destructor
		sparse_nz_ = n_unknowns_;
	}

	void cleanup_dense_hessian() {
		if (dense_hessian_) {
			for (int i = 0; i < sparse_nz_; ++i)
				delete[] dense_hessian_[i];

			delete[] dense_hessian_;
		}
	}

	void cleanup_sparse_hessian() {

		if (sparse_r_ind_p_ != NULL)
			delete[] sparse_r_ind_p_;
		if (sparse_c_ind_p_ != NULL)
			delete[] sparse_c_ind_p_;
		if (sparse_val_p_ != NULL)
			delete[] sparse_val_p_;
	}

    void print_stats() {

    	size_t tape_stats[11];
    	tapestats(tape_, tape_stats);
		std::cout << "Status values for tape " << tape_ << std::endl;
		std::cout << "===============================================" << std::endl;
		std::cout << "Number of independent variables:\t" << tape_stats[0] << std::endl;
		std::cout << "Number of dependent variables:\t\t" << tape_stats[1] << std::endl;
		std::cout << "Max. number of live active variables:\t" << tape_stats[2] << std::endl;
		std::cout << "Size of value stack:\t\t\t" << tape_stats[3] << std::endl;
		std::cout << "Buffer size:\t\t\t\t" << tape_stats[4] << std::endl;
		std::cout << "Total number of operations recorded:\t" << tape_stats[5] << std::endl;
		std::cout << "Other stats [6]:\t\t\t" << tape_stats[6] << std::endl;
		std::cout << "Other stats [7]:\t\t\t" << tape_stats[7] << std::endl;
		std::cout << "Other stats [8]:\t\t\t" << tape_stats[8] << std::endl;
		std::cout << "Other stats [9]:\t\t\t" << tape_stats[9] << std::endl;
		std::cout << "Other stats [10]:\t\t\t" << tape_stats[10] << std::endl;
		std::cout << "===============================================" << std::endl;
    }

    // Number of unknowns
    int n_unknowns_;

    bool tape_available_;
    bool use_tape_;

    SMatrixNC constant_hessian_;

    const short int tape_;

//    std::auto_ptr<adouble> active_vars_;
//    std::auto_ptr<double> gradient_;

    // dense hessian (if required)
    double** dense_hessian_;

    // Sparse hessian data
	int sparse_nz_;
	unsigned int* sparse_r_ind_p_;
	unsigned int* sparse_c_ind_p_;
	double* sparse_val_p_ ;
};

//=============================================================================
}// namespace COMISO
//=============================================================================
#endif // COMISO_ADOLC_AVAILABLE
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================
#endif // ACG_NCONSTRAINTINTERFACEAD_HH defined
//=============================================================================

