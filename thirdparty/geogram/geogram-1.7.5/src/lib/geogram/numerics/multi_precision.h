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

#ifndef GEOGRAM_NUMERICS_MULTI_PRECISION
#define GEOGRAM_NUMERICS_MULTI_PRECISION

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/assert.h>
#include <iostream>
#include <new>

/**
 * \file geogram/numerics/multi_precision.h
 * \brief Implementation of multi-precision arithmetics
 * \details
 *  Multi-precision arithmetics based on expansions, as described by
 *  Jonathan Shewchuk in:
 *  Adaptive Precision Floating-Point Arithmetic and Fast Robust
 *  Geometric Predicates,
 *  Discrete & Computational Geometry 18(3):305-363, October 1997
 */

namespace GEO {

    extern double expansion_splitter_;
    extern double expansion_epsilon_;

    /**
     * \brief Sums two doubles into a length 2 expansion.
     * \details By Jonathan Shewchuk.
     * \param[in] a one of the numbers to sum.
     * \param[in] b the other numbers to sum.
     * \param[out] x high-magnitude component of the result.
     * \param[out] y low-magnitude component of the result.
     * \relates expansion
     */
    inline void two_sum(double a, double b, double& x, double& y) {
        x = a + b;
        double bvirt = x - a;
        double avirt = x - bvirt;
        double bround = b - bvirt;
        double around = a - avirt;
        y = around + bround;
    }

    /**
     * \brief Subtracts two doubles into a length 2 expansion.
     * \details By Jonathan Shewchuk.
     * \param[in] a first number.
     * \param[in] b the number to subtract from \p a.
     * \param[out] x high-magnitude component of the result.
     * \param[out] y low-magnitude component of the result.
     * \relates expansion
     */
    inline void two_diff(double a, double b, double& x, double& y) {
        x = a - b;
        double bvirt = a - x;
        double avirt = x + bvirt;
        double bround = bvirt - b;
        double around = a - avirt;
        y = around + bround;
    }

    /**
     * \brief Splits a number into two components, ready for
     * computing a product.
     * \details By Jonathan Shewchuk.
     * \param[in] a input number.
     * \param[out] ahi split number, high-magnitude part.
     * \param[out] alo split number, low-magnitude part.
     * \relates expansion
     */
    inline void split(double a, double& ahi, double& alo) {
        double c = expansion_splitter_ * a;
        double abig = c - a;
        ahi = c - abig;
        alo = a - ahi;
    }

    /**
     * \brief Multiplies two doubles into a length 2 expansion.
     * \details By Jonathan Shewchuk.
     * \param[in] a first number to multiply.
     * \param[in] b second number to multiply.
     * \param[out] x high-magnitude component of the result.
     * \param[out] y low-magnitude component of the result.
     * \relates expansion
     */
    inline void two_product(double a, double b, double& x, double& y) {
#ifdef FP_FAST_FMA
        // If the target processor supports the FMA (Fused Multiply Add)
        // instruction, then the product of two doubles into a length-2
        // expansion can be implemented as follows. Thanks to Marc Glisse
        // for the information.
        // Note: under gcc, automatic generations of fma() for a*b+c needs
        // to be deactivated, using -ffp-contract=off, else it may break
        // other functions such as fast_expansion_sum_zeroelim().
        x = a*b;
        y = fma(a,b,-x);
#else
        x = a * b;
        double ahi, alo;
        split(a, ahi, alo);
        double bhi, blo;
        split(b, bhi, blo);
        double err1 = x - (ahi * bhi);
        double err2 = err1 - (alo * bhi);
        double err3 = err2 - (ahi * blo);
        y = (alo * blo) - err3;
#endif
    }

    /**
     * \brief Squares a number into a length 2 expansion.
     * \details By Jonathan Shewchuk.
     * \param[in] a number to square.
     * \param[out] x high-magnitude component of the result.
     * \param[out] y low-magnitude component of the result.
     * \relates expansion
     */
    inline void square(double a, double& x, double& y) {
#ifdef FP_FAST_FMA
        // If the target processor supports the FMA (Fused Multiply Add)
        // instruction, then the product of two doubles into a length-2
        // expansion can be implemented as follows. Thanks to Marc Glisse
        // for the information.
        // Note: under gcc, automatic generations of fma() for a*b+c needs
        // to be deactivated, using -ffp-contract=off, else it may break
        // other functions such as fast_expansion_sum_zeroelim().
        x = a*a;
        y = fma(a,a,-x);
#else
        x = a * a;
        double ahi, alo;
        split(a, ahi, alo);
        double err1 = x - (ahi * ahi);
        double err3 = err1 - ((ahi + ahi) * alo);
        y = (alo * alo) - err3;
#endif
    }

    /************************************************************************/

