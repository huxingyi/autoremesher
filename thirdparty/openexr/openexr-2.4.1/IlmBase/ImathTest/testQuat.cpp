///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Industrial Light & Magic, a division of Lucas
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

#include <testQuat.h>
#include "ImathQuat.h"
#include "ImathMatrixAlgo.h"
#include "ImathFun.h"
#include "ImathLimits.h"
#include "ImathPlatform.h" /* [i_a] M_PI_2 */
#include <iostream>
#include <cassert>
#include <cmath>


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

template <class T>
void
testQuatT ()
{
    const T s = limits<T>::smallest();
    const T e = 4 * limits<T>::epsilon();

    //
    // constructors, r(), v()
    //

    {
	Quat<T> q = Quat<T>();
	assert (q.r == 1 && q.v == Vec3<T> (0, 0, 0));

	q = Quat<T> (2, 3, 4, 5);
	assert (q.r == 2 && q.v == Vec3<T> (3, 4, 5));

	q = Quat<T> (6, Vec3<T> (7, 8, 9));
	assert (q.r == 6 && q.v == Vec3<T> (7, 8, 9));

	Quat<T> q1 = Quat<T> (q);
	assert (q1.r == 6 && q1.v == Vec3<T> (7, 8, 9));
    }

    //
    // invert(), inverse()
    //

    {
	Quat<T> q = Quat<T> (1, 0, 0, 1);
	assert (q.inverse() == Quat<T> (0.5, 0, 0, -0.5));

	q.invert();
	assert (q == Quat<T> (0.5, 0, 0, -0.5));
    }

    //
    // normalize(), normalized()
    //

    {
	Quat<T> q = Quat<T> (2, Vec3<T> (0, 0, 0));
	assert (q.normalized() == Quat<T> (1, 0, 0, 0));

	q.normalize();
	assert (q == Quat<T> (1, 0, 0, 0));

	q = Quat<T> (0, Vec3<T> (0, 2, 0));
	assert (q.normalized() == Quat<T> (0, 0, 1, 0));

	q.normalize();
	assert (q == Quat<T> (0, 0, 1, 0));
    }

    //
    // length()
    //

    {
	Quat<T> q = Quat<T> (3, 0, 4, 0);
	assert (q.length() == 5);
    }

    //
    // setAxisAngle(), angle(), axis()
    //

    {
	Quat<T> q;
	q.setAxisAngle (Vec3<T> (0, 0, 1), M_PI_2);
	Vec3<T> v = q.axis();
	T a = q.angle();
	assert (v.equalWithAbsError (Vec3<T> (0, 0, 1), e));
	assert (IMATH_INTERNAL_NAMESPACE::equal (a, T (M_PI_2), e));
    }

    //
    // Accuracy of angle() for very small angles
    // and when real part is slightly greater than 1.
    //

    {
	T t = 10 * Math<T>::sqrt (s);

	Quat<T> q;
	q.setAxisAngle (Vec3<T> (0, 0, 1), t);
	Vec3<T> v = q.axis();
	T a = q.angle();
	assert (v.equalWithAbsError (Vec3<T> (0, 0, 1), e));
	assert (IMATH_INTERNAL_NAMESPACE::equal (a, t, t * e));

	q.r *= 1.1;
	q.v *= 1.1;
	v = q.axis();
	a = q.angle();
	assert (v.equalWithAbsError (Vec3<T> (0, 0, 1), e));
	assert (IMATH_INTERNAL_NAMESPACE::equal (a, t, t * e));
    }

    {
	T t = 0.001 * Math<T>::sqrt (s);

	Quat<T> q;
	q.setAxisAngle (Vec3<T> (0, 0, 1), t);
	Vec3<T> v = q.axis();
	T a = q.angle();
	assert (v.equalWithAbsError (Vec3<T> (0, 0, 1), e));
	assert (IMATH_INTERNAL_NAMESPACE::equal (a, t, t * e));

	q.r *= 1.1;
	q.v *= 1.1;
	v = q.axis();
	a = q.angle();
	assert (v.equalWithAbsError (Vec3<T> (0, 0, 1), e));
	assert (IMATH_INTERNAL_NAMESPACE::equal (a, t, t * e));
    }

    //
    // toMatrix33(), toMatrix44()
    //

    {
	Quat<T> q;
	q.setRotation (Vec3<T> (1, 0, 0), Vec3<T> (0, 1, 0));

	Matrix33<T> m1 = q.toMatrix33();

	assert (m1.equalWithAbsError (Matrix33<T> (0, 1, 0,
						  -1, 0, 0,
						   0, 0, 1),
				      e));

	Matrix44<T> m2 = q.toMatrix44();

	assert (m2.equalWithAbsError (Matrix44<T> (0, 1, 0, 0,
						  -1, 0, 0, 0,
						   0, 0, 1, 0,
						   0, 0, 0, 1),
				      e));
    }

    //
    // +, - (unary and binary), ~ *, /, ^
    //

    assert (Quat<T> (1, 2, 3, 4) + Quat<T> (5, 6, 7, 8) ==
	    Quat<T> (6, 8, 10, 12));

    assert (Quat<T> (-1, -2, -3, -4) - Quat<T> (5, 6, 7, 8) ==
	    Quat<T> (-6, -8, -10, -12));

    assert (-Quat<T> (1, 2, 3, 4) == Quat<T> (-1, -2, -3, -4));
    
    assert (~Quat<T> (1, 2, 3, 4) == Quat<T> (1, -2, -3, -4));

    assert (T (2) * Quat<T> (1, 2, 3, 4) == Quat<T> (2, 4, 6, 8));

    assert (Quat<T> (1, 2, 3, 4) * T (2 )== Quat<T> (2, 4, 6, 8));

    assert (Quat<T> (1, 0, 0, 1) * Quat<T> (1, 1, 0, 0) ==
	    Quat<T> (1, 1, 1, 1));

    assert (Quat<T> (1, 1, 0, 0) * Quat<T> (1, 0, 0, 1) ==
	    Quat<T> (1, 1, -1, 1));
    
    assert (Quat<T> (1, 0, 0, 1) / Quat<T> (0.5, -0.5, 0, 0) ==
	    Quat<T> (1, 1, 1, 1));

    assert (Quat<T> (2, 4, 6, 8) / T (2) == Quat<T> (1, 2, 3, 4));

    assert ((Quat<T> (1, 2, 3, 4) ^ Quat<T> (2, 2, 2, 2)) == 20);

    //
    // extract()
    //

    {
	Vec3<T> vFrom (1, 0, 0);
	Vec3<T> vTo (0, 1, 1);
	Matrix44<T> m1 = rotationMatrix (vFrom, vTo);
	
	Quat<T> q = extractQuat (m1);;

	Matrix44<T> m2 = q.toMatrix44();

	assert (m2.equalWithAbsError (m1, 2 * e));
    }
}


void
testQuatConversions ()
{
    {
	Quatf q (1, V3f (2, 3, 4));
	Quatd q1 = Quatd (q);
	assert (q1.r == 1 && q1.v == V3d (2, 3, 4));
    }

    {
	Quatd q (1, V3d (2, 3, 4));
	Quatf q1 = Quatf (q);
	assert (q1.r == 1 && q1.v == V3f (2, 3, 4));
    }
}

} // namespace


void
testQuat ()
{
    cout << "Testing basic quaternion operations" << endl;

    testQuatT<float>();
    testQuatT<double>();
    testQuatConversions();

    cout << "ok\n" << endl;
}
