/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
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


