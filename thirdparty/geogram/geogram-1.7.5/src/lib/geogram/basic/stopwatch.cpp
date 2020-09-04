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

#include <geogram/basic/stopwatch.h>
#include <iostream>

#ifdef GEO_OS_EMPSCRITEN
#include <empscripten.h>
#endif

namespace GEO {

    /************************************************************************/

    SystemStopwatch::SystemStopwatch() {
#if defined(GEO_OS_WINDOWS)
        start_ = long(GetTickCount());
#elif defined(GEO_OS_EMSCRIPTEN)
        startf_ = now();
#else
        clock_t init_user = times(&start_);
        while((start_user_ = times(&start_)) == init_user) {
        }
#endif
    }

    double SystemStopwatch::elapsed_user_time() const {
#if defined(GEO_OS_WINDOWS)
        return double(long(GetTickCount()) - start_) / 1000.0;
#elif defined(GEO_OS_EMSCRIPTEN)
        return now() - startf_;
#else        
        clock_t end_user;
        tms end;
        end_user = times(&end);
        return double(end_user - start_user_) / 100.0;
#endif
    }

    double SystemStopwatch::now() {
#if defined(GEO_OS_WINDOWS)
        return double(GetTickCount()) / 1000.0;
#elif defined(GEO_OS_EMSCRIPTEN)
        // times() hangs on Emscripten but
        // clock() works. TODO: check with emscripten_get_now();        
        return double(clock()) / double(CLOCKS_PER_SEC);
#else
        tms now_tms;
        return double(times(&now_tms)) / 100.0;
#endif
    }

    void SystemStopwatch::print_elapsed_time(std::ostream& os) const {
#if defined(GEO_OS_WINDOWS)
        os << "---- Times (seconds) ----"
            << "\n  Elapsed time: " << elapsed_user_time()
            << std::endl;
#elif defined(GEO_OS_EMSCRIPTEN)
        os << "---- Times (seconds) ----"
            << "\n  Elapsed time: " << elapsed_user_time()
            << std::endl;
#else
        clock_t end_user;
        tms end;
        end_user = times(&end);

        os << "---- Times (seconds) ----"
            << "\n  Real time: "
            << double(end_user - start_user_) / 100.0

    << "\n  User time: "
    << double(end.tms_utime - start_.tms_utime) / 100.0

    << "\n  Syst time: "
    << double(end.tms_stime - start_.tms_stime) / 100.0
    << std::endl;
#endif
    }

    /************************************************************************/

    // For now, uses SystemStopwatch,
    // TODO: If need be, reimplement with ASM RDTSC instruction
    // (read processor timer).
    Numeric::uint64 ProcessorStopwatch::now() {
        return Numeric::uint64(SystemStopwatch::now() * 1000.0);
    }

    /************************************************************************/
}

