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

#ifndef GEOGRAM_BASIC_GEOMETRY_ND
#define GEOGRAM_BASIC_GEOMETRY_ND

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/memory.h>

/**
 * \file geogram/basic/geometry_nd.h
 * \brief Geometric functions in arbitrary dimension
 */

namespace GEO {

    namespace Geom {

        /**
         * \brief Computes the squared distance between two nd points.
         * \param[in] p1 a pointer to the coordinates of the first point
         * \param[in] p2 a pointer to the coordinates of the second point
         * \param[in] dim dimension (number of coordinates of the points)
         * \return the squared distance between \p p1 and \p p2
         * \tparam COORD_T the numeric type of the point coordinates
         */
        template <class COORD_T>
        inline double distance2(
            const COORD_T* p1, const COORD_T* p2, coord_index_t dim
        ) {
            double result = 0.0;
            for(coord_index_t i = 0; i < dim; i++) {
                result += GEO::geo_sqr(double(p2[i]) - double(p1[i]));
            }
            return result;
        }

        /**
         * \brief Computes the distance between two nd points.
         * \param[in] p1 a pointer to the coordinates of the first point
         * \param[in] p2 a pointer to the coordinates of the second point
         * \param[in] dim dimension (number of coordinates of the points)
         * \return the distance between \p p1 and \p p2
         * \tparam COORD_T the numeric type of the point coordinates
         */
        template <class COORD_T>
        inline double distance(
            const COORD_T* p1, const COORD_T* p2, coord_index_t dim
        ) {
            return ::sqrt(distance2(p1, p2, dim));
        }

        /**
         * \brief Computes the squared distance between two nd points.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \tparam VEC the class that represents the points. VEC needs to
         *   implement data(), that returns a pointer to the coordinates
         *   of the point.
         * \return the squared distance between \p p1 and \p p2
         */
        template <class VEC>
        inline double distance2(
            const VEC& p1, const VEC& p2
        ) {
            geo_debug_assert(p1.dimension() == p2.dimension());
            return distance2(
                p1.data(), p2.data(), coord_index_t(p1.dimension())
            );
        }

        /**

         * \brief Computes the distance between two nd points.
         * \param[in] p1 first point
         * \param[in] p2 second point
         * \tparam VEC the class that represents the points. VEC needs to
         *   implement data(), that returns a pointer to the coordinates
         *   of the point.
         * \return the distance between \p p1 and \p p2
         */
        template <class VEC>
        inline double distance(
            const VEC& p1, const VEC& p2
        ) {
            geo_debug_assert(p1.dimension() == p2.dimension());
            return distance(p1.data(), p2.data(), p1.dimension());
        }

        /**
         * \brief Computes the area of a nd triangle
         * \details Uses Heron formula (that computes the area
         *  from the lengths of the three edges).
         * \param[in] p1 a pointer to the coordinates of the
         *  first vertex of the triangle
         * \param[in] p2 a pointer to the coordinates of the
         *  second vertex of the triangle
         * \param[in] p3 a pointer to the coordinates of the
         *  third vertex of the triangle
         * \param[in] dim dimension of the points
         * \tparam COORD_T the numeric type that represents the coordinates
         *  of the points
         * \return the area of triangle ( \p p1, \p p2, \p p3)
         */
        template <class COORD_T>
        inline double triangle_area(
            const COORD_T* p1,
            const COORD_T* p2,
            const COORD_T* p3,
            coord_index_t dim
        ) {
            double a = distance(p1, p2, dim);
            double b = distance(p2, p3, dim);
            double c = distance(p3, p1, dim);
            double s = double(0.5) * (a + b + c);
            double A2 = s * (s - a) * (s - b) * (s - c);
            // the max is there to avoid some numerical problems.
            return ::sqrt(std::max(A2, 0.0));
        }

