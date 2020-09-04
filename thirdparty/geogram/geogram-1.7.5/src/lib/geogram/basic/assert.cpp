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

#include <geogram/basic/assert.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/process.h>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>

#ifdef GEO_OS_WINDOWS
#include <intrin.h> // For __debugbreak()
#else
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef GEO_OS_ANDROID
#ifndef GEO_OS_EMSCRIPTEN
#include <execinfo.h>
#endif
#endif

#endif

namespace GEO {

    namespace {
#ifdef GEO_DEBUG
        AssertMode assert_mode_ = ASSERT_ABORT;        
#else
        AssertMode assert_mode_ = ASSERT_THROW;
#endif        
        bool aborting = false;
    }

    void set_assert_mode(AssertMode mode) {
        assert_mode_ = mode;
    }

    AssertMode assert_mode() {
        return assert_mode_;
    }

    void geo_abort() {
        // Avoid assert in assert !!
        if(aborting) {
            Process::brute_force_kill();
        }
        aborting = true;
        abort();
    }

    void geo_breakpoint() {
#ifdef GEO_COMPILER_MSVC
	__debugbreak();
#else
	geo_abort();
#endif	
    }
    
    void geo_assertion_failed(
        const std::string& condition_string,
        const std::string& file, int line
    ) {
        std::ostringstream os;
        os << "Assertion failed: " << condition_string << ".\n";
        os << "File: " << file << ",\n";
        os << "Line: " << line;

        if(assert_mode_ == ASSERT_THROW) {
	    if(Logger::instance()->is_quiet()) {
		std::cerr << os.str()
			  << std::endl;
	    }
	    throw std::runtime_error(os.str());
        } else if(assert_mode_ == ASSERT_ABORT) {
            Logger::err("Assert") << os.str() << std::endl;
            geo_abort();
        } else {
            Logger::err("Assert") << os.str() << std::endl;
	    geo_breakpoint();
	}
    }

    void geo_range_assertion_failed(
        double value, double min_value, double max_value,
        const std::string& file, int line
    ) {
        std::ostringstream os;
        os << "Range assertion failed: " << value
            << " in [ " << min_value << " ... " << max_value << " ].\n";
        os << "File: " << file << ",\n";
        os << "Line: " << line;

        if(assert_mode_ == ASSERT_THROW) {
            if(Logger::instance()->is_quiet()) {
                std::cerr << os.str()
                          << std::endl;
            }
            throw std::runtime_error(os.str());
        } else {
            Logger::err("Assert") << os.str() << std::endl;
            geo_abort();
        }
    }

    void geo_should_not_have_reached(
        const std::string& file, int line
    ) {
        std::ostringstream os;
        os << "Control should not have reached this point.\n";
        os << "File: " << file << ",\n";
        os << "Line: " << line;

        if(assert_mode_ == ASSERT_THROW) {
            if(Logger::instance()->is_quiet()) {
                std::cerr << os.str()
                          << std::endl;
            }
            throw std::runtime_error(os.str());
        } else {
            Logger::err("Assert") << os.str() << std::endl;
            geo_abort();
        }
    }
}

