
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with 
 *     the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_LINEAR_LEAST_SQUARES_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_LINEAR_LEAST_SQUARES_H

#include <exploragram/basic/common.h>
#include <geogram/basic/matrix.h>

/**
 * \file exploragram/optimal_transport/linear_least_squares.h
 * \brief Functions to compute linear regression with low-degree
 *  polynomials, used to upscale functions sampled on pointsets
 *  in Merigot's multilevel algorithm for optimal transport.
 */

namespace GEO {
    
    /**
     * \brief Computes the linear least squares
     *  regression of a function evaluated
     *  in 3d.
     *
     * \TODO: have a linear solve function that does
     * not require a template argument...
     */
    class EXPLORAGRAM_API LinearLeastSquares {
    public:
        /**
         * \brief Constructs a new LinearLeastSquares
         * \param[in] degree one of 1 (linear), 2 (quadratic)
         */
        LinearLeastSquares(index_t degree);

        /**
         * \brief Starts a new computation.
         */
        void begin();

        /**
         * \brief Ends the current computation.
         * \details Computes the current equation
         *  from the set of samples declared with
         *  add_point().
         */
        void end();

        /**
         * \brief Adds a sample to the current computation.
         * \details This function needs to be called between
         *  a begin() / end() pair.
         * \param[in] p 3d coordinates of the point
         * \param[in] v function value associated with \p p_in
         */
        void add_point(const double* p, double v);

        /**
         * \brief Evaluates the least-squares linear estimate
         *  at a given point.
         * \details This function beeds to be called after end().
         * \param[in] p 3d coordinates of the point
         * \return the linear estimate at \p p
         */
        double eval(const double* p) const;

    protected:

        /**
         * \brief Implementation of add_point() for degree 1.
         * \param[in] p 3d coordinates of the point
         * \param[in] v function value associated with \p p_in
         */
	void add_point_degree_1(const double* p, double v);

        /**
         * \brief Implementation of add_point() for degree 2.
         * \param[in] p 3d coordinates of the point
         * \param[in] v function value associated with \p p_in
         */
	void add_point_degree_2(const double* p, double v);	
	
        /**
         * \brief Gets the dimension of the function basis.
         */
        index_t dim() const {
            return dim_;
        }

        /**
         * \brief Evaluates the function basis at a given
         *  point.
         * \param[in] p 3d coordinates of the point
         * \param[out] b array of size dim(), value of the
         *  function basis at \p p
         */
        void eval_basis(const double* p, double* b) const;
        
        /**
         * \brief Maximum dimension of the function basis
         */
        static const int MAX_DIM = 10;

    private:
        index_t degree_;
        index_t dim_;
        Matrix<4,double> AtA_4_;
        Matrix<10,double> AtA_10_;
        double Atb_[MAX_DIM];
        double eqn_[MAX_DIM];
    };
}

#endif
