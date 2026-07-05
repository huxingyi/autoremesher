/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */
 

#ifndef H_OGF_IMAGE_TYPES_COLOR_H
#define H_OGF_IMAGE_TYPES_COLOR_H

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram/image/color.h
 * \brief Color types.
 */
namespace GEO {

    /**
     * \brief A generic color type.
     * \tparam T type of the components
     */
    template <class T> class GenColor : public vecng<4,T> {
    public:
        /**
         * \brief The superclass type.
         */
        typedef vecng<4,T> superclass;

        /**
         * \brief GenColor copy constructor
         * \param[in] rhs the GenColor to be copied.
         */
        inline GenColor(const superclass& rhs) : superclass(rhs) {
        }

        /**
         * \brief GenColor constructor from T array.
         * \param[in] rhs a pointer to an array of 4 Ts
         */
        inline GenColor(const T* rhs) : superclass(rhs) {
        }

        /**
         * \brief GenColor constructor from 4 parameters.
         * \param[in] r , g , b , a the components of the color
         */
        inline GenColor(
            T r=0, T g=0, T b=0, T a=1
        ):superclass(r,g,b,a) {
        }

        /**
         * \brief assignment operator.
         * \param[in] rhs the GenColor to be copied
         * \return the new value of this GenColor after assignment
         */
        inline GenColor& operator=(const superclass& rhs) {
            superclass::operator=(rhs);
            return *this;
        }

        /**
         * \brief Gets the red component.
         * \return the value of the red component
         */
        T r() const {
            return superclass::x;
        }

        /**
         * \brief Gets the green component.
         * \return the value of the green component
         */
        T g() const {
            return superclass::y;
        }

        /**
         * \brief Gets the blue component.
         * \return the value of the blue component
         */
        T b() const {
            return superclass::z;
        }

        /**
         * \brief Gets the alpha component (transparency).
         * \return the value of the alpha component
         */
        T a() const {
            return superclass::w;
        }        

        /**
         * \brief Sets the red component.
         * \param[in] r the value of the red component
         */
        void set_r(T r) {
            superclass::x = r;
        }

        /**
         * \brief Sets the green component.
         * \param[in] g the value of the green component
         */
        void set_g(T g) {
            superclass::y = g;
        }

        /**
         * \brief Sets the blue component.
         * \param[in] b the value of the blue component
         */
        void set_b(T b) {
            superclass::z = b;
        }

        /**
         * \brief Sets the alpha component (transparency).
         * \param[in] a the value of the alpha component
         */
        void set_a(T a) {
            superclass::w = a;
        }
        
    };

    /**
     * \brief Default representation of colors.
     * \details r,g,b,a components are double-precision
     *  number between 0.0 and 1.0.
     */
    typedef GenColor<Numeric::float64> Color;
}

#endif


