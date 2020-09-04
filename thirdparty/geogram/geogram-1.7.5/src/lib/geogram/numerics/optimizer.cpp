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

#include <geogram/numerics/optimizer.h>
#include <geogram/numerics/lbfgs_optimizers.h>
#include <geogram/basic/logger.h>

namespace GEO {

    Optimizer::Optimizer() :
        n_(0),
        m_(0),
        max_iter_(1000),
        funcgrad_callback_(nullptr),
        newiteration_callback_(nullptr),
        evalhessian_callback_(nullptr),
        epsg_(0),
        epsf_(0),
        epsx_(0),
        verbose_(true) {
    }

    Optimizer::~Optimizer() {
    }

    Optimizer* Optimizer::create(const std::string& name) {

#ifdef GEOGRAM_WITH_HLBFGS	
        geo_register_Optimizer_creator(HLBFGSOptimizer, "default");
        geo_register_Optimizer_creator(HLBFGSOptimizer, "HLBFGS");
        geo_register_Optimizer_creator(HLBFGS_M1QN3Optimizer, "HM1QN3");
        geo_register_Optimizer_creator(HLBFGS_CGOptimizer, "HCG");
        geo_register_Optimizer_creator(HLBFGS_HessOptimizer, "HLBFGS_HESS");
#endif
        Optimizer* opt = OptimizerFactory::create_object(name);
        if(opt != nullptr) {
            return opt;
        }

        Logger::err("Optimizer")
            << "Could not create optimizer: " << name
            << std::endl;
        return nullptr;
    }
}

