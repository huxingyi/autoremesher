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

#include <geogram/mesh/triangle_intersection.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/logger.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/algorithm.h>

namespace {

    using namespace GEO;
    
    /**
     * \brief Number of possible values of TriangleRegion
     */
    const index_t trgl_rgn_dim_size = 7;

    /**
     * \brief For each possible value of TriangleRergion
     *  gives the dimension of the corresponding triangle
     *  subset.
     * \details Used to implement region_dim()
     */
    coord_index_t trgl_rgn_dim[trgl_rgn_dim_size] = {
        0, 0, 0, 1, 1, 1, 2
    };

    /**
     * \brief Gets the dimension of a triangle region
     * \param[in] r a triangle region
     * \retval 0 for vertices
     * \retval 1 for edges
     * \retval 2 for the interior
     */
    inline coord_index_t region_dim(TriangleRegion r) {
        geo_debug_assert(index_t(r) < trgl_rgn_dim_size);
        return trgl_rgn_dim[index_t(r)];
    }

    /**
     * \brief Adds a new triangle intersection to a list
     * \details Creates a new triangle intersection and appends it
     *  to the list \p result.
     *  The triangle intersection is a pair made of the triangle
     *  regions \p R1 and \p R2 (or \p R2 and \p R1 if parameter \p swapped is
     *  set to \c true).
     * \param[out] result a list of triangle intersection to extend
     * \param[in] R1 , R2 the two triangle regions
     * \param[in] swapped reverses the order of the triangle regions if set to
     * \c true
     */
    inline void add_intersection(
        vector<TriangleIsect>& result,
        TriangleRegion R1, TriangleRegion R2,
        bool swapped
    ) {
        if(swapped) {
            result.push_back(std::make_pair(R2, R1));
        } else {
            result.push_back(std::make_pair(R1, R2));
        }
    }

    
    /**
     * \brief Encodes a sign as an hexadecimal digit used as a 
     *  mnemonic code.
     * \param[in] s the sign
     * \return a 1-byte mnemonic code that represents the sign
     *  of the argument:
     *  - 0x1 if positive
     *  - 0x0 if zero
     *  - 0xa if negative.
     */
    inline index_t icode(Sign s) {
        return (s == POSITIVE) ? 0x1 : ((s == NEGATIVE) ? 0xa : 0x0);
    }

    /**
     * \brief Encodes the sign of 3 double values
     * \details Encodes the signs \p s1, \p s2 and \p s3 in a 3-bytes
     *  mnemonic using icode:
     *  - the sign \p s1 is encoded in byte xx0000
     *  - the sign \p s2 in encoded in byte 00xx00
     *  - the sign \p s3 in encoded in byte 0000xx
     * \return a 3-byte mnemonic code that represents the
     *  sign of \p s1, \p s2 and \p s3
     * \see icode(double)
     */
    inline index_t icode(Sign s1, Sign s2, Sign s3) {
        return (icode(s1) << 8) | (icode(s2) << 4) | icode(s3);
    }

