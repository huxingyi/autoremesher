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

#include <geogram/basic/numeric.h>
#include <stdlib.h>

#ifdef GEO_COMPILER_EMSCRIPTEN
#pragma GCC diagnostic ignored "-Wc++11-long-long"
#endif

namespace GEO {

    namespace Numeric {

        bool is_nan(float32 x) {
#ifdef GEO_COMPILER_MSVC
            return _isnan(x) || !_finite(x);	    
#else	    
            return std::isnan(x) || !std::isfinite(x);
#endif	    
        }

        bool is_nan(float64 x) {
#ifdef GEO_COMPILER_MSVC
            return _isnan(x) || !_finite(x);	    	    
#else	    
            return std::isnan(x) || !std::isfinite(x);
#endif	    
        }

        void random_reset() {
#ifdef GEO_OS_WINDOWS
            srand(1);
#else
            srandom(1);
#endif
        }

        int32 random_int32() {
#ifdef GEO_OS_WINDOWS
            return rand();
#else
            return int32(random() % std::numeric_limits<int32>::max());
#endif
        }

        float32 random_float32() {
#if defined(GEO_OS_WINDOWS)
            return float(rand()) / float(RAND_MAX);
#elif defined(GEO_OS_ANDROID)
            // TODO: find a way to call drand48()
            // (problem at link time)
            return
                float(random_int32()) /
                float(std::numeric_limits<int32>::max());
#else
            return float(drand48());
#endif
        }

        float64 random_float64() {
#if defined(GEO_OS_WINDOWS)
            return double(rand()) / double(RAND_MAX);
#elif defined(GEO_OS_ANDROID)
            // TODO: find a way to call drand48()
            // (problem at link time)
            return
                double(random_int32()) /
                double(std::numeric_limits<int32>::max());
#else
            return double(drand48());
#endif
        }
    }
}

