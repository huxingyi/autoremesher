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

#ifndef GEOGRAM_BASIC_QUATERNION
#define GEOGRAM_BASIC_QUATERNION

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <iostream>


/**
 * \file geogram/basic/quaternion.h
 * \brief a class that represents quaternions (eases 
 *   manipulation of 3D rotations)
 */

namespace GEO {

   /**
    * \brief Quaternions are useful for representing rotations. 
    * \details This class is inspired by an implementation written 
    * by Paul Rademacher, in his glui library.
    */
    class GEOGRAM_API Quaternion {
    public:
        
        /**
         * \brief Constructs a new Quaternion.
         */
        Quaternion() : v_(0.0,0.0,0.0), s_(1.0) {
        }

        /**
         * \brief Copy-constructs a new Quaternion.
         * \param[in] rhs the Quaternion to be copied.
         */
        Quaternion(const Quaternion& rhs) : v_(rhs.v_), s_(rhs.s_) {
        }

        /**
         * \brief Constructs a new quaternion from its coefficients
         * \param[in] x a coefficient of the quaternion
         * \param[in] y a coefficient of the quaternion
         * \param[in] z a coefficient of the quaternion
         * \param[in] w a coefficient of the quaternion
         */
        Quaternion(
            double x, double y, double z, double w
        ) : v_(x,y,z), s_(w) {
        }

        /**
         * \brief Constructs a new Quaternion from a vector and
         *  a scalar.
         * \param[in] v a const reference to the vector
         * \param[in] s the scalalr
         */
        Quaternion( const vec3& v, double s ) : v_(v), s_(s) {
        }

        /**
         * \brief Copies a Quaternion
         * \param[in] q the Quaternion to be copied
         * \return a reference to this Quaternion
         */
        Quaternion& operator = ( const Quaternion &q ) {
            v_ = q.v_ ;  
            s_ = q.s_ ; 
            return *this ; 
        }

        /**
         * \brief Sets the coefficients of this quaterion
         * \param[in] v a const reference to the vector components
         * \param[in] s the scalar component
         */
        void set( const vec3& v, double s ) {
            v_ = v;
            s_ = s;
        }

        /**
         * \brief Displays this Quaternion 
         * \param[in] out a reference to the std::ostream
         *  where this Quaternion should be displayed
         */
        void print( std::ostream& out ) const {
            out << v_.x << " " << v_.y << " " << v_.z << " " << s_ ;            
        }


        /**
         * \brief Converts this Quaternion into a matrix
         * \return a matrix (mat4) that represents this Quaternion
         */
        mat4 to_matrix() const;

        /**
         * \brief Sets the rotation angle.
         * \param[in] f the rotation angle
         */
        void  set_angle( double f ) {
            vec3 ax = axis();
            s_ = ::cos(f / 2.0);
            v_ = ax * ::sin(f / 2.0);
        }

        /**
         * \brief Scales the rotation angle.
         */
        void scale_angle( double f ) {
            set_angle( f * angle() );            
        }

        /**
         * \brief Gets the rotation angle
         * \return the angle
         */
        double angle() const {
            return 2.0 * acos( s_ ) ;            
        }

        /**
         * \brief Gets the axis.
         * \return The axis.
         */
        vec3 axis() const;

        /**
         * \brief Computes the interpolation between two quaternions
         * \param[in] from a const reference to the first quaternion
         * \param[in] to a const reference to the second quaternion
         * \param[in] t time, in [0.0,1.0]
         * \return a smooth interpolation between \p from and \p to
         *  parameterized by \p t
         */
        static Quaternion spherical_interpolation( 
            const Quaternion& from, const Quaternion& to, 
            double t 
        );
        
        /**
         * \brief Gets the vector component
         * \return the vector component
         * \note the vector part is not the axis of rotation.
         *  The axis of rotation is obtained by calling axis().
         */
        const vec3& v() const {
            return v_;
        }

        /**
         * \brief Gets the scalar component
         * \return the scalar component
         * \note the scalar component is not the rotation angle.
         *  The rotation angle is obtained by calling angle().
         */
        double s() const {
            return s_;
        }
        
    private:
        vec3 v_ ;
        double s_ ;
    } ;


    /*************************************************************************/

    /**
     * \brief Writes a Quaternion to a stream
     * \param[in,out] out the stream
     * \param[in] q a const reference to the quaternion
     * \return a reference to the stream
     */
    inline std::ostream& operator<<(std::ostream& out, const Quaternion& q) {
        q.print(out) ;
        return out ;
    }


    /**
     * \brief Reads a Quaternion from a stream
     * \param[in,out] in the stream
     * \param[out] q a reference to the quaternion
     * \return a reference to the stream
     */
    inline std::istream& operator>>(std::istream& in, Quaternion& q) {
        double x=0.0,y=0.0,z=0.0,w=0.0 ;
        in >> x >> y >> z >> w ;
        q.set(vec3(x,y,z),w) ;
        return in ;
    }


    /**
     * \brief Computes the sum of two Quaternion
     * \param[in] a a const reference to the first Quaternion
     * \param[in] b a const reference to the second Quaternion     
     * \return the sum of \p a and \p b
     */
    inline Quaternion operator + (const Quaternion& a, const Quaternion& b) {
        return Quaternion(
            a.v() + b.v(), 
            a.s() + b.s() 
        ) ;
    }

    /**
     * \brief Computes the difference between two Quaternion
     * \param[in] a a const reference to the first Quaternion
     * \param[in] b a const reference to the second Quaternion     
     * \return the difference between \p a and \p b
     */
    inline Quaternion operator - (const Quaternion& a, const Quaternion& b) {
        return Quaternion( 
            a.v() - b.v(), 
            a.s() - b.s() 
        ) ;
    }

    /**
     * \brief Computes the opposite of a Quaternion
     * \param[in] a a const reference to the Quaternion
     * \return the opposite of \p a 
     */
    inline Quaternion operator - (const Quaternion& a ) {
        return Quaternion( -1.0 * a.v(), -a.s() );
    }

    /**
     * \brief Computes the product of two Quaternion
     * \param[in] a a const reference to the first Quaternion
     * \param[in] b a const reference to the second Quaternion     
     * \return the product of \p a and \p b
     */
    inline Quaternion operator * ( const Quaternion& a, const Quaternion& b) {
        return Quaternion( 
            a.s() * b.v() + b.s() * a.v() + cross(a.v(),b.v()), 
            a.s() * b.s() - dot(a.v() , b.v()) 
        );
    }

    /**
     * \brief Computes the product of a Quaternion and a scalar
     * \param[in] a a const reference to the Quaternion
     * \param[in] t the scalar
     * \return the product of \p a and \p t
     */
    inline Quaternion operator * ( const Quaternion& a, double t ) {
        return Quaternion( t * a.v(), a.s() * t );
    }


    /**
     * \brief Computes the product of a scalar and a Quaternion
     * \param[in] t the scalar
     * \param[in] a a const reference to the second Quaternion     
     * \return the product of \p t and \p a
     */
    inline Quaternion operator * ( double t, const Quaternion& a ) {
        return Quaternion( t * a.v(), a.s() * t );
    }

}

#endif
