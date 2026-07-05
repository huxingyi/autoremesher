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

#include <exploragram/optimal_transport/linear_least_squares.h>

namespace GEO {
    
    LinearLeastSquares::LinearLeastSquares(
        index_t degree
    ) :
        degree_(degree)
    {
        switch(degree_) {
        case 1:
            dim_ = 4;
            break;
        case 2:
            dim_ = 10;
            break;
        default:
            geo_assert_not_reached;
        }
    }
    
    void LinearLeastSquares::begin() {
        AtA_4_.load_zero();
        AtA_10_.load_zero();
        for(index_t i = 0; i < MAX_DIM; ++i) {
            Atb_[i] = 0.0;
        }
    }


    void LinearLeastSquares::end() {
        switch(degree_) {
        case 1: {
            Matrix<4,double> M = AtA_4_.inverse();
            mult(M, Atb_, eqn_);
        } break;
        case 2: {
            Matrix<10,double> M = AtA_10_.inverse();
            mult(M, Atb_, eqn_);
        } break;
        default:
            geo_assert_not_reached;
        }
    }


    void LinearLeastSquares::add_point(const double* p, double v) {
	switch(degree_) {
	    case 1:
		add_point_degree_1(p,v);
		break;
	    case 2:
		add_point_degree_2(p,v);
		break;
	    default:
		geo_assert_not_reached;
	}
    }

    
    void LinearLeastSquares::add_point_degree_1(const double* p, double v) {
	geo_debug_assert(degree_ == 1);
        double b[MAX_DIM];
        eval_basis(p, b);
        for(index_t i = 0; i < dim(); ++i) {
            for(index_t j = 0; j < dim(); ++j) {
		AtA_4_(i, j) += b[i] * b[j];
	    }
            Atb_[i] += b[i] * v;
        }
    }

    void LinearLeastSquares::add_point_degree_2(const double* p, double v) {
	geo_debug_assert(degree_ == 2);	
        double b[MAX_DIM];
        eval_basis(p, b);
        for(index_t i = 0; i < dim(); ++i) {
            for(index_t j = 0; j < dim(); ++j) {
		AtA_10_(i, j) += b[i] * b[j];
            }
            Atb_[i] += b[i] * v;
        }
    }
    
    double LinearLeastSquares::eval(const double* p) const {
        double b[MAX_DIM];
        for(index_t i = 0; i < MAX_DIM; ++i) {
            b[i] = 0.0;
        }
        eval_basis(p, b);
        double result = 0;
        for(index_t i = 0; i < dim(); ++i) {
            result += eqn_[i] * b[i];
        }
        return result;
    }

    void LinearLeastSquares::eval_basis(const double* p, double* b) const {
        double x = p[0];
        double y = p[1];
        double z = p[2];
        b[0] = 1.0;
        b[1] = x;
        b[2] = y;
        b[3] = z;
        if(degree_ >= 2) {
            b[4] = x * x;
            b[5] = y * y;
            b[6] = z * z;
            b[7] = x * y;
            b[8] = y * z;
            b[9] = z * x;
        }
    }
    
    
}