    /**
     * \brief Edge-triangle intersection in 3D
     * \pre [p0,p1] straddles the supporting plane of
     *  [q1,q2,q3]
     * \param[in] p0 , p1 extremities of the segment
     * \param[in] E the TriangleRegion encoding of the segment p0,p1
     * \param[in] q0 , q1 , q2 vertices of the triangle
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \return true if there was an intersection, false
     *  otherwise.
     */
    bool stradET3(
        const vec3& p0, const vec3& p1, TriangleRegion E,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& out, bool swp
    ) {
        geo_debug_assert(
            PCK::orient_3d(q0, q1, q2, p0) != PCK::orient_3d(q0, q1, q2, p1) 
        );

        geo_debug_assert(region_dim(E) == 1);

        Sign s0 = PCK::orient_3d(p0, p1, q1, q2);
        Sign s1 = PCK::orient_3d(p0, p1, q2, q0);
        if(s0 * s1 < 0) {
            return false;
        }
        Sign s2 = PCK::orient_3d(p0, p1, q0, q1);
        if(s0 * s2 < 0 || s1 * s2 < 0) {
            return false;
        }

        switch(icode(s0, s1, s2)) {

            // Same sign strictly: intersection is inside triangle
            // (two configurations)
            case 0xaaa:
            case 0x111:
                add_intersection(out, E, T_RGN_T, swp);
                return true;

            // Same sign and one zero: intersection is on edge
            // (six configurations)
            case 0x0aa:
            case 0x011:
                add_intersection(out, E, T_RGN_E0, swp);
                return true;
            case 0xa0a:
            case 0x101:
                add_intersection(out, E, T_RGN_E1, swp);
                return true;
            case 0xaa0:
            case 0x110:
                add_intersection(out, E, T_RGN_E2, swp);
                return true;

            // Two zeros: intersection is on vertex
            // (six configurations)
            case 0xa00:
            case 0x100:
                add_intersection(out, E, T_RGN_P0, swp);
                return true;
            case 0x0a0:
            case 0x010:
                add_intersection(out, E, T_RGN_P1, swp);
                return true;
            case 0x00a:
            case 0x001:
                add_intersection(out, E, T_RGN_P2, swp);
                return true;

                // In debug mode, in case of problem, we detect
                // which invalid configuration
                // was triggered (should not occur, but if
                // it occurs, we want to know why !)
#ifdef GEO_DEBUG
            case 0x000:
                geo_assert_not_reached; // zero-area T

            // 6 cases with (negative,zero,positive)
            // (cannot occur)
            case 0xa01:
                geo_assert_not_reached;
            case 0xa10:
                geo_assert_not_reached;
            case 0x0a1:
                geo_assert_not_reached;
            case 0x01a:
                geo_assert_not_reached;
            case 0x1a0:
                geo_assert_not_reached;
            case 0x10a:
                geo_assert_not_reached;

            // 6 cases with mixture of negative and positive
            // (already treated at the beginning of the function)
            case 0xaa1:
                geo_assert_not_reached;
            case 0xa1a:
                geo_assert_not_reached;
            case 0xa11:
                geo_assert_not_reached;
            case 0x1aa:
                geo_assert_not_reached;
            case 0x1a1:
                geo_assert_not_reached;
            case 0x11a:
                geo_assert_not_reached;
#endif
        }
        geo_assert_not_reached;
    }

    /**
     * \brief Computes the 2d orientation of three 3d points
     * \details The points are projected onto a direction
     * \param[in] p1_in first point
     * \param[in] p2_in second point
     * \param[in] p3_in third point
     * \param[in] proj the coordinate along which the points are projected
     *  (0,1 or 2)
     * \return the orientation of the projections of \p p1, \p p2, \p p3
     *  along coordinate \p proj
     */
    Sign orient2d_3d(
        const vec3& p1_in, const vec3& p2_in, const vec3& p3_in,
        coord_index_t proj
    ) {
        double p1[2];
        double p2[2];
        double p3[2];
        for(coord_index_t c = 0; c < 2; c++) {
            p1[c] = p1_in[index_t((proj + 1 + c) % 3)];
            p2[c] = p2_in[index_t((proj + 1 + c) % 3)];
            p3[c] = p3_in[index_t((proj + 1 + c) % 3)];
        }
        return PCK::orient_2d(p1, p2, p3);
    }

