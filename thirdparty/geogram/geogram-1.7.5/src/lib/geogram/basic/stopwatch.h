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

#ifndef GEOGRAM_BASIC_STOPWATCH
#define GEOGRAM_BASIC_STOPWATCH

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/logger.h>

/****************************************************************************/

#ifdef GEO_OS_WINDOWS
#else
#include <sys/types.h>
#include <sys/times.h>
#endif

/****************************************************************************/

/**
 * \file geogram/basic/stopwatch.h
 * \brief Classes for measuring time
 */

namespace GEO {

    /**
     * \brief Measures the time taken by an algorithm.
     * \details
     * SystemStopwatch provides functions to get or print the time
     * elapsed since its construction. The times computed by
     * SystemStopwatch are expressed as system ticks, which is a system
     * dependent unit. SystemStopwatch prints three different times:
     *
     * - real time: the really elapsed time (depends on the load of the
     *   machine, i.e. on the others programs that are executed at the
     *   same time).
     * - system time: the time spent in system calls.
     * - user time: the time really spent in the process.
     *
     * Example:
     * \code
     * {
     *     SystemStopwatch clock ;
     *     do_something() ;
     *     clock.print_elapsed_time(std::cout) ;
     * }
     * \endcode
     *
     */

    class GEOGRAM_API SystemStopwatch {
    public:
        /**
         * \brief SystemStopwatch constructor
         * \details It remembers the current time as the reference time
         * for functions elapsed_user_time() and print_elapsed_time().
         */
        SystemStopwatch();

        /**
         * \brief Prints elapsed time to a stream
         * \details Prints real, user and system times since the
         * construction of this SystemStopWatch (in seconds).
         */
        void print_elapsed_time(std::ostream& os) const;

        /**
         * \brief Get the user elapsed time
         * \details Returns the user time elapsed since the SystemStopWatch
         * construction (in seconds)
         */
        double elapsed_user_time() const;

        /**
         * \details Gets the current time (in seconds).
         */
        static double now();

    private:
#if defined(GEO_OS_WINDOWS)
        long start_;
#elif defined(GEO_OS_EMSCRIPTEN)
        double startf_;
#else        
        tms start_;
        clock_t start_user_;
#endif
    };

    /************************************************************************/

    /**
     * \brief A more precise stopwatch.
     * \details ProcessorStopwatch behaves like SystemStopwatch except that
     * all measured times are given in microseconds.
     */
    class GEOGRAM_API ProcessorStopwatch {
    public:
        /**
         * \brief ProcessorStopwatch constructor
         * \details It remembers the current time as the reference time
         * for functions elapsed_time() and print_elapsed_time().
         */
        ProcessorStopwatch() {
            start_ = now();
        }

        /**
         * \details Gets the current time (in microseconds).
         */
        static Numeric::uint64 now();

        /**
         * \brief Get the elapsed time
         * \details Returns time elapsed since the ProcessorStopwatch
         * construction (in microseconds)
         */
        Numeric::uint64 elapsed_time() const {
            return now() - start_;
        }

    private:
        Numeric::uint64 start_;
    };

    /************************************************************************/

    /**
     * \brief Scope restricted stopwatch
     * \details Stopwatch prints the elapsed time
     * since its construction when it goes out of scope.
     * It uses SystemStopwatch to measure time.
     *
     * \code
     * {
     *     Stopwatch W("compute my stuff") ;
     *     ... do something ...
     * } // <- W prints the elapsed time here.
     * \endcode
     */
    class GEOGRAM_API Stopwatch {
    public:
        /**
         * \brief Stopwatch constructor
         * \param[in] task_name name of the job to measure. This name is
         * used as a Logger feature when displaying the elapsed time.
	 * \param[in] verbose if true, then elapsed time is displayed
	 *  when this Stopwatch is destroyed, else nothing is displayed.
         */
        Stopwatch(const std::string& task_name, bool verbose=true) :
  	    task_name_(task_name), verbose_(verbose) {
        }

        /**
         * \brief Get the user elapsed time
         * \details Returns the user time elapsed since the SystemStopWatch
         * construction (in seconds)
         */
        double elapsed_time() const {
            return W_.elapsed_user_time();
        }


        /**
         * \brief Stopwatch destructor
         * \details This prints the time epalsed since the Stopwatch
         * construction
         */
        ~Stopwatch() {
	    if(verbose_) {
		Logger::out(task_name_)
		    << "Elapsed time: " << W_.elapsed_user_time()
		    << " s" << std::endl;
	    }
        }

        

    private:
        std::string task_name_;
	bool verbose_;
        SystemStopwatch W_;
    };
}

#endif

