/*
 *  Copyright (c) 2010-2016, Inria, project ALICE
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

#include <geogram/basic/quaternion.h>

namespace {
    using namespace GEO;

    static const double SMALL = .00001 ;
}

namespace GEO {

    mat4 Quaternion::to_matrix() const {
        double t, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
        t  = 2.0 / (dot(v_, v_) + (s_ * s_));
            
        xs = v_.x * t ; 
        ys = v_.y * t ;
        zs = v_.z * t ;
            
        wx = s_ * xs ;
        wy = s_ * ys ; 
        wz = s_ * zs ;
            
        xx = v_.x * xs ;
        xy = v_.x * ys ;
        xz = v_.x * zs ;
            
        yy = v_.y * ys ;
        yz = v_.y * zs ;
        zz = v_.z * zs ;
            
        mat4 matrix ;
        matrix(0,0) = 1.0 - (yy+zz) ;
        matrix(1,0) = xy + wz ;
        matrix(2,0) = xz - wy ;
        matrix(0,1) = xy - wz ;
        matrix(1,1) = 1.0 - (xx+zz) ;
        matrix(2,1) = yz+wx ;
        matrix(0,2) = xz + wy ;
        matrix(1,2) = yz - wx ;
        matrix(2,2) = 1.0 - (xx+yy) ;
        return matrix;
    }

    vec3 Quaternion::axis() const {
        double scale;
        scale = ::sin( ::acos( s_ ) );
        if ( scale < SMALL && scale > -SMALL ) {
            return vec3( 0.0, 0.0, 0.0 );
        } else {
            return  v_ / scale;
        }
    }
    
    Quaternion Quaternion::spherical_interpolation( 
        const Quaternion& from, const Quaternion& to, 
        double t 
    ) {
        Quaternion to1;

        
        // calculate cosine 
        double cosom = dot(from.v(),to.v()) + from.s() + to.s();
        
        // Adjust signs (if necessary)
        if ( cosom < 0.0 ) {
            cosom = -cosom;
            to1 = -to;
        } else {
            to1 = to;
        }

        double scale0, scale1;
                
        // Calculate coefficients
        if ((1.0 - cosom) > SMALL ) {
            // standard case (slerp) 
            double omega = acos( cosom );
            double sinom = sin( omega );
            scale0 = sin((1.0 - t) * omega) / sinom;
            scale1 = sin(t * omega) / sinom;
        } else {
            // 'from' and 'to' are very close - just do linear interpolation 
            scale0 = 1.0 - t;
            scale1 = t;      
        }
        return scale0 * from + scale1 * to1;
    }
}
