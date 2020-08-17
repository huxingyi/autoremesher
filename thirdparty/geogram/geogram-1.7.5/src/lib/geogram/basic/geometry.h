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

#ifndef GEOGRAM_BASIC_GEOMETRY
#define GEOGRAM_BASIC_GEOMETRY

#include <geogram/basic/common.h>
#include <geogram/basic/matrix.h>

/**
 * \file geogram/basic/geometry.h
 * \brief Geometric functions in 2d and 3d
 */

namespace GEO {

    /************************************************************************/

    /**
     * \brief Represents points and vectors in 2d.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<2, Numeric::float64> vec2;

    /**
     * \brief Represents points and vectors in 3d.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<3, Numeric::float64> vec3;

    /**
     * \brief Represents points and vectors in 4d.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<4, Numeric::float64> vec4;

    /**
     * \brief Represents points and vectors in 2d with
     *  single-precision coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<2, Numeric::float32> vec2f;

    /**
     * \brief Represents points and vectors in 3d with
     *  single-precision coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<3, Numeric::float32> vec3f;

    /**
     * \brief Represents points and vectors in 4d with
     *  single-precision coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<4, Numeric::float32> vec4f;

   
    /**
     * \brief Represents points and vectors in 2d with
     *  integer coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<2, Numeric::int32> vec2i;

    /**
     * \brief Represents points and vectors in 3d with
     *  integer coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<3, Numeric::int32> vec3i;

    /**
     * \brief Represents points and vectors in 4d with
     *  integer coordinates.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef vecng<4, Numeric::int32> vec4i;
   
   
    /**
     * \brief Represents a 2x2 matrix.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef Matrix<2, Numeric::float64> mat2;

    /**
     * \brief Represents a 3x3 matrix.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef Matrix<3, Numeric::float64> mat3;

    /**
     * \brief Represents a 4x4 matrix.
     * \details Syntax is (mostly) compatible with GLSL.
     */
    typedef Matrix<4, Numeric::float64> mat4;

    /************************************************************************/

    /**
     * \brief Geometric functions and utilities.
     */
    namespace Geom {

        /**
         * \brief Computes the barycenter of two points in 3d.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \return the barycenter of \p p1 and \p p2
         */
        inline vec3 barycenter(const vec3& p1, const vec3& p2) {
            return vec3(
                0.5 * (p1.x + p2.x),
                0.5 * (p1.y + p2.y),
                0.5 * (p1.z + p2.z)
            );
        }

        /**
         * \brief Computes the barycenter of two points in 2d.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \return the barycenter of \p p1 and \p p2
         */
        inline vec2 barycenter(const vec2& p1, const vec2& p2) {
            return vec2(
                0.5 * (p1.x + p2.x),
                0.5 * (p1.y + p2.y)
            );
        }

        /**
         * \brief Computes the barycenter of three points in 3d.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \param[in] p3 third point
         * \return the barycenter of \p p1, \p p2 and \p p3
         */
        inline vec3 barycenter(
            const vec3& p1, const vec3& p2, const vec3& p3
        ) {
            return vec3(
                (p1.x + p2.x + p3.x) / 3.0,
                (p1.y + p2.y + p3.y) / 3.0,
                (p1.z + p2.z + p3.z) / 3.0
            );
        }

        /**
         * \brief Computes the barycenter of three points in 2d.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \param[in] p3 third point
         * \return the barycenter of \p p1, \p p2 and \p p3
         */
        inline vec2 barycenter(
            const vec2& p1, const vec2& p2, const vec2& p3
        ) {
            return vec2(
                (p1.x + p2.x + p3.x) / 3.0,
                (p1.y + p2.y + p3.y) / 3.0
            );
        }

        /**
         * \brief Computes the cosine of the angle between two 3d vectors.
         * \param[in] a first vector
         * \param[in] b second vector
         * \return the cosine of the angle between \p a and \p b
         */
        inline double cos_angle(const vec3& a, const vec3& b) {
	    double lab = ::sqrt(length2(a)*length2(b));
            double result = (lab > 1e-20) ? (dot(a, b) / lab) : 1.0;
            // Numerical precision problem may occur, and generate
            // normalized dot products that are outside the valid
            // range of acos.
	    geo_clamp(result, -1.0, 1.0);
	    return result;
        }

        /**
         * \brief Computes the angle between two 3d vectors.
         * \param[in] a first vector
         * \param[in] b second vector
         * \return the angle between \p a and \p b in radians, in
         *  the interval \f$ [ 0 \ldots \pi ] \f$.
         */
        inline double angle(const vec3& a, const vec3& b) {
            return ::acos(cos_angle(a, b));
        }

