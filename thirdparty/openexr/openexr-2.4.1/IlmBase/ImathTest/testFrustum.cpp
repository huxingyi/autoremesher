///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testFrustum.h>
#include "ImathFrustum.h"
#include "ImathEuler.h"
#include "ImathFun.h"
#include <iostream>
#include <assert.h>


using namespace std;


namespace 
{

void
testFrustumPlanes  (IMATH_INTERNAL_NAMESPACE::Frustumf &frustum)
{
    bool ortho = frustum.orthographic();
    IMATH_INTERNAL_NAMESPACE::V3f o (0.0f, 0.0f, 0.0f);
    float eps = 5.0e-4;

    for (float xRo = 0.0f; xRo < 360.0f; xRo += 100.0f)
    {
        for (float yRo = 0.0f; yRo < 360.0f; yRo += 105.0f)
        {
            for (float zRo = 0.0f; zRo < 360.0f; zRo += 110.0f)
            {
                for (float xTr = -10.0f; xTr < 10.0f; xTr += 2)
                {
                    for (float yTr = -10.0f; yTr < 10.0f; yTr += 3)
                    {
                        for (float zTr = -10.0f; zTr < 10.0f; zTr += 4)
                        {
                            float xRoRad = xRo * (2.0f * float(M_PI) / 360.0f);
                            float yRoRad = yRo * (2.0f * float(M_PI) / 360.0f);
                            float zRoRad = zRo * (2.0f * float(M_PI) / 360.0f);
                            IMATH_INTERNAL_NAMESPACE::Eulerf e(xRoRad, yRoRad, zRoRad);
                            IMATH_INTERNAL_NAMESPACE::M44f mView = e.toMatrix44();
                            mView.translate (IMATH_INTERNAL_NAMESPACE::V3f(xTr, yTr, zTr));
                            
                            IMATH_INTERNAL_NAMESPACE::Plane3f planes0[6];
                            frustum.planes (planes0);
                            
                            IMATH_INTERNAL_NAMESPACE::Plane3f planes[6];
                            frustum.planes (planes, mView);
                            
                            IMATH_INTERNAL_NAMESPACE::V3f up = IMATH_INTERNAL_NAMESPACE::V3f(0, 1, 0);
                            assert ((up ^ planes0[0].normal) > 0.0);
                            mView.multDirMatrix (up, up);
                            assert ((up ^ planes[0].normal) > 0.0);

                            IMATH_INTERNAL_NAMESPACE::V3f pt = (! ortho) ? o :
                                IMATH_INTERNAL_NAMESPACE::V3f (0.0f, frustum.top(), 0.0f);
                            float d = planes0[0].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[0].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));

                            IMATH_INTERNAL_NAMESPACE::V3f right = IMATH_INTERNAL_NAMESPACE::V3f(1, 0, 0);
                            assert ((right ^ planes0[1].normal) > 0.0);
                            mView.multDirMatrix (right, right);
                            assert ((right ^ planes[1].normal) > 0.0);
                            
                            pt = (! ortho) ? o :
                                IMATH_INTERNAL_NAMESPACE::V3f (frustum.right(), 0.0f, 0.0f);
                            d = planes0[1].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[1].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));

                            IMATH_INTERNAL_NAMESPACE::V3f down = IMATH_INTERNAL_NAMESPACE::V3f(0, -1, 0);
                            assert ((down ^ planes0[2].normal) > 0.0);
                            mView.multDirMatrix (down, down);
                            assert ((down ^ planes[2].normal) > 0.0);
                            
                            pt = (! ortho) ? o :
                                IMATH_INTERNAL_NAMESPACE::V3f (0.0f, frustum.bottom(), 0.0f);
                            d = planes0[2].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[2].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));

                            IMATH_INTERNAL_NAMESPACE::V3f left = IMATH_INTERNAL_NAMESPACE::V3f(-1, 0, 0);
                            assert ((left ^ planes0[3].normal) > 0.0);
                            mView.multDirMatrix (left, left);
                            assert ((left ^ planes[3].normal) > 0.0);
                            
                            pt = (! ortho) ? o :
                                IMATH_INTERNAL_NAMESPACE::V3f (frustum.left(), 0.0f, 0.0f);
                            d = planes0[3].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[3].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));

                            IMATH_INTERNAL_NAMESPACE::V3f front = IMATH_INTERNAL_NAMESPACE::V3f(0, 0, 1);
                            assert ((front ^ planes0[4].normal) > 0.0);
                            mView.multDirMatrix (front, front);
                            assert ((front ^ planes[4].normal) > 0.0);
                            
                            pt = IMATH_INTERNAL_NAMESPACE::V3f (0.0f, 0.0f, -frustum.nearPlane());
                            d = planes0[4].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[4].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));

                            IMATH_INTERNAL_NAMESPACE::V3f back = IMATH_INTERNAL_NAMESPACE::V3f(0, 0, -1);
                            assert ((back ^ planes0[5].normal) > 0.0);
                            mView.multDirMatrix (back, back);
                            assert ((back ^ planes[5].normal) > 0.0);
                            
                            pt = IMATH_INTERNAL_NAMESPACE::V3f (0.0f, 0.0f, -frustum.farPlane());
                            d = planes0[5].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                            pt = pt * mView;
                            d = planes[5].distanceTo (pt);
                            assert (IMATH_INTERNAL_NAMESPACE::iszero (d, eps));
                        }
                    }
                }
            }
        }
    }

}

}


