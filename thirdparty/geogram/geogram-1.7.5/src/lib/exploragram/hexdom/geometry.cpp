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

#include <exploragram/hexdom/geometry.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/logger.h>

namespace {
    using namespace GEO;

    const index_t cot_edges[6][2] = {
	{ 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 2, 3 }, { 3, 1 }
    } ;
    
}

namespace GEO {

    CoTan3D::CoTan3D(vec3 P[4], double* anisotropy_as_xx_yy_zz_xy_yz_xz ) {
	tetvol = (1. / 6.) * dot(P[3] - P[0], cross(P[1] - P[0], P[2] - P[0]));
	Matrix<6, double> M;
	M.load_zero();
	double isotrope_objective[6] = { 1, 1, 1, 0, 0, 0 };
	double *RHS = anisotropy_as_xx_yy_zz_xy_yz_xz;
	if (RHS == nullptr)
	    RHS = isotrope_objective;
	FOR(e, 6){
	    vec3 geom = P[cot_edges[e][1]] - P[cot_edges[e][0]];
	    M(0, e) = geom.x*geom.x;
	    M(1, e) = geom.y*geom.y;
	    M(2, e) = geom.z*geom.z;
	    M(3, e) = 2. * geom.x*geom.y;
	    M(4, e) = 2. * geom.y*geom.z;
	    M(5, e) = 2. * geom.z*geom.x;
	}
	Matrix<6, double> inv;
	bool invertible = M.compute_inverse(inv);
	if (!invertible) GEO::Logger::out("HexDom")  << "Solve did not work" <<  std::endl;
	mult(inv, RHS, w);
    }
    

    index_t CoTan3D::org(index_t e) { return cot_edges[e][0]; }
    index_t CoTan3D::dest(index_t e) { return cot_edges[e][1]; }
    double CoTan3D::coeff(index_t e) { return w[e]; }

    void CoTan3D::check_for_grad(vec3 P[4], vec3 grad){
	double v[4];
	FOR(i, 4) v[i] = dot(grad, P[i]);
	double sum = 0;
	FOR(e, 6) sum += (v[dest(e)] - v[org(e)])*(v[dest(e)] - v[org(e)])* coeff(e);
	std::cerr << "NRJ = " << grad.length2()
		  << "\t\twith cot = " << sum
		  << "\t\tratio = " << grad.length2() / sum
		  << std::endl;
    }

    /*******************************************************************************/

    TrglGradient::TrglGradient(const vec3& p0, const vec3& p1, const vec3& p2) {
	initialize(p0, p1, p2);
    }
    
    TrglGradient::TrglGradient() {
    }

