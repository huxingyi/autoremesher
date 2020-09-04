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

#ifndef GEOGRAM_NUMERICS_PREDICATES
#define GEOGRAM_NUMERICS_PREDICATES

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram/numerics/predicates.h
 * \brief Filtered exact predicates for restricted Voronoi diagrams.
 */

namespace GEO {

    /**
     * \brief PCK (Predicate Construction Kit) implements a set of
     *  geometric predicates. PCK uses arithmetic filters (Meyer and Pion),
     *  expansion arithmetics (Shewchuk) and simulation of simplicity 
     *  (Edelsbrunner).
     */
    namespace PCK {

	/**
	 * \brief Mode for symbolic perturbations.
	 */
	enum SOSMode { SOS_ADDRESS, SOS_LEXICO };

	/**
	 * \brief Sets the current mode for handling symbolic perturbations 
	 *  (SOS for Simulation Of Simplicity).
	 * \param[in] m one of SOS_ADDRESS, SOS_LEXICO
	 * \details If SOS_ADDRESS mode is used, then points are supposed
	 *  to be allocated in a fixed array, and the same point always
	 *  designated by the same address. If SOS_LEXICO is used then points
	 *  are sorted in lexicographic order for computing the symbolic 
	 *  perturbation. SOS_LEXICO works for points that are generated
	 *  dynamically (with no fixed address).
	 */
	void GEOGRAM_API set_SOS_mode(SOSMode m);

	/**
	 * \brief Gets the current mode for handling symbolic perturbations.
	 * \return one of SOS_ADDRESS, SOS_LEXICO
	 * \see set_SOS_mode()
	 */
	SOSMode GEOGRAM_API get_SOS_mode();
	
        /**
         * \brief Computes the side of a point (given directly)
         *  relative to a bisector.
         * \details Computes the side of \f$ q0 \f$ relative to
         * \f$ \Pi(p0,p1) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * \param[in] p0 , p1 extremities of the bisector
         * \param[in] q0 point to be tested
         * \param[in] DIM number of coordinates of the point
         * \retval POSITIVE if d(p0,q0) < d(p1,q0)
         * \retval NEGATIVE if d(p0,q0) > d(p1,q1)
         * \retval perturb() if f(p0,q0) = d(p1,q1),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         * \note Only some specific dimensions are implemented (3,4,6 and 7)
         */
        Sign GEOGRAM_API side1_SOS(
            const double* p0, const double* p1,
            const double* q0,
            coord_index_t DIM
        );

        /**
         * \brief Computes the side of a point (given as the intersection
         *  between a segment and a bisector) relative to another bisector.
         * \details Computes the side of \f$ q = \Pi(p0,p1) \cap [q0,q1] \f$
         * relative to \f$ \Pi(p0,p2) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (against which orientation is tested)
         * \param[in] q0 , q1 extremities of the segment
         *  (that defines the intersection q)
         * \retval POSITIVE if d(p0,q) < d(p2,q)
         * \retval NEGATIVE if d(p0,q) > d(p2,q)
         * \retval perturb() if d(p0,q) = d(p2,q),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         * \note Only some specific dimensions are implemented (3,4,6 and 7)
         */
        Sign GEOGRAM_API side2_SOS(
            const double* p0, const double* p1, const double* p2,
            const double* q0, const double* q1,
            coord_index_t DIM
        );

        /**
         * \brief Computes the side of a point (given as the intersection
         *  between a facet and two bisectors) relative to another bisector.
         * \details Computes the side of
         *  \f$ q = \Pi(p0,p1) \cap Pi(p0,p2) \cap \Delta[q0,q1,q2] \f$
         * relative to \f$ \Pi(p0,p3) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (that defines the intersection q)
         * \param[in] p3 second extremity of the third bisector
         *  (against which orientation is tested)
         * \param[in] q0 , q1 , q2 vertices of the triangle
         *  (that defines the intersection q)
         * \retval POSITIVE if d(p0,q) < d(p3,q)
         * \retval NEGATIVE if d(p0,q) > d(p3,q)
         * \retval perturb() if d(p0,q) = d(p3,q),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         * \note Only some specific dimensions are implemented (3,4,6 and 7)
         */
        Sign GEOGRAM_API side3_SOS(
            const double* p0, const double* p1, 
            const double* p2, const double* p3,
            const double* q0, const double* q1, const double* q2,
            coord_index_t DIM
        );

