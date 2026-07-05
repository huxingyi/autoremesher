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

#ifndef GEOGRAM_MESH_TRIANGLE_INTERSECTION
#define GEOGRAM_MESH_TRIANGLE_INTERSECTION

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/memory.h>
#include <utility>

/**
 * \file geogram/mesh/triangle_intersection.h
 * \brief Symbolic computation of triangle-triangle intersection
 */

namespace GEO {

    /**
     * \brief Encodes the location of a point within a triangle.
     * \details A point can be located in 6 different regions, that
     * correspond to the three vertices, three edges and interior
     * of a triangle.
     *   - RGN_P0, RGN_P1, RGN_P2 when point is exactly on a vertex
     *   - RGN_E0, RGN_E1, RGN_E2 when point is on an edge
     *   - RGN_T when point is on the interior of the triangle
     */
    enum TriangleRegion {
        T1_RGN_P0 = 0,  
        T1_RGN_P1 = 1,  
        T1_RGN_P2 = 2,

        T2_RGN_P0 = 3,  
        T2_RGN_P1 = 4,  
        T2_RGN_P2 = 5,  
        
        T1_RGN_E0 = 6,  
        T1_RGN_E1 = 7,  
        T1_RGN_E2 = 8,

        T2_RGN_E0 = 9,  
        T2_RGN_E1 = 10, 
        T2_RGN_E2 = 11, 
        
        T1_RGN_T  = 12, 
        T2_RGN_T  = 13,
        
        T_RGN_NB  = 14
    };


    /**
     * \brief Tests whether a region belongs to triangle T1
     * \retval true if region belongs to T1
     * \retval false if region belongs to T2
     */
    inline bool is_in_T1(TriangleRegion R) {
        return (R == T1_RGN_P0) ||
               (R == T1_RGN_P1) ||
               (R == T1_RGN_P2) ||
               (R == T1_RGN_E0) ||
               (R == T1_RGN_E1) ||
               (R == T1_RGN_E2) ||
               (R == T1_RGN_T ) ;
    }

    /**
     * \brief Replaces T1 with T2 or T2 with T1 in a region code
     * \param[in] R a region
     * \return the region in T2 corresponding to a region in T1,
     *  or the region in T1 corresponding to a region in T2.
     */
    TriangleRegion GEOGRAM_API swap_T1_T2(TriangleRegion R);
    
    /**
     * \brief Encodes the symbolic representation of a triangle intersection,
     *  as a pair of TriangleRegion.
     */
    typedef std::pair<TriangleRegion, TriangleRegion> TriangleIsect;

    /**
     * \brief Triangle-triangle intersection with symbolic information
     * \details The input triangles are supposed to be non-degenerate
     *  (their three vertices are supposed to be distinct and not co-linear).
     *  For now, when intersection is surfacic (overlapping pair
     *  of co-planar triangles), the vertices of the intersection are 
     *  not sorted. One can order them by computing their convex hull.
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
    bool GEOGRAM_API triangles_intersections(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& result
    );


    /**
     * \brief Triangle-triangle intersection predicate
     * \param[in] p0 , p1 , p2 first triangle
     * \param[in] q0 , q1 , q2 second triangle
     * \retval true if there is a non-degenerate intersection
     * \retval false otherwise. Degenerate intersection cases are:
     *  - one vertex in common
     *  - two vertices (an edge) in common
     *  - or duplicated triangles.
     */
    bool triangles_intersections(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2
    ); 
    
    /**
     * \brief Converts a triangle region code to a string.
     * \param[in] rgn the triangle region code.
     * \return the string representation of \p rgn.
     */
    std::string GEOGRAM_API region_to_string(TriangleRegion rgn);

    /**
     * \brief Gets the dimension of a triangle region
     * \param[in] r a triangle region
     * \retval 0 for vertices
     * \retval 1 for edges
     * \retval 2 for the interior
     */
    coord_index_t GEOGRAM_API region_dim(TriangleRegion r);

    /**
     * \brief Gets the vertices of a triangle
     * \param[in] T one of T1_RGN_T, T2_RGN_T
     * \param[out] p0 , p1 , p2 the region codes of the 
     *   three vertices of the triangle
     */
    void GEOGRAM_API get_triangle_vertices(
        TriangleRegion T,
        TriangleRegion& p0, TriangleRegion& p1, TriangleRegion& p2
    );

    /**
     * \brief Gets the edges of a triangle
     * \param[in] T one of T1_RGN_T, T2_RGN_T
     * \param[out] e0 , e1 , e2 the region codes of the
     *  three edges of the triangle
     */
    void GEOGRAM_API get_triangle_edges(
        TriangleRegion T,
        TriangleRegion& e0, TriangleRegion& e1, TriangleRegion& e2
    );

    /**
     * \brief Gets the vertices of an edge
     * \param[in] E the region code of an edge
     * \param[out] q0 , q1 the region codes of the two vertices
     *  of the edge
     */
    void GEOGRAM_API get_edge_vertices(
        TriangleRegion E, TriangleRegion& q0, TriangleRegion& q1
    );


    /**
     * \brief Computes the convex hull of two regions
     * \details The function is purely combinatorial.
     *  - The convex hull of twice the same region is 
     *    that region
     *  - the convex hull of an edge and a vertex this edge
     *    is incident to is that edge
     *  - the convex hull of two vertices is the edge incident
     *    to the vertices
     *  - in all other cases, the convex hull is the triangle
     * \pre \p R1 and \p R2 are in the same triangle (they
     *  both start with T1_ or they both start with T2_).
     * \param[in] R1 , R2 the two regions
     * \return The convex hull of R1 and R2
     */
    TriangleRegion GEOGRAM_API regions_convex_hull(
        TriangleRegion R1, TriangleRegion R2
    );
    
    /**
     * \brief Prints a triangle intersection element to a stream.
     * \details Used for debugging purposes.
     * \param[in] out the stream.
     * \param[in] I the intersection element to be printed.
     */
    inline std::ostream& operator<<(
	std::ostream& out, const TriangleIsect& I
    ) {
	return (
	    out << "("
	    << region_to_string(I.first) << ","
	    << region_to_string(I.second)
	    << ")"
	);
    }
    
    /**
     * \brief Prints the result of a triangle intersection to a stream.
     * \details Used for debugging purposes.
     * \param[in] out the stream.
     * \param[in] II the intersections to be printed.
     */
    inline std::ostream& operator<<(
	std::ostream& out, vector<TriangleIsect>& II
    ) {
	for(const TriangleIsect& I : II) {
	    out << I << " ";
	}
	return out;
    }
}

#endif