        /**
         * \brief Computes the cosine of the angle between two 2d vectors.
         * \param[in] a first vector
         * \param[in] b second vector
         * \return the cosine of the angle between \p a and \p b
         */
        inline double cos_angle(const vec2& a, const vec2& b) {
	    double lab = ::sqrt(length2(a)*length2(b));
            double result = (lab > 1e-20) ? (dot(a, b) / lab) : 1.0;
            // Numerical precision problem may occur, and generate
            // normalized dot products that are outside the valid
            // range of acos.
	    geo_clamp(result, -1.0, 1.0);
	    return result;
        }

        /**
         * \brief Computes the determinant of two vectors.
         * \param[in] a first vector
         * \param[in] b second vector
         * \return the determinant of \p a and \p b
         */
        inline double det(const vec2& a, const vec2& b) {
            return a.x * b.y - a.y * b.x;
        }

        /**
         * \brief Computes the angle between two 2d vectors.
         * \param[in] a first vector
         * \param[in] b second vector
         * \return the angle between \a and \b in radians,
         *  in the interval \f$ [-\pi \ldots \pi] \f$
         */
        inline double angle(const vec2& a, const vec2& b) {
            return det(a, b) > 0 ?
                   ::acos(cos_angle(a, b)) :
                   -::acos(cos_angle(a, b));
        }

        /**
         * \brief Computes the normal of a 3d triangle
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the normal of the triangle (\p p1, \p p2, \p p3).
         */
        inline vec3 triangle_normal(
            const vec3& p1, const vec3& p2, const vec3& p3
        ) {
            return cross(p2 - p1, p3 - p1);
        }

        /**
         * \brief Computes the area of a 3d triangle
         * \param[in] p1 , p2 , p3 the three vertices of the triangle
         * \return the area of the triangle (\p p1, \p p2, \p p3).
         */
	inline double triangle_area_3d(
	    const double* p1, const double* p2, const double* p3
	) {
	    double Ux = p2[0] - p1[0];
	    double Uy = p2[1] - p1[1];
	    double Uz = p2[2] - p1[2];
	    
	    double Vx = p3[0] - p1[0];
	    double Vy = p3[1] - p1[1];
	    double Vz = p3[2] - p1[2];
	    
	    double Nx = Uy*Vz - Uz*Vy;
	    double Ny = Uz*Vx - Ux*Vz;
	    double Nz = Ux*Vy - Uy*Vx;
	    return 0.5 * ::sqrt(Nx*Nx+Ny*Ny+Nz*Nz);
	}
	
        /**
         * \brief Computes the area of a 3d triangle
         * \param[in] p1 , p2 , p3 the three vertices of the triangle
         * \return the area of the triangle (\p p1, \p p2, \p p3).
         */
        inline double triangle_area(
            const vec3& p1, const vec3& p2, const vec3& p3
        ) {
	    return triangle_area_3d(p1.data(), p2.data(), p3.data());
        }

        /**
         * \brief Computes the area of a 2d triangle
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the signed area of the 2D triangle (\p p1, \p p2, \p p3),
         *  positive if the triangle is oriented clockwise, negative otherwise.
         */
        inline double triangle_signed_area_2d(
            const double* p1, const double* p2, const double* p3
        ) {
	    double a = p2[0]-p1[0];
	    double b = p3[0]-p1[0];
	    double c = p2[1]-p1[1];
	    double d = p3[1]-p1[1];
	    return 0.5*(a*d-b*c);
        }
	
        /**
         * \brief Computes the area of a 2d triangle
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the signed area of the triangle (\p p1, \p p2, \p p3),
         *  positive if the triangle is oriented clockwise, negative otherwise.
         */
        inline double triangle_signed_area(
            const vec2& p1, const vec2& p2, const vec2& p3
        ) {
            return 0.5 * det(p2 - p1, p3 - p1);
        }

        /**
         * \brief Computes the area of a 2d triangle
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the area of the triangle (\p p1, \p p2, \p p3).
         */
        inline double triangle_area(
            const vec2& p1, const vec2& p2, const vec2& p3
        ) {
            return ::fabs(triangle_signed_area(p1, p2, p3));
        }

        /**
         * \brief Computes the area of a 2d triangle
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the area of the triangle (\p p1, \p p2, \p p3).
         */
        inline double triangle_area_2d(
            const double* p1, const double* p2, const double* p3
        ) {
	    return ::fabs(triangle_signed_area_2d(p1,p2,p3));
	}
	
