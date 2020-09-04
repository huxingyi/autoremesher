/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
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
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#ifndef H_HEXDOM_ALGO_SPHERICALHARMONICSL4_H
#define H_HEXDOM_ALGO_SPHERICALHARMONICSL4_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/attributes.h>

namespace GEO {
    
    struct EXPLORAGRAM_API SphericalHarmonicL4 {
        vecng<9, Numeric::float64> coeff;
	
        SphericalHarmonicL4() {
	    FOR(i, 9)  coeff[i] = 0.;
	}
	
        SphericalHarmonicL4(const vecng<9, Numeric::float64>& rhs) : coeff(rhs){
	}

	SphericalHarmonicL4(double *fv) {
	    FOR(i, 9)  coeff[i] = fv[i];
	}
	    
        SphericalHarmonicL4(
	    double x0, double x1, double x2,
	    double x3, double x4, double x5,
	    double x6, double x7, double x8
	) {
	    coeff[0] = x0; coeff[1] = x1; coeff[2] = x2;
	    coeff[3] = x3; coeff[4] = x4; coeff[5] = x5;
	    coeff[6] = x6; coeff[7] = x7; coeff[8] = x8;
	}

        double& operator[](index_t i) {
	    geo_debug_assert(i<9);
	    return coeff[i];	    
	}
	    
	double norm() const {
	    return coeff.length();
	}
	    
	double operator *(const SphericalHarmonicL4 &other) const {
	    return dot(coeff, other.coeff);
	}
	    
	SphericalHarmonicL4 operator -(const SphericalHarmonicL4 &other) const {
	    return SphericalHarmonicL4(coeff - other.coeff);
	}
	    
	SphericalHarmonicL4 operator *(double s) const {
	    return SphericalHarmonicL4(s*coeff);
	}
	    
	SphericalHarmonicL4 operator /(double s) const {
	    return SphericalHarmonicL4(coeff / s);
	}
	    
	SphericalHarmonicL4 operator +(const SphericalHarmonicL4 &v) const {
	    return SphericalHarmonicL4(coeff + v.coeff);
	}

	double value(const vec3& v) const {
	    double res = 0;
	    FOR(i, 9)res += coeff[i]*basis(i,v);
	    return res;
	}
	    
        static double basis(index_t id, const vec3& v);
	    
        void Rz(double alpha);
        void Ry(double alpha);
        void Rx(double alpha);
	    
	void euler_rot(const vec3& rot_vec) {
	    Rx(rot_vec[0]);
	    Ry(rot_vec[1]);
	    Rz(rot_vec[2]);
	}
	    
        SphericalHarmonicL4 Ex() const;
        SphericalHarmonicL4 Ey() const;
        SphericalHarmonicL4 Ez() const;

        static SphericalHarmonicL4 rest_frame() {
            return SphericalHarmonicL4(0, 0, 0, 0, std::sqrt(7. / 12.), 0, 0, 0, std::sqrt(5. / 12.));
        }
	    
        mat3 project_mat3(double grad_threshold = 1e-3, double dot_threshold = 1e-5, vec3* euler_prev = nullptr);

    };

    inline std::istream& operator>> (std::istream& input, SphericalHarmonicL4 &gna) {
	return input >> gna.coeff;
    }

    inline std::ostream& operator<< (std::ostream& output, const SphericalHarmonicL4 &gna) {
	return output << gna.coeff;
    }

    /*
    template <> struct can_be_used_as_attribute<SphericalHarmonicL4> {
	static constexpr auto value = std::integral_constant<bool,true>();
    };
    */
}


#endif //__SPHERICALHARMONICSL4_H__

	