        /**
         * \brief Computes the centroid of a 3d triangle with weighted points.
         * \details The integrated weight varies linearly in the triangle.
         * \param[in] p a pointer to the coordinates of the
         *  first vertex of the triangle
         * \param[in] q a pointer to the coordinates of the
         *  second vertex of the triangle
         * \param[in] r a pointer to the coordinates of the
         *  third vertex of the triangle
         * \param[in] a the weight associated with vertex \p p
         * \param[in] b the weight associated with vertex \p q
         * \param[in] c the weight associated with vertex \p r
         * \param[out] Vg the total weight times the centroid (
         *  a pointer to a caller-allocated array of dim COORD_T%s)
         * \param[out] V the total weight
         * \param[in] dim the dimension of the vertices
         * \tparam COORD_T the numeric type that represents the coordinates
         *  of the points
         */
        template <class COORD_T>
        inline void triangle_centroid(
            const COORD_T* p,
            const COORD_T* q,
            const COORD_T* r,
            COORD_T a, COORD_T b, COORD_T c,
            double* Vg,
            double& V,
            coord_index_t dim
        ) {
            double abc = a + b + c;
            double area = Geom::triangle_area(p, q, r, dim);
            V = area / 3.0 * abc;
            double wp = a + abc;
            double wq = b + abc;
            double wr = c + abc;
            double s = area / 12.0;
            for(coord_index_t i = 0; i < dim; i++) {
                Vg[i] = s * (wp * p[i] + wq * q[i] + wr * r[i]);
            }
        }

        /********************************************************************/

        /**
         * \brief Computes the area of a nd triangle.
         * \details Uses Heron formula (that compures the area
         *  from the lengths of the three edges).
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \tparam VEC the class used to represent the vertices of the triangle
         * \return the area of triangle (\p p1, \p p2, \p p3)
         */
        template <class VEC>
        inline double triangle_area(
            const VEC& p1, const VEC& p2, const VEC& p3
        ) {
            // Heron formula
            double a = distance(p1, p2);
            double b = distance(p2, p3);
            double c = distance(p3, p1);
            double s = double(0.5) * (a + b + c);
            return ::sqrt(s * (s - a) * (s - b) * (s - c));
        }

        /**
         * \brief Computes the mass of a nd triangle with weighted points.
         * \details The integrated weight varies linearly in the triangle.
         * \param[in] p first vertex of the triangle
         * \param[in] q second vertex of the triangle
         * \param[in] r third vertex of the triangle
         * \param[in] a the weight associated with vertex \p p
         * \param[in] b the weight associated with vertex \p q
         * \param[in] c the weight associated with vertex \p r
         * \tparam VEC the class used to represent the vertices of the triangle
         * \return the mass of the weighted triangle ( \p p, \p a),
         *  ( \p q, \p b), ( \p r, \p c)
         */
        template <class VEC>
        inline double triangle_mass(
            const VEC& p, const VEC& q, const VEC& r,
            double a, double b, double c
        ) {
            // TODO: try to better understand the formula and
            // determine why there are these sqrt's
            // (probably due to the relation between the
            //  user-provided density and the one achieved
            //  by CVT), but I'm pretty sure that the formula
            //  is correct (at least, dimensions match).
            // Note: the ::fabs() are there to avoid numerical
            // errors.
            return Geom::triangle_area(p, q, r) / 3.0 * (
                ::sqrt(::fabs(a)) + sqrt(::fabs(b)) + sqrt(::fabs(c))
            );
        }

        /**
         * \brief Computes the center of the circumscribed circle of
         *   a nd triangle.
         * \param[in] Q1 first vertex of the triangle
         * \param[in] Q2 second vertex of the triangle
         * \param[in] Q3 third vertex of the triangle
         * \param[out] denom if the parameter is non null, it is set to the
         * denominator of the barycentric coordinates of the circumcenter.
         * \tparam POINT the class used to represent the vertices
         *  of the triangle
         * \return the circumcenter of the triangle (\p p1, \p p2, \p p3).
         */
        template <class POINT>
        POINT triangle_circumcenter(
            const POINT& Q1,
            const POINT& Q2,
            const POINT& Q3,
            double* denom = nullptr
        ) {
            const POINT q2 = Q2 - Q1;
            const POINT q3 = Q3 - Q1;

            double l2 = length2(q2);
            double l3 = length2(q3);

            double a12 = -2.0 * dot(q2, q2);
            double a13 = -2.0 * dot(q3, q2);
            double a22 = -2.0 * dot(q2, q3);
            double a23 = -2.0 * dot(q3, q3);

            double c31 = (a23 * a12 - a22 * a13);
            double d = c31;
            double s = 1.0 / d;
            double lambda1 = s * ((a23 - a22) * l2 + (a12 - a13) * l3 + c31);
            double lambda2 = s * ((-a23) * l2 + (a13) * l3);
            double lambda3 = s * ((a22) * l2 + (-a12) * l3);
            if(denom != nullptr) {
                *denom = d;
            }
            return lambda1 * Q1 + lambda2 * Q2 + lambda3 * Q3;
        }

