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

#ifndef GEOGRAM_BASIC_ASSERT
#define GEOGRAM_BASIC_ASSERT

#include <geogram/basic/common.h>
#include <string>

/**
 * \file geogram/basic/assert.h
 * \brief Assertion checking mechanism
 */

namespace GEO {

    /**
     * \brief Assert termination mode
     * \details Defines how assertion failures should terminate the program.
     * By default, Assertion failures throw an exception.
     */
    enum AssertMode {
        /** Assertion failures throw an exception */
        ASSERT_THROW,
        /** Assertion failures call abort() */
        ASSERT_ABORT,
	/** Assertion failures generate a breakpoint in the debugger */
	ASSERT_BREAKPOINT
    };

    /**
     * \brief Sets assertion mode.
     * \param[in] mode assert termination mode
     * \see AssertMode
     */
    void GEOGRAM_API set_assert_mode(AssertMode mode);

    /**
     * \brief Returns the current assert termination mode
     */
    AssertMode GEOGRAM_API assert_mode();

    /**
     * \brief Aborts the program
     * \details On Linux, this calls the system function abort(). On Windows,
     * abort() is more difficult to see under debugger, so this creates a
     * segmentation fault by deferencing a null pointer.
     */
    GEO_NORETURN_DECL void GEOGRAM_API geo_abort() GEO_NORETURN;

    /**
     * \brief Generates a debugger breakpoint programmatically.
     * \details On Windows, generates a breakpoint using __debugbreak(),
     *  on other systems, calls geo_abort().
     */
    GEO_NORETURN_DECL void GEOGRAM_API geo_breakpoint() GEO_NORETURN;
    
    /**
     * \brief Prints an assertion failure
     * \details This function is called when a boolean condition is not met.
     * It prints an error message and terminates the program according to
     * the current assert termination mode.
     * \param[in] condition_string string representation of the condition
     * \param[in] file file where the assertion failed
     * \param[in] line line where the assertion failed
     */
    GEO_NORETURN_DECL void GEOGRAM_API geo_assertion_failed(
        const std::string& condition_string,
        const std::string& file, int line
    ) GEO_NORETURN;

    /**
     * \brief Prints a range assertion failure
     * \details This function is called when a value is out of a legal range.
     * It prints an error message and terminates the program according to
     * the current assert termination mode.
     * \param[in] value the illegal value
     * \param[in] min_value minimum allowed value
     * \param[in] max_value maximum allowed value
     * \param[in] file file where the assertion failed
     * \param[in] line line where the assertion failed
     */
    GEO_NORETURN_DECL void GEOGRAM_API geo_range_assertion_failed(
        double value, double min_value, double max_value,
        const std::string& file, int line
    ) GEO_NORETURN;

    /**
     * \brief Prints an unreachable location failure
     * \details This function is called when execution reaches a point that it
     * should not reach. It prints an error message and terminates the
     * program according to the current assert termination mode.
     * \param[in] file file containing the unreachable location
     * \param[in] line line of the unreachable location
     */
    GEO_NORETURN_DECL void GEOGRAM_API geo_should_not_have_reached(
        const std::string& file, int line
    ) GEO_NORETURN;
}

// Three levels of assert:
// use geo_assert() and geo_range_assert()               non-expensive asserts
// use geo_debug_assert() and geo_debug_range_assert()   expensive asserts
// use geo_parano_assert() and geo_parano_range_assert() very exensive asserts

/**
 * \brief Verifies that a condition is met
 * \details Checks if the condition \p x. If the condition is false, it prints
 * an error messages and terminates the program.
 * \param[in] x the boolean expression of the condition
 * \see geo_assertion_failed()
 */
#define geo_assert(x) {                                      \
        if(!(x)) {                                               \
            GEO::geo_assertion_failed(#x, __FILE__, __LINE__);   \
        }                                                        \
}

/**
 * \brief Verifies that a value is in a legal range
 * \details Verifies that value \p x is in the range [\p min_value, \p
 * max_value]. If this is false, it prints an error messages and terminates
 * the program.
 * \param[in] x the value to verify
 * \param[in] min_val minimum allowed value
 * \param[in] max_val maximum allowed value
 * \see geo_range_assertion_failed()
 */
#define geo_range_assert(x, min_val, max_val) {              \
        if(((x) < (min_val)) || ((x) > (max_val))) {             \
            GEO::geo_range_assertion_failed(x, min_val, max_val, \
                __FILE__, __LINE__                               \
            );                                                   \
        }                                                        \
}

/**
 * \brief Sets a non reachable point in the program
 * \details
 */
#define geo_assert_not_reached {                             \
        GEO::geo_should_not_have_reached(__FILE__, __LINE__);    \
}

/**
 * \def geo_debug_assert(x)
 * \copydoc geo_assert()
 * \note This assertion check is only active in debug mode.
 */
/**
 * \def geo_debug_range_assert(x, min_val, max_val)
 * \copydoc geo_range_assert()
 * \note This assertion check is only active in debug mode.
 */
#ifdef GEO_DEBUG
#define geo_debug_assert(x) geo_assert(x)
#define geo_debug_range_assert(x, min_val, max_val) geo_range_assert(x, min_val, max_val)
#else
#define geo_debug_assert(x)
#define geo_debug_range_assert(x, min_val, max_val)
#endif

/**
 * \def geo_parano_assert(x)
 * \copydoc geo_assert()
 * \note This assertion check is only active in paranoid mode.
 */
/**
 * \def geo_parano_range_assert(x, min_val, max_val)
 * \copydoc geo_range_assert()
 * \note This assertion check is only active in paranoid mode.
 */
#ifdef GEO_PARANOID
#define geo_parano_assert(x) geo_assert(x)
#define geo_parano_range_assert(x, min_val, max_val) geo_range_assert(x, min_val, max_val)
#else
#define geo_parano_assert(x)
#define geo_parano_range_assert(x, min_val, max_val)
#endif

#endif