void
testFrustum ()
{
    cout << "Testing functions in ImathFrustum.h";

    cout << "\nperspective ";

    float n = 1.7;
    float f = 567.0;
    float l = -3.5;
    float r = 2.0;
    float b = -1.3;
    float t = 0.9;

    IMATH_INTERNAL_NAMESPACE::Frustum<float> frustum (n, f, l, r, t, b, false);

    assert (IMATH_INTERNAL_NAMESPACE::abs<float> (frustum.fovx() -
			       (atan2(r,n) - atan2(l,n))) < 1e-6);
    assert (IMATH_INTERNAL_NAMESPACE::abs<float> (frustum.fovy() -
			       (atan2(t,n) - atan2(b,n))) < 1e-6);
    cout << "1";
    assert (IMATH_INTERNAL_NAMESPACE::abs<float> (frustum.aspect() - ((r-l)/(t-b))) < 1e-6);
    cout << "2";

    IMATH_INTERNAL_NAMESPACE::M44f m = frustum.projectionMatrix();
    assert (IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][0] - ((2*n)/(r-l)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][1])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][2])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][3])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][0])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][1] - ((2*n)/(t-b)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][2])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][3])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][0] - ((r+l)/(r-l)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][1] - ((t+b)/(t-b)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][2] - (-(f+n)/(f-n)))    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][3] - -1.0)		    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][0])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][1])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][2] - ((-2*f*n)/(f-n)))  < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][3])			    < 1e-6);
    cout << "3";

    cout << "\nplanes ";
    testFrustumPlanes (frustum);

    cout << "\nexceptions ";
    IMATH_INTERNAL_NAMESPACE::Frustum<float> badFrustum;

    bool caught;

    badFrustum.set (n, n, l, r, t, b, false);
    caught = false;
    try
    {
	(void)badFrustum.projectionMatrix();
	assert (!"near == far didn't throw an exception");
    }
    catch (IEX_NAMESPACE::DivzeroExc &)
    {
        caught = true;
    }
    assert (caught);
    cout << "1";

    badFrustum.set (n, f, l, l, t, b, false);
    caught = false;
    try
    {
	(void)badFrustum.projectionMatrix();
	assert (!"left == right didn't throw an exception");
    }
    catch (IEX_NAMESPACE::DivzeroExc &)
    {
        caught = true;
    }
    assert (caught);
    cout << "2";

    badFrustum.set (n, f, l, r, t, t, false);
    caught = false;
    try
    {
	(void)badFrustum.projectionMatrix();
	assert (!"top == bottom didn't throw an exception");
    }
    catch (IEX_NAMESPACE::DivzeroExc &)
    {
        caught = true;
    }
    assert (caught);
    cout << "3";

    cout << "\northographic ";

    frustum.setOrthographic (true);

    m = frustum.projectionMatrix();
    assert (IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][0] - (2/(r-l)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][1])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][2])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[0][3])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][0])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][1] - (2/(t-b)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][2])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[1][3])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][0])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][1])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][2] - (-2/(f-n)))	    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[2][3])			    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][0] - (-(r+l)/(r-l)))    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][1] - (-(t+b)/(t-b)))    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][2] - (-(f+n)/(f-n)))    < 1e-6 &&
	    IMATH_INTERNAL_NAMESPACE::abs<float> (m[3][3] - 1.0)		    < 1e-6);
    cout << "1";

    cout << "\nplanes ";
    testFrustumPlanes (frustum);

    // TODO - There are many little functions in IMATH_INTERNAL_NAMESPACE::Frustum which
    // aren't tested here.  Those test should be added.  But this is
    // a start.

    IMATH_INTERNAL_NAMESPACE::Frustum<float> f1 (n, f, l, r, t, b, false);
    IMATH_INTERNAL_NAMESPACE::Frustum<float> f2 (n, f, l, r, t, b, true);
    assert (f1 != f2);
    f2.set(n + 0.1, f, l, r, t, b, false);
    assert (f1 != f2);
    f2.set(n, f + 0.1, l, r, t, b, false);
    assert (f1 != f2);
    f2.set(n, f, l + 0.1, r, t, b, false);
    assert (f1 != f2);
    f2.set(n, f, l, r + 0.1, t, b, false);
    assert (f1 != f2);
    f2.set(n, f, l, r, t + 0.1, b, false);
    assert (f1 != f2);
    f2.set(n, f, l, r, t, b + 0.1, false);
    assert (f1 != f2);
    cout << "\npassed inequality test";

    f1 = f2;
    assert (f1 == f2);
    cout << "\npassed equality test";

    cout << "\nok\n\n";
}