    /**
     * \brief Represents numbers in arbitrary precision with a low-level API.
     * \details The three basic operations sum, difference and product
     *  are implemented, as well as some geometric functions
     *  (squared distance and dot product). The sign of
     *  an expansion can be exactly computed. expansion
     *  is useful to implement exact geometric predicates.
     *  Some of Jonathan Shewchuk's expansion manipulation functions
     *  are used.
     *    A higher-level (but less efficient) interface is available
     *  through the \ref expansion_nt class (expansion number type, that
     *  overloads operators). 
     */
    class GEOGRAM_API expansion {
    public:
        /**
         * \brief Gets the length of this expansion.
         * \return the number of components of this expansion
         */
        index_t length() const {
            return length_;
        }

        /**
         * \brief Gets the capacity of this expansion.
         * \return the maximum number of components
         *  that can be stored in this expansion
         */
        index_t capacity() const {
            return capacity_;
        }

        /**
         * \brief Changes the length of an expansion.
         * \param[in] new_length new length of the expansion
         * \pre new_length <= capacity()
         */
        void set_length(index_t new_length) {
            geo_debug_assert(new_length <= capacity());
            length_ = new_length;
        }

        /**
         * \brief Low level access to a component.
         * \return a const reference to the \p i%th component
         *  of this expansion
         */
        const double& operator[] (index_t i) const {
            // Note: we allocate capacity+1 storage
            // systematically, since basic functions
            // may access one additional value (without
            // using it)
            geo_debug_assert(i <= capacity_);
            return x_[i];
        }

        /**
         * \brief Low level access to a component.
         * \return a reference to the \p i%th component
         *  of this expansion
         */
        double& operator[] (index_t i) {
            // Note: we allocate capacity+1 storage
            // systematically, since basic functions
            // may access one additional value (without
            // using it)
            geo_debug_assert(i <= capacity_);
            return x_[i];
        }

        /**
         * \brief Low level access to the array of components.
         * \return a pointer to the array that stores
         *  the components
         */
        double* data() {
            return x_;
        }

        /**
         * \brief Low level access to the array of components.
         * \return a const pointer to the array that stores
         *  the components
         */
        const double* data() const {
            return x_;
        }

        /**
         * \brief Computes the amount of memory required to store
         *  an expansion.
         * \param[in] capa the required capacity
         * \return the total number of bytes required to store
         *  an expansion of capacity \p capa.
         */
        static size_t bytes(index_t capa) {
            // --> 2*sizeof(double) because x_ is declared of size [2]
            // to avoid compiler's warning.
            // --> capa+1 to have an additional 'sentry' at the end
            // because fast_expansion_sum_zeroelim() may access
            // an entry past the end (without using it).
            return
                sizeof(expansion) - 2 * sizeof(double) +
                (capa + 1) * sizeof(double);
        }

        /**
         * \brief Client code should not use this constructor.
         * \details This constructor should not be used by client code,
         *  use new_expansion_on_stack() and new_expansion_on_heap()
         *  instead. This constructor initializes this expansion's length
         *  and capacity. Note that it should be created with enough space,
         *  using placement syntax of operator new.
         * \param[in] capa the capacity
         */
        expansion(index_t capa) :
            length_(0),
            capacity_(capa) {
        }

        /**
         * \brief Allocates an expansion on the stack.
         * \details It can only be a macro (and not an inline function)
         *  since alloca() cannot be called from inline functions.
         * \arg capa required capacity of the expansion.
         * \relates GEO::expansion
         */
#ifdef CPPCHECK
        // cppcheck does not understand that the result
        // of alloca() is passed to the placement syntax
        // of operator new.
    expansion& new_expansion_on_stack(index_t capa);         
#else
#define new_expansion_on_stack(capa)                           \
    (new (alloca(expansion::bytes(capa)))expansion(capa))
#endif

        /**
         * \brief Allocates an expansion on the heap.
         * \details Allocates also some space for the reference counter.
         * \param[in] capa capacity (i.e. maximum length) of the expansion.
         * \return a pointer to the newly allocated expansion
         */
        static expansion* new_expansion_on_heap(index_t capa);

        /**
         * \brief Deallocates an expansion on the heap.
         * \param[in] e the expansion
         * \pre \p e should have been previously allocated
         *  by new_expansion_on_heap()
         */
        static void delete_expansion_on_heap(expansion* e);

        // ========================== Initialization from doubles

	/**
	 * \brief Assigns a number to this expansion.
	 * \param[in] a the number
	 * \return the new value of this expansion (\p a)
	 */
	expansion& assign(double a) {
	    set_length(1);
	    x_[0] = a;
	    return *this;
	}
	
        /**
         * \brief Computes the required capacity to store the
         *  sum of two doubles.
         * \param[in] a first number
         * \param[in] b second number
         * \return the required capacity of an expansion
         *  to store the exact sum of two doubles
         * \note The result does not depend on the values of the
         *  two numbers.
         */
        static index_t sum_capacity(double a, double b) {
            geo_argused(a);
            geo_argused(b);
            return 2;
        }

