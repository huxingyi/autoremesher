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

#ifndef H_HEXDOM_ALGO_GEOMETRY_H
#define H_HEXDOM_ALGO_GEOMETRY_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <geogram/basic/geometry.h>

namespace GEO {

    struct EXPLORAGRAM_API CoTan3D{
	double w[6];
	double tetvol;

	CoTan3D(vec3 P[4], double* anisotropy_as_xx_yy_zz_xy_yz_xz = nullptr);

	index_t org(index_t e);
	index_t dest(index_t e);
	double coeff(index_t e);
	
	void check_for_grad(vec3 P[4], vec3 grad);
    };

    /*******************************************************************************/
    
    class EXPLORAGRAM_API TrglGradient {
    public:
	TrglGradient(const vec3& p0, const vec3& p1, const vec3& p2);
	
	/** Creates an uninitialized TrglGradient */
	TrglGradient();
	
	void initialize(const vec3& p0, const vec3& p1, const vec3& p2);

	/**
	 * Returns the ith vertex of the triangle.
	 * @param i is the index of the vertex,
	 *        which can be one of 0,1,2.
	 */
	const vec3& vertex(index_t i) const;

	/**
	 * Returns the orthonormal basis in which gradient computation are
	 * performed.
	 */
	void basis(vec3& origin, vec3& X, vec3& Y, vec3& Z) const;

	/**
	 * Returns the coefficients determining the gradient in this triangle.
	 *
	 * grad_X = sum_i { TX(i) * vertex(i)-> embedding(prop) }
	 * Note: TX(2) == 0
	 */
	double TX(index_t i) const;

	/**
	 * Returns the coefficients determining the gradient in this triangle.
	 *
	 * grad_Y = sum_i { TY(i) * vertex(i)-> embedding(prop) }
	 */
	double TY(index_t i) const;
	bool is_flat() const { return is_flat_; }

	vec3 gradient_3d(double value0, double value1, double value2) const;
	
    private:
	double TX_[3];
	double TY_[3];
	vec3 vertex_[3];
	bool is_flat_;
    };

    /*******************************************************************************/
    
    struct Basis3d {
	Basis3d(vec3 z) { // TODO DOCUMENT THIS!
	    v[2] = normalize(z);
	    if (std::fabs(v[2].z) < .8)
		v[0] = cross(v[2], vec3(0, 0, 1));
	    else v[0] = cross(v[2], vec3(1, 0, 0));
	    v[0] = normalize(v[0]);
	    v[1] = cross(v[2], v[0]);
	    geo_assert(std::abs(v[1].length2() - 1) < .001);
	}
	vec2 project_xy(vec3 in){
	    return vec2(dot(in, v[0]), dot(in, v[1]));
	}
	vec3 un_project_xy(vec2 in){
	    return in[0] * v[0] + in[1] * v[1];
	}
	vec3 v[3];
    };

    /*******************************************************************************/    

    /*     _____   _____            ____  _____                                _                    _                    _ 
     *    |  __ \ / ____|   /\     |___ \|  __ \                              | |                  | |                  | |
     *    | |__) | |       /  \      __) | |  | |  ______ ______   _ __   ___ | |_    ___ ___ _ __ | |_ ___ _ __ ___  __| |
     *    |  ___/| |      / /\ \    |__ <| |  | | |______|______| | '_ \ / _ \| __|  / __/ _ \ '_ \| __/ _ \ '__/ _ \/ _` |
     *    | |    | |____ / ____ \   ___) | |__| |                 | | | | (_) | |_  | (_|  __/ | | | ||  __/ | |  __/ (_| |
     *    |_|     \_____/_/    \_\ |____/|_____/                  |_| |_|\___/ \__|  \___\___|_| |_|\__\___|_|  \___|\__,_|
     *                                                                                                                     
     */                                                                                                                   
    
    struct EXPLORAGRAM_API UncenteredPCA3D {
    public:
	void begin_points();
	void end_points();
	void point(const vec3& p, double weight = 1.0);
	vec3 axis[3];
	double eigen_value[3];
    private:
	double M_[6];
	int nb_points_;
	double sum_weights_;
    };


    /*******************************************************************************/    

    
    /* Triangle/triangle intersection test routine,
     * by Tomas Moller, 1997.
     * See article "A Fast Triangle-Triangle Intersection Test",
     * Journal of Graphics Tools, 2(2), 1997
     *
     * Updated June 1999: removed the divisions -- a little faster now!
     * Updated October 1999: added {} to CROSS and SUB macros
     *
     * int NoDivTriTriIsect(double V0[3],double V1[3],double V2[3],
     *                      double U0[3],double U1[3],double U2[3])
     *
     * parameters: vertices of triangle 1: V0,V1,V2
     *             vertices of triangle 2: U0,U1,U2
     * result    : returns 1 if the triangles intersect, otherwise 0
     *
     */
    int EXPLORAGRAM_API NoDivTriTriIsect(
        double V0[3], double V1[3], double V2[3],
	double U0[3], double U1[3], double U2[3]
    );

    /********************************************************/
    /* AABB-triangle overlap test code                      */
    /* by Tomas Akenine-Möller                              */
    /* Function: int triBoxOverlap(float boxcenter[3],      */
    /*          float boxhalfsize[3],float triverts[3][3]); */
    /* History:                                             */
    /*   2001-03-05: released the code in its first version */
    /*   2001-06-18: changed the order of the tests, faster */
    /*                                                      */
    /* Acknowledgement: Many thanks to Pierre Terdiman for  */
    /* suggestions and discussions on how to optimize code. */
    /* Thanks to David Hunt for finding a ">="-bug!         */
    /********************************************************/
    int EXPLORAGRAM_API triBoxOverlap(
	float boxcenter[3], float boxhalfsize[3], float triverts[3][3]
    );

    /*******************************************************************************/        
    
}

#endif