        /**
         * \brief Computes the side of a point (given as the intersection
         *  between a facet and two bisectors) relative to another bisector.
         * \details Computes the side of
         *  \f$ q = \Pi(p0 h0,p1 h1) \cap Pi(p0 h0,p2 h2) \cap \Delta[q0, q1, q2] \f$
         * relative to \f$ \Pi(p0 hp0,p3 hp3) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (that defines the intersection q)
         * \param[in] p3 second extremity of the third bisector
         *  (against which orientation is tested)
         * \param h0 , h1 , h2 , h3 lifted coordinates of \p p0, \p p1, \p p2 
         *  and \p p3
         * \param[in] q0 , q1 , q2 vertices of the triangle
         *  (that defines the intersection q)
	 * \param[in] SOS if true, do the symbolic perturbation in the degenerate
	 *  case
         * \retval POSITIVE if d(p0 hp0,q) < d(p3 hp3, q)
         * \retval NEGATIVE if d(p0 hp0,q) > d(p3 hp3, q)
         * \retval perturb() if d(p0 hp0,q) = d(p3 hp3, q),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         */
        Sign GEOGRAM_API side3_3dlifted_SOS(
            const double* p0, const double* p1, 
            const double* p2, const double* p3,
            double h0, double h1, double h2, double h3,
            const double* q0, const double* q1, const double* q2,
	    bool SOS=true
        );
        
        /**
         * \brief Computes the side of a point (given as the intersection
         *   between a tetrahedron and three bisectors) relative to
         *  another bisector.
         * \details Computes the side of
         *  \f$ q = \Pi(p0,p1) \cap Pi(p0,p2) \cap Pi(p0,p3)
         * \cap \Delta[q0,q1,q2,q3] \f$ relative to \f$ \Pi(p0,p4) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (that defines the intersection q)
         * \param[in] p3 second extremity of the third bisector
         *  (that defines the intersection q)
         * \param[in] p4 second extremity of the fourth bisector
         *  (against which orientation is tested)
         * \param[in] q0 , q1 , q2 , q3 vertices of the tetrahedron
         *  (that defines the intersection q)
         *  (that defines the intersection q)
         * \retval POSITIVE if d(p0,q) < d(p4,q)
         * \retval NEGATIVE if d(p0,q) > d(p4,q)
         * \retval perturb() if d(p0,q) = d(p4,q),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         * \note Only some specific dimensions are implemented (3,4,6 and 7)
         */
        Sign GEOGRAM_API side4_SOS(
            const double* p0,
            const double* p1, const double* p2,
            const double* p3, const double* p4,
            const double* q0, const double* q1,
            const double* q2, const double* q3,
            coord_index_t DIM
        );


        /**
         * \brief Computes the side of a point (given as the intersection
         *   between three bisectors) relative to another bisector.
         * \details Computes the side of
         *  \f$ q = \Pi(p0,p1) \cap \Pi(p0,p2) \cap \Pi(p0,p3) \f$
         * relative to \f$ Pi(p0,p4) \f$.
         * This version does not apply symbolic perturbation when equality
         * holds.
         * side4_3d() is a special case of side4(), where the ambient and
         * intrinsic dimensions coincide (therefore no embedding tetrahedron
         * is needed).
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (that defines the intersection q)
         * \param[in] p3 second extremity of the third bisector
         *  (that defines the intersection q)
         * \param[in] p4 second extremity of the fourth bisector
         *  (against which orientation is tested)
         * \retval POSITIVE if d(p0,q) < d(p4,q)
         * \retval NEGATIVE if d(p0,q) > d(p4,q)
         * \retval ZERO if d(p0,q) = d(p4,q),
         */
        Sign GEOGRAM_API side4_3d(
            const double* p0,
            const double* p1, const double* p2,
            const double* p3, const double* p4
        );

