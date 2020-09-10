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

#include <testLineAlgo.h>
#include "ImathLineAlgo.h"
#include "ImathRandom.h"
#include <iostream>
#include <assert.h>


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

void
testClosestPoints
    (const Line3f &line1,
     const Line3f &line2,
     bool returnValue,
     const V3f &point1,
     const V3f &point2)
{
    V3f p1;
    V3f p2;
    bool rv = closestPoints (line1, line2, p1, p2);

    assert (rv == returnValue);

    if (rv)
    {
	float e = 10 * limits<float>::epsilon();
	assert (point1.equalWithAbsError (p1, e));
	assert (point2.equalWithAbsError (p2, e));
    }
}


void
testClosestPoints ()
{
    cout << "closest points on two lines" << endl;

    cout << "  non-intersecting, non-parallel lines" << endl;

    testClosestPoints
	(Line3f (V3f ( 0, -1, -1), V3f ( 0,  1, -1)),
	 Line3f (V3f (-1,  0,  1), V3f ( 1,  0,  1)),
	 true,
	 V3f ( 0,  0, -1),
	 V3f ( 0,  0,  1));

    testClosestPoints
	(Line3f (V3f ( 2, -1, -1), V3f ( 2,  1, -1)),
	 Line3f (V3f (-1,  3,  1), V3f ( 1,  3,  1)),
	 true,
	 V3f ( 2,  3, -1),
	 V3f ( 2,  3,  1));

    cout << "  intersecting, non-parallel lines" << endl;

    testClosestPoints
	(Line3f (V3f ( 2, -1,  0), V3f ( 2,  1,  0)),
	 Line3f (V3f (-1,  3,  0), V3f ( 1,  3,  0)),
	 true,
	 V3f ( 2,  3,  0),
	 V3f ( 2,  3,  0));

    cout << "  parallel lines" << endl;

    testClosestPoints
	(Line3f (V3f ( 2, -1,  0), V3f ( 2,  1,  0)),
	 Line3f (V3f ( 2, -1,  1), V3f ( 2,  1,  1)),
	 false,
	 V3f ( 0,  0,  0),
	 V3f ( 0,  0,  0));

    testClosestPoints
	(Line3f (V3f ( 2, -1,  0), V3f ( 2,  1,  0)),
	 Line3f (V3f ( 2,  1,  1), V3f ( 2, -1,  1)),
	 false,
	 V3f ( 0,  0,  0),
	 V3f ( 0,  0,  0));

    cout << "  coincident lines" << endl;

    testClosestPoints
	(Line3f (V3f ( 2, -1,  0), V3f ( 2, -1,  1)),
	 Line3f (V3f ( 2, -1,  0), V3f ( 2, -1,  1)),
	 false,
	 V3f ( 0,  0,  0),
	 V3f ( 0,  0,  0));

    cout << "  random lines" << endl;

    Rand48 rand (7);

    for (int i = 0; i < 10000; ++i)
    {
	Line3f line1 (solidSphereRand<V3f> (rand) * 100.f,
		      solidSphereRand<V3f> (rand) * 100.f);

	Line3f line2 (solidSphereRand<V3f> (rand) * 100.f,
		      solidSphereRand<V3f> (rand) * 100.f);

	V3f point1;
	V3f point2;
	bool rv = closestPoints (line1, line2, point1, point2);

	if (rv)
	{
	    //
	    // We test if the line that connects point1 and point2
	    // is perpendicular to line1 and line2.  The numerical
	    // accuracy of point1 and point2 depends strongly on
	    // the relative directions of line1 and line2; accuracy
	    // degrades rather quickly if line1 and line2 become
	    // close to parallel.
	    //

	    float e = 2000 * limits<float>::epsilon();
	    float d = 1 - (line1.dir ^ line2.dir) * (line1.dir ^ line2.dir);
	    V3f n = point1 - point2;

	    assert (equalWithAbsError (0.0f, (line1.dir ^ n) * d, e));
	    assert (equalWithAbsError (0.0f, (line2.dir ^ n) * d, e));
	}
    }
}


void
testIntersect
    (const Line3f &line,
     const V3f &v0,
     const V3f &v1,
     const V3f &v2,
     const V3f &point,
     bool front,
     bool returnValue)
{
    V3f pt;
    V3f bary;
    bool fr;

    bool rv = intersect (line, v0, v1, v2, pt, bary, fr);

    assert (rv == returnValue);

    float e = 10 * limits<float>::epsilon();

    if (rv)
    {
	assert (front == fr);
	assert (pt.equalWithAbsError (point, e));
	V3f pt2 = v0 * bary.x + v1 * bary.y + v2 * bary.z;
	assert (pt.equalWithAbsError (pt2, e));
    }

}