    void TrglGradient::initialize(const vec3& p0, const vec3& p1, const vec3& p2) {
	
	// Computing TX[] and TY[],
	// i.e. the coefficients such that:
	//    | df/dX = TX[0].f(v0) + TX[1].f(v1) + TX[2].f(v2)
	//    | df/dY = TY[0].f(v0) + TY[1].f(v1) + TY[2].f(v2)
	//      
	// (in other words, these coefficient give the gradient of a property
	// interpolated in the triangle T = (a0, a1, a2).
	
	// The equations of the coefficients can be simplified, 
	// the general equations are given as code in comments
	// marked by the tag [simplified]
	
	vertex_[0] = p0;
	vertex_[1] = p1;
	vertex_[2] = p2;
	
	// Step1: find an orthonormal basis (X, Y) for the triangle.
	//----------------------------------------------------------
	
	vec3 origin;
	vec3 X;
	vec3 Y;
	vec3 Z;
	
	basis(origin, X, Y, Z); //  Rem: origin = v0. 
	
	// Step2: compute the coordinates of the three vertices of the
	// triangle in this basis.
	//------------------------------------------------------------
	
	// [simplified] double x0 = 0.0 ;
	// [simplified] double y0 = 0.0 ; 
	vec3 V1 = p1 - p0;
	double x1 = sqrt(dot(V1, V1));
	
	// [simplified] double y1 = 0.0 ;
	vec3 V2 = p2 - p0;
	double x2 = dot(V2, X);
	double y2 = dot(V2, Y);
	

	// Step3: compute the six coefficients TXi and TYi allowing to
	// compute the two components of the gradient of a function in
	// the basis (I,X,Y).
	//------------------------------------------------------------    
	
	// [simplified] 
	// double d = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0) ;
	
	double d = x1 * y2;
	
	if (fabs(d) < 1e-10)  {
	    Logger::warn("TrglGrad")
		<< "Attempted gradient computation on a flat triangle"
		<< std::endl;
	    d = 1.0;
	    is_flat_ = true;
	} else {
	    is_flat_ = false;
	}

	// [simplified] TX_[0] = (y1 - y2) / d ;
	// [simplified] TX_[1] = (y2 - y0) / d ;
	// [simplified] TX_[2] = (y0 - y1) / d ;
	
	double xx1 = (x1 == 0.0) ? 1.0 : x1;
	
	TX_[0] = -1.0 / xx1;
	TX_[1] = 1.0 / xx1;
	TX_[2] = 0.0;
	
	// [simplified] TY_[0] = (x2 - x1) / d ;
	// [simplified] TY_[1] = (x0 - x2) / d ;
	// [simplified] TY_[2] = (x1 - x0) / d ;
	
	double yy2 = (y2 == 0.0) ? 1.0 : y2;
	
	TY_[0] = x2 / d - 1.0 / yy2;
	TY_[1] = -x2 / d;
	TY_[2] = 1.0 / yy2;
    }
    
    void TrglGradient::basis(vec3& origin, vec3& X, vec3& Y, vec3& Z) const {
	const vec3& p0 = vertex_[0];
	const vec3& p1 = vertex_[1];
	const vec3& p2 = vertex_[2];
	
	X = normalize(p1 - p0);
	Z = normalize(cross(X, p2 - p0));
	Y = cross(Z, X);
	origin = p0;
    }
    
    vec3 TrglGradient::gradient_3d(double value0, double value1, double value2) const {
	double x = TX_[0] * value0 + TX_[1] * value1; // Note: TX_[2] = 0
	double y = TY_[0] * value0 + TY_[1] * value1 + TY_[2] * value2;
	vec3 O;
	vec3 X, Y, Z;
	basis(O, X, Y, Z);
	return x*X + y*Y;
    }

    /**********************************************************************************/

    //     _____   _____            ____  _____                                _                    _                    _ 
    //    |  __ \ / ____|   /\     |___ \|  __ \                              | |                  | |                  | |
    //    | |__) | |       /  \      __) | |  | |  ______ ______   _ __   ___ | |_    ___ ___ _ __ | |_ ___ _ __ ___  __| |
    //    |  ___/| |      / /\ \    |__ <| |  | | |______|______| | '_ \ / _ \| __|  / __/ _ \ '_ \| __/ _ \ '__/ _ \/ _` |
    //    | |    | |____ / ____ \   ___) | |__| |                 | | | | (_) | |_  | (_|  __/ | | | ||  __/ | |  __/ (_| |
    //    |_|     \_____/_/    \_\ |____/|_____/                  |_| |_|\___/ \__|  \___\___|_| |_|\__\___|_|  \___|\__,_|
    //                                                                                                                     
    //                                                                                                                   

    void UncenteredPCA3D::begin_points(){
	nb_points_ = 0;
	sum_weights_ = M_[0] = M_[1] = M_[2] = M_[3] = M_[4] = M_[5] = 0;
    }
    
    void UncenteredPCA3D::point(const vec3& p, double weight){
	M_[0] += weight * p.x*p.x;
	M_[1] += weight * p.x*p.y; M_[2] += weight * p.y*p.y;
	M_[3] += weight * p.x*p.z; M_[4] += weight * p.y*p.z; M_[5] += weight * p.z*p.z;
	nb_points_++;
	sum_weights_ += weight;
    }
    
