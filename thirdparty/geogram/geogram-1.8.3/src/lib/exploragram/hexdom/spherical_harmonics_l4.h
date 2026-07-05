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

	