        /**
         * \brief Computes the side of a point (given as the intersection
         *   between three bisectors) relative to another bisector.
         * \details Computes the side of
         *  \f$ q = \Pi(p0,p1) \cap \Pi(p0,p2) \cap \Pi(p0,p3) \f$
         * relative to \f$ Pi(p0,p4) \f$.
         * Symbolic perturbation is applied whenever equality holds.
         * side4_3d() is a special case of side4(), where the ambient and
         * intrinsic dimensions coincide (therefore no embedding tetrahedron
         * is needed).
         * \param[in] p0 first extremity of the bisectors
         * \param[in] p1 second extremity of the first bisector
         *  (that defines the intersection q)
         * \param[in] p2 second extremity of the second bisector
         *  (that defines the intersection q)
         * \param[in] p3 second extremity of the third bisector
         *  (that defines the intersection q)
         * \param[in] p4 second extremity of the fourth bisector
         *  (against which orientation is tested)
         * \retval POSITIVE if d(p0,q) < d(p4,q)
         * \retval NEGATIVE if d(p0,q) > d(p4,q)
         * \retval perturb() if d(p0,q) = d(p4,q),
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         */
        Sign GEOGRAM_API side4_3d_SOS(
            const double* p0, const double* p1, 
            const double* p2, const double* p3, const double* p4
        );
       
        /**
         * \brief Tests whether a 3d point is inside the circumscribed 
         *  sphere of a 3d tetrahedron.
         * \param[in] p0 first vertex of the tetrahedron
         * \param[in] p1 second vertex of the tetrahedron
         * \param[in] p2 third vertex of the tetrahedron
         * \param[in] p3 fourth vertex of the tetrahedron
         * \param[in] p4 the point to be tested
         * \retval POSITIVE whenever \p p4 is inside the circumscribed sphere
         *  of the tetrahedron \p p0, \p p1, \p p2, \p p3
         * \retval NEGATIVE whenever \p p4 is outside the circumscribed sphere
         *  of the tetrahedron \p p0, \p p1, \p p2, \p p3
         * \retval perturb() if \p p4 is exactly on the circumscribed sphere
         *  of the tetrahedron \p p0, \p p1, \p p2, \p p3, where \c perturb()
         *  denotes a globally consistent perturbation, that returns
         *  either POSITIVE or NEGATIVE
         * \pre orient_3d(p0,p1,p2,p3) > 0
         */
         Sign GEOGRAM_API in_sphere_3d_SOS(
            const double* p0, const double* p1, 
            const double* p2, const double* p3,
            const double* p4
         );


        /**
         * \brief Tests whether a 2d point is inside the 
         *  circumscribed circle of a 3d triangle.
         * \param[in] p0 , p1 , p2 vertices of the triangle
         * \param[in] p3 the point to be tested
         * \retval POSITIVE whenever \p p3 is inside the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2
         * \retval NEGATIVE whenever \p p2 is outside the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2
         * \retval perturb() if \p p3 is exactly on the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2, where \c perturb()
         *  denotes a globally consistent perturbation, that returns
         *  either POSITIVE or NEGATIVE
         * \pre \p p3 belongs to the plane yielded by \p p0, \p p1 and \p p2
         */
         Sign GEOGRAM_API in_circle_2d_SOS(
             const double* p0, const double* p1, const double* p2,
             const double* p3
         );

	 
        /**
         * \brief Tests whether a 3d point is inside the 
         *  circumscribed circle of a 3d triangle.
         * \param[in] p0 , p1 , p2 vertices of the triangle
         * \param[in] p3 the point to be tested
         * \retval POSITIVE whenever \p p3 is inside the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2
         * \retval NEGATIVE whenever \p p2 is outside the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2
         * \retval perturb() if \p p3 is exactly on the circumscribed circle
         *  of the triangle \p p0, \p p1, \p p2, where \c perturb()
         *  denotes a globally consistent perturbation, that returns
         *  either POSITIVE or NEGATIVE
         * \pre \p p3 belongs to the plane yielded by \p p0, \p p1 and \p p2
         */
         Sign GEOGRAM_API in_circle_3d_SOS(
             const double* p0, const double* p1, const double* p2,
             const double* p3
         );