    void UncenteredPCA3D::end_points(){
	for (int i = 0; i < 6; i++) M_[i] = M_[i] / sum_weights_;
	if (M_[0] <= 0) M_[0] = 1.e-30;
	if (M_[2] <= 0) M_[2] = 1.e-30;
	if (M_[5] <= 0) M_[5] = 1.e-30;
	double eigen_vectors[9];
	// TODO: remove this dependance
	GEO::MatrixUtil::semi_definite_symmetric_eigen(M_, 3, eigen_vectors, eigen_value);
	
	for (int i = 0; i < 3; i++) axis[i] = normalize(vec3(eigen_vectors[3 * i + 0], eigen_vectors[3 * i + 1], eigen_vectors[3 * i + 2]));
    }
    
    
    /**********************************************************************************/
    /* Triangle-Triangle and Triangle-Box intersection routines by Tomas Moller, 1997 */
    /**********************************************************************************/

    // [Bruno] Modernized it a bit and replaced some macros with templates.
    
    template <class T> inline T FABS(T x) {
	return T(::fabs(double(x)));
    }
    
    /* if USE_EPSILON_TEST is true then we do a check:
     *  if |dv|<EPSILON then dv=0.0;
     *  else no check is done (which is less robust)
     */
    #define USE_EPSILON_TEST
    
    const double EPSILON = 0.000001;

    template <class T> inline void CROSS(T dest[3], const T v1[3], const T v2[3]) {
	dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; 
	dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; 
	dest[2] = v1[0] * v2[1] - v1[1] * v2[0]; 
    }

    template <class T> inline T DOT(T v1[3], T v2[3]) {
	return (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]);
    }

    template <class T> inline void SUB(T dest[3], const T v1[3], const T v2[3]) {
	dest[0] = v1[0] - v2[0]; 
	dest[1] = v1[1] - v2[1]; 
	dest[2] = v1[2] - v2[2];
    }

    /* sort so that a<=b */    
    template <class T> inline void SORT(T& a, T& b) {
	if(a>b) {
	    std::swap(a,b);
	}
    }

    /* this edge to edge test is based on Franlin Antonio's gem:
     *  "Faster Line Segment Intersection", in Graphics Gems III,
     *  pp. 199-202 
     */
#define EDGE_EDGE_TEST(V0,U0,U1)                                \
    Bx = U0[i0] - U1[i0];					\
    By = U0[i1] - U1[i1];					\
    Cx = V0[i0] - U0[i0];					\
    Cy = V0[i1] - U0[i1];					\
    f = Ay*Bx - Ax*By;						\
    d = By*Cx - Bx*Cy;						\
    if ((f>0 && d >= 0 && d <= f) || (f<0 && d <= 0 && d >= f))	\
	{							\
	    e = Ax*Cy - Ay*Cx;					\
	    if (f>0)						\
		{						\
		    if (e >= 0 && e <= f) return 1;		\
		}						\
	    else						\
		{						\
		    if (e <= 0 && e >= f) return 1;		\
		}						\
	}

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2)                \
    {							      \
	double Ax, Ay, Bx, By, Cx, Cy, e, d, f;               \
	Ax = V1[i0] - V0[i0];				      \
	Ay = V1[i1] - V0[i1];				      \
	/* test edge U0,U1 against V0,V1 */		      \
	EDGE_EDGE_TEST(V0, U0, U1);			      \
	/* test edge U1,U2 against V0,V1 */		      \
	EDGE_EDGE_TEST(V0, U1, U2);			      \
	/* test edge U2,U1 against V0,V1 */		      \
	EDGE_EDGE_TEST(V0, U2, U0);			      \
    }
    