void
testIntersect ()
{
    cout << "line-triangle intersection" << endl;

    cout << "  line-plane intersection inside triangle" << endl;
    
    testIntersect (Line3f (V3f (0, 0, -1), V3f (0, 0, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (-1, -2, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, -2, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (-1, 1, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, 1, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (-1, 1, 7)),
		   V3f (4, -4, 7), V3f (-4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, 1, 7),
		   false,
		   true);

    testIntersect (Line3f (V3f (1, 1, 2), V3f (0, 0, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (2, 3, -5), V3f (-1, -2, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, -2, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (2, 8, -10), V3f (-1, 1, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, 1, 7),
		   true,
		   true);

    testIntersect (Line3f (V3f (-10, 2, -1), V3f (-1, 1, 7)),
		   V3f (4, -4, 7), V3f (-4, -4, 7), V3f (0, 6, 7),
		   V3f (-1, 1, 7),
		   false,
		   true);

    cout << "  line-plane intersection outside triangle" << endl;

    testIntersect (Line3f (V3f (0, 0, -1), V3f (4, 0, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (-4, 1, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (0, -5, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (0, 0, -1), V3f (0, -7, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    cout << "  line parallel to triangle" << endl;

    testIntersect (Line3f (V3f (0, 0, -1), V3f (4, 0, -1)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (0, 4, 7), V3f (4, 0, 7)),
		   V3f (-4, -4, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    cout << "  zero-area triangle" << endl;

    testIntersect (Line3f (V3f (2, 3, -5), V3f (-1, -2, 7)),
		   V3f (0, 6, 7), V3f (4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (2, 3, -5), V3f (-1, -2, 7)),
		   V3f (-4, -4, 7), V3f (-4, -4, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (2, 3, -5), V3f (-1, -2, 7)),
		   V3f (-4, -4, 7), V3f (0, 6, 7), V3f (0, 6, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    testIntersect (Line3f (V3f (2, 3, -5), V3f (-1, -2, 7)),
		   V3f (-4, -4, 7), V3f (-4, -4, 7), V3f (-4, -4, 7),
		   V3f (0, 0, 0),
		   false,
		   false);

    cout << "  random lines and triangles" << endl;

    Rand48 rand (8);

    for (int i = 0; i < 10000; ++i)
    {
	//
	// Generate a random triangle with non-zero area
	//

	V3f v0, v1, v2;
	V3f normal;

	do
	{
	    v0 = solidSphereRand<V3f> (rand);
	    v1 = solidSphereRand<V3f> (rand);
	    v2 = solidSphereRand<V3f> (rand);
	    normal = (v2 - v1) % (v1 - v0);
	}
	while (normal.length() < 0.01);

	{
	    //
	    // Generate a line that intersects inside the triangle
	    //

	    V3f b;

	    do
	    {
		b.x = rand.nextf (0.001, 0.999);
		b.y = rand.nextf (0.001, 0.999);
		b.z = 1 - b.x - b.y;
	    }
	    while (b.x + b.y > 0.999);

	    V3f p1 = v0 * b.x + v1 * b.y + v2 * b.z;

	    V3f p0;

	    do
	    {
		p0 = solidSphereRand<V3f> (rand);
	    }
	    while (abs (normal.normalized() ^ (p1 - p0).normalized()) < 0.1);

	    //
	    // Test for intersection
	    //

	    V3f point;
	    V3f bary;
	    bool front;

	    bool rv = intersect (Line3f (p0, p1),
	                         v0, v1, v2, point,
				 bary, front);

	    assert (rv == true);

	    float nd = abs (normal.normalized() ^ (p1 - p0).normalized());
	    float ep = 20 * limits<float>::epsilon() / nd;

	    assert (point.equalWithAbsError (p1, ep));
	}

	{
	    //
	    // Generate a line that intersects the triangle's plane
	    // but outside the triangle
	    //

	    V3f b;

	    do
	    {
		b.x = rand.nextf (-3, 3);
		b.y = rand.nextf (-3, 3);
		b.z = 1 - b.x - b.y;
	    }
	    while (b.x > -0.001 && b.y > -0.001 && b.z > -0.001);

	    V3f p1 = v0 * b.x + v1 * b.y + v2 * b.z;

	    V3f p0;

	    do
	    {
		p0 = solidSphereRand<V3f> (rand) * 10;
	    }
	    while (abs (normal.normalized() ^ (p1 - p0).normalized()) < 0.1);

	    //
	    // Test for intersection
	    //

	    V3f point;
	    V3f bary;
	    bool front;

	    bool rv = intersect (Line3f (p0, p1),
	                         v0, v1, v2, point,
				 bary, front);

	    assert (rv == false);
	}
    }
}


} // namespace


void
testLineAlgo ()
{
    cout << "Testing line algorithms" << endl;

    testClosestPoints();
    testIntersect();

    cout << "ok\n" << endl;
}