        /**
         * \brief Computes the center of the circumscribed circle of
         *   a 2d triangle.
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return the circumcenter of the triangle (\p p1, \p p2, \p p3).
         */
        vec2 GEOGRAM_API triangle_circumcenter(
            const vec2& p1, const vec2& p2, const vec2& p3
        );

        /**
         * \brief Tests whether a 3d vector has a NaN (not a number) coordinate.
         * \param[in] v a 3d vector
         * \return true if one of the coordinates is a NaN, false otherwise
         */
        inline bool has_nan(const vec3& v) {
            return
                Numeric::is_nan(v.x) ||
                Numeric::is_nan(v.y) ||
                Numeric::is_nan(v.z);
        }

        /**
         * \brief Tests whether a 2d vector has a NaN (not a number) coordinate.
         * \param[in] v a 2d vector
         * \return true if one of the coordinates is a NaN, false otherwise
         */
        inline bool has_nan(const vec2& v) {
            return
                Numeric::is_nan(v.x) ||
                Numeric::is_nan(v.y);
        }

        /**
         * \brief Computes a 3d vector orthogonal to another one.
         * \param[in] V a 3d vector
         * \return a 3d vector orthogonal to \p V
         */
        vec3 GEOGRAM_API perpendicular(const vec3& V);

        /**
         * \brief Computes the signed volume of a 3d tetrahedron
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \return the signed volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        inline double tetra_signed_volume(
            const vec3& p1, const vec3& p2,
            const vec3& p3, const vec3& p4
        ) {
            return dot(p2 - p1, cross(p3 - p1, p4 - p1)) / 6.0;
        }

        /**
         * \brief Computes the signed volume of a 3d tetrahedron
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \return the signed volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        inline double tetra_signed_volume(
            const double* p1, const double* p2,
            const double* p3, const double* p4
        ) {
            return tetra_signed_volume(
                *reinterpret_cast<const vec3*>(p1),
                *reinterpret_cast<const vec3*>(p2),
                *reinterpret_cast<const vec3*>(p3),
                *reinterpret_cast<const vec3*>(p4)
            );
        }

        /**
         * \brief Computes the volume of a 3d tetrahedron
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \return the volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        inline double tetra_volume(
            const vec3& p1, const vec3& p2,
            const vec3& p3, const vec3& p4
        ) {
            return ::fabs(tetra_signed_volume(p1, p2, p3, p4));
        }

        /**
         * \brief Computes the center of the circumscribed sphere
         *  of 3d tetrahedron
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \return the circumcenter of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        vec3 GEOGRAM_API tetra_circum_center(
            const vec3& p1, const vec3& p2,
            const vec3& p3, const vec3& p4
        );

        /**
         * \brief Computes the centroid of a 3d triangle with weighted points.
         * \details The integrated weight varies linearly in the triangle.
         * \param[in] p first vertex of the triangle
         * \param[in] q second vertex of the triangle
         * \param[in] r third vertex of the triangle
         * \param[in] a the weight associated with vertex \p p
         * \param[in] b the weight associated with vertex \p q
         * \param[in] c the weight associated with vertex \p r
         * \param[out] Vg the total weight times the centroid
         * \param[out] V the total weight
         */
        inline void triangle_centroid(
            const vec3& p, const vec3& q, const vec3& r,
            double a, double b, double c,
            vec3& Vg, double& V
        ) {
            double abc = a + b + c;
            double area = Geom::triangle_area(p, q, r);
            V = area / 3.0 * abc;
            double wp = a + abc;
            double wq = b + abc;
            double wr = c + abc;
            double s = area / 12.0;
            Vg.x = s * (wp * p.x + wq * q.x + wr * r.x);
            Vg.y = s * (wp * p.y + wq * q.y + wr * r.y);
            Vg.z = s * (wp * p.z + wq * q.z + wr * r.z);
        }

        /**
         * \brief Computes the mass of a 3d triangle with weighted points.
         * \details The integrated weight varies linearly in the triangle.
         * \param[in] p first vertex of the triangle
         * \param[in] q second vertex of the triangle
         * \param[in] r third vertex of the triangle
         * \param[in] a the weight associated with vertex \p p
         * \param[in] b the weight associated with vertex \p q
         * \param[in] c the weight associated with vertex \p r
         * \return the mass of the weighted triangle ( \p p, \p a),
         *  ( \p q, \p b), ( \p r, \p c)
         */
        inline double triangle_mass(
            const vec3& p, const vec3& q, const vec3& r,
            double a, double b, double c
        ) {
            return Geom::triangle_area(p, q, r) / 3.0 * (
                sqrt(::fabs(a)) + sqrt(::fabs(b)) + sqrt(::fabs(c))
            );
        }