        /**
         * \brief Tests whether a lifted 3d point is inside the 
         *  circumscribed circle of a lifted 3d triangle.
         * \param[in] p0 , p1 , p2 vertices of the triangle
         * \param[in] p3 the point to be tested
         * \param[in] h0 , h1 , h2 lifted coordinate of the triangle vertices
         * \param[in] h3 lifted coordinate of the point to be tested
	 * \param[in] SOS if true, do the symbolic perturbation in the degenerate
	 *  cases
         * \retval POSITIVE whenever (\p p3, \p h3) is inside the 
         *  circumscribed circle of the triangle (\p p0,\p h0) (\p p1,\p h1), 
         *  (\p p2, \p h2)
         * \retval NEGATIVE whenever (\p p3, \p h3) is outside the 
         *  circumscribed circle
         *  of the triangle (\p p0,\p h0) (\p p1,\p h1), (\p p2, \p h2)
         * \retval perturb() if (\p p3, \p h3) is exactly 
         *  on the circumscribed circle
         *  of the triangle (\p p0,\p h0) (\p p1,\p h1), (\p p2, \p h2)
         *  where \c perturb() denotes a globally consistent perturbation, 
         *  that returns either POSITIVE or NEGATIVE
         * \pre (\p p3, \p h3) belongs to the hyperplane yielded by 
         *   (\p p0, \p h0), (\p p1, \p h1) and (\p p2, \p h2)
         */
        Sign GEOGRAM_API in_circle_3dlifted_SOS(
            const double* p0, const double* p1, const double* p2,
            const double* p3,
            double h0, double h1, double h2, double h3,
	    bool SOS=true
        );
        
        /**
         * \brief Computes the orientation predicate in 3d.
         * \details Computes the sign of the signed area of
         *  the triangle p0, p1, p2.
         * \param[in] p0 , p1 , p2 vertices of the triangle
         * \retval POSITIVE if the triangle is oriented positively
         * \retval ZERO if the triangle is flat
         * \retval NEGATIVE if the triangle is oriented negatively
         * \todo check whether orientation is inverted as compared to 
         *   Shewchuk's version.
         */
        Sign GEOGRAM_API orient_2d(
            const double* p0, const double* p1, const double* p2
        );


#ifndef GEOGRAM_PSM        
        /**
         * \brief Computes the orientation predicate in 2d.
         * \details Computes the sign of the signed area of
         *  the triangle p0, p1, p2.
         * \param[in] p0 , p1 , p2 vertices of the triangle
         * \retval POSITIVE if the triangle is oriented positively
         * \retval ZERO if the triangle is flat
         * \retval NEGATIVE if the triangle is oriented negatively
         * \todo check whether orientation is inverted as compared to 
         *   Shewchuk's version.
         */
        inline Sign orient_2d(
            const vec2& p0, const vec2& p1, const vec2& p2
        ) {
            return orient_2d(p0.data(),p1.data(),p2.data());
        }
#endif

	
        /**
         * \brief Computes the 3d orientation test with lifted points.
         * \details Given three lifted points p0', p1', p2' in 
         *  R^3, tests if the lifted point p3' in R^3 lies below or above 
         *  the plane passing through the three points 
         *  p0', p1', p2'.
         *  The first two coordinates and the
         *  third one are specified in separate arguments for each vertex.
         * \param[in] p0 , p1 , p2 , p3 first 2 coordinates 
         *   of the vertices of the 3-simplex
         * \param[in] h0 , h1 , h2 , h3 heights of the vertices of 
         *  the 3-simplex
         * \retval POSITIVE if p3' lies below the plane
         * \retval NEGATIVE if p3' lies above the plane
         * \retval perturb() if p3' lies exactly on the hyperplane
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         */
        Sign GEOGRAM_API orient_2dlifted_SOS(
            const double* p0, const double* p1,
            const double* p2, const double* p3, 
            double h0, double h1, double h2, double h3
        );
	
        
        /**
         * \brief Computes the orientation predicate in 3d.
         * \details Computes the sign of the signed volume of
         *  the tetrahedron p0, p1, p2, p3.
         * \param[in] p0 , p1 , p2 , p3 vertices of the tetrahedron
         * \retval POSITIVE if the tetrahedron is oriented positively
         * \retval ZERO if the tetrahedron is flat
         * \retval NEGATIVE if the tetrahedron is oriented negatively
         * \todo check whether orientation is inverted as compared to 
         *   Shewchuk's version.
         */
        Sign GEOGRAM_API orient_3d(
            const double* p0, const double* p1,
            const double* p2, const double* p3
        );


#ifndef GEOGRAM_PSM        
        /**
         * \brief Computes the orientation predicate in 3d.
         * \details Computes the sign of the signed volume of
         *  the tetrahedron p0, p1, p2, p3.
         * \param[in] p0 , p1 , p2 , p3 vertices of the tetrahedron
         * \retval POSITIVE if the tetrahedron is oriented positively
         * \retval ZERO if the tetrahedron is flat
         * \retval NEGATIVE if the tetrahedron is oriented negatively
         * \todo check whether orientation is inverted as compared to 
         *   Shewchuk's version.
         */
        inline Sign orient_3d(
            const vec3& p0, const vec3& p1,
            const vec3& p2, const vec3& p3
        ) {
            return orient_3d(p0.data(),p1.data(),p2.data(),p3.data());
        }
#endif
        
