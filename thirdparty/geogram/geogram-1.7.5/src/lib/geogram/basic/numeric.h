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

#ifndef GEOGRAM_BASIC_NUMERIC
#define GEOGRAM_BASIC_NUMERIC

#include <geogram/basic/common.h>
#include <cmath>
#include <float.h>
#include <limits.h>
#include <algorithm> // for std::min / std::max

// Visual C++ ver. < 2010 does not have C99 stdint.h,
// using a fallback portable one.
#if defined(GEO_OS_WINDOWS) && (_MSC_VER < 1600)
#include <geogram/third_party/pstdint.h>
#else
#include <stdint.h>
#endif

#include <limits>

#ifndef M_PI
/**
 * \brief Value of the constant PI if not defined by the system
 */
#define M_PI 3.14159265358979323846
#endif

/**
 * \file geogram/basic/numeric.h
 * \brief Types and functions for numbers manipulation
 */

namespace GEO {

    /**
     * \brief Defines numeric types used in Vorpaline.
     * \details
     * These types names have the form (u)int<size> or float<size>,
     * where the (optional) u denotes an unsigned type,
     * and the size is in bits.
     */
    namespace Numeric {

        /** Generic pointer type */
        typedef void* pointer;

        /** Integer type with a width of 8 bits */
        typedef int8_t int8;

        /** Integer type with a width of 16 bits */
        typedef int16_t int16;

        /** Integer type with a width of 32 bits */
        typedef int32_t int32;

        /** Integer type with a width of 64 bits */
        typedef int64_t int64;

        /** Unsigned integer type with a width of 8 bits */
        typedef uint8_t uint8;

        /** Unsigned integer type with a width of 16 bits */
        typedef uint16_t uint16;

        /** Unsigned integer type with a width of 32 bits */
        typedef uint32_t uint32;

        /** Unsigned integer type with a width of 64 bits */
        typedef uint64_t uint64;

        /** Floating point type with a width of 32 bits */
        typedef float float32;

        /** Floating point type with a width of 64 bits */
        typedef double float64;

        /**
         * \brief Gets 32 bits float maximum positive value
         */
        inline float32 max_float32() {
            return std::numeric_limits<float32>::max();
        }

        /**
         * \brief Gets 32 bits float minimum negative value
         */
        inline float32 min_float32() {
            // Note: numeric_limits<>::min() is not
            // what we want (it returns the smallest
            // positive non-denormal).
            return -max_float32();
        }

        /**
         * \brief Gets 64 bits float maximum positive value
         */
        inline float64 max_float64() {
            return std::numeric_limits<float64>::max();
        }

        /**
         * \brief Gets 64 bits float minimum negative value
         */
        inline float64 min_float64() {
            // Note: numeric_limits<>::min() is not
            // what we want (it returns the smallest
            // positive non-denormal).
            return -max_float64();
        }

        /**
         * \brief Checks whether a 32 bits float is "not a number"
         */
        bool GEOGRAM_API is_nan(float32 x);

        /**
         * \brief Checks whether a 64 bits float is "not a number"
         */
        bool GEOGRAM_API is_nan(float64 x);

        /**
         * \brief Resets the random number generator.
         */
        void GEOGRAM_API random_reset();

        /**
         * \brief Returns a 32 bits integer between 0 and RAND_MAX
         */
        int32 GEOGRAM_API random_int32();

        /**
         * \brief Returns a 32 bits float between 0 and 1
         */
        float32 GEOGRAM_API random_float32();

        /**
         * \brief Returns a 64 bits float between 0 and 1
         */
        float64 GEOGRAM_API random_float64();

        /**
         * \brief Limits helper class that extends std::numeric_limits
         * \details LimitsHelper extends std::numeric_limits to provide
         * additional information about numeric types \p T.
         * Template parameter \p is_numeric receives the value \c
         * std::numeric_limits<T>::is_specialized which is \c true for all
         * numeric types and \c false for the other types. The template is
         * specialized for \p is_numeric == \c true to define additional
         * information. For non-numeric types, the default template does not
         * define anything.
         * \tparam T an object type
         * \tparam is_numeric is true if type \p T is a numeric type, false
         * otherwise.
         */
        template <class T, bool is_numeric>
        struct LimitsHelper : std::numeric_limits<T> {
        };