        /**
         * \brief Assigns the sum of two doubles to this expansion
         *  (should not be used by client code).
         * \details Do not use directly,
         * use expansion_sum() macro instead.
         * \param[in] a first number to sum
         * \param[in] b second number to sum
         * \return the new value of this expansion (\p a + \p b)
         * \pre capacity() >= sum_capacity(a,b)
         */
        expansion& assign_sum(double a, double b) {
            set_length(2);
            two_sum(a, b, x_[1], x_[0]);
            return *this;
        }

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact difference of two doubles.
         * \param[in] a first number
         * \param[in] b second number
         * \return the required capacity of an expansion
         *  to store the exact difference of two doubles
         * \note The result does not depend on the values of the
         *  two numbers.
         */
        static index_t diff_capacity(double a, double b) {
            geo_argused(a);
            geo_argused(b);
            return 2;
        }

        /**
         * \brief Assigns the difference of two doubles to this expansion
         *  (should not be used by client code).
         * \details Do not use directly,
         * use expansion_diff() macro instead.
         * \param[in] a first number
         * \param[in] b second number
         * \return the new value of this expansion (\p a - \p b)
         * \pre capacity() >= diff_capacity(a,b)
         */
        expansion& assign_diff(double a, double b) {
            set_length(2);
            two_diff(a, b, x_[1], x_[0]);
            return *this;
        }

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact product of two doubles.
         * \param[in] a first number
         * \param[in] b second number
         * \return the required capacity of an expansion
         *  to store the exact product of two doubles
         * \note The result does not depend on the values of the
         *  two numbers.
         */
        static index_t product_capacity(double a, double b) {
            geo_argused(a);
            geo_argused(b);
            return 2;
        }

        /**
         * \brief Assigns the product of two doubles to this expansion
         *  (should not be used by client code).
         * \details Do not use directly,
         * use expansion_product() macro instead.
         * \param[in] a first number
         * \param[in] b second number
         * \return the new value of this expansion (\p a * \p b)
         * \pre capacity() >= product_capacity(a,b)
         */
        expansion& assign_product(double a, double b) {
            set_length(2);
            two_product(a, b, x_[1], x_[0]);
            return *this;
        }

        /**
         * \brief Computes the required capacity of an expansion
         * to store the exact square of a double.
         * \param[in] a the number to be squared
         * \return the required capacity of an expansion
         * to store the exact square of a double.
         * \note The result does not depend on the value of the
         *  number \p a.
         */
        static index_t square_capacity(double a) {
            geo_argused(a);
            return 2;
        }

        /**
         * \brief Assigns the square of a double to this expansion
         *  (should not be used by client code).
         * \details Do not use directly,
         * use expansion_square() macro instead.
         * \param[in] a the number to be squared
         * \return the new value of this expansion (\p a * \p a)
         * \pre capacity() >= square_capacity(a)
         */
        expansion& assign_square(double a) {
            set_length(2);
            square(a, x_[1], x_[0]);
            return *this;
        }

        // ====== Initialization from expansion and double

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact sum of an expansion and a double.
         * \param[in] a first number as an expansion
         * \param[in] b second number as a double
         * \return the required capacity of an expansion
         *  to store the exact sum of \p a and \p b
         * \note The result does not depend on the value of the
         *  double argument \p b.
         */
        static index_t sum_capacity(const expansion& a, double b) {
            geo_argused(b);
            return a.length() + 1;
        }

        /**
         * \brief Assigns the sum of an expansion and a double
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_sum() macro instead.
         * \param[in] a the expansion
         * \param[in] b the double
         * \return the new value of this expansion (\p a + \p b)
         * \pre capacity() >= sum_capacity(a,b)
         */
        expansion& assign_sum(const expansion& a, double b);

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact difference between an expansion and a double.
         * \param[in] a first number as an expansion
         * \param[in] b second number as a double
         * \return the required capacity of an expansion
         *  to store the exact difference of \p a and \p b
         * \note The result does not depend on the value of the
         *  double argument \p b.
         */
        static index_t diff_capacity(const expansion& a, double b) {
            geo_argused(b);
            return a.length() + 1;
        }

        /**
         * \brief Assigns the difference between an expansion and a double
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_diff() macro instead.
         * \param[in] a the expansion
         * \param[in] b the double
         * \return the new value of this expansion (\p a - \p b)
         * \pre capacity() >= diff_capacity(a,b)
         */
        expansion& assign_diff(const expansion& a, double b);

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact product between an expansion and a double.
         * \param[in] a the expansion
         * \param[in] b the double
         * \return the required capacity of an expansion to store
         *  the exact product between \p a and \p b
         * \note The result does not depend on the value of the
         *  double argument \p b.
         */
        static index_t product_capacity(const expansion& a, double b) {
            geo_argused(b);
            // TODO: implement special case where the double argument
            // is a power of two.
            return a.length() * 2;
        }

