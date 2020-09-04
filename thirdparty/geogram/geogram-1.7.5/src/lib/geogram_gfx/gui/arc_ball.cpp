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

#include <geogram_gfx/gui/arc_ball.h>
#include <geogram/basic/quaternion.h>

namespace GEO {

/******************************************************************/

    ArcBall::ArcBall() :
	center_(0.0,0.0),
	radius_(1.0),
	last_point_(0.0,0.0)
    {
        constrain_x_ = false ;
        constrain_y_ = false ;
        grabbed_ = false ;
    }

    vec3 ArcBall::constrain_vector( 
        const vec3& vector, 
        const vec3& axis 
    ) const {
        vec3 result = normalize(vector - dot(vector, axis)*axis) ;
        return result ;
    }

    vec3 ArcBall::mouse_to_sphere( 
        const vec2& p_in 
    ) {

        vec2 p(p_in.x / 1.96, -p_in.y / 1.96) ;

        double mag;
        vec2 v2 = (center_ - p) / radius_ ;
        vec3 v3( v2.x, v2.y, 0.0 );

        mag = dot(v2, v2);
    
        if ( mag > 1.0 ) {
            v3 = normalize(v3) ;
        }
        else {
            v3 = vec3(v3.x, v3.y, -sqrt(1.0 - mag)) ;
        }
    
        /* Now we add constraints - X takes precedence over Y */
        if ( constrain_x_ ) {
            v3 = constrain_vector( v3, vec3( 1.0, 0.0, 0.0 ));
        } else if ( constrain_y_ ) {
            v3 = constrain_vector( v3, vec3( 0.0, 1.0, 0.0 ));
        }
    
        return v3 ;
    }


    void ArcBall::grab(const vec2& value) {
        last_point_ = value ;
        grabbed_ = true ;
    }

    void ArcBall::release(const vec2&) {
        grabbed_ = false ;
    }

    void ArcBall::drag(const vec2& value) {
        if(!grabbed_) {
            return ;
        }

        vec2 new_point = value ;

        vec3 v0 = mouse_to_sphere( last_point_ );
        vec3 v1 = mouse_to_sphere( new_point );
        vec3 cross_ = cross(v0, v1) ;
    
        Quaternion update_quaternion(cross_, -dot(v0, v1) );
        mat4 update_matrix = update_quaternion.to_matrix() ;
    
        matrix_ =  matrix_  * update_matrix ;
        last_point_ = new_point ;
    }

    void ArcBall::set_value(const mat4& m) {
        matrix_ = m ;
    }

    void ArcBall::reset() {
	matrix_.load_identity();
	grabbed_ = false;
	last_point_ = vec2(0.0, 0.0);
    }
    
/*****************************************************/

}