        /**
         * \brief Specialization of LimitsHelper for numeric types
         * \details This specialization defines the following values:
         * - size - the size of the numeric type in bytes
         * - numbits - the size of the numeric type in bits
         * \tparam T a numeric type
         */
        template <class T>
        struct LimitsHelper<T, true> : std::numeric_limits<T> {
            /** The size of the numeric type in bytes */
            static const size_t size = sizeof(T);
            /** The size of the numeric type in bits */
            static const size_t numbits = 8 * sizeof(T);
        };

        /**
         * \brief Extends std::numeric_limits with additional information
         * \details Limits provides additional information about numeric types
         * that are not available in std::numeric_limits:
         * - size: the size of the numeric type in bytes
         * - numbits: the size of the numeric type in bits
         * These types are defined in the helper class LimitsHelper for
         * numeric types only. They are not defined for non-numeric types.
         */
        template <class T>
        struct Limits : 
            LimitsHelper<T, std::numeric_limits<T>::is_specialized> {
        };
    }

    /************************************************************************/

    /**
     * \brief Integer constants that represent the sign of a value
     */
    enum Sign {
        /** Value is negative */
        NEGATIVE = -1,
        /** Value is zero */
        ZERO = 0,
        /** Value is positive */
        POSITIVE = 1
    };

    /**
     * \brief Gets the sign of a value
     * \details Returns -1, 0, or 1 whether value \p x is resp. negative, zero
     * or positive. The function uses operator<() and operator>() to compare
     * the value to 0 (zero). The integer constant zero must make
     * senses for the type of the value, or T must be constructible from
     * integer constant zero.
     * \param[in] x the value to test
     * \tparam T the type of the value
     * \return the sign of the value
     * \see Sign
     */
    template <class T>
    inline Sign geo_sgn(const T& x) {
        return (x > 0) ? POSITIVE : (
            (x < 0) ? NEGATIVE : ZERO
        );
    }

    /**
     * \brief Gets the square value of a value
     * \param[in] x a value of type \p T
     * \tparam T the type of the value
     * \return the square value of \p x
     */
    template <class T>
    inline T geo_sqr(T x) {
        return x * x;
    }

    /**
     * \brief Clamps a value to a range
     * \details Clamps the value \p x to a range defined by \p min and \p max.
     * This modifies the value of \p x directly.
     * \param[in,out] x a value of type \p T
     * \param[in] min the lower bound of the clamping range
     * \param[in] max the upper bound of the clamping range
     */
    template <class T>
    inline void geo_clamp(T& x, T min, T max) {
        if(x < min) {
            x = min;
        } else if(x > max) {
            x = max;
        }
    }

    /**
     * \brief The type for storing and manipulating indices.
     * \internal
     * Vorpaline uses 32 bit indices (can be changed to 64 bits
     * if need be, but this will double memory consumption of
     * all combinatorial data structures).
     */
    typedef geo_index_t index_t;

    /**
     * \brief Gets the maximum positive value of type index_t.
     */
    inline index_t max_index_t() {
        return std::numeric_limits<index_t>::max();
    }

    /**
     * \brief The type for storing and manipulating indices differences.
     * \details Can be negative (for instance to indicate special values like
     * borders).
     */
    typedef geo_signed_index_t signed_index_t;

    /**
     * \brief Gets the maximum positive value of type signed_index_t.
     */
    inline signed_index_t max_signed_index_t() {
        return std::numeric_limits<signed_index_t>::max();
    }

    /**
     * \brief Gets the minimum negative value of type signed_index_t.
     */
    inline signed_index_t min_signed_index_t() {
        return std::numeric_limits<signed_index_t>::min();
    }

    /**
     * \brief The type for storing coordinate indices, and iterating on
     *  the coordinates of a point.
     */
    typedef geo_coord_index_t coord_index_t;

    /**
     * \TODOC
     */
    inline double round(double x) {
	return ((x - floor(x)) > 0.5 ? ceil(x) : floor(x));
    }
}

#endif

