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

#ifndef GEOGRAM_NUMERICS_LBFGS_OPTIMIZERS
#define GEOGRAM_NUMERICS_LBFGS_OPTIMIZERS

#ifdef GEOGRAM_WITH_HLBFGS

#include <geogram/basic/common.h>
#include <geogram/numerics/optimizer.h>
#include <geogram/basic/assert.h>

/**
 * \file geogram/numerics/lbfgs_optimizers.h
 * \brief Implementation of optimizer using the HLBFGS library.
 */

namespace GEO {

    /**
     * \brief Base class for Optimizer implementation using the HLBFGS
     * library.
     */
    class GEOGRAM_API HLBFGSOptimizer : public Optimizer {
    public:
        /**
         * \brief Constructs a new HLBFGSOptimizer.
         */
        HLBFGSOptimizer();

        virtual void optimize(double* x);

        /**
         * \brief Enables or disables M1QN3 mode.
         * \param[in] m1qn3 true if M1QN3 mode should be used, false otherwise
         */
        void set_m1qn3(bool m1qn3) {
            b_m1qn3_ = m1qn3;
        }

        /**
         * \brief Enables or disables Conjugate Gradient mode.
         * \param[in] cg true if conjugate gradient mode should be used,
         *  false otherwise
         */
        void set_cg(bool cg) {
            b_cg_ = cg;
        }

    protected:
        /**
         * \brief HLBFGSOptimizer destructor .
         */
        virtual ~HLBFGSOptimizer();

    protected:
        bool b_m1qn3_;
        bool b_cg_;
    };

    /************************************************************************/

    /**
     * \brief Optimizer implementation using the M1QN3 mode
     */
    class HLBFGS_M1QN3Optimizer : public HLBFGSOptimizer {
    public:
        /*
         * \brief Optimizer constructor
         * \details This sets the M1QN3 parameter to true
         * \see set_m1qn3()
         */
        HLBFGS_M1QN3Optimizer();

    protected:
        /*
         * \brief HLBFGS_M1QN3Optimizer destructor
         */
        virtual ~HLBFGS_M1QN3Optimizer();
    };

    /************************************************************************/

    /**
     * \brief Optimizer implementation using the Conjugate Gradient mode
     */
    class HLBFGS_CGOptimizer : public HLBFGSOptimizer {
    public:
        /*
         * \brief Optimizer constructor
         * \details This sets the CG parameter to true
         * \see set_cg()
         */
        HLBFGS_CGOptimizer();

    protected:
        /*
         * \brief HLBFGS_CGOptimizer destructor
         */
        virtual ~HLBFGS_CGOptimizer();
    };

    /************************************************************************/

    /**
     * \brief Optimizer implementation using the LBFGS method with
     *  Hessian evaluation.
     * \details This version needs the callback that evaluates the Hessian,
     * specified with set_evalhessian_callback()
     */
    class GEOGRAM_API HLBFGS_HessOptimizer : public HLBFGSOptimizer {
    public:
        /**
         * \brief Constructs a new HLBFGS_HessOptimizer.
         */
        HLBFGS_HessOptimizer();

        virtual void optimize(double* x);

        /**
         * \brief Sets the update interval of the Hessian.
         * \param[in] T the update interval of the Hessian
         */
        void set_T(index_t T) {
            T_ = T;
        }

    protected:
        /**
         * \brief HLBFGS_HessOptimizer destructor
         */
        virtual ~HLBFGS_HessOptimizer();

    protected:
        index_t T_;
    };
}

#endif

#endif