#define POINT_IN_TRI(V0,U0,U1,U2)                       \
    {							\
	double a, b, c, d0, d1, d2;                     \
	/* is T1 completly inside T2? */		\
	/* check if V0 is inside tri(U0,U1,U2) */	\
	a = U1[i1] - U0[i1];				\
	b = -(U1[i0] - U0[i0]);				\
	c = -a*U0[i0] - b*U0[i1];			\
	d0 = a*V0[i0] + b*V0[i1] + c;                   \
							\
	a = U2[i1] - U1[i1];				\
	b = -(U2[i0] - U1[i0]);				\
	c = -a*U1[i0] - b*U1[i1];			\
	d1 = a*V0[i0] + b*V0[i1] + c;                   \
							\
	a = U0[i1] - U2[i1];				\
	b = -(U0[i0] - U2[i0]);				\
	c = -a*U2[i0] - b*U2[i1];			\
	d2 = a*V0[i0] + b*V0[i1] + c;                   \
	if (d0*d1>0.0)					\
	    {						\
		if (d0*d2>0.0) return 1;		\
	    }						\
    }

    static int coplanar_tri_tri(
	 double N[3], double V0[3], double V1[3], double V2[3],
	 double U0[3], double U1[3], double U2[3]
    ) {
	double A[3];
	short i0, i1;
	/* first project onto an axis-aligned plane, that maximizes the area */
	/* of the triangles, compute indices: i0,i1. */
	A[0] = FABS(N[0]);
	A[1] = FABS(N[1]);
	A[2] = FABS(N[2]);
	if (A[0]>A[1])
	    {
		if (A[0]>A[2])
		    {
			i0 = 1;      /* A[0] is greatest */
			i1 = 2;
		    }
		else
		    {
			i0 = 0;      /* A[2] is greatest */
			i1 = 1;
		    }
	    }
	else   /* A[0]<=A[1] */
	    {
		if (A[2]>A[1])
		    {
			i0 = 0;      /* A[2] is greatest */
			i1 = 1;
		    }
		else
		    {
			i0 = 0;      /* A[1] is greatest */
			i1 = 2;
		    }
	    }
	
	/* test all edges of triangle 1 against the edges of triangle 2 */
	EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);
	
	/* finally, test if tri1 is totally contained in tri2 or vice versa */
	POINT_IN_TRI(V0, U0, U1, U2);
	POINT_IN_TRI(U0, V0, V1, V2);
	
	return 0;
    }



