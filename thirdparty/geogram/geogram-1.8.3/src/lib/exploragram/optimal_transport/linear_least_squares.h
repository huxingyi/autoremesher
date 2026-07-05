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
