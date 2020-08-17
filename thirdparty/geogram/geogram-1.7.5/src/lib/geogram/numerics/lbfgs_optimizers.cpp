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

#ifdef GEOGRAM_WITH_HLBFGS

#include <geogram/numerics/lbfgs_optimizers.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/argused.h>
#include <geogram/third_party/HLBFGS/HLBFGS.h>
#include <geogram/bibliography/bibliography.h>

#include <setjmp.h>
#include <iostream>

namespace GEO {

    /**
     * \brief Optimizer configuration
     * \details Manages global variables and callbacks for the
     * communication between the Optimizer class and the HLBFGS
     * library.
     * \internal
     * The current implementation is actually a bottleneck: optimizer
     * execution config is stored in static variables which prevents multiple
     * optimizers to execute in parallel.
     */
    namespace OptimizerConfig {

        static Optimizer::newiteration_callback newiteration_callback_ = nullptr;
        static Optimizer::funcgrad_callback funcgrad_callback_ = nullptr;
        static Optimizer::evalhessian_callback evalhessian_callback_ = nullptr;
        static index_t N_ = 0;

        /**
         * \brief Initializes Optimizer configuration
         * \details Sets the problem dimension and the various optimizer
         * callbacks for the optimizer execution
         * \param[in] N dimension of the problem
         * \param[in] funcgrad_callback callback that evaluates
         *  the function to be minimized and its gradient
         * \param[in] newiteration_callback callback that will be
         *  called at each iteration
         * \param[in] evalhessian_callback callback that evaluates
         *  the Hessian of function to be minimized (second
         *  order derivatives)
         */
        static void init(
            index_t N,
            Optimizer::funcgrad_callback funcgrad_callback,
            Optimizer::newiteration_callback newiteration_callback,
            Optimizer::evalhessian_callback evalhessian_callback
        ) {
            N_ = N;
            funcgrad_callback_ = funcgrad_callback;
            newiteration_callback_ = newiteration_callback;
            evalhessian_callback_ = evalhessian_callback;
        }

        /**
         * \brief HLBFGS callback called at each iteration.
         * \param[in] iter current iteration
         * \param[in] call_iter total number of evaluations
         * \param[in] x value of the parameters at current iteration
         * \param[in] f value of the function
         * \param[in] g gradient of the function
         * \param[in] gnorm norm of the gradient
         */
        static void HLBFGS_newiteration_callback(
            int iter, int call_iter, double* x, double* f, double* g,
            double* gnorm
        ) {
            GEO::geo_argused(iter);
            GEO::geo_argused(call_iter);
            (* newiteration_callback_)(N_, x, * f, g, * gnorm);
        }

        /**
         * \brief HLBFGS callback that evaluates the function and its
         * gradient.
         * \param[in] N dimension of the problem
         * \param[in] x value of the parameters at current iteration
         * \param[in] prev_x value of the parameters at previous iteration
         * \param[out] f value of the function
         * \param[out] g gradient of the function
         */
        static void HLBFGS_funcgrad_callback(
            int N, double* x, double* prev_x, double* f, double* g
        ) {
            GEO::geo_argused(prev_x);
            (* funcgrad_callback_)((index_t) N, x, * f, g);
        }

        /**
         * \brief HLBFGS callback that evaluates the function, its gradient
         * and Hessian.
         * \param[in] N dimension of the problem
         * \param[in] x value of the parameters at current iteration
         * \param[in] prev_x value of the parameters at previous iteration
         * \param[out] f value of the function
         * \param[out] g gradient of the function
         * \param[out] m_hessian Hessian of the function
         */
        static void HLBFGS_evalhessian_callback(
            int N, double* x, double* prev_x, double* f, double* g,
            HESSIAN_MATRIX& m_hessian
        ) {
            GEO::geo_argused(prev_x);
            (* evalhessian_callback_)((index_t) N, x, * f, g, m_hessian);
        }
    }

    /************************************************************************/

    HLBFGSOptimizer::HLBFGSOptimizer() :
        b_m1qn3_(false),
        b_cg_(false) {
	geo_cite("WEB:HLBFGS");
    }

    HLBFGSOptimizer::~HLBFGSOptimizer() {
    }

    void HLBFGSOptimizer::optimize(double* x) {
        geo_assert(newiteration_callback_ != nullptr);
        geo_assert(funcgrad_callback_ != nullptr);
        geo_assert(n_ > 0);
        geo_assert(x != nullptr);

        OptimizerConfig::init(
            n_,
            funcgrad_callback_,
            newiteration_callback_,
            nullptr
        );

        double parameter[20];
        int hlbfgs_info[20];

        // initialize parameters and infos
        INIT_HLBFGS(parameter, hlbfgs_info);
        hlbfgs_info[3] = b_m1qn3_ ? 1 : 0; // determines whether we use m1qn3
        hlbfgs_info[4] = (int) max_iter_;  // max iterations
        hlbfgs_info[5] =
            GEO::CmdLine::get_arg_bool("debug") ? 1 : 0;  // verbose
        hlbfgs_info[10] = b_cg_ ? 1 : 0; // determines whether we use cg
        parameter[5] = 0; // disabled
        parameter[6] = epsg_;

        HLBFGS(
            (int) n_,
            (int) m_,
            x,
            OptimizerConfig::HLBFGS_funcgrad_callback,
            nullptr,
            HLBFGS_UPDATE_Hessian,
            OptimizerConfig::HLBFGS_newiteration_callback,
            parameter,
            hlbfgs_info
        );
    }

    /************************************************************************/

    HLBFGS_M1QN3Optimizer::HLBFGS_M1QN3Optimizer() {
        set_m1qn3(true);
    }

    HLBFGS_M1QN3Optimizer::~HLBFGS_M1QN3Optimizer() {
    }

    /************************************************************************/

    HLBFGS_CGOptimizer::HLBFGS_CGOptimizer() {
        set_cg(true);
    }

    HLBFGS_CGOptimizer::~HLBFGS_CGOptimizer() {
    }

    /************************************************************************/

    HLBFGS_HessOptimizer::HLBFGS_HessOptimizer() :
        T_(0) {
    }

    HLBFGS_HessOptimizer::~HLBFGS_HessOptimizer() {
    }

    void HLBFGS_HessOptimizer::optimize(double* x) {
        geo_assert(newiteration_callback_ != nullptr);
        geo_assert(funcgrad_callback_ != nullptr);
        geo_assert(evalhessian_callback_ != nullptr);
        geo_assert(n_ > 0);
        geo_assert(x != nullptr);

        OptimizerConfig::init(
            n_,
            funcgrad_callback_,
            newiteration_callback_,
            evalhessian_callback_
        );

        double parameter[20];
        int hlbfgs_info[20];

        // initialize parameters and infos
        INIT_HLBFGS(parameter, hlbfgs_info);
        hlbfgs_info[4] = (int) max_iter_;  // max iterations
        hlbfgs_info[6] = (int) T_;  // update interval of hessian
        hlbfgs_info[7] = 1;   // 0: without hessian, 1: with accurate hessian
        
        HLBFGS(
            (int) n_,
            (int) m_,
            x,
            OptimizerConfig::HLBFGS_funcgrad_callback,
            OptimizerConfig::HLBFGS_evalhessian_callback,
            HLBFGS_UPDATE_Hessian,
            OptimizerConfig::HLBFGS_newiteration_callback,
            parameter,
            hlbfgs_info
        );
    }
}

#endif