        /**
         * \brief Assigns the product between an expansion and a double
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_product() macro instead.
         * \param[in] a the expansion
         * \param[in] b the double
         * \return the new value of this expansion (\p a * \p b)
         * \pre capacity() >= product_capacity(a,b)
         */
        expansion& assign_product(const expansion& a, double b);

        // ========================== Initialization from expansions

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact sum of two expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the required capacity of an expansion
         *  to store the exact sum of \p a and \p b
         */
        static index_t sum_capacity(const expansion& a, const expansion& b) {
            return a.length() + b.length();
        }

        /**
         * \brief Assigns the sum of two expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_sum() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the new value of this expansion (\p a + \p b)
         * \pre capacity() >= sum_capacity(a,b)
         */
        expansion& assign_sum(const expansion& a, const expansion& b);

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact sum of three expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \return the required capacity of an expansion
         *  to store the exact sum of \p a, \p b and \p c
         */
        static index_t sum_capacity(
            const expansion& a, const expansion& b, const expansion& c
        ) {
            return a.length() + b.length() + c.length();
        }

        /**
         * \brief Assigns the sum of three expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_sum3() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \return the new value of this expansion (\p a + \p b + \p c)
         * \pre capacity() >= sum_capacity(a,b,c)
         */
        expansion& assign_sum(
            const expansion& a, const expansion& b, const expansion& c
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact sum of four expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \param[in] d fourth expansion
         * \return the required capacity of an expansion
         *  to store the exact sum of \p a, \p b, \p c and \p d
         */
        static index_t sum_capacity(
            const expansion& a, const expansion& b,
            const expansion& c, const expansion& d
        ) {
            return a.length() + b.length() + c.length() + d.length();
        }

        /**
         * \brief Assigns the sum of four expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_sum4() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \param[in] d fourth expansion
         * \return the new value of this expansion (\p a + \p b + \p c + \p d)
         * \pre capacity() >= sum_capacity(a,b,c,d)
         */
        expansion& assign_sum(
            const expansion& a, const expansion& b,
            const expansion& c, const expansion& d
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact difference of two expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the required capacity of an expansion
         *  to store the exact difference between \p a and \p b
         */
        static index_t diff_capacity(const expansion& a, const expansion& b) {
            return a.length() + b.length();
        }

        /**
         * \brief Assigns the difference between two expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_diff() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the new value of this expansion (\p a - \p b)
         * \pre capacity() >= diff_capacity(a,b)
         */
        expansion& assign_diff(const expansion& a, const expansion& b);

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact product of two expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the required capacity of an expansion
         *  to store the exact product of \p a and \p b
         */
        static index_t product_capacity(
            const expansion& a, const expansion& b
        ) {
            return a.length() * b.length() * 2;
        }

        /**
         * \brief Assigns the product of two expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_product() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \return the new value of this expansion (\p a * \p b)
         * \pre capacity() >= product_capacity(a,b)
         */
        expansion& assign_product(const expansion& a, const expansion& b);

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact product of three expansions.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \return the required capacity of an expansion
         *  to store the exact product of \p a, \p b and \p c
         */
        static index_t product_capacity(
            const expansion& a, const expansion& b, const expansion& c
        ) {
            return a.length() * b.length() * c.length() * 4;
        }

        /**
         * \brief Assigns the product of three expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_product3() macro instead.
         * \param[in] a first expansion
         * \param[in] b second expansion
         * \param[in] c third expansion
         * \return the new value of this expansion (\p a * \p b * \p c)
         * \pre capacity() >= product_capacity(a,b,c)
         */
        expansion& assign_product(
            const expansion& a, const expansion& b, const expansion& c
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact square of an expansion.
         * \param[in] a the expansion to be squared
         * \return the required capacity of an expansion
         *  to store the exact square \p a * \p a
         */
        static index_t square_capacity(const expansion& a) {
            if(a.length() == 2) {
                return 6;
            }                                  // see two_square()
            return a.length() * a.length() * 2;
        }

        /**
         * \brief Assigns the product of an expansions
         * to this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_square() macro instead.
         * \param[in] a the expansion to be squared
         * \return the new value of this expansion (\p a * \p a )
         * \pre capacity() >= square_capacity(a)
         */
        expansion& assign_square(const expansion& a);

        // ====== Determinants =============================

        /**
         * \brief Computes the required capacity of an expansion
         *  to store an exact 2x2 determinant.
         * \param[in] a11 , a12 , a21 , a22 coefficients of the determinant
         * \return the required capacity of an expansion to store
         *  the exact determinant \p a11 * \p a22 - \p a21 * \p a12
         */
        static index_t det2x2_capacity(
            const expansion& a11, const expansion& a12,
            const expansion& a21, const expansion& a22
        ) {
            return
                product_capacity(a11, a22) +
                product_capacity(a21, a12);
        }

        /**
         * \brief Assigns a 2x2 determinant to this expansion
         *  (should not be used by client code).
         * \details Do not use directly, use expansion_det2x2()
         * macro instead.
         * \param[in] a11 , a12 , a21 , a22 coefficients of the determinant
         * \return the new value of this expansion, with
         *  the exact determinant \p a11 * \p a22 - \p a21 * \p a12
         * \pre capacity() >= det_2x2_capacity(a11,a12,,a21,a22)
         */
        expansion& assign_det2x2(
            const expansion& a11, const expansion& a12,
            const expansion& a21, const expansion& a22
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store an exact 3x3 determinant.
         * \param[in] a11 , a12 , a13 , a21 , a22 , a23 , a31 , a32 , a33
         *   coefficients of the determinant
         * \return the required capacity of an expansion to store
         *  the exact value of the determinant
         */
        static index_t det3x3_capacity(
            const expansion& a11, const expansion& a12, const expansion& a13,
            const expansion& a21, const expansion& a22, const expansion& a23,
            const expansion& a31, const expansion& a32, const expansion& a33
        ) {
            // Development w.r.t. first row
            index_t c11_capa = det2x2_capacity(a22, a23, a32, a33);
            index_t c12_capa = det2x2_capacity(a21, a23, a31, a33);
            index_t c13_capa = det2x2_capacity(a21, a22, a31, a32);
            return 2 * (
                a11.length() * c11_capa +
                a12.length() * c12_capa +
                a13.length() * c13_capa
            );
        }

        /**
         * \brief Assigns a 3x3 determinant to this expansion
         *  (should not be used by client code).
         * \details Do not use directly, use expansion_det3x3()
         * macro instead.
         * \param[in] a11 , a12 , a13 , a21 , a22 , a23 , a31 , a32 , a33
         *   coefficients of the determinant
         * \return the new value of this expansion, with
         *  the exact 3x3 determinant
         * \pre capacity() >=
         *  det_3x3_capacity(a11,a12,a13,a21,a22,a23,a31,a32,a33)
         */
        expansion& assign_det3x3(
            const expansion& a11, const expansion& a12, const expansion& a13,
            const expansion& a21, const expansion& a22, const expansion& a23,
            const expansion& a31, const expansion& a32, const expansion& a33
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store an exact 3x3 determinant where the
         *  first row is 1 1 1.
         * \param[in] a21 , a22 , a23 , a31 , a32 , a33 coefficients 
         *  of the determinant
         * \return the required capacity of an expansion to store
         *  the exact value of the determinant
         */
        static index_t det_111_2x3_capacity(
            const expansion& a21, const expansion& a22, const expansion& a23,
            const expansion& a31, const expansion& a32, const expansion& a33
        ) {
            return
                det2x2_capacity(a22, a23, a32, a33) +
                det2x2_capacity(a23, a21, a33, a31) +
                det2x2_capacity(a21, a22, a31, a32);
        }

        /**
         * \brief Assigns a 3x3 determinant to this expansion
         *  where the first row is 1 1 1(should not be used by client code).
         * \details Do not use directly, use expansion_det_111_3x3()
         * macro instead.
         * \param[in] a21 , a22 , a23 , a31 , a32 , a33 coefficients 
         *  of the determinant
         * \return the new value of this expansion, with
         *  the exact 3x3 determinant
         * \pre capacity() >= det__111_2x3capacity(a21,a22,a23,a31,a32,a33)
         */
        expansion& assign_det_111_2x3(
            const expansion& a21, const expansion& a22, const expansion& a23,
            const expansion& a31, const expansion& a32, const expansion& a33
        );

        // ======= Geometry-specific initializations =======

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact squared distance between two points
         *  of specified dimension.
         * \param[in] dim dimension of the points
         * \return the required capacity of an expansion to store
         *  the exact squared distance between points of dimension \p dim
         */
        static index_t sq_dist_capacity(coord_index_t dim) {
            return index_t(dim) * 6;
        }

        /**
         * \brief Assigns the squared distance between two points to
         *  this expansion (should not be used by client code).
         * \details Do not use directly,
         *  use expansion_sq_dist() macro instead.
         * \param[in] p1 pointer to the coordinates of the first point
         * \param[in] p2 pointer to the coordinates of the second point
         * \param[in] dim dimension of the points
         * \return the new value of this expansion, with the squared distance
         *  between \p p1 and \p p2
         * \pre capacity() >= sq_dist_capacity(dim)
         */
        expansion& assign_sq_dist(
            const double* p1, const double* p2, coord_index_t dim
        );

        /**
         * \brief Computes the required capacity of an expansion
         *  to store the exact dot product between two vectors.
         * \param[in] dim dimension of the vectors
         * \return the required capacity of an expansion to store
         *  the dot product between two \p dim%-dimensional vectors
         *  specified by differences of doubles.
         */
        static index_t dot_at_capacity(coord_index_t dim) {
            return index_t(dim) * 8;
        }

        /**
         * \brief Assigns the dot product of two vectors to
         *  this expansion (should not be used by client code).
         * \details Do not use directly,
         * use expansion_dot_at() macro instead.
         * \param[in] p1 pointer to the coordinates of a point
         * \param[in] p2 pointer to the coordinates of a point
         * \param[in] p0 pointer to the coordinates of a point
         * \param[in] dim dimension of the points
         * \return the new value of this expansion, with the dot
         *  product (p1-p0).(p2-p0)
         */
        expansion& assign_dot_at(
            const double* p1, const double* p2, const double* p0,
            coord_index_t dim
        );


        /**
         * \brief Computes the required capacity to store the 
         *  length of a 3d vector.
         * \param[in] x , y , z coordinates of the vector
         * \return the capacity required to store the squared norm
         *   of [x,y,z]
         */
        static index_t length2_capacity(
            const expansion& x, const expansion& y, const expansion& z
        ) {
            return square_capacity(x) + square_capacity(y) + square_capacity(z);
        }

        /**
         * \brief Assigns the length of a vector to this expansion 
         *  (should not be used by client code). Do not call this
         *  function directly, use expansion_length2() macro instead.
         * \param[in] x , y , z coordinates of the vector
         * \return the new value of this expansion, with the squared
         *  length of [x,y,z]
         */
        expansion& assign_length2(
            const expansion& x, const expansion& y, const expansion& z
        );
        
        // =============== some general purpose functions =========

        /**
         * \brief Initializes the expansion class.
         * \details This function needs to be called once in the program,
         *  before using any expansion object and operation (it computes
         *  some internally-used constants).
         */
        static void initialize();

        /**
         * \brief Changes the sign of an expansion.
         * \return the new value of this expansion
         */
        expansion& negate() {
            for(index_t i = 0; i < length_; ++i) {
                x_[i] = -x_[i];
            }
            return *this;
        }

        /**
         * \brief Multiplies this expansion by a power of two.
         * \param[in] s the factor to be used to scale this expansion.
         * \return the new value of this expansion
         * \note Does not check for overflows/underflows
         * \pre \p s should be a (possibly negative) power of two.
         */
        expansion& scale_fast(double s) {
            // TODO: debug assert is_power_of_two(s)
            for(index_t i = 0; i < length_; ++i) {
                x_[i] *= s;
            }
            return *this;
        }

        /**
         * \brief Computes an approximation of the stored
         *  value in this expansion.
         * \return an approximation of the stored value.
         */
        double estimate() const {
            double result = 0.0;
            for(index_t i = 0; i < length(); ++i) {
                result += x_[i];
            }
            return result;
        }

        /**
         * \brief Gets the sign of the expansion.
         * \return the sign of this expansion, computed exactly.
         */
        Sign sign() const {
            if(length() == 0) {
                return ZERO;
            }
            return geo_sgn(x_[length() - 1]);
        }

        /**
         * \brief Displays all the components of this expansion
         * (for debugging purposes).
         * \param[out] os an output stream used to print the components
         */
        std::ostream& show(std::ostream& os) const {
            for(index_t i = 0; i < length(); ++i) {
                os << i << ':' << x_[i] << ' ';
            }
            return os << std::endl;
        }

    protected:
        /**
         * \brief Computes the required capacity of an expansion
         *  to store an exact sub-product.
         * \param[in] a_length number of components in first sub-expansion
         * \param[in] b_length number of components in second sub-expansion
         * \return the required capacity of an expansion to store the
         *  exact product of two expansions of lengths \p a_length
         *  and \p b_length
         */
        static index_t sub_product_capacity(
            index_t a_length, index_t b_length
        ) {
            return a_length * b_length * 2;
        }

        /**
         * \brief Assigns a sub-product to this expansion.
         * \param[in] a a pointer to the first component of the first term
         * \param[in] a_length number of components in first term
         * \param[in] b second term
         * \return the new value of this expansion, with [a1...a_length]*b.
         */
        expansion& assign_sub_product(
            const double* a, index_t a_length, const expansion& b
        );

        /**
         * \brief Computes an expansion that represents the exact
         *  product of its arguments (a[0]+a[1]+ ... +a[length-1])*b.
         *
         * \details Used internally to implement product ("distillation", see
         *  Shewchuk's paper).
         *
         * \param[in] a an expansion, specified as a const double*
         * \param[in] a_length number of coefficients in a
         * \param[in] b an expansion, specified as a const expansion&
         *
         * \return a reference to an expansion, allocated on the stack.
         * \warning Do not return or use the returned reference outside the
         * calling function.
         * \relates GEO::expansion
         */
#define expansion_sub_product(a, a_length, b)           \
    new_expansion_on_stack(                       \
        sub_product_capacity(a_length, b.length()) \
    )->assign_sub_product(a, a_length, b)

    private:
        /**
         * \brief Expansion%s cannot be copied.
         */
        expansion(const expansion& rhs);

        /**
         * \brief Expansion%s cannot be copied.
         */
        expansion& operator= (const expansion& rhs);

    private:
        index_t length_;
        index_t capacity_;
        double x_[2];  // x_ is in fact of size [capacity_]

        friend class expansion_nt;
    };

    // =============== arithmetic operations ===========================

    /**
     * \brief Creates an expansion from a double.
     * \param[in] a the double
     * \return a reference to an expansion, allocated on the
     *  stack.
     * \code
     *  const expansion& e1 = expansion_create(a);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     *  calling function.
     * \relates GEO::expansion
     */
#define expansion_create(a)	      \
    new_expansion_on_stack(1)->assign(a)

    
    /**
     * \brief Computes an expansion that represents the exact
     *  sum of its arguments.
     * \param[in] a a double or an expansion
     * \param[in] b a double or an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = expansion_sum(e1,e2);
     * double x = ...;
     * expansion& e4 = expansion_sum(e1,x);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_sum(a, b)            \
    new_expansion_on_stack(           \
        expansion::sum_capacity(a, b)   \
    )->assign_sum(a, b)

    /**
     * \brief Computes an expansion that represents the exact
     *  sum of its arguments.
     * \param[in] a an expansion
     * \param[in] b an expansion
     * \param[in] c an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = ...;
     * expansion& e4 = expansion_sum3(e1,e2,e3);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_sum3(a, b, c)          \
    new_expansion_on_stack(            \
        expansion::sum_capacity(a, b, c) \
    )->assign_sum(a, b, c)

    /**
     * \brief Computes an expansion that represents the exact
     *  sum of its arguments.
     * \param[in] a an expansion
     * \param[in] b an expansion
     * \param[in] c an expansion
     * \param[in] d an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = ...;
     * expansion& e4 = ...;
     * expansion& e5 = expansion_sum4(e1,e2,e3,e4);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */

#define expansion_sum4(a, b, c, d)          \
    new_expansion_on_stack(              \
        expansion::sum_capacity(a, b, c, d) \
    )->assign_sum(a, b, c, d)

    /**
     * \brief Computes an expansion that represents the exact
     *  difference of its arguments.
     * \param[in] a a double or an expansion
     * \param[in] b a double or an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = expansion_diff(e1,e2);
     * double x = ...;
     * expansion& e4 = expansion_diff(e1,x);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_diff(a, b)             \
    new_expansion_on_stack(             \
        expansion::diff_capacity(a, b)   \
    )->assign_diff(a, b)

    /**
     * \brief Computes an expansion that represents the exact
     *  product of its arguments.
     * \param[in] a a double or an expansion
     * \param[in] b a double or an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = expansion_product(e1,e2);
     * double x = ...;
     * expansion& e4 = expansion_product(e1,x);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_product(a, b)            \
    new_expansion_on_stack(               \
        expansion::product_capacity(a, b)  \
    )->assign_product(a, b)

    /**
     * \brief Computes an expansion that represents the exact
     *  product of its arguments.
     * \param[in] a an expansion
     * \param[in] b an expansion
     * \param[in] c an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = ...;
     * expansion& e3 = ...;
     * expansion& e4 = expansion_product3(e1,e2,e3);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_product3(a, b, c)           \
    new_expansion_on_stack(                 \
        expansion::product_capacity(a, b, c)  \
    )->assign_product(a, b, c)

    /**
     * \brief Computes an expansion that represents the exact
     *  square of its argument.
     * \param[in] a a double or an expansion
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * expansion& e1 = ...;
     * expansion& e2 = expansion_square(e1);
     * double x = ...;
     * expansion& e3 = expansion_square(x);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_square(a)             \
    new_expansion_on_stack(             \
        expansion::square_capacity(a)   \
    )->assign_square(a)

    // =============== determinants =====================================

    /**
     * \brief Computes an expansion that represents the exact
     *  2x2 determinant of its arguments.
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const expansion& a11 = ...;
     * const expansion& a12 = ...;
     * const expansion& a21 = ...;
     * const expansion& a22 = ...;
     * expansion& d12 = expansion_set2x2(a11,a12,a21,a22);
     * \endcode
     * \relates GEO::expansion
     */
#define expansion_det2x2(a11, a12, a21, a22)          \
    new_expansion_on_stack(                        \
        expansion::det2x2_capacity(a11, a12, a21, a22) \
    )->assign_det2x2(a11, a12, a21, a22)

    /**
     * \brief Computes an expansion that represents the exact
     *  3x3 determinant of its arguments.
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const expansion& a11 = ...;
     * ...
     * const expansion& a33 = ...;
     * expansion& d = expansion_det3x3(
     *     a11,a12,a13,a21,a22,a23,a31,a32,a33
     * );
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_det3x3(a11, a12, a13, a21, a22, a23, a31, a32, a33)   \
    new_expansion_on_stack(                                             \
        expansion::det3x3_capacity(a11,a12,a13,a21,a22,a23,a31,a32,a33) \
    )->assign_det3x3(a11, a12, a13, a21, a22, a23, a31, a32, a33)

    /**
     * \brief Computes an expansion that represents the exact
     *  3x3 determinant of its arguments where the first row
     *  is 1 1 1.
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const expansion& a21 = ...;
     * ...
     * const expansion& a33 = ...;
     * expansion& d = expansion_det_111_2x3(
     *     a21,a22,a23,a31,a32,a33
     * );
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_det_111_2x3(a21, a22, a23, a31, a32, a33)           \
    new_expansion_on_stack(                                      \
        expansion::det_111_2x3_capacity(a21, a22, a23, a31, a32, a33) \
    )->assign_det_111_2x3(a21, a22, a23, a31, a32, a33)

    // =============== geometric functions ==============================

    /**
     * \brief Computes an expansion that represents the exact
     *  squared distance between its argument.
     * \param[in] a first point (specified as a const double*)
     * \param[in] b second point (specified as a const double*)
     * \param[in] dim dimension of the points
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const double* p1 = ...;
     * const double* p2 = ...;
     * expansion& d12 = expansion_sq_dist(p1,p2,3);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_sq_dist(a, b, dim)           \
    new_expansion_on_stack(                  \
        expansion::sq_dist_capacity(dim)     \
    )->assign_sq_dist(a, b, dim)

    /**
     * \brief Computes an expansion that represents the exact
     *  dot product dot(a-c,b-c)
     * \param[in] a first point (specified as a const double*)
     * \param[in] b second point (specified as a const double*)
     * \param[in] c third point (specified as a const double*)
     * \param[in] dim dimension of the points
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const double* p1 = ...;
     * const double* p2 = ...;
     * const double* p0 = ...;
     * expansion& dot12 = expansion_dot_at(p1,p2,p0,3);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_dot_at(a, b, c, dim)           \
    new_expansion_on_stack(                   \
        expansion::dot_at_capacity(dim)       \
    )->assign_dot_at(a, b, c, dim)


    /**
     * \brief Computes an expansion that represents the exact
     *  squared length of a 3d vector
     * \param[in] x,y,z coordinates of the vector (specified as expansion)
     * \return a reference to an expansion, allocated on the stack.
     * \code
     * const expansion& x = ...;
     * const expansion& y = ...;
     * const expansion& z = ...;
     * expansion& l = expansion_length2(x,y,z);
     * \endcode
     * \warning Do not return or use the returned reference outside the
     * calling function.
     * \relates GEO::expansion
     */
#define expansion_length2(x,y,z)              \
    new_expansion_on_stack(                   \
       expansion::length2_capacity(x,y,z)     \
    )->assign_length2(x,y,z)
    
    /************************************************************************/

    /**
     * \brief Computes the sign of a 2x2 determinant
     * \details Specialization using the low-evel API for expansions. 
     *  This gains some performance as compared to using CGAL's 
     *  determinant template with expansion_nt.
     */
    Sign GEOGRAM_API sign_of_expansion_determinant(
        const expansion& a00,const expansion& a01,  
        const expansion& a10,const expansion& a11
    );
    
    /**
     * \brief Computes the sign of a 3x3 determinant
     * \details Specialization using the low-evel API for expansions. 
     *  This gains some performance as compared to using CGAL's determinant 
     *  template with expansion_nt.
     */
    Sign GEOGRAM_API sign_of_expansion_determinant(
        const expansion& a00,const expansion& a01,const expansion& a02,
        const expansion& a10,const expansion& a11,const expansion& a12,
        const expansion& a20,const expansion& a21,const expansion& a22
    );

    /**
     * \brief Computes the sign of a 4x4 determinant
     * \details Specialization using the low-evel API for expansions. 
     *  This gains some performance as compared to using CGAL's determinant 
     *  template with expansion_nt.
     */
    Sign GEOGRAM_API sign_of_expansion_determinant(
        const expansion& a00,const expansion& a01,
        const expansion& a02,const expansion& a03,
        const expansion& a10,const expansion& a11,
        const expansion& a12,const expansion& a13,
        const expansion& a20,const expansion& a21,
        const expansion& a22,const expansion& a23,
        const expansion& a30,const expansion& a31,
        const expansion& a32,const expansion& a33 
    );
    
    /************************************************************************/
}

#endif

