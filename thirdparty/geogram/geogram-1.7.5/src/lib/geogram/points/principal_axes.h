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

#ifndef GEOGRAM_POINTS_PRINCIPAL_AXES
#define GEOGRAM_POINTS_PRINCIPAL_AXES

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/logger.h>
#include <geogram/numerics/matrix_util.h>

/**
 * \file geogram/points/principal_axes.h
 * \brief A class to compute the principal axes of a point
 *  cloud.
 */

namespace GEO {

    /**
     * PrincipalAxes3d enables the center and inertia axes of
     * a cloud of 3d points to be computed.
     */
    class GEOGRAM_API PrincipalAxes3d {
    public:
        /**
	 * \brief PrincipalAxes3d constructor.
	 */
        PrincipalAxes3d();

        /**
	 * \brief Begins a principal axes estimation.
	 */
        void begin();

        /**
	 * \brief Ends a principal axes estimation.
	 */
        void end();

        /**
         * \brief Adds a point to the current principal axes
         *  estimation.
         * \param[in] p the current point
	 * \param[in] weight an optional weight
         */
        void add_point(const vec3& p, double weight = 1.0);

        /**
	 * \brief Gets the center.
	 * \details Can be called only after end_points().
	 * \return the center of the point cloud.
	 */
        vec3 center() const {
	    return vec3(center_[0], center_[1], center_[2]);
	}

        /**
	 * \brief Gets one of the axes.
	 * \details Can be called only after end_points().
	 * \param[in] i one of 0,1,2
	 * \return the axis.
	 */
	const vec3& axis(index_t i) const {
	    return axis_[i];
	}

        /**
	 * \brief Gets one of the eigenvalues.
	 * \details Can be called only after end_points().
	 * \param[in] i one of 0,1,2
	 * \return the eigenvalue.
	 */
	double eigen_value(index_t i) const {
	    return eigen_value_[i];
	}

        /**
	 * \brief Gets the estimated normal to the point cloud.
	 * \details Equivalent to axis(2).
	 * \return the estimated normal.
	 */
        vec3 normal() const {
	    return axis(2);
	}
    
    private:
        double center_[3] ;
        vec3 axis_[3] ;
        double eigen_value_[3] ;
        
        double M_[6] ;
        int nb_points_ ;
        double sum_weights_ ;
    };
}

#endif