        /**
         * \brief Computes the centroid of a nd triangle with weighted points.
         * \details The integrated weight varies linearly in the triangle.
         * \param[in] p first vertex of the triangle
         * \param[in] q second vertex of the triangle
         * \param[in] r third vertex of the triangle
         * \param[in] a the weight associated with vertex \p p
         * \param[in] b the weight associated with vertex \p q
         * \param[in] c the weight associated with vertex \p r
         * \param[out] Vg the total weight times the centroid
         * \param[out] V the total weight
         * \tparam VEC the class used to represent the vertices
         *  of the triangle
         */
        template <class VEC>
        inline void triangle_centroid(
            const VEC& p, const VEC& q, const VEC& r,
            double a, double b, double c,
            VEC& Vg, double& V
        ) {
            double abc = a + b + c;
            double area = Geom::triangle_area(p, q, r);
            V = area / 3.0 * abc;
            double wp = a + abc;
            double wq = b + abc;
            double wr = c + abc;
            double s = area / 12.0;
            Vg = s * (wp * p + wq * q + wr * r);
        }

        /**
         * \brief Generates a random point in a nd triangle.
         * \details Uses Greg Turk's second method
         *  (see article in Graphic Gems).
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \return a random point in triangle ( \p p1, \p p2, \p p3 )
         * \tparam VEC the class used to represent the vertices
         *  of the triangle
         */
        template <class VEC>
        inline VEC random_point_in_triangle(
            const VEC& p1, const VEC& p2, const VEC& p3
        ) {
            double l1 = Numeric::random_float64();
            double l2 = Numeric::random_float64();
            if(l1 + l2 > 1.0) {
                l1 = 1.0 - l1;
                l2 = 1.0 - l2;
            }
            double l3 = 1.0 - l1 - l2;
            return l1 * p1 + l2 * p2 + l3 * p3;
        }

        /**
         * \brief Generates a random point in a nd tetrahedron.
         * \details Uses Greg Turk's second method
         *  (see article in Graphic Gems).
         * \param[in] p1 first vertex of the triangle
         * \param[in] p2 second vertex of the triangle
         * \param[in] p3 third vertex of the triangle
         * \param[in] p4 fourth vertex of the triangle
         * \return a random point in tetrahedron ( \p p1, \p p2, \p p3, \p p4)
         * \tparam VEC the class used to represent the vertices
         *  of the triangle
         */
        template <class VEC>
        inline VEC random_point_in_tetra(
            const VEC& p1, const VEC& p2, const VEC& p3, const VEC& p4
        ) {
            double s = Numeric::random_float64();
            double t = Numeric::random_float64();
            double u = Numeric::random_float64();
            if(s + t > 1.0) {
                s = 1.0 - s;
                t = 1.0 - t;
            }
            if(t + u > 1.0) {
                double tmp = u;
                u = 1.0 - s - t;
                t = 1.0 - tmp;
            } else if(s + t + u > 1.0) {
                double tmp = u;
                u = s + t + u - 1.0;
                s = 1.0 - t - tmp;
            }
            double a = 1.0 - s - t - u;
            return a * p1 + s * p2 + t * p3 + u * p4;
        }

        /**
         * \brief Computes the point closest to a given point in a nd segment
         * \param[in] point the query point
         * \param[in] V0 first extremity of the segment
         * \param[in] V1 second extremity of the segment
         * \param[out] closest_point the point closest to \p point in the
         *  segment [\p V0, \p V1]
         * \param[out] lambda0 barycentric coordinate of the closest point
         *  relative to \p V0
         * \param[out] lambda1 barycentric coordinate of the closest point
         *  relative to \p V1
         * \tparam VEC the class that represents the points.
         * \return the squared distance between the point and
         *  the segment [\p V0, \p V1]
         */
        template <class VEC>
        inline double point_segment_squared_distance(
            const VEC& point,
            const VEC& V0,
            const VEC& V1,
            VEC& closest_point,
            double& lambda0,
            double& lambda1
        ) {
            double l2 = distance2(V0,V1);
            double t = dot(point - V0, V1 - V0);
            if(t <= 0.0 || l2 == 0.0) {
                closest_point = V0;
                lambda0 = 1.0;
                lambda1 = 0.0;
                return distance2(point, V0);
            } else if(t > l2) {
                closest_point = V1;
                lambda0 = 0.0;
                lambda1 = 1.0;
                return distance2(point, V1);
            } 
            lambda1 = t / l2;
            lambda0 = 1.0-lambda1;
            closest_point = lambda0 * V0 + lambda1 * V1;
            return distance2(point, closest_point);
        }
        

