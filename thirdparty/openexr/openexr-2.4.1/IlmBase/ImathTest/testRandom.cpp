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

#include <testRandom.h>
#include "ImathRandom.h"
#include "ImathVec.h"
#include "ImathFun.h"
#include <iostream>
#include <iomanip>
#include <assert.h>


using namespace std;
using IMATH_INTERNAL_NAMESPACE::abs;

namespace {

void
testErand48 ()
{
    //
    // Our implementation of erand48(), nrand48(), etc.
    // assumes that sizeof (unsigned short) == 2.
    //

    assert (sizeof (unsigned short) == 2);

    //
    // starting with a given seed, erand48() and nrand48()
    // must generate the same sequence as the standard
    // Unix/Linux functions.
    //

    unsigned short state[3];
    state[0] = 0;
    state[1] = 1;
    state[2] = 2;

    assert (abs (IMATH_INTERNAL_NAMESPACE::erand48 (state) - 0.671004) < 0.00001);
    assert (abs (IMATH_INTERNAL_NAMESPACE::erand48 (state) - 0.786905) < 0.00001);
    assert (abs (IMATH_INTERNAL_NAMESPACE::erand48 (state) - 0.316850) < 0.00001);
    assert (abs (IMATH_INTERNAL_NAMESPACE::erand48 (state) - 0.384870) < 0.00001);
    assert (abs (IMATH_INTERNAL_NAMESPACE::erand48 (state) - 0.854650) < 0.00001);

    assert (IMATH_INTERNAL_NAMESPACE::nrand48 (state) == 0x4f4e8cb0);
    assert (IMATH_INTERNAL_NAMESPACE::nrand48 (state) == 0x063e864b);
    assert (IMATH_INTERNAL_NAMESPACE::nrand48 (state) == 0x2d10f1dd);
    assert (IMATH_INTERNAL_NAMESPACE::nrand48 (state) == 0x1aadc122);
    assert (IMATH_INTERNAL_NAMESPACE::nrand48 (state) == 0x1836a71f);

    assert (state[0] == 0x2a42);
    assert (state[1] == 0x4e3e);
    assert (state[2] == 0x306d);
}


template <class Rand>
void
testGenerator ()
{
    //
    // Test if the values, and the differences between
    // successive values, are evenly distributed.
    //

    const int N = 10;
    const int M = 100000;

    int values[N + 1];
    int diffs[2 * N + 3];
    int *v = &values[0];
    int *d = &diffs[N + 2];

    for (int i = 0; i <= N; ++i)
	v[i] = 0;

    for (int i = -N; i <= N; ++i)
	d[i] = 0;

    Rand rand (0);
    float previous = 0;

    for (int i = 0; i < M * N; ++i)
    {
	float r = rand.nextf (0.0, 1.0);
	float diff = r - previous;
	previous = r;

	v[int (r * N)] += 1;
	d[IMATH_INTERNAL_NAMESPACE::floor (diff * N + 0.5)] += 1;
    }

    cout << "  values" << endl;

    for (int i = 0; i < N; ++i)
    {
	// cout << setw (4) << i << ' ' << setw(6) << v[i] << ' ';
	assert (abs (v[i] - M) < 0.01 * M);

	// for (int j = 0; j < v[i] * 60 / M; ++j)
	//      cout << '*';

	// cout << endl;
    }

    assert (v[N] == 0);

    cout << "  differences between successive values" << endl;

    for (int i = -N; i <= N; ++i)
    {
	// cout << setw (4) << i << ' ' << setw (6) << d[i] << ' ';
	assert (abs ((N - abs (i)) * M / N - d[i]) < 0.05 * M);

	// for (int j = 0; j < d[i] * 60 / M; ++j)
	//     cout << '*';

	// cout << endl;
    }

    cout << "  range" << endl;

    double rMin = 1.0;
    double rMax = 0.0;

    for (int i = 0; i <= 10000000; ++i)
    {
	double r = rand.nextf (0.0, 1.0);

	if (rMin > r)
	    rMin = r;

	if (rMax < r)
	    rMax = r;
    }

    assert (rMin < 0.0001 && rMax > 0.9999);

    const double pow_2_60 = double (1073741824) * double (1073741824);

    for (int i = 0; i <= 10000000; ++i)
    {
	double r0 = rand.nextf (-2.0, 3.0);
	assert (r0 >= -2.0 && r0 <= 3.0);

	double r1 = rand.nextf (-pow_2_60, 1);
	assert (r1 >= -pow_2_60 && r1 <= 1);

	double r2 = rand.nextf (-1, pow_2_60);
	assert (r2 >= -1 && r2 <= pow_2_60);
    }
}


template <class Rand>
void
testSolidSphere ()
{
    const int N = 10;
    const int M = 10000;
    int v[N + 1];

    for (int i = 0; i <= N; ++i)
	v[i] = 0;

    Rand rand (0);

    for (int i = 0; i < M * N; ++i)
    {
	IMATH_INTERNAL_NAMESPACE::V3f p = IMATH_INTERNAL_NAMESPACE::solidSphereRand<IMATH_INTERNAL_NAMESPACE::V3f> (rand);
	float l = p.length();
	v[IMATH_INTERNAL_NAMESPACE::floor (l * N)] += 1;

	assert (l < 1.00001);
    }

    for (int i = 0; i < N; ++i)
	assert (v[i] > 0);
}


template <class Rand>
void
testHollowSphere ()
{
    const int M = 100000;
    Rand rand (0);

    for (int i = 0; i < M; ++i)
    {
	IMATH_INTERNAL_NAMESPACE::V3f p = IMATH_INTERNAL_NAMESPACE::hollowSphereRand<IMATH_INTERNAL_NAMESPACE::V3f> (rand);
	float l = p.length();

	assert (abs (l - 1) < 0.00001);
    }
}


} // namespace


void
testRandom ()
{
    cout << "Testing random number generators" << endl;

    cout << "erand48(), nrand48()" << endl;
    testErand48();

    cout << "Rand32" << endl;
    testGenerator<IMATH_INTERNAL_NAMESPACE::Rand32>();

    cout << "Rand48" << endl;
    testGenerator<IMATH_INTERNAL_NAMESPACE::Rand48>();

    cout << "solidSphereRand()" << endl;
    testSolidSphere<IMATH_INTERNAL_NAMESPACE::Rand32>();

    cout << "hollowSphereRand()" << endl;
    testHollowSphere<IMATH_INTERNAL_NAMESPACE::Rand32>();

    cout << "ok\n" << endl;
}
