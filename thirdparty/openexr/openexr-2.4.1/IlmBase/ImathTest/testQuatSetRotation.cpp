///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
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

#include <testQuatSetRotation.h>
#include "ImathQuat.h"
#include "ImathRandom.h"
#include <iostream>
#include <assert.h>


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

void
testRotation (const V3f &from, const V3f &to)
{
    //
    // Build a quaternion.
    //

    Quatf Q;
    Q.setRotation (from, to);
    M44f M = Q.toMatrix44();

    //
    // Verify that the quaternion rotates vector from into vector to.
    //

    float e = 20 * limits<float>::epsilon();

    V3f fromM = from * M;
    V3f fromQ = from * Q;
    V3f t0 = to.normalized();
    V3f fM0 = fromM.normalized();
    V3f fQ0 = fromQ.normalized();

    assert (t0.equalWithAbsError (fM0, e));
    assert (t0.equalWithAbsError (fQ0, e));

    //
    // Verify that the rotation axis is the cross product of from and to.
    //

    V3f f0 = from.normalized();

    if (abs (f0 ^ t0) < 0.9)
    {
	V3f n0 = (from % to).normalized();
	V3f n0M = n0 * M;

	assert (n0.equalWithAbsError (n0M, e));
    }
}


void
specificVectors ()
{
    cout << "  exact 90-degree rotations" << endl;

    testRotation (V3f (1, 0, 0), V3f (0, 1, 0));
    testRotation (V3f (1, 0, 0), V3f (0, 0, 1));
    testRotation (V3f (0, 1, 0), V3f (1, 0, 0));
    testRotation (V3f (0, 1, 0), V3f (0, 0, 1));
    testRotation (V3f (0, 0, 1), V3f (1, 0, 0));
    testRotation (V3f (0, 0, 1), V3f (0, 1, 0));

    cout << "  exact zero-degree rotations" << endl;

    testRotation (V3f (1, 0, 0), V3f (1, 0, 0));
    testRotation (V3f (0, 1, 0), V3f (0, 1, 0));
    testRotation (V3f (0, 0, 1), V3f (0, 0, 1));
    testRotation (V3f (1, 2, 3), V3f (2, 4, 6));

    cout << "  exact 180-degree rotations" << endl;

    testRotation (V3f (1, 0, 0), V3f (-1, 0, 0));
    testRotation (V3f (0, 1, 0), V3f (0, -1, 0));
    testRotation (V3f (0, 0, 1), V3f (0, 0, -1));
    testRotation (V3f (1, 2, 3), V3f (-2, -4, -6));
    testRotation (V3f (1, 3, 2), V3f (-2, -6, -4));
    testRotation (V3f (2, 1, 3), V3f (-4, -2, -6));
    testRotation (V3f (3, 1, 2), V3f (-6, -2, -4));
    testRotation (V3f (2, 3, 1), V3f (-4, -6, -2));
    testRotation (V3f (3, 2, 1), V3f (-6, -4, -2));

    cout << "  other angles" << endl;

    testRotation (V3f (1, 2, 3), V3f (4, 5, 6));
    testRotation (V3f (1, 2, 3), V3f (4, 6, 5));
    testRotation (V3f (1, 2, 3), V3f (5, 4, 6));
    testRotation (V3f (1, 2, 3), V3f (6, 4, 5));
    testRotation (V3f (1, 2, 3), V3f (5, 6, 4));
    testRotation (V3f (1, 2, 3), V3f (6, 5, 4));
    testRotation (V3f (1, 2, 3), V3f (-4, -5, -6));
    testRotation (V3f (1, 2, 3), V3f (-4, -6, -5));
    testRotation (V3f (1, 2, 3), V3f (-5, -4, -6));
    testRotation (V3f (1, 2, 3), V3f (-6, -4, -5));
    testRotation (V3f (1, 2, 3), V3f (-5, -6, -4));
    testRotation (V3f (1, 2, 3), V3f (-6, -5, -4));
}


void
randomVectors ()
{
    cout << "  random from and to vectors" << endl;

    Rand48 rand (17);

    for (int i = 0; i < 500000; ++i)
    {
	V3f from = hollowSphereRand<V3f> (rand) * rand.nextf (0.1, 10.0);
	V3f to = hollowSphereRand<V3f> (rand) * rand.nextf (0.1, 10.0);
	testRotation (from, to);
    }
}


void
nearlyEqualVectors ()
{
    cout << "  nearly equal from and to vectors" << endl;

    Rand48 rand (19);
    float e = 100 * limits<float>::epsilon();

    for (int i = 0; i < 500000; ++i)
    {
	V3f from = hollowSphereRand<V3f> (rand);
	V3f to = from + e * hollowSphereRand<V3f> (rand);
	testRotation (from, to);
    }
}


void
nearlyOppositeVectors ()
{
    cout << "  nearly opposite from and to vectors" << endl;

    Rand48 rand (19);
    float e = 100 * limits<float>::epsilon();

    for (int i = 0; i < 500000; ++i)
    {
	V3f from = hollowSphereRand<V3f> (rand);
	V3f to = -from + e * hollowSphereRand<V3f> (rand);
	testRotation (from, to);
    }
}



} // namespace


void
testQuatSetRotation ()
{
    cout << "Testing quaternion rotations" << endl;

    specificVectors();
    randomVectors();
    nearlyEqualVectors();
    nearlyOppositeVectors();

    cout << "ok\n" << endl;
}
