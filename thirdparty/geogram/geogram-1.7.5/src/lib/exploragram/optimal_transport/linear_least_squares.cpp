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