        /**
         * \brief Computes the 4d orientation test.
         * \details Given four lifted points p0', p1', p2', and p3' in 
         *  R^4, tests if the lifted point p4' in R^4 lies below or above 
         *  the hyperplance passing through the four points 
         *  p0', p1', p2', and p3'.
         *  This version does not apply symbolic perturbation.
         *  The first three coordinates and the
         *  fourth one are specified in separate arguments for each vertex.
         * \param[in] p0 , p1 , p2 , p3 , p4 first 3 coordinates 
         *   of the vertices of the 4-simplex
         * \param[in] h0 , h1 , h2 , h3 , h4 heights of the vertices of 
         *  the 4-simplex
         * \retval POSITIVE if p4' lies below the hyperplane
         * \retval NEGATIVE if p4' lies above the hyperplane
         * \retval ZERO if p4' lies exactly on the hyperplane
         */
        Sign GEOGRAM_API orient_3dlifted(
            const double* p0, const double* p1,
            const double* p2, const double* p3, const double* p4,
            double h0, double h1, double h2, double h3, double h4
        );


        /**
         * \brief Computes the 4d orientation test with symbolic perturbation.
         * \details Given four lifted points p0', p1', p2', and p3' in 
         *  R^4, tests if the lifted point p4' in R^4 lies below or above 
         *  the hyperplance passing through the four 
         *  points p0', p1', p2', and p3'.
         *  Symbolic perturbation is applied whenever the 5 vertices are
         *  not linearly independent. The first three coordinates and the
         *  fourth one are specified in separate arguments for each vertex.
         * \param[in] p0 , p1 , p2 , p3 , p4 first 3 coordinates 
         *   of the vertices of the 4-simplex
         * \param[in] h0 , h1 , h2 , h3 , h4 heights of the vertices of 
         *  the 4-simplex
         * \retval POSITIVE if p4' lies below the hyperplane
         * \retval NEGATIVE if p4' lies above the hyperplane
         * \retval perturb() if p4' lies exactly on the hyperplane
         *  where \c perturb() denotes a globally
         *  consistent perturbation, that returns either POSITIVE or NEGATIVE
         */
        Sign GEOGRAM_API orient_3dlifted_SOS(
            const double* p0, const double* p1,
            const double* p2, const double* p3, const double* p4,
            double h0, double h1, double h2, double h3, double h4
        );


	/**
	 * \brief Computes the sign of the determinant of a 3x3 
	 *  matrix formed by three 3d points.
	 * \param[in] p0 , p1 , p2 the three points
	 * \return the sign of the determinant of the matrix.
	 */
	Sign GEOGRAM_API det_3d(
	    const double* p0, const double* p1, const double* p2
	);

	/**
	 * \brief Computes the sign of the determinant of a 4x4 
	 *  matrix formed by four 4d points.
	 * \param[in] p0 , p1 , p2 , p3 the four points
	 * \return the sign of the determinant of the matrix.
	 */
	Sign GEOGRAM_API det_4d(
	    const double* p0, const double* p1,
	    const double* p2, const double* p3
	);

	/**
	 * \brief Computes the sign of the determinant of a 
	 *   4x4 matrix formed by three 4d points and the
	 *   difference of two 4d points.
	 * \param[in] p0 , p1 , p2 , p3 , p4 the four points
	 * \return the sign of the determinant of the matrix
	 *   p0 p1 p2 p4-p3
	 */
	Sign GEOGRAM_API det_compare_4d(
	    const double* p0, const double* p1,
	    const double* p2, const double* p3,
	    const double* p4
	);
	