    /**
     * \brief Computes the coordinate along which a triangle can be
     *  projected without introducting degeneracies.
     * \param[in] p1 , p2 , p3 the three vertices of the triangle
     * \return the coordinate to be used for 2d computations (0,1 or 2)
     */
    coord_index_t dominant_axis(
        const vec3& p1, const vec3& p2, const vec3& p3
    ) {
        vec3 N = Geom::triangle_normal(p1, p2, p3);
        N.x = ::fabs(N.x);
        N.y = ::fabs(N.y);
        N.z = ::fabs(N.z);

	// Particular case: the triangle is degenerate and has
	// zero area. 
	if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
	    N = (p2 - p1);
	    if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
		N = (p3 - p1);
	    }
	    if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
		N = vec3(0.0, 0.0, 1.0);
	    } else {
		N = Geom::perpendicular(N);
	    }
	}
	
        if((N.x > N.y) && (N.x > N.z)) {
            return 0;
        }
        if(N.y > N.z) {
            return 1;
        }
        return 2;
    }

    /**
     * \brief Computes the coordinate along which four points
     *  projected without introducting degeneracies.
     * \param[in] p1 , p2 , p3 , p4 the four points.
     * \return the coordinate to be used for 2d computations (0,1 or 2).
     */
    coord_index_t dominant_axis(
        const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4
    ) {
	vec3 N = Geom::triangle_normal(p1,p2,p3);
	if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
	    N = Geom::triangle_normal(p1,p3,p4);
	}
	if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
	    N = Geom::triangle_normal(p4,p1,p2);	    
	}
	if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
	    N = Geom::triangle_normal(p4,p2,p3);	    
	}
	if(N.x == 0.0 && N.y == 0.0 && N.z == 0.0) {
	    N.z = 1.0; // TODO something smarter
	}
        N.x = ::fabs(N.x);
        N.y = ::fabs(N.y);
        N.z = ::fabs(N.z);
        if((N.x > N.y) && (N.x > N.z)) {
            return 0;
        }
        if(N.y > N.z) {
            return 1;
        }
        return 2;
    }
    
    /**
     * \brief Tests whether two 3d points are identical.
     * \param[in] p1 first point
     * \param[in] p2 second point
     * \retval true if \p p1 and \p p2 have exactly the same
     *  coordinates
     * \retval false otherwise
     */
    inline bool points_are_identical(
        const double* p1,
        const double* p2
    ) {
        return
            (p1[0] == p2[0]) &&
            (p1[1] == p2[1]) &&
            (p1[2] == p2[2])
        ;
    }

    /**
     * \brief Computes the dimension of a triange.
     * \retval 2 if the triangle is non-degenerate.
     * \retval 1 if the three vertices of the triangle are aligned.
     * \retval 0 if the three vertices of the triangle coincide.
     */
    index_t triangle_dim(
	const vec3& p1, const vec3& p2, const vec3& p3
    ) {
	if(!PCK::aligned_3d(p1.data(), p2.data(), p3.data())) {
	    return 2;
	}
	if(
	    points_are_identical(p1.data(), p2.data()) &&
	    points_are_identical(p2.data(), p3.data())
	) {
	    return 0;
	}
	return 1;
    }
    
    /**
     * \brief Point-triangle intersection in 2d
     * \details Input points are in 3d. The two most varying
     * coordinates are used.
     * \param[in] dom_axis the dominant axis of triangle \p q0, \p q1, \p q2
     * \param[in] p0 the point
     * \param[in] P the TriangleRegion encoding of p0 within the
     *  triangle it comes from, should be one of (T_RGN_P0, T_RGN_P1, T_RGN_P2)
     * \param[in] q0 , q1 , q2 the triangle
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \return true if an intersection point was detected, false otherwise
     * \pre p0 , q0 , q1 , q2 are in the same 3d plane.
     */
    bool PT2(
	coord_index_t dom_axis,
        const vec3& p0, TriangleRegion P,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& out, bool swp
    ) {
        geo_debug_assert(PCK::orient_3d(q0, q1, q2, p0) == 0.0);
        geo_debug_assert(region_dim(P) == 0);
        Sign s0 = orient2d_3d(p0, q1, q2, dom_axis);
        Sign s1 = orient2d_3d(p0, q2, q0, dom_axis);
        Sign s2 = orient2d_3d(p0, q0, q1, dom_axis);

        switch(icode(s0, s1, s2)) {
            case 0xaaa:
            case 0x111:
                add_intersection(out, P, T_RGN_T, swp);
                return true;

            case 0xa00:
            case 0x100:
                add_intersection(out, P, T_RGN_P0, swp);
                return true;
            case 0x0a0:
            case 0x010:
                add_intersection(out, P, T_RGN_P1, swp);
                return true;
            case 0x00a:
            case 0x001:
                add_intersection(out, P, T_RGN_P2, swp);
                return true;

            case 0x0aa:
            case 0x011:
                add_intersection(out, P, T_RGN_E0, swp);
                return true;
            case 0xa0a:
            case 0x101:
                add_intersection(out, P, T_RGN_E1, swp);
                return true;
            case 0xaa0:
            case 0x110:
                add_intersection(out, P, T_RGN_E2, swp);
                return true;

            case 0xaa1:
                return false;
            case 0xa01:
                return false;
            case 0xa1a:
                return false;
            case 0xa10:
                return false;
            case 0xa11:
                return false;
            case 0x0a1:
                return false;
            case 0x01a:
                return false;
            case 0x1aa:
                return false;
            case 0x1a0:
                return false;
            case 0x1a1:
                return false;
            case 0x10a:
                return false;
            case 0x11a:
                return false;

            case 0x000:
                geo_assert_not_reached;
        }

        geo_assert_not_reached;
    }


    /**
     * \brief Point-triangle intersection in 2d
     * \details Input points are in 3d. The two most varying
     *  coordinates are used. This version determines the dominant
     *  axis of q0,q1,q2 and calls the other variant of PT2().
     * \param[in] p0 the point
     * \param[in] P the TriangleRegion encoding of p0 within the
     *  triangle it comes from, should be one of (T_RGN_P0, T_RGN_P1, T_RGN_P2)
     * \param[in] q0 , q1 , q2 the triangle
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \return true if an intersection point was detected, false otherwise
     * \pre p0 , q0 , q1 , q2 are in the same 3d plane.
     */
    bool PT2(
        const vec3& p0, TriangleRegion P,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& out, bool swp
    ) {
	coord_index_t dom_axis = dominant_axis(q0,q1,q2);
	return PT2(dom_axis, p0, P, q0, q1, q2, out, swp);
    }
    
    /**
     * \brief Edge_edge intersection in 2d
     * \details Input points are in 3d. The most varying 
     *   coordinates are used. 
     * \param[in] dom_axis the dominant axis
     * \param[in] p0 , p1 the first edge
     * \param[in] E1 the TriangleRegion encoding of (p0,p1) within the triangle
     *  it comes from, should be one of (T_RGN_E0, T_RGN_E1, T_RGN_E2)
     * \param[in] q0 , q1 the second edge
     * \param[in] E2 the TriangleRegion encoding of (q0,q1) within the triangle
     *  it comes from, should be one of (T_RGN_E0, T_RGN_E1, T_RGN_E2)
     */
    bool EE2_strict(
	coord_index_t dom_axis,
        const vec3& p0, const vec3& p1, TriangleRegion E1,
        const vec3& q0, const vec3& q1, TriangleRegion E2,
        vector<TriangleIsect>& out, bool swp
    ) {
	geo_debug_assert(region_dim(E1) == 1);
	geo_debug_assert(region_dim(E2) == 1);

        Sign s0 = orient2d_3d(p0, q0, q1, dom_axis);
        Sign s1 = orient2d_3d(p1, q0, q1, dom_axis);
        Sign s2 = orient2d_3d(q0, p0, p1, dom_axis);
        Sign s3 = orient2d_3d(q1, p0, p1, dom_axis);	

	bool result = (s0*s1 == -1 && s2*s3 == -1);
	if(result) {
	    add_intersection(out, E1, E2, swp);
	}
	return result;
    }
    
    /**
     * \brief Edge-triangle intersection in 2d
     * \details Input points are in 3d. The two most varying
     *  coordinates are used.
     * \param[in] p0 , p1 the edge
     * \param[in] E the TriangleRegion encoding of (p0,p1) within the triangle
     *  it comes from, should be one of (T_RGN_E0, T_RGN_E1, T_RGN_E2)
     * \param[in] q0 , q1 , q2 the triangle
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \retval true if one or two intersection points were detected
     * \retval false otherwise
     * \pre p0 , p1 , q0 , q1 , q2 are in the same 3d plane.
     */
    bool ET2(
        const vec3& p0, const vec3& p1, TriangleRegion E,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& out, bool swp
    ) {
        geo_debug_assert(PCK::orient_3d(q0, q1, q2, p0) == ZERO);
        geo_debug_assert(PCK::orient_3d(q0, q1, q2, p1) == ZERO);
        geo_debug_assert(region_dim(E) == 1);

        coord_index_t dom_axis = dominant_axis(q0, q1, q2);
	
        TriangleRegion P0_rgn = T_RGN_T;
        TriangleRegion P1_rgn = T_RGN_T;
        switch(E) {
            case T_RGN_E0:
                P0_rgn = T_RGN_P1;
                P1_rgn = T_RGN_P2;
                break;
            case T_RGN_E1:
                P0_rgn = T_RGN_P2;
                P1_rgn = T_RGN_P0;
                break;
            case T_RGN_E2:
                P0_rgn = T_RGN_P0;
                P1_rgn = T_RGN_P1;
                break;
            case T_RGN_P0:
            case T_RGN_P1:
            case T_RGN_P2:
            case T_RGN_T:
                geo_assert_not_reached;
        }

	bool p0t = PT2(dom_axis, p0, P0_rgn, q0, q1, q2, out, swp);
	bool p1t = PT2(dom_axis, p1, P1_rgn, q0, q1, q2, out, swp);
	
        if( p0t && p1t) {
            return true;
        }

	bool ee0 = EE2_strict(dom_axis, p0, p1, E, q1, q2, T_RGN_E0, out, swp);
	bool ee1 = EE2_strict(dom_axis, p0, p1, E, q2, q0, T_RGN_E1, out, swp);
	bool ee2 = EE2_strict(dom_axis, p0, p1, E, q0, q1, T_RGN_E2, out, swp);
	
        return (ee0 || ee1 || ee2) ;
    }

    /**
     * \brief Computes an intersection between a point and a segment.
     * \param[in] p0 the point
     * \param[in] P the triangle region that corresponds to \p p0, 
     *  one of T_RGN_P0, T_RGN_P1, T_RGN_P2
     * \param[in] q0 , q1 the segment
     * \param[in] E the region that corresponds to \p q0 , \p q1, 
     *  one of T_RGN_E0, T_RGN_E1, T_RGN_E2
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \retval true if there was an intersection. Note that if \p p0 coincides
     *  with \p q0 or \p q1, then no intersection is reported.
     * \retval false otherwise
     */
    bool PE3(
	const vec3& p0, TriangleRegion P, const vec3& q0, const vec3& q1, TriangleRegion E,
        vector<TriangleIsect>& out, bool swp
    ) {
	if(
	    PCK::aligned_3d(p0.data(), q0.data(), q1.data()) &&
	    PCK::dot_3d(p0.data(), q0.data(), q1.data()) < 0
	) {
	    add_intersection(out, P, E, swp);
	    return true;
	}
	return false;
    }

    /**
     * \brief Computes an intersection between two segments.
     * \param[in] p0 , p1 the first segment.
     * \param[in] E1 the region that corresponds to \p p0 , \p p1, 
     *  one of T_RGN_E0, T_RGN_E1, T_RGN_E2
     * \param[in] q0 , q1 the segment
     * \param[in] E2 the region that corresponds to \p q0 , \p q1, 
     *  one of T_RGN_E0, T_RGN_E1, T_RGN_E2
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \retval true if there was an intersection inside the two edges
     *  (not at one edge extremity).
     * \retval false otherwise
     */
    bool EE3_strict(
	const vec3& p0, const vec3& p1, TriangleRegion E1,
	const vec3& q0, const vec3& q1, TriangleRegion E2,
        vector<TriangleIsect>& out, bool swp
    ) {
	// If the two segments are not in the same plane, then there is
	// no intersection.
	if(
	    PCK::orient_3d(p0.data(), p1.data(), q0.data(), q1.data()) != 0
	) {
	    return false;
	}

	// If the two segments are aligned, then there is no strict
	// intersection.
	if(
	    (PCK::aligned_3d(p0.data(), q0.data(), q1.data())) &&
	    (PCK::aligned_3d(p1.data(), q0.data(), q1.data()))
        ) {
	    return false;
	}
	
	// Compute dominant axis and test for intersection in 2D.
	coord_index_t dom_axis = dominant_axis(p0, p1, q0, q1);
	return EE2_strict(dom_axis, p0, p1, E1, q0, q1, E2, out, swp);
    }

    
    /**
     * \brief Encodes the type of intersection
     */
    enum IsectResult {
        NO_ISECT,  /**< no intersection */
        TANGENT,   /**< triangles are tangent along edge or vertex */
        STRADDLES, /**< an edge of a triangle traverses the other triangle */
        PLANAR     /**< the two triangles are in the same plane */
    };

    /**
     * \brief Triangle-plane intersection in 3d
     * \param[in] p0 , p1 , p2 the triangle
     * \param[in] q0 , q1 , q2 the plane
     * \param[out] out where to append the result (in symbolic form)
     * \param[in] swp if set, the TriangleRegion codes of the
     *  intersection are swapped in the result.
     * \return an IsectResult that gives more information about
     *  the geometric relation between the triangle and the plane.
     */
    IsectResult triangle_intersect_supporting_plane(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& out, bool swp
    ) {

        Sign s0 = PCK::orient_3d(p0, q0, q1, q2);
        Sign s1 = PCK::orient_3d(p1, q0, q1, q2);
        Sign s2 = PCK::orient_3d(p2, q0, q1, q2);

        switch(icode(s0, s1, s2)) {
            // same sign -> same side (no intersection)
            case 0xaaa:
                return NO_ISECT;
            case 0x111:
                return NO_ISECT;

            // all zero -> switch to 2D code
            case 0x000:
                return PLANAR;

            // Mixture of negative and positive ->
            // potentially two edge-triangle intersections
            case 0xaa1:
            case 0x11a:
                stradET3(p1, p2, T_RGN_E0, q0, q1, q2, out, swp);
                stradET3(p2, p0, T_RGN_E1, q0, q1, q2, out, swp);
                return STRADDLES;
            case 0xa1a:
            case 0x1a1:
                stradET3(p0, p1, T_RGN_E2, q0, q1, q2, out, swp);
                stradET3(p1, p2, T_RGN_E0, q0, q1, q2, out, swp);
                return STRADDLES;
            case 0xa11:
            case 0x1aa:
                stradET3(p0, p1, T_RGN_E2, q0, q1, q2, out, swp);
                stradET3(p2, p0, T_RGN_E1, q0, q1, q2, out, swp);
                return STRADDLES;

            // One zero and same sign twice ->
            // point-triangle intersection in 2d
            case 0xaa0:
            case 0x110:
                PT2(p2, T_RGN_P2, q0, q1, q2, out, swp);
                return TANGENT;
            case 0xa0a:
            case 0x101:
                PT2(p1, T_RGN_P1, q0, q1, q2, out, swp);
                return TANGENT;
            case 0x0aa:
            case 0x011:
                PT2(p0, T_RGN_P0, q0, q1, q2, out, swp);
                return TANGENT;

            // One zero and different signs ->
            //   point-triangle intersection in 2d and
            //   segment-triangle intersection in 3d
            case 0xa01:
            case 0x10a:
                PT2(p1, T_RGN_P1, q0, q1, q2, out, swp);
                stradET3(p0, p2, T_RGN_E1, q0, q1, q2, out, swp);
                return STRADDLES;
            case 0xa10:
            case 0x1a0:
                PT2(p2, T_RGN_P2, q0, q1, q2, out, swp);
                stradET3(p0, p1, T_RGN_E2, q0, q1, q2, out, swp);
                return STRADDLES;
            case 0x0a1:
            case 0x01a:
                PT2(p0, T_RGN_P0, q0, q1, q2, out, swp);
                stradET3(p1, p2, T_RGN_E0, q0, q1, q2, out, swp);
                return STRADDLES;

            // Two zeroes -> edge-triangle intersection in 2d
            case 0xa00:
            case 0x100:
                ET2(p1, p2, T_RGN_E0, q0, q1, q2, out, swp);
                return TANGENT;
            case 0x0a0:
            case 0x010:
                ET2(p2, p0, T_RGN_E1, q0, q1, q2, out, swp);
                return TANGENT;
            case 0x00a:
            case 0x001:
                ET2(p0, p1, T_RGN_E2, q0, q1, q2, out, swp);
                return TANGENT;
        }
        geo_assert_not_reached;
    }

    /**
     * \brief Triangle-triangle intersection in 2d
     * \details Input points are in 3d. The two most varying
     *  coordinates are used.
     * \param[in] p0 , p1 , p2 first triangle
     * \param[in] q0 , q1 , q2 second triangle
     * \param[out] result where to append the result (in symbolic form)
     * \pre p0,p1,p2,q0,q1,q2 are in the same 3d plane.
     */
    void TT2(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& result
    ) {
	// Brute force for now: test all point-triangle and
	// edge-edge intersections.
	
	coord_index_t dom_axis = dominant_axis(q0, q1, q2);

	PT2(dom_axis, p0, T_RGN_P0, q0, q1, q2, result, false);
	PT2(dom_axis, p1, T_RGN_P1, q0, q1, q2, result, false);
	PT2(dom_axis, p2, T_RGN_P2, q0, q1, q2, result, false);

	PT2(dom_axis, q0, T_RGN_P0, p0, p1, p2, result, true);
	PT2(dom_axis, q1, T_RGN_P1, p0, p1, p2, result, true);
	PT2(dom_axis, q2, T_RGN_P2, p0, p1, p2, result, true);	

	EE2_strict(dom_axis, p1, p2, T_RGN_E0, q1, q2, T_RGN_E0, result, false);
	EE2_strict(dom_axis, p2, p0, T_RGN_E1, q1, q2, T_RGN_E0, result, false);
	EE2_strict(dom_axis, p0, p1, T_RGN_E2, q1, q2, T_RGN_E0, result, false);

	EE2_strict(dom_axis, p1, p2, T_RGN_E0, q2, q0, T_RGN_E1, result, false);
	EE2_strict(dom_axis, p2, p0, T_RGN_E1, q2, q0, T_RGN_E1, result, false);
	EE2_strict(dom_axis, p0, p1, T_RGN_E2, q2, q0, T_RGN_E1, result, false);
	
	EE2_strict(dom_axis, p1, p2, T_RGN_E0, q0, q1, T_RGN_E2, result, false);
	EE2_strict(dom_axis, p2, p0, T_RGN_E1, q0, q1, T_RGN_E2, result, false);
	EE2_strict(dom_axis, p0, p1, T_RGN_E2, q0, q1, T_RGN_E2, result, false);
    }


    /**
     * \brief Triangle-triangle intersection routine used when at least one
     *  of the triangles is degenerate (0 area).
     * \pre dim1 = triangle_dim(p0,p1,p2) && dim2 = triangle_dim(q0,q1,q2) &&
     *    ((dim1 != 2) || (dim2 != 2))
     * \param[in] dim1 , dim2 dimensions of the two triangles (0 if all vertices
     *  coindide, 1 if they are colinear, 2 if the triangle is non-degenerate).
     * \param[in] p0 , p1 , p2 first triangle
     * \param[in] q0 , q1 , q2 second triangle
     * \param[out] result the intersection in symbolic
     *  form, as TriangleRegion pairs. There can be
     *  between 0 and 6 intersection pairs in the result.
     * \retval true if there is a non-degenerate intersection
     * \retval false otherwise. Degenerate intersection cases are:
     *  - one vertex in common
     *  - two vertices (an edge) in common
     *  - or duplicated triangles.
     */
    bool triangles_intersections_degenerate(
	index_t dim1, index_t dim2,
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& result
    ) {
	result.resize(0);
	geo_debug_assert(dim1 != 2 || dim2 != 2);
	const vec3 *pp0=nullptr, *pp1=nullptr, *pp2=nullptr;
	const vec3 *qq0=nullptr, *qq1=nullptr, *qq2=nullptr;
	bool swp = false;
	if(dim1 < dim2) {
	    pp0 = &p0; pp1 = &p1; pp2 = &p2;
	    qq0 = &q0; qq1 = &q1; qq2 = &q2;
	} else {
	    std::swap(dim1, dim2);
	    swp = true;
	    pp0 = &q0; pp1 = &q1; pp2 = &q2;
	    qq0 = &p0; qq1 = &p1; qq2 = &p2;
	}
	if(dim1 == 0 && dim2 == 0) {
	    if(points_are_identical(p0.data(), q0.data())) {
		add_intersection(result, T_RGN_P0, T_RGN_P0, false);
	    }
	} else if(dim1 == 0 && dim2 == 1) {

	    if(!points_are_identical(pp0->data(), qq2->data())) {	    
		PE3(*pp0, T_RGN_P0, *qq0, *qq1, T_RGN_E2, result, swp);
	    }
	    if(!points_are_identical(pp0->data(), qq0->data())) {	    	    
		PE3(*pp0, T_RGN_P0, *qq1, *qq2, T_RGN_E0, result, swp);
	    }
	    if(!points_are_identical(pp0->data(), qq1->data())) {	    	    
		PE3(*pp0, T_RGN_P0, *qq2, *qq0, T_RGN_E1, result, swp);
	    }
	} else if(dim1 == 0 && dim2 == 2) {
	    if(PCK::orient_3d(*pp0, *qq0, *qq1, *qq2) == 0) {
		coord_index_t dom_axis = dominant_axis(
		    *qq0, *qq1, *qq2
		);
		PT2(
		    dom_axis, *pp0, T_RGN_P0, *qq0, *qq1, *qq2,
		    result, swp
		    );
	    }
	} else if(dim1 == 1 && dim2 == 1) {


	    if(!points_are_identical(pp0->data(), qq0->data())) {
		PE3(*pp0, T_RGN_P0, *qq1, *qq2, T_RGN_E0, result, swp);
	    }

	    if(!points_are_identical(pp0->data(), qq1->data())) {
		PE3(*pp0, T_RGN_P0, *qq2, *qq0, T_RGN_E1, result, swp);
	    }

	    if(!points_are_identical(pp0->data(), qq2->data())) {
		PE3(*pp0, T_RGN_P0, *qq0, *qq1, T_RGN_E2, result, swp);
	    }

	    if(!points_are_identical(pp1->data(), qq0->data())) {
		PE3(*pp1, T_RGN_P1, *qq1, *qq2, T_RGN_E0, result, swp);
	    }

	    if(!points_are_identical(pp1->data(), qq1->data())) {
		PE3(*pp1, T_RGN_P1, *qq2, *qq0, T_RGN_E1, result, swp);
	    }

	    if(!points_are_identical(pp1->data(), qq2->data())) {
		PE3(*pp1, T_RGN_P1, *qq0, *qq1, T_RGN_E2, result, swp);
	    }
	    
	    if(!points_are_identical(pp2->data(), qq0->data())) {
		PE3(*pp2, T_RGN_P1, *qq1, *qq2, T_RGN_E0, result, swp);
	    }

	    if(!points_are_identical(pp2->data(), qq1->data())) {
		PE3(*pp2, T_RGN_P1, *qq2, *qq0, T_RGN_E1, result, swp);
	    }

	    if(!points_are_identical(pp2->data(), qq2->data())) {
		PE3(*pp2, T_RGN_P1, *qq0, *qq1, T_RGN_E2, result, swp);
	    }
	    
	    EE3_strict(*pp0, *pp1, T_RGN_E2, *qq0, *qq1, T_RGN_E2, result, swp);
	    EE3_strict(*pp0, *pp1, T_RGN_E2, *qq1, *qq2, T_RGN_E0, result, swp);
	    EE3_strict(*pp0, *pp1, T_RGN_E2, *qq2, *qq0, T_RGN_E1, result, swp);

	    EE3_strict(*pp1, *pp2, T_RGN_E0, *qq0, *qq1, T_RGN_E2, result, swp);
	    EE3_strict(*pp1, *pp2, T_RGN_E0, *qq1, *qq2, T_RGN_E0, result, swp);
	    EE3_strict(*pp1, *pp2, T_RGN_E0, *qq2, *qq0, T_RGN_E1, result, swp);	    

	    EE3_strict(*pp2, *pp0, T_RGN_E1, *qq0, *qq1, T_RGN_E2, result, swp);
	    EE3_strict(*pp2, *pp0, T_RGN_E1, *qq1, *qq2, T_RGN_E0, result, swp);
	    EE3_strict(*pp2, *pp0, T_RGN_E1, *qq2, *qq0, T_RGN_E1, result, swp);	    
	} else if(dim1 == 1 && dim2 == 2) {
	    Sign s1 = PCK::orient_3d(*pp0, *qq0, *qq1, *qq2);
	    Sign s2 = PCK::orient_3d(*pp1, *qq0, *qq1, *qq2);
	    Sign s3 = PCK::orient_3d(*pp2, *qq0, *qq1, *qq2);
	    if((s1 == 0) && (s2 == 0) && (s3 == 0)) {
		ET2(
		    *pp0, *pp1, T_RGN_E2, *qq0, *qq1, *qq2,
		    result, swp
		);
		ET2(
		    *pp1, *pp2, T_RGN_E0, *qq0, *qq1, *qq2,
		    result, swp
		);
		ET2(
		    *pp2, *pp0, T_RGN_E1, *qq0, *qq1, *qq2,
		    result, swp
		);
	    } else {
		if(s1 != s2) {
		    stradET3(
			*pp0, *pp1, T_RGN_E2, *qq0, *qq1, *qq2,
			result, swp
		    );
		}
		if(s2 != s3) {
		    stradET3(
			*pp1, *pp2, T_RGN_E0, *qq0, *qq1, *qq2,
			result, swp
		    );
		}
		if(s3 != s1) {
		    stradET3(
			*pp2, *pp0, T_RGN_E1, *qq0, *qq1, *qq2,
			result, swp
		    );
		}
	    }
	}
	
	coord_index_t max_dim = 0;
	
	//   The same intersection can appear several times,
	// remove the duplicates
	sort_unique(result);
	
	//   Determine whether there is a strict intersection, i.e.
	// at least one of the objects that determine intersection
	// vertices is of dimension >= 1.
	for(index_t i = 0; i < result.size(); i++) {
	    max_dim = std::max(max_dim, region_dim(result[i].first));
	    max_dim = std::max(max_dim, region_dim(result[i].second));
	}
	return (max_dim > 0);
    }


}

