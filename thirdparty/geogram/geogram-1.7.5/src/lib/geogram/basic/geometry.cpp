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

#include <geogram/basic/geometry.h>

namespace {

    using namespace GEO;
    using namespace Geom;

    /**
     * \brief Computes a vector orthogonal to a segment
     *  and the barycenter of the segment
     * \param[in] p1 first extremity of the segment
     * \param[in] p2 second extremity of the segment
     * \param[out] p barycenter of [ \p p1, \p p2 ].
     * \param[out] v a vector orthogonal to [ \p p1, \p p2 ].
     */
    inline void perp(
        const vec2& p1, const vec2& p2, vec2& p, vec2& v
    ) {
        v = p2 - p1;
        v = vec2(-v.y, v.x);
        p = barycenter(p1, p2);
    }

    /**
     * \brief Computes a parameter that determines the intersection
     *  between two 2d lines.
     * \param[in] p1 origin of the first line
     * \param[in] v1 direction of the first line
     * \param[in] p2 origin of the second line
     * \param[in] v2 direction of the second line
     * \return the parameter t such that p1 + t * v1 is the intersection
     * between the two 2d lines (p1, v1) and (p2, v2).
     */
    inline double segments_intersection_parameter(
        const vec2& p1, const vec2& v1,
        const vec2& p2, const vec2& v2
    ) {
        double delta = v1.x * v2.y - v1.y * v2.x;
        double t1 = (v2.y * (p2.x - p1.x) - v2.x * (p2.y - p1.y)) / delta;
        return t1;
    }

    /**
     * \brief Computes the intersection between two 2d lines
     *  specified by origin and vector.
     * \param[in] p1 origin of the first line
     * \param[in] v1 direction of the first line
     * \param[in] p2 origin of the second line
     * \param[in] v2 direction of the second line
     * \return the intersection between the two 2d lines (p1, v1) and (p2, v2)
     */
    inline vec2 segments_intersection_pv(
        const vec2& p1, const vec2& v1,
        const vec2& p2, const vec2& v2
    ) {
        double t1 = segments_intersection_parameter(p1, v1, p2, v2);
        return p1 + t1 * v1;
    }

#ifdef REMOVE_ME    
    /**
     * \brief Computes the intersection between the supporting lines
     *  of 2d segments specified by their extremities.
     * \param[in] p1 first extremity of the first segment
     * \param[in] p2 second extremity of the first segment
     * \param[in] p3 first extremity of the second segment
     * \param[in] p4 second extremity of the second segment
     * \return the intersection between the two supporting lines of
     *  segments [ \p p1, \p p2] and [ \p p3, \p p4].
     * \note The intersection may be outside of the two segments.
     */
    inline vec2 segments_intersection_pp(
        const vec2& p1, const vec2& p2,
        const vec2& p3, const vec2& p4
    ) {
        return segments_intersection_pv(
            p1, (p2 - p1), p3, (p4 - p3)
        );
    }

    /**
     * \brief Computes the determinant of a 3x3 matrix given
     *  by coefficients.
     */
    inline double det3x3(
        double a00, double a01, double a02,
        double a10, double a11, double a12,
        double a20, double a21, double a22
    ) {
        double m01 = a00 * a11 - a10 * a01;
        double m02 = a00 * a21 - a20 * a01;
        double m12 = a10 * a21 - a20 * a11;
        return m01 * a22 - m02 * a12 + m12 * a02;
    }
#endif
    
}

/****************************************************************************/

namespace GEO {

    namespace Geom {

        vec2 triangle_circumcenter(
            const vec2& q1, const vec2& q2, const vec2& q3
        ) {
            vec2 p1, p2;
            vec2 v1, v2;
            perp(q1, q2, p1, v1);
            perp(q1, q3, p2, v2);
            return segments_intersection_pv(p1, v1, p2, v2);
        }

        vec3 perpendicular(const vec3& V) {
            int min_index = 0;
            double c = ::fabs(V[0]);
            double cur = ::fabs(V[1]);
            if(cur < c) {
                min_index = 1;
                c = cur;
            }
            cur = ::fabs(V[2]);
            if(cur < c) {
                min_index = 2;
            }
            vec3 result;
            switch(min_index) {
                case 0:
                    result = vec3(0, -V.z, V.y);
                    break;
                case 1:
                    result = vec3(V.z, 0, -V.x);
                    break;
                case 2:
                    result = vec3(-V.y, V.x, 0);
                    break;
            }
            return result;
        }

        vec3 tetra_circum_center(
            const vec3& p, const vec3& q,
            const vec3& r, const vec3& s
        ) {
            vec3 qp = q - p;
            double qp2 = length2(qp);
            vec3 rp = r - p;
            double rp2 = length2(rp);
            vec3 sp = s - p;
            double sp2 = length2(sp);

            double num_x = det3x3(
                qp.y, qp.z, qp2,
                rp.y, rp.z, rp2,
                sp.y, sp.z, sp2
            );

            double num_y = det3x3(
                qp.x, qp.z, qp2,
                rp.x, rp.z, rp2,
                sp.x, sp.z, sp2
            );

            double num_z = det3x3(
                qp.x, qp.y, qp2,
                rp.x, rp.y, rp2,
                sp.x, sp.y, sp2
            );

            double den = det3x3(
                qp.x, qp.y, qp.z,
                rp.x, rp.y, rp.z,
                sp.x, sp.y, sp.z
            );

            geo_assert(::fabs(den) > 1e-30);

            den *= 2.0;

            return vec3(
                p.x + num_x / den,
                p.y - num_y / den,
                p.z + num_z / den
            );
        }
    }
}