        /**
         * \brief Computes the point closest to a given point in a nd segment
         * \param[in] point the query point
         * \param[in] V0 first extremity of the segment
         * \param[in] V1 second extremity of the segment
         * \tparam VEC the class that represents the points.
         * \return the squared distance between the point and
         *  the segment [\p V0, \p V1]
         */
        template <class VEC>
        inline double point_segment_squared_distance(
            const VEC& point,
            const VEC& V0,
            const VEC& V1
        ) {
            VEC closest_point;
            double lambda0;
            double lambda1;
            return point_segment_squared_distance(
                point, V0, V1, closest_point, lambda0, lambda1
            );
        }
        
        /**
         * \brief Computes the point closest to a given point in a nd triangle
         * \details See
         *  http://www.geometrictools.com/LibMathematics/Distance/Distance.html
         * \param[in] point the query point
         * \param[in] V0 first vertex of the triangle
         * \param[in] V1 second vertex of the triangle
         * \param[in] V2 third vertex of the triangle
         * \param[out] closest_point the point closest to \p point in the
         *  triangle (\p V0, \p V1, \p V2)
         * \param[out] lambda0 barycentric coordinate of the closest point
         *  relative to \p V0
         * \param[out] lambda1 barycentric coordinate of the closest point
         *  relative to \p V1
         * \param[out] lambda2 barycentric coordinate of the closest point
         *  relative to \p V2
         * \tparam VEC the class that represents the points.
         * \return the squared distance between the point and
         *  the triangle (\p V0, \p V1, \p V2)
         */
        template <class VEC>
        inline double point_triangle_squared_distance(
            const VEC& point,
            const VEC& V0,
            const VEC& V1,
            const VEC& V2,
            VEC& closest_point,
            double& lambda0, double& lambda1, double& lambda2
        ) {
            VEC diff = V0 - point;
            VEC edge0 = V1 - V0;
            VEC edge1 = V2 - V0;
            double a00 = length2(edge0);
            double a01 = dot(edge0, edge1);
            double a11 = length2(edge1);
            double b0 = dot(diff, edge0);
            double b1 = dot(diff, edge1);
            double c = length2(diff);
            double det = ::fabs(a00 * a11 - a01 * a01);
            double s = a01 * b1 - a11 * b0;
            double t = a01 * b0 - a00 * b1;
            double sqrDistance;

	    // If the triangle is degenerate
	    if(det < 1e-30) {
		double cur_l1, cur_l2;
		VEC cur_closest;
		double result;
		double cur_dist = point_segment_squared_distance(point, V0, V1, cur_closest, cur_l1, cur_l2);
		result = cur_dist;
		closest_point = cur_closest;
		lambda0 = cur_l1;
		lambda1 = cur_l2;
		lambda2 = 0.0;
		cur_dist = point_segment_squared_distance(point, V0, V2, cur_closest, cur_l1, cur_l2);
		if(cur_dist < result) {
		    result = cur_dist;
		    closest_point = cur_closest;
		    lambda0 = cur_l1;
		    lambda2 = cur_l2;
		    lambda1 = 0.0;
		}
		cur_dist = point_segment_squared_distance(point, V1, V2, cur_closest, cur_l1, cur_l2);
		if(cur_dist < result) {
		    result = cur_dist;
		    closest_point = cur_closest;
		    lambda1 = cur_l1;
		    lambda2 = cur_l2;
		    lambda0 = 0.0;
		}
		return result;
	    }
	    
            if(s + t <= det) {
                if(s < 0.0) {
                    if(t < 0.0) {   // region 4
                        if(b0 < 0.0) {
                            t = 0.0;
                            if(-b0 >= a00) {
                                s = 1.0;
                                sqrDistance = a00 + 2.0 * b0 + c;
                            } else {
                                s = -b0 / a00;
                                sqrDistance = b0 * s + c;
                            }
                        } else {
                            s = 0.0;
                            if(b1 >= 0.0) {
                                t = 0.0;
                                sqrDistance = c;
                            } else if(-b1 >= a11) {
                                t = 1.0;
                                sqrDistance = a11 + 2.0 * b1 + c;
                            } else {
                                t = -b1 / a11;
                                sqrDistance = b1 * t + c;
                            }
                        }
                    } else {  // region 3
                        s = 0.0;
                        if(b1 >= 0.0) {
                            t = 0.0;
                            sqrDistance = c;
                        } else if(-b1 >= a11) {
                            t = 1.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        } else {
                            t = -b1 / a11;
                            sqrDistance = b1 * t + c;
                        }
                    }
                } else if(t < 0.0) {  // region 5
                    t = 0.0;
                    if(b0 >= 0.0) {
                        s = 0.0;
                        sqrDistance = c;
                    } else if(-b0 >= a00) {
                        s = 1.0;
                        sqrDistance = a00 + 2.0 * b0 + c;
                    } else {
                        s = -b0 / a00;
                        sqrDistance = b0 * s + c;
                    }
                } else {  // region 0
                    // minimum at interior point
                    double invDet = double(1.0) / det;
                    s *= invDet;
                    t *= invDet;
                    sqrDistance = s * (a00 * s + a01 * t + 2.0 * b0) +
                        t * (a01 * s + a11 * t + 2.0 * b1) + c;
                }
            } else {
                double tmp0, tmp1, numer, denom;

                if(s < 0.0) {   // region 2
                    tmp0 = a01 + b0;
                    tmp1 = a11 + b1;
                    if(tmp1 > tmp0) {
                        numer = tmp1 - tmp0;
                        denom = a00 - 2.0 * a01 + a11;
                        if(numer >= denom) {
                            s = 1.0;
                            t = 0.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else {
                            s = numer / denom;
                            t = 1.0 - s;
                            sqrDistance = s * (a00 * s + a01 * t + 2.0 * b0) +
                                t * (a01 * s + a11 * t + 2.0 * b1) + c;
                        }
                    } else {
                        s = 0.0;
                        if(tmp1 <= 0.0) {
                            t = 1.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        }
                        else if(b1 >= 0.0) {
                            t = 0.0;
                            sqrDistance = c;
                        } else {
                            t = -b1 / a11;
                            sqrDistance = b1 * t + c;
                        }
                    }
                } else if(t < 0.0) {  // region 6
                    tmp0 = a01 + b1;
                    tmp1 = a00 + b0;
                    if(tmp1 > tmp0) {
                        numer = tmp1 - tmp0;
                        denom = a00 - 2.0 * a01 + a11;
                        if(numer >= denom) {
                            t = 1.0;
                            s = 0.0;
                            sqrDistance = a11 + 2.0 * b1 + c;
                        } else {
                            t = numer / denom;
                            s = 1.0 - t;
                            sqrDistance = s * (a00 * s + a01 * t + 2.0 * b0) +
                                t * (a01 * s + a11 * t + 2.0 * b1) + c;
                        }
                    } else {
                        t = 0.0;
                        if(tmp1 <= 0.0) {
                            s = 1.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else if(b0 >= 0.0) {
                            s = 0.0;
                            sqrDistance = c;
                        } else {
                            s = -b0 / a00;
                            sqrDistance = b0 * s + c;
                        }
                    }
                } else { // region 1
                    numer = a11 + b1 - a01 - b0;
                    if(numer <= 0.0) {
                        s = 0.0;
                        t = 1.0;
                        sqrDistance = a11 + 2.0 * b1 + c;
                    } else {
                        denom = a00 - 2.0 * a01 + a11;
                        if(numer >= denom) {
                            s = 1.0;
                            t = 0.0;
                            sqrDistance = a00 + 2.0 * b0 + c;
                        } else {
                            s = numer / denom;
                            t = 1.0 - s;
                            sqrDistance = s * (a00 * s + a01 * t + 2.0 * b0) +
                                t * (a01 * s + a11 * t + 2.0 * b1) + c;
                        }
                    }
                }
            }

            // Account for numerical round-off error.
            if(sqrDistance < 0.0) {
                sqrDistance = 0.0;
            }

            closest_point = V0 + s * edge0 + t * edge1;
            lambda0 = 1.0 - s - t;
            lambda1 = s;
            lambda2 = t;
            return sqrDistance;
        }

        /**
         * \brief Computes the squared distance between a point and a nd
         *  triangle.
         * \details See
         *  http://www.geometrictools.com/LibMathematics/Distance/Distance.html
         * \param[in] p the query point
         * \param[in] q1 first vertex of the triangle
         * \param[in] q2 second vertex of the triangle
         * \param[in] q3 third vertex of the triangle
         * \tparam VEC the class that represents the points.
         * \return the squared distance between the point and
         *  the triangle (\p V0, \p V1, \p V2)
         */

        template <class VEC>
        inline double point_triangle_squared_distance(
            const VEC& p, const VEC& q1, const VEC& q2, const VEC& q3
        ) {
            VEC closest_point;
            double lambda1, lambda2, lambda3;
            return point_triangle_squared_distance(
                p, q1, q2, q3, closest_point, lambda1, lambda2, lambda3
            );
        }

        /**
         * \brief Computes the volume of a tetrahedron from
         *  edge lengths.
         * \details Uses a form of generalized Heron formula:
         *  W. Kahan, "What has the Volume of a Tetrahedron
         *  to do with Computer Programming Languages?"
         * \param[in] u distance between p1 and p4
         * \param[in] U distance between p2 and p3
         * \param[in] v distance between p2 and p4
         * \param[in] V distance between p3 and p1
         * \param[in] w distance between p3 and p4
         * \param[in] W distance between p1 and p2
         * \return the volume of the tetrahedron
         */
        inline double tetra_volume_from_edge_lengths(
            double u, double U,
            double v, double V,
            double w, double W
        ) {
            double X = (w - U + v) * (U + v + w);
            double x = (U - v + w) * (v - w + U);
            double Y = (u - V + w) * (V + w + u);
            double y = (V - w + u) * (w - u + V);
            double Z = (v - W + u) * (W + u + v);
            double z = (W - u + v) * (u - v + W);
            double a = ::sqrt(::fabs(x * Y * Z));
            double b = ::sqrt(::fabs(y * Z * X));
            double c = ::sqrt(::fabs(z * X * Y));
            double d = ::sqrt(::fabs(x * y * z));
            return ::sqrt(::fabs(
                    (-a + b + c + d) *
                    (a - b + c + d) *
                    (a + b - c + d) *
                    (a + b + c - d)
                )) / (192.0 * u * v * w);
        }

        /**
         * \brief Computes the volume of a nd tetrahedron
         * \details Uses a form of generalized Heron formula:
         *  W. Kahan, "What has the Volume of a Tetrahedron
         *  to do with Computer Programming Languages?"
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \tparam VEC the class that represents the points
         * \return the volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        template <class VEC>
        inline double tetra_volume(
            const VEC& p1, const VEC& p2, const VEC& p3, const VEC& p4
        ) {
            double U = distance(p1, p2);
            double u = distance(p3, p4);
            double V = distance(p2, p3);
            double v = distance(p1, p4);
            double W = distance(p3, p1);
            double w = distance(p2, p4);
            return tetra_volume_from_edge_lengths(u, U, v, V, w, W);
        }

        /**
         * \brief Computes the volume of a nd tetrahedron
         * \details Uses a form of generalized Heron formula:
         *  W. Kahan, "What has the Volume of a Tetrahedron
         *  to do with Computer Programming Languages?"
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \tparam DIM dimension of the points
         * \return the volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        template <int DIM>
        inline double tetra_volume(
            const double* p1, const double* p2,
            const double* p3, const double* p4
        ) {
            double U = distance(p1, p2, DIM);
            double u = distance(p3, p4, DIM);
            double V = distance(p2, p3, DIM);
            double v = distance(p1, p4, DIM);
            double W = distance(p3, p1, DIM);
            double w = distance(p2, p4, DIM);
            return tetra_volume_from_edge_lengths(u, U, v, V, w, W);
        }

        /**
         * \brief Computes the volume of a 3d tetrahedron
         * \details Partial specialization of tetra_volume() for DIM == 3.
         *  It uses the standard formula, much simpler than the
         *  N dimension version.
         * \param[in] p1 first vertex of the tetrahedron
         * \param[in] p2 second vertex of the tetrahedron
         * \param[in] p3 third vertex of the tetrahedron
         * \param[in] p4 fourth vertex of the tetrahedron
         * \return the volume of the tetrahedron
         *  (\p p1, \p p2, \p p3, \p p4)
         */
        template <>
        inline double tetra_volume<3>(
            const double* p1, const double* p2,
            const double* p3, const double* p4
        ) {
            return tetra_volume(
                *reinterpret_cast<const vec3*>(p1),
                *reinterpret_cast<const vec3*>(p2),
                *reinterpret_cast<const vec3*>(p3),
                *reinterpret_cast<const vec3*>(p4)
            );
        }
    }
}

#endif