#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
    {									\
	if (D0D1>0.0)							\
	    {								\
		/* here we know that D0D2<=0.0 */			\
		/* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
		A = VV2; B = (VV0 - VV2)*D2; C = (VV1 - VV2)*D2; X0 = D2 - D0; X1 = D2 - D1; \
	    }								\
	else if (D0D2>0.0)						\
	    {								\
		/* here we know that d0d1<=0.0 */			\
		A = VV1; B = (VV0 - VV1)*D1; C = (VV2 - VV1)*D1; X0 = D1 - D0; X1 = D1 - D2; \
	    }								\
	else if (D1*D2>0.0 || D0 != 0.0)				\
	    {								\
		/* here we know that d0d1<=0.0 or that D0!=0.0 */	\
		A = VV0; B = (VV1 - VV0)*D0; C = (VV2 - VV0)*D0; X0 = D0 - D1; X1 = D0 - D2; \
	    }								\
	else if (D1 != 0.0)						\
	    {								\
		A = VV1; B = (VV0 - VV1)*D1; C = (VV2 - VV1)*D1; X0 = D1 - D0; X1 = D1 - D2; \
	    }								\
	else if (D2 != 0.0)						\
	    {								\
		A = VV2; B = (VV0 - VV2)*D2; C = (VV1 - VV2)*D2; X0 = D2 - D0; X1 = D2 - D1; \
	    }								\
	else								\
	    {								\
		/* triangles are coplanar */				\
		return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2);	\
	    }								\
    }

    int NoDivTriTriIsect(
        double V0[3], double V1[3], double V2[3],
	double U0[3], double U1[3], double U2[3]
    ) {
	double E1[3], E2[3];
	double N1[3], N2[3], d1, d2;
	double du0, du1, du2, dv0, dv1, dv2;
	double D[3];
	double isect1[2], isect2[2];
	double du0du1, du0du2, dv0dv1, dv0dv2;
	short index;
	double vp0, vp1, vp2;
	double up0, up1, up2;
	double bb, cc, max;

	/* compute plane equation of triangle(V0,V1,V2) */
	SUB(E1, V1, V0);
	SUB(E2, V2, V0);
	CROSS(N1, E1, E2);
	d1 = -DOT(N1, V0);
	/* plane equation 1: N1.X+d1=0 */

	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	du0 = DOT(N1, U0) + d1;
	du1 = DOT(N1, U1) + d1;
	du2 = DOT(N1, U2) + d1;

	/* coplanarity robustness check */
#ifdef USE_EPSILON_TEST
	if (FABS(du0)<EPSILON) du0 = 0.0;
	if (FABS(du1)<EPSILON) du1 = 0.0;
	if (FABS(du2)<EPSILON) du2 = 0.0;
#endif
	du0du1 = du0*du1;
	du0du2 = du0*du2;

	if (du0du1>0.0 && du0du2>0.0) /* same sign on all of them + not equal 0 ? */
		return 0;                    /* no intersection occurs */

	/* compute plane of triangle (U0,U1,U2) */
	SUB(E1, U1, U0);
	SUB(E2, U2, U0);
	CROSS(N2, E1, E2);
	d2 = -DOT(N2, U0);
	/* plane equation 2: N2.X+d2=0 */

	/* put V0,V1,V2 into plane equation 2 */
	dv0 = DOT(N2, V0) + d2;
	dv1 = DOT(N2, V1) + d2;
	dv2 = DOT(N2, V2) + d2;

#ifdef USE_EPSILON_TEST
	if (FABS(dv0)<EPSILON) dv0 = 0.0;
	if (FABS(dv1)<EPSILON) dv1 = 0.0;
	if (FABS(dv2)<EPSILON) dv2 = 0.0;
#endif

	dv0dv1 = dv0*dv1;
	dv0dv2 = dv0*dv2;

	if (dv0dv1>0.0 && dv0dv2>0.0) /* same sign on all of them + not equal 0 ? */
		return 0;                    /* no intersection occurs */

	/* compute direction of intersection line */
	CROSS(D, N1, N2);

	/* compute and index to the largest component of D */
	max = (double)FABS(D[0]);
	index = 0;
	bb = (double)FABS(D[1]);
	cc = (double)FABS(D[2]);
	if (bb>max) {max = bb; index = 1;}
	if (cc>max) {max = cc; index = 2;}

	/* this is the simplified projection onto L*/
	vp0 = V0[index];
	vp1 = V1[index];
	vp2 = V2[index];

	up0 = U0[index];
	up1 = U1[index];
	up2 = U2[index];

	/* compute interval for triangle 1 */
	double a, b, c, x0, x1;
	NEWCOMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

	/* compute interval for triangle 2 */
	double d, e, f, y0, y1;
	NEWCOMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

	double xx, yy, xxyy, tmp;
	xx = x0*x1;
	yy = y0*y1;
	xxyy = xx*yy;

	tmp = a*xxyy;
	isect1[0] = tmp + b*x1*yy;
	isect1[1] = tmp + c*x0*yy;

	tmp = d*xxyy;
	isect2[0] = tmp + e*xx*y1;
	isect2[1] = tmp + f*xx*y0;

	SORT(isect1[0], isect1[1]);
	SORT(isect2[0], isect2[1]);

	if (isect1[1]<isect2[0] || isect2[1]<isect1[0]) return 0;
	return 1;
    }    
    
    /*******************************************************************************/

    #define X 0
    #define Y 1
    #define Z 2

    template <class T> inline void FINDMINMAX(T x0, T x1, T x2, T& min, T& max) {
	min = max = x0;				
	if(x1<min) min=x1;
	if(x1>max) max=x1;
	if(x2<min) min=x2;
	if(x2>max) max=x2;
    }

    static int planeBoxOverlap(float normal[3], float d, float maxbox[3]) {
	int q;
	float vmin[3], vmax[3];
	for (q = X; q <= Z; q++)
	    {
		if (normal[q]>0.0f)
		    {
			vmin[q] = -maxbox[q];
			vmax[q] = maxbox[q];
		    }
		else
		    {
			vmin[q] = maxbox[q];
			vmax[q] = -maxbox[q];
		    }
	    }
	if (DOT(normal, vmin) + d>0.0f) return 0;
	if (DOT(normal, vmax) + d >= 0.0f) return 1;
	
	return 0;
    }

    /*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p2 = a*v2[Y] - b*v2[Z];				   \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}	   \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];	   \
    if(min>rad || max<-rad) return 0;
    
#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p1 = a*v1[Y] - b*v1[Z];				   \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}	   \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];	   \
    if(min>rad || max<-rad) return 0;
    
/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[X] + b*v0[Z];			   \
    p2 = -a*v2[X] + b*v2[Z];				   \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}	   \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];	   \
    if(min>rad || max<-rad) return 0;
    
#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[X] + b*v0[Z];			   \
    p1 = -a*v1[X] + b*v1[Z];				   \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}	   \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];	   \
    if(min>rad || max<-rad) return 0;
    
/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[X] - b*v1[Y];                    \
    p2 = a*v2[X] - b*v2[Y];				   \
    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}	   \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];	   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)	   \
    p0 = a*v0[X] - b*v0[Y];		       \
    p1 = a*v1[X] - b*v1[Y];				   \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}	   \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];	   \
    if(min>rad || max<-rad) return 0;


    int triBoxOverlap(
	float boxcenter[3], float boxhalfsize[3], float triverts[3][3]
    ) {

	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */
	float v0[3], v1[3], v2[3];
	float min, max, d, p0, p1, p2, rad, fex, fey, fez;
	float normal[3], e0[3], e1[3], e2[3];

	/* This is the fastest branch on Sun */
	/* move everything so that the boxcenter is in (0,0,0) */
	SUB(v0, triverts[0], boxcenter);
	SUB(v1, triverts[1], boxcenter);
	SUB(v2, triverts[2], boxcenter);

	/* compute triangle edges */
	SUB(e0, v1, v0);      /* tri edge 0 */
	SUB(e1, v2, v1);      /* tri edge 1 */
	SUB(e2, v0, v2);      /* tri edge 2 */

	/* Bullet 3:  */
	/*  test the 9 tests first (this was faster) */
	fex = FABS(e0[X]);
	fey = FABS(e0[Y]);
	fez = FABS(e0[Z]);
	AXISTEST_X01(e0[Z], e0[Y], fez, fey);
	AXISTEST_Y02(e0[Z], e0[X], fez, fex);
	AXISTEST_Z12(e0[Y], e0[X], fey, fex);

	fex = FABS(e1[X]);
	fey = FABS(e1[Y]);
	fez = FABS(e1[Z]);
	AXISTEST_X01(e1[Z], e1[Y], fez, fey);
	AXISTEST_Y02(e1[Z], e1[X], fez, fex);
	AXISTEST_Z0(e1[Y], e1[X], fey, fex);

	fex = FABS(e2[X]);
	fey = FABS(e2[Y]);
	fez = FABS(e2[Z]);
	AXISTEST_X2(e2[Z], e2[Y], fez, fey);
	AXISTEST_Y1(e2[Z], e2[X], fez, fex);
	AXISTEST_Z12(e2[Y], e2[X], fey, fex);

	/* Bullet 1: */
	/*  first test overlap in the {x,y,z}-directions */
	/*  find min, max of the triangle each direction, and test for overlap in */
	/*  that direction -- this is equivalent to testing a minimal AABB around */
	/*  the triangle against the AABB */

	/* test in X-direction */
	FINDMINMAX(v0[X], v1[X], v2[X], min, max);
	if (min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

	/* test in Y-direction */
	FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);
	if (min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

	/* test in Z-direction */
	FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);
	if (min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

	/* Bullet 2: */
	/*  test if the box intersects the plane of the triangle */
	/*  compute plane equation of triangle: normal*x+d=0 */
	CROSS(normal, e0, e1);
	d = -DOT(normal, v0);  /* plane eq: normal.x+d=0 */
	if (!planeBoxOverlap(normal, d, boxhalfsize)) return 0;

	return 1;   /* box and triangle overlaps */
    }
    
    /*******************************************************************************/    
    
    
}