        /**
         * \brief Generates a random point in a 3d triangle.
         * \details Uses Greg Turk's second method.
         *  Reference: Greg Turk, Generating Random Points 
         *  in Triangles, Graphics Gems, p. 24-28, code: p. 649-650.
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return a random point in triangle ( \p p1, \p p2, \p p3 )
         */
        inline vec3 random_point_in_triangle(
            const vec3& p1,
            const vec3& p2,
            const vec3& p3
        ) {
            double s = Numeric::random_float64();
            double t = Numeric::random_float64();
            if(s + t > 1) {
                s = 1.0 - s;
                t = 1.0 - t;
            }
            double u = 1.0 - s - t;
            return vec3(
                u * p1.x + s * p2.x + t * p3.x,
                u * p1.y + s * p2.y + t * p3.y,
                u * p1.z + s * p2.z + t * p3.z
            );
        }
    }

    /**
     * \brief A 3D Plane.
     * \details The plane is represented by the coefficients
     *  a,b,c,d of its equation \f$ ax + by + cz + d = 0 \f$.
     */
    struct Plane {

        /**
         * \brief Constructs the plane passing through three points.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \param[in] p3 third point
         */
        Plane(const vec3& p1, const vec3& p2, const vec3& p3) {
            vec3 n = cross(p2 - p1, p3 - p1);
            a = n.x;
            b = n.y;
            c = n.z;
            d = -(a * p1.x + b * p1.y + c * p1.z);
        }

        /**
         * \brief Constructs a plane passign through a point and orthogonal
         *  to a vector.
         * \param[in] p the point
         * \param[in] n the vector
         */
        Plane(const vec3& p, const vec3& n) {
            a = n.x;
            b = n.y;
            c = n.z;
            d = -(a * p.x + b * p.y + c * p.z);
        }

        /**
         * \brief Constructs a plane from the coefficients of its equation.
         */
        Plane(
            double a_in, double b_in, double c_in, double d_in
        ) :
            a(a_in),
            b(b_in),
            c(c_in),
            d(d_in) {
        }

        /**
         * \brief Constructs an uninitialized plane.
         */
        Plane() {
        }

        /**
         * \brief Gets the normal vector of the plane.
         */
        vec3 normal() const {
            return vec3(a, b, c);
        }

        double a, b, c, d;
    };


    /**
     * \brief Axis-aligned bounding box.
     */
    class Box {
    public:
        double xyz_min[3];
        double xyz_max[3];

        /**
         * \brief Tests whether a box contains a point.
         * \param[in] b the point
         * \return true if this box contains \p b, false otherwise
         */
        bool contains(const vec3& b) const {
            for(coord_index_t c = 0; c < 3; ++c) {
                if(b[c] < xyz_min[c]) {
                    return false;
                }
                if(b[c] > xyz_max[c]) {
                    return false;
                }
            }
            return true;
        }
    };

    /**
     * \brief Tests whether two Boxes have a non-empty intersection.
     * \param[in] B1 first box
     * \param[in] B2 second box
     * \return true if \p B1 and \p B2 have a non-empty intersection,
     *  false otherwise.
     */
    inline bool bboxes_overlap(const Box& B1, const Box& B2) {
        for(coord_index_t c = 0; c < 3; ++c) {
            if(B1.xyz_max[c] < B2.xyz_min[c]) {
                return false;
            }
            if(B1.xyz_min[c] > B2.xyz_max[c]) {
                return false;
            }
        }
        return true;
    }

    /**
     * \brief Computes the smallest Box that encloses two Boxes.
     * \param[out] target the smallest axis-aligned box
     *  that encloses \p B1 and \p B2
     * \param[in] B1 first box
     * \param[in] B2 second box
     */
    inline void bbox_union(Box& target, const Box& B1, const Box& B2) {
        for(coord_index_t c = 0; c < 3; ++c) {
            target.xyz_min[c] = std::min(B1.xyz_min[c], B2.xyz_min[c]);
            target.xyz_max[c] = std::max(B1.xyz_max[c], B2.xyz_max[c]);
        }
    }

   /*******************************************************************/