/****************************************************************************/

namespace GEO {

    std::string region_to_string(TriangleRegion rgn) {
	const char* strs[] = {
	    "P0", "P1", "P2", "E0", "E1", "E2", "T"
	};
	geo_assert(int(rgn) < 7);
	return strs[int(rgn)];
    }
    
    bool triangles_intersections(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& result
    ) {
	result.resize(0);	
        {
	    index_t dim1 = triangle_dim(p0,p1,p2);
	    index_t dim2 = triangle_dim(q0,q1,q2);
	    if(dim1 != 2 || dim2 != 2) {
		return triangles_intersections_degenerate(
		    dim1, dim2,
		    p0, p1, p2,
		    q0, q1, q2,
		    result
		);
	    }
	} 
	
	switch(
	    triangle_intersect_supporting_plane(
		p0, p1, p2, q0, q1, q2, result, false
	    )
        ) {
            case STRADDLES:
                triangle_intersect_supporting_plane(
                    q0, q1, q2, p0, p1, p2, result, true
                );
                break;
            case PLANAR:  
		TT2(
                    p0, p1, p2, q0, q1, q2, result
		); 
		break;
            case NO_ISECT:
            case TANGENT:
                break;
        }
        coord_index_t max_dim = 0;

	//   The same intersection can appear several times,
	// remove the duplicates
	sort_unique(result);

	//   Determine whether there is a strict intersection, i.e.
	// at least one of the objects that determine intersection
	// vertices is of dimension >= 1.
        for(index_t i = 0; i < result.size(); i++) {
            max_dim = std::max(max_dim, region_dim(result[i].first));
            max_dim = std::max(max_dim, region_dim(result[i].second));
        }
        bool has_isect = (max_dim > 0);
	/*
	if(has_isect) {
	    std::cout << result << std::endl;
	}
	*/
	return has_isect;
    }
}

