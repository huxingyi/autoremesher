/*
 * NProblemInterfaceADOLC.hh
 *
 *  Created on: Feb 20, 2013
 *      Author: bommes / kremer
 */

#ifndef NPROBLEMINTERFACEADOLC_HH
#define NPROBLEMINTERFACEADOLC_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_ADOLC_AVAILABLE
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================

#include <vector>

#include "NProblemInterface.hh"

#include <CoMISo/Config/CoMISoDefines.hh>

#include <dco.hpp>
//#define USE_DCO_VECTOR

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

/** \class NProblemmInterfaceAD NProblemInterfaceADOC.hh <ACG/.../NProblemInterfaceADOLC.hh>

 Brief Description.

 Extend the problem interface with auto differentiation using ADOL-C
 */
  class COMISODLLEXPORT NProblemInterfaceDCO : public NProblemInterface
  {
  public:
#ifdef USE_DCO_VECTOR
    typedef dco::gt1v<double, 7> MODE_grad;
    typedef MODE_grad::type ad_t_grad;
#else
    typedef dco::ga1s<double> MODE_grad;
    typedef MODE_grad::type ad_t_grad;
#endif
    typedef dco::ga1s<dco::gt1s<double>::type> MODE_hess;
    typedef MODE_hess::type ad_t_hess;
    
    /// Default constructor
    NProblemInterfaceDCO() :
#ifndef USE_DCO_VECTOR
      tape_grad_(MODE_grad::global_tape),
#endif
      tape_hess_(MODE_hess::global_tape),
      dense_hessian_(0),
      sparse_nz_(0),
      sparse_r_ind_p_(0),
      sparse_c_ind_p_(0),
      sparse_val_p_(0)
  {
#ifndef USE_DCO_VECTOR
    MODE_grad::tape_options_t opts_grad;
    opts_grad.tapesize() = 2e3; // used in chunk tape
    tape_grad_ = MODE_grad::tape_t::create(opts_grad);
#endif

    MODE_hess::tape_options_t opts_hess;
    opts_hess.tapesize() = 2e3; // used in chunk tape
    tape_hess_ = MODE_hess::tape_t::create(opts_hess);
  }

    /// Destructor
    virtual ~NProblemInterfaceDCO()
    {
      cleanup_dense_hessian();
      cleanup_sparse_hessian();
#ifndef USE_DCO_VECTOR
      MODE_grad::tape_t::remove(tape_grad_);
#endif
      MODE_hess::tape_t::remove(tape_hess_);
    }

    // ================================================
    // Override these four methods to define a problem
    // ================================================

    virtual int     n_unknowns       (                   ) = 0;
    virtual void    initial_x        (       double*  _x ) = 0;
    virtual void    store_result     ( const double*  _x ) = 0;
    virtual ad_t_grad    eval_f_adouble   ( const ad_t_grad*    _x)  = 0;
    virtual ad_t_hess    eval_f_adouble_hess ( const ad_t_hess*   _x)  = 0;
    virtual double  eval_f_double    ( const double*  _x)  = 0;

    // ================================================
    // Optionally override these methods, too
    // ================================================

    // advanced properties
    virtual bool   constant_gradient() const { return false; }
    virtual bool   constant_hessian()  const { return false; }
    

    /**
     * \brief Indicate whether the gradient or hessian is sparse.
     * If so, the computations (as well as the memory
     * consumption) can be performed more efficiently.
     */
    virtual bool sparse_gradient() { return false;}
    virtual bool sparse_hessian () { return true;}


    // ================================================
    // The following functions implement all methods for
    // NProblemInterface. No changes required there
    // ================================================


    virtual double eval_f( const double* _x )
    {
      double y = 0.0;

      // Call virtual function to compute
      // functional value
      y = eval_f_double(_x);

      return y;
    }

    // automatic evaluation of gradient
    virtual void eval_gradient(const double* _x, double* _g)
    {

      std::vector<ad_t_grad> ax(n_unknowns());
      for (int i = 0; i < n_unknowns(); ++i)
	ax[i] = _x[i];

#ifndef USE_DCO_VECTOR
      tape_grad_->register_variable(ax);

      // Evaluate original functional to get new tape
      ad_t_grad ay = eval_f_adouble(ax.data());

      dco::derivative(ay) = 1.0;

      // dco::logger::reporting_level() = dco::logDEBUG;
      // DCO_LOG(dco::logDEBUG) << "stack = " << tape_->get_position()._stackcounter() << "; progvar = " << tape_->get_position()._progvarcounter();

      tape_grad_->interpret_adjoint();
      
      for (int i = 0; i < n_unknowns(); ++i)
	_g[i] = dco::value(dco::derivative(ax[i]));

      tape_grad_->reset();
#else
      for (int i = 0; i < n_unknowns(); ++i)
	MODE_grad::set(ax[i], 1.0, 1, i);

      // Evaluate original functional and compute partials
      ad_t_grad ay = eval_f_adouble(ax.data());

      for (int i = 0; i < n_unknowns(); ++i)
	MODE_grad::get(ay, _g[i], 1, i);
#endif

    }

    // automatic evaluation of hessian
    virtual void eval_hessian(const double* _x, SMatrixNP& _H)
    {
      allocate_dense_hessian();
      eval_hessian_dense(_x, dense_hessian_);

      // efficiently copy into eigen-matrix
      typedef Eigen::Triplet<double> T;
      std::vector<T> triplets;
      triplets.reserve(this->n_unknowns()*this->n_unknowns());


      for(int i = 0; i < this->n_unknowns(); ++i)
      {
        // diagonal
        triplets.push_back(T(i,i,dense_hessian_[i][i]));
        for(int j = 0; j < i; ++j)
        {
          triplets.push_back(T(i,j,dense_hessian_[i][j]));
          // store also upper diagonal part?
          triplets.push_back(T(j,i,dense_hessian_[i][j]));
        }
      }

      _H.resize(this->n_unknowns(), this->n_unknowns());
      _H.setZero();
      _H.setFromTriplets(triplets.begin(), triplets.end());

      //       _H.resize(this->n_unknowns(), this->n_unknowns());
//       _H.setZero();

//       // tape update required?
//       if(!tape_available_ || always_retape_)
//         eval_f(_x);

//       if(sparse_hessian())
//       {
//         int opt[2] = {0, 0};

//         bool sparsity_pattern_available = bool(sparse_r_ind_p_);


//         int ec = sparse_hess( tape_,
//             this->n_unknowns(),
//             sparsity_pattern_available,
//             _x,
//             &sparse_nz_,
//             &sparse_r_ind_p_,
//             &sparse_c_ind_p_,
//             &sparse_val_p_,
//             opt);

//         if(ec < 0)
//         {
// #ifdef DCO_RET_CODES
// std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
// #endif
// // Retape function if return code indicates discontinuity
// tape_available_ = false;
// eval_f(_x);
// ec = sparse_hess( tape_,
//     this->n_unknowns(),
//     sparsity_pattern_available,
//     _x,
//     &sparse_nz_,
//     &sparse_r_ind_p_,
//     &sparse_c_ind_p_,
//     &sparse_val_p_,
//     opt);
//         }

//         // data should be available now
//         assert(sparse_r_ind_p_ != NULL);
//         assert(sparse_c_ind_p_ != NULL);
//         assert(sparse_val_p_   != NULL);

// #ifdef DCO_RET_CODES
//         std::cout << "Info: sparse_hessian() returned code " << ec << std::endl;
// #endif
//         // efficiently copy into eigen-matrix
//         typedef Eigen::Triplet<double> T;
//         std::vector<T> triplets;
//         triplets.reserve(2*sparse_nz_);

//         // store data;
//         for(int i = 0; i < sparse_nz_; ++i)
//         {
//           triplets.push_back(T(sparse_r_ind_p_[i], sparse_c_ind_p_[i], sparse_val_p_[i]));

//           // provide also upper diagonal part?
//           if(sparse_r_ind_p_[i] != sparse_c_ind_p_[i])
//             triplets.push_back(T(sparse_c_ind_p_[i], sparse_r_ind_p_[i], sparse_val_p_[i]));
//         }

//         _H.setFromTriplets(triplets.begin(), triplets.end());
//       }
//       else
//       {
//         allocate_dense_hessian();

//         int ec = hessian(tape_, this->n_unknowns(), const_cast<double*>(_x), dense_hessian_);

//         if(ec < 0) {
// #ifdef DCO_RET_CODES
//           std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
// #endif
//           // Retape function if return code indicates discontinuity
//           tape_available_ = false;
//           eval_f(_x);
//           ec = hessian(tape_, this->n_unknowns(), const_cast<double*>(_x), dense_hessian_);
//         }

// #ifdef DCO_RET_CODES
//         std::cout << "Info: hessian() returned code " << ec << std::endl;
// #endif

//         // efficiently copy into eigen-matrix
//         typedef Eigen::Triplet<double> T;
//         std::vector<T> triplets;
//         triplets.reserve(this->n_unknowns()*this->n_unknowns());


//         for(int i = 0; i < this->n_unknowns(); ++i)
//         {
//           // diagonal
//           triplets.push_back(T(i,i,dense_hessian_[i][i]));
//           for(int j = 0; j < i; ++j)
//           {
//             triplets.push_back(T(i,j,dense_hessian_[i][j]));
//             // store also upper diagonal part?
//             triplets.push_back(T(j,i,dense_hessian_[i][j]));
//           }
//         }
//         _H.setFromTriplets(triplets.begin(), triplets.end());
//       }
    }

    // automatic evaluation of hessian
    // this function avoids data structure conversions and gives access to the raw sparse data type
    virtual void eval_hessian_sparse(const double* _x, int& _nz, unsigned int*& _r_idx, unsigned int*& _c_idx, double*& _val)
    {
      assert(false);
//       // tape update required?
//       if(!tape_available_ || always_retape_)
//         eval_f(_x);
//       int opt[2] = {0, 0};

//       bool sparsity_pattern_available = bool(sparse_r_ind_p_);


//       int ec = sparse_hess( tape_,
//           this->n_unknowns(),
//           sparsity_pattern_available,
//           _x,
//           &sparse_nz_,
//           &sparse_r_ind_p_,
//           &sparse_c_ind_p_,
//           &sparse_val_p_,
//           opt);

//       if(ec < 0)
//       {
// #ifdef DCO_RET_CODES
//         std::cout << __FUNCTION__ << " invokes retaping of function due to discontinuity! Return code: " << ec << std::endl;
// #endif
//         // Retape function if return code indicates discontinuity
//         tape_available_ = false;
//         eval_f(_x);
//         ec = sparse_hess( tape_,
//             this->n_unknowns(),
//             sparsity_pattern_available,
//             _x,
//             &sparse_nz_,
//             &sparse_r_ind_p_,
//             &sparse_c_ind_p_,
//             &sparse_val_p_,
//             opt);
//       }

//       // data should be available now
//       assert(sparse_r_ind_p_ != NULL);
//       assert(sparse_c_ind_p_ != NULL);
//       assert(sparse_val_p_   != NULL);

//       // return result
//       _nz    = sparse_nz_;
//       _r_idx = sparse_r_ind_p_;
//       _c_idx = sparse_c_ind_p_;
//       _val   = sparse_val_p_;

// #ifdef DCO_RET_CODES
//       std::cout << "Info: sparse_hessian() returned code " << ec << std::endl;
// #endif
    }


    // automatic evaluation of hessian
    // this function avoids data structure conversions and gives access to the raw dense data type
    virtual void eval_hessian_dense(const double* _x, double**& _H)
    {
      std::vector<ad_t_hess> ax(n_unknowns());
      for (int i = 0; i < n_unknowns(); ++i)
	ax[i] = _x[i];

      allocate_dense_hessian();

      tape_hess_->register_variable(ax);
      MODE_hess::tape_t::iterator pos = tape_hess_->get_position();
      
      
      for (int col = 0; col < n_unknowns(); ++col) {
	dco::derivative(dco::value(ax[col])) = 1.0;
	ad_t_hess y = eval_f_adouble_hess(ax.data());
	dco::derivative(y) = 1.0;
	
	tape_hess_->interpret_adjoint_and_reset_to(pos);
	for (int row = 0; row < col+1; ++row) {
	  dense_hessian_[col][row] = dco::derivative(dco::derivative(ax[row]));
	}
	dco::derivative(dco::value(ax[col])) = 0.0;
      }

      tape_hess_->reset();

      _H = dense_hessian_;
      
    }


private:

    void allocate_dense_hessian()
    {
      if(!dense_hessian_)
      {
        dense_hessian_ = new double*[this->n_unknowns()];
        for(int i = 0; i < this->n_unknowns(); ++i)
          dense_hessian_[i] = new double[i+1];
      }

      // store #unknowns to access in destructor
      sparse_nz_ = this->n_unknowns();
    }

    void cleanup_dense_hessian()
    {
      if(dense_hessian_)
      {
        for(int i = 0; i < sparse_nz_; ++i)
          delete[] dense_hessian_[i];

        delete[] dense_hessian_;
      }
    }

    void cleanup_sparse_hessian()
    {
      if(sparse_r_ind_p_)
        delete[] sparse_r_ind_p_;
      if(sparse_c_ind_p_)
        delete[] sparse_c_ind_p_;
      if(sparse_val_p_)
        delete[] sparse_val_p_;
    }

public:

    // void print_stats()
    // {
    //   size_t tape_stats[11];
    //   tapestats(tape_, tape_stats);
    //   std::cout << "Status values for tape " << tape_ << std::endl;
    //   std::cout << "===============================================" << std::endl;
    //   std::cout << "Number of independent variables:\t" << tape_stats[0] << std::endl;
    //   std::cout << "Number of dependent variables:\t\t" << tape_stats[1] << std::endl;
    //   std::cout << "Max. number of live active variables:\t" << tape_stats[2] << std::endl;
    //   std::cout << "Size of value stack:\t\t\t" << tape_stats[3] << std::endl;
    //   std::cout << "Buffer size:\t\t\t\t" << tape_stats[4] << std::endl;
    //   std::cout << "Total number of operations recorded:\t" << tape_stats[5] << std::endl;
    //   std::cout << "Other stats [6]:\t\t\t" << tape_stats[6] << std::endl;
    //   std::cout << "Other stats [7]:\t\t\t" << tape_stats[7] << std::endl;
    //   std::cout << "Other stats [8]:\t\t\t" << tape_stats[8] << std::endl;
    //   std::cout << "Other stats [9]:\t\t\t" << tape_stats[9] << std::endl;
    //   std::cout << "Other stats [10]:\t\t\t" << tape_stats[10] << std::endl;
    //   std::cout << "===============================================" << std::endl;
    // }

private:

    // index of tape
#ifndef USE_DCO_VECTOR
    MODE_grad::tape_t* &tape_grad_;
#endif
    MODE_hess::tape_t* &tape_hess_;

    // dense hessian (if required)
    double** dense_hessian_;

    // sparse hessian data
    int           sparse_nz_;
    unsigned int* sparse_r_ind_p_;
    unsigned int* sparse_c_ind_p_;
    double*       sparse_val_p_  ;
};

//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_DCO_AVAILABLE
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================
#endif /* NPROBLEMINTERFACEDCO_HH */
