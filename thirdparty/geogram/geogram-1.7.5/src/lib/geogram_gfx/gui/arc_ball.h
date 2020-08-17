/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef H_GEOGRAM_GFX_GLUP_VIEWER_ARCBALL_H
#define H_GEOGRAM_GFX_GLUP_VIEWER_ARCBALL_H

#include <geogram_gfx/basic/common.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram_gfx/gui/arc_ball.h
 * \brief Controls a 3d rotation from user mouse input.
 */

namespace GEO {

    /*******************************************************************/
    
   /**
    * \brief Enables to interactively define a rotation. 
    * \details This class is inspired by an implementation written by 
    *  Paul Rademacher, in his glui library.
    *  Initial documentation by Paul Rademacher:  
    *  A C++ class that implements the Arcball, 
    *  as described by Ken Shoemake in Graphics Gems IV.  
    *  This class takes as input mouse events (mouse down, mouse drag,
    *  mouse up), and creates the appropriate quaternions and 4x4 matrices
    *  to represent the rotation given by the mouse.  
    */
    class GEOGRAM_GFX_API ArcBall {
    public:
        /**
         * \brief ArcBall constructor.
         */
        ArcBall();

        /**
         * \brief Gets the value of the rotation.
         * \return the computed rotation, as a 4x4 matrix
         *  (with a zero translational component).
         */
        const mat4& get_value() const {
            return matrix_;            
        }

        /**
         * \brief Sets the value of the rotation.
         * \param[in] value the rotation, as a 4x4 matrix
         *  (with a zero translational component).
         */
        void set_value(const mat4& value);

        /**
         * \brief Tests whether the X axis is constrained.
         * \retval true if the X axis is constrained
         * \retval false otherwise
         */
        bool get_x_constraint() const {
            return constrain_x_;            
        }

        /**
         * \brief Specifies whether the X axis is constrained.
         * \param[in] value true if the X axis should be constrained,
         *  false otherwise
         */
        void set_x_constraint(bool value) {
            constrain_x_ = value;
        }

        /**
         * \brief Tests whether the Y axis is constrained.
         * \retval true if the Y axis is constrained
         * \retval false otherwise
         */
        bool get_y_constraint() const {
            return constrain_y_;
        }

        /**
         * \brief Specifies whether the Y axis is constrained.
         * \param[in] value true if the Y axis should be constrained,
         *  false otherwise
         */
        void set_y_constraint(bool value) {
            constrain_y_ = value;
        }

        /**
         * \brief Callback called when the mouse is clicked.
         * \param[in] value the picked point, in normalized device
         *  coordinates (x and y both in [-1.0, 1.0]).
         */
        void grab(const vec2& value);

        /**
         * \brief Callback called when the mouse is moved.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void drag(const vec2& value);

        /**
         * \brief Callback called when the mouse button is released.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void release(const vec2& value);

        /**
	 * \brief Resets this ArcBall to the default value.
	 */
        void reset();

        /**
	 * \brief Tests whether this ArcBall is grabbed.
	 * \retval true if this ArcBall is grabbed.
	 * \retval false otherwise.
	 */
        bool grabbed() const {
	    return grabbed_;
	}
    
    protected:
        /**
         * \brief Discards the component of a vector that is 
         *  along another vector.
         * \param[in] vector the input vector
         * \param[in] axis the axis along which the component should
         *  be discarded
         * \return the projection of \p vector onto the plane perpendicular
         *  to \p axis
         */
        vec3 constrain_vector( 
            const vec3& vector, 
            const vec3& axis 
        ) const;

        /**
         * \brief Lifts a 2d screen coordinate to 3d sphere coordinates,
         *  and applies the X or Y axis constraints.
         * \param[in] p the point, in normalized device coordinates (both
         *  X and Y coordinates in [-1.0, 1.0])
         * \return the point lifted on the 3d sphere. X or Y can be constrained
         *  to zero according to the status of get_x_constraint() and 
         *  get_y_constraint() respectively.
         */
        vec3 mouse_to_sphere( const vec2& p );

    private:
        bool grabbed_;
        bool constrain_x_;
        bool constrain_y_;
        vec2 center_;
        double radius_;

        vec2 last_point_;
        mat4 matrix_;
    };

    /*******************************************************************/
    
}
#endif