    /**
     * \brief Applies a 3d transform to a 3d vector.
     * \details Convention is the same as in OpenGL, i.e.
     *  vector is a row vector, multiplied on the left
     *  of the transform.
     *  Internally, the vector is converted into
     *  a 4d vector, with w coordinate set to zero.
     * \param[in] v the input 3d vector to be transformed
     * \param[in] m the transform, as a 4x4 matrix, using
     *  homogeneous coordinates
     * \tparam FT type of the coordinates
     * \return the transformed 3d vector
     */
    template <class FT> vecng<3,FT> transform_vector(
        const vecng<3,FT>& v,
        const Matrix<4,FT>& m
    ){
        index_t i,j ;
        FT result[4] ;

        for(i=0; i<4; i++) {
            result[i] = 0 ;
        }
        for(i=0; i<4; i++) {
            for(j=0; j<3; j++) {
                result[i] += v[j] * m(j,i) ;
            }
        }
        
        return vecng<3,FT>(
            result[0], result[1], result[2]
        ) ; 
    }

    /**
     * \brief Applies a 3d transform to a 3d point.
     * \details Convention is the same as in OpenGL, i.e.
     *  vector is a row vector, multiplied on the left
     *  of the transform.
     *  Internally, the point is converted into
     *  a 4d vector, with w coordinate set to one. Transformed
     *  coordinates are divided by the transformed w to form
     *  a 3d point.
     * \param[in] v the input 3d point to be transformed
     * \param[in] m the transform, as a 4x4 matrix, using
     *  homogeneous coordinates
     * \tparam FT type of the coordinates
     * \return the transformed 3d point
     */
    template <class FT> vecng<3,FT> transform_point(
        const vecng<3,FT>& v,
        const Matrix<4,FT>& m
    ){
        index_t i,j ;
        FT result[4] ;
        
        for(i=0; i<4; i++) {
            result[i] = 0 ;
        }
        for(i=0; i<4; i++) {
            for(j=0; j<3; j++) {
                result[i] += v[j] * m(j,i) ;
            }
            result[i] += m(3,i);
        }
    
        return vecng<3,FT>(
            result[0] / result[3],
            result[1] / result[3],
            result[2] / result[3] 
        ) ; 
    }

    /**
     * \brief Applies a 4d transform to a 4d point.
     * \details Convention is the same as in OpenGL, i.e.
     *  vector is a row vector, multiplied on the left
     *  of the transform.
     * \param[in] v the input 4d point to be transformed
     * \param[in] m the transform, as a 4x4 matrix
     * \tparam FT type of the coordinates
     * \return the transformed 4d vector
     */
    template <class FT> vecng<4,FT> transform_vector(
        const vecng<4,FT>& v,
        const Matrix<4,FT>& m
    ) {
        index_t i,j ;
        FT res[4] = {FT(0), FT(0), FT(0), FT(0)};
        
        for(i=0; i<4; i++) {
            for(j=0; j<4; j++) {
                res[i] += v[j] * m(j,i) ;
            }
        }
    
        return vecng<4,FT>(res[0], res[1], res[2], res[3]) ; 
    }

    /******************************************************************/
    
    /**
     * \brief Creates a translation matrix from a vector.
     * \details The translation matrix is in homogeneous coordinates,
     *  with the same convention as OpenGL (transforms row vectors
     *  mutliplied on the left).
     * \param[in] T a const reference to the translation vector
     * \return the translation matrix
     */
    inline mat4 create_translation_matrix(const vec3& T) {
        mat4 result;
        result.load_identity();
        result(3,0) = T.x;
        result(3,1) = T.y;
        result(3,2) = T.z;
        return result;
    }
    
    /**
     * \brief Creates a scaling matrix.
     * \details The scaling matrix is in homogeneous coordinates,
     *  with the same convention as OpenGL (transforms row vectors
     *  mutliplied on the left).
     * \param[in] s the scaling coefficient
     * \return the scaling matrix
     */
    inline mat4 create_scaling_matrix(double s) {
        mat4 result;
        result.load_identity();
        result(0,0) = s;
        result(1,1) = s;
        result(2,2) = s;        
        return result;
    }
    
    /******************************************************************/

    /**
     * \brief A Ray, in parametric form.
     */
    struct Ray {
	/**
	 * \brief Ray constructor.
	 * \param[in] O the origin of the ray.
	 * \param[in] D the direction of the ray.
	 */
	Ray(vec3 O, vec3 D) : origin(O), direction(D) {
	}
	/**
	 * \brief Ray constructor.
	 */
	Ray() {
	}
	vec3 origin;
	vec3 direction;
    };

    /******************************************************************/        
    
}

#endif

