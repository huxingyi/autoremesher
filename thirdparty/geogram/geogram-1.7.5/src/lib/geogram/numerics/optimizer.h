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

#ifndef GEOGRAM_NUMERICS_OPTIMIZER
#define GEOGRAM_NUMERICS_OPTIMIZER

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/factory.h>

class HESSIAN_MATRIX;

/**
 * \file geogram/numerics/optimizer.h
 * \brief Abstract base class for numerical optimizers, used
 *  to minimize a multivariate function
 */

namespace GEO {

    /**
     * \brief Optimizer minimizes a multivariate function.
     *
     * \details The dimension of the problem is defined by set_N().
     * The multivariate function to be minimized is defined
     * by a callback that evaluates the function and its
     * gradient, specified by set_funcgrad_callback().
     * Optimizer implements the numeric part of
     * CentroidalVoronoiTesselation.
     *
     * Optimizer objects are created using method create() which
     * uses the Factory service. New Optimizer%s can be implemented and
     * registered to the factory using
     * geo_register_Optimizer_creator().
     * \see OptimizerFactory
     * \see geo_register_Optimizer_creator
     */
    class GEOGRAM_API Optimizer : public Counted {
    public:
        /**
         * \brief Optimizer callback that evaluates a function
         *  and its gradient.
         * \see set_funcgrad_callback()
         */
        typedef void (* funcgrad_callback)(
            index_t N, double* x, double& f, double* g
        );

        /**
         * \brief Optimizer callback that is called at each iteration.
         * \see set_newiteration_callback()
         */
        typedef void (* newiteration_callback)(
            index_t N, const double* x, double f, const double* g, double gnorm
        );

        /**
         * \brief Optimizer callback that evaluates a function,
         *  its gradient and its Hessian.
         * \see set_evalhessian_callback()
         */
        typedef void (* evalhessian_callback)(
            index_t N, double* x, double& f, double* g, HESSIAN_MATRIX& hessian
        );

        /**
         * \brief Creates an Optimizer.
         * \param[in] name name of the Optimizer to create:
         *  - "HLBFG" - BFGS (quasi-Newton)
         *  - "HM1QN3" - for non-smooth functions
         *  - "HCG" - non-linear conjugate gradient
         *  - "HLBFGS_HESS" - BFGS with Hessian (full Newton)
         *  - "default" - equivalent to "HLBFGS"
         * \retval nullptr if \p name is not a valid Optimizer algorithm name
         * \retval otherwise, a pointer to an Optimizer object. The returned
         * pointer must be stored in a Optimizer_var that does automatic
         * destruction:
         * \code
         * Optimizer_var optimizer = Optimizer::create("HLBFGS") ;
         * \endcode
         */
        static Optimizer* create(const std::string& name = "default");

        /**
         * \brief Minimizes a function, starting from initial value x.
         * \param[in] x is of size n, where n was defined with set_N()
         */
        virtual void optimize(double* x) = 0;

        /**
         * \brief Defines the number of variables.
         */
        void set_N(index_t N) {
            n_ = N;
        }

        /**
         * \brief Returns the number of variables.
         */
        index_t get_N() const {
            return n_;
        }

        /**
         * \brief Defines the inner number of iterations.
         * \details Used by HLBFGSOptimizer.
         */
        void set_M(index_t M) {
            m_ = M;
        }

        /**
         * \brief Returns the inner number of iterations.
         */
        index_t get_M() const {
            return m_;
        }

        /**
         * \brief Defines the maximum number of iterations.
         */
        void set_max_iter(index_t maxiter) {
            max_iter_ = maxiter;
        }

        /**
         * \brief Returns the maximum number of iterations.
         */
        index_t get_max_iter() const {
            return max_iter_;
        }

        /**
         * \brief Defines the callback that evaluates
         *  the function to be minimized and its gradient.
         */
        void set_funcgrad_callback(funcgrad_callback fp) {
            funcgrad_callback_ = fp;
        }

        /**
         * \brief Defines a callback that will be
         *  called at each iteration.
         */
        void set_newiteration_callback(newiteration_callback fp) {
            newiteration_callback_ = fp;
        }

        /**
         * \brief Defines the callback that evaluates
         *  the Hessian of function to be minimized (second
         *  order derivatives).
         *
         * \details Only used in "HLBFGS_HESS" mode.
         */
        void set_evalhessian_callback(evalhessian_callback fp) {
            evalhessian_callback_ = fp;
        }

        /**
         * \brief Defines the stopping criterion
         *  in terms of gradient magnitude.
         */
        void set_epsg(double eg) {
            epsg_ = eg;
        }

        /**
         * \brief Defines the stopping criterion
         *  in terms of function value.
         */
        void set_epsf(double ef) {
            epsf_ = ef;
        }

        /**
         * \brief Defines the stopping criterion
         *  in terms of variation of x.
         */
        void set_epsx(double ex) {
            epsx_ = ex;
        }

        /**
         * \brief Enables or disables verbose logs.
         */
        void set_verbose(bool verb) {
            verbose_ = verb;
        }

    protected:
        /**
         * \brief Optimizer constructor
         * \details Should never be called directly, use create() instead.
         */
        Optimizer();

        /**
         * \brief Optimizer destructor
         */
        virtual ~Optimizer();

    protected:
        /** Size of the problem */
        index_t n_;
        /**
         * Number of corrections in the BFGS scheme of Hessian
         * approximation update
         */
        index_t m_;
        /** Max iterations */
        index_t max_iter_;

        funcgrad_callback funcgrad_callback_;
        newiteration_callback newiteration_callback_;
        evalhessian_callback evalhessian_callback_;

        /** Error tolerance on x, f and g */
        double epsg_, epsf_, epsx_;

        bool verbose_;
    };

    /**
     * \brief Smart pointer that contains an Optimizer object
     * \relates Optimizer
     */
    typedef SmartPointer<Optimizer> Optimizer_var;

    /**
     * \brief Optimizer Factory
     * \details This Factory is used to create Optimizer objects. It can also
     * be used to register new Optimizer implementations.
     * \see geo_register_Optimizer_creator
     * \see Factory
     * \relates Optimizer
     */
    typedef Factory0<Optimizer> OptimizerFactory;

    /**
     * \brief Helper macro to register an Optimizer implementation
     * \see OptimizerFactory
     * \relates Optimizer
     */
#define geo_register_Optimizer_creator(type, name) \
    geo_register_creator(GEO::OptimizerFactory, type, name)
}

#endif