	/**
	 * \brief Tests whether three points are aligned.
	 * \param[in] p0 , p1 , p2 the three points
	 * \retval true if the three points are aligned.
	 * \retval false otherwise.
	 * \details Function to be tested, use points_are_colinear_3d()
	 *  instead.
	 */
	bool GEOGRAM_API aligned_3d(
	    const double* p0, const double* p1, const double* p2
	);
	
	/**
	 * \brief Computes the sign of the dot product between two
	 *  vectors.
	 * \param[in] p0 , p1 , p2 three 3d points.
	 * \return the sign of the dot product between the vectors
	 *  p0p1 and p0p2.
	 */
	Sign GEOGRAM_API dot_3d(
	    const double* p0, const double* p1, const double* p2
	);

	/**
	 * \brief Compares two dot products.
	 * \param[in] v0 , v1 , v2 three vectors.
	 * \return the sign of v0.v1 - v0.v2
	 */
	Sign GEOGRAM_API dot_compare_3d(
	    const double* v0, const double* v1, const double* v2
	);
	
	/**
	 * \brief Tests whether two 2d points are identical.
	 * \param[in] p1 first point
	 * \param[in] p2 second point
	 * \retval true if \p p1 and \p p2 have exactly the same
	 *  coordinates
	 * \retval false otherwise
	 */
	bool points_are_identical_2d(
	    const double* p1,
	    const double* p2
	);
    
	/**
	 * \brief Tests whether two 3d points are identical.
	 * \param[in] p1 first point
	 * \param[in] p2 second point
	 * \retval true if \p p1 and \p p2 have exactly the same
	 *  coordinates
	 * \retval false otherwise
	 */
	bool GEOGRAM_API points_are_identical_3d(
	    const double* p1,
	    const double* p2
	);

	/**
	 * \brief Tests whether three 3d points are colinear.
	 * \param[in] p1 first point
	 * \param[in] p2 second point
	 * \param[in] p3 third point
	 * \retval true if \p p1, \p p2 and \p p3 are colinear
	 * \retbal false otherwise
	 */
	bool GEOGRAM_API points_are_colinear_3d(
	    const double* p1,
	    const double* p2,
	    const double* p3
        );

	/**
	 * \brief Computes the (approximate) orientation predicate in 3d.
	 * \details Computes the sign of the (approximate) signed volume of
	 *  the tetrahedron p0, p1, p2, p3.
	 * \param[in] p0 first vertex of the tetrahedron
	 * \param[in] p1 second vertex of the tetrahedron
	 * \param[in] p2 third vertex of the tetrahedron
	 * \param[in] p3 fourth vertex of the tetrahedron
	 * \retval POSITIVE if the tetrahedron is oriented positively
	 * \retval ZERO if the tetrahedron is flat
	 * \retval NEGATIVE if the tetrahedron is oriented negatively
	 * \todo check whether orientation is inverted as compared to 
	 *   Shewchuk's version.
	 */
	inline Sign orient_3d_inexact(
	    const double* p0, const double* p1,
	    const double* p2, const double* p3
	) {
	    double a11 = p1[0] - p0[0] ;
	    double a12 = p1[1] - p0[1] ;
	    double a13 = p1[2] - p0[2] ;
	    
	    double a21 = p2[0] - p0[0] ;
	    double a22 = p2[1] - p0[1] ;
	    double a23 = p2[2] - p0[2] ;
	    
	    double a31 = p3[0] - p0[0] ;
	    double a32 = p3[1] - p0[1] ;
	    double a33 = p3[2] - p0[2] ;
	    
	    double Delta = det3x3(
		a11,a12,a13,
		a21,a22,a23,
		a31,a32,a33
	    );

	    return geo_sgn(Delta);
	}
	
        /**
         * \brief Displays some statistics about predicates,
         *  including the number of calls, the number of exact arithmetics 
         *  calls, and the number of Simulation of Simplicity calls.
         */
        void GEOGRAM_API show_stats();

        /**
         * \brief Needs to be called before using any predicate.
         */
        void GEOGRAM_API initialize();

        /**
         * \brief Needs to be called at the end of the program.
         */
        void GEOGRAM_API terminate();
    }
}

#endif

