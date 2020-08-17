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
