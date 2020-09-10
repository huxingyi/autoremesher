///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2012, Industrial Light & Magic, a division of Lucas
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

#include <ImfLut.h>
#include <ImfArray.h>
#include "ImathRandom.h"
#include <iostream>

#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

half
one (half)
{
    return 1;
}


void
testHalfLut ()
{
    const int NX = 67;
    const int NY = 31;

    Array2D<half> h (NY, NX);
    HalfLut lut (one);

    //
    // apply (data, nData, stride);
    //

    for (int y = 0; y < NY; ++y)
	for (int x = 0; x < NX; ++x)
	    h[y][x] = 0;

    lut.apply (&h[0][0], NX * NY, 1);

    for (int y = 0; y < NY; ++y)
	for (int x = 0; x < NX; ++x)
	    assert (h[y][x] == 1);

    //
    // apply (slice, dataWindow);
    //

    for (int y = 0; y < NY; ++y)
	for (int x = 0; x < NX; ++x)
	    h[y][x] = 0;

    Slice s (HALF,			// type
	     (char *) &h[0][0],		// base
	     sizeof (h[0][0]),		// xStride
	     sizeof (h[0][0]) * NX,	// yStride
	     1, 1); 			// xSampling, ySampling

    Box2i dw (V2i (3, 5), V2i (45, 27));

    lut.apply (s, dw);

    for (int y = 0; y < NY; ++y)
	for (int x = 0; x < NX; ++x)
	    if (dw.intersects (V2i (x, y)))
		assert (h[y][x] == 1);
	    else
		assert (h[y][x] == 0);
}


void
testRgbaLut ()
{
    const int NX = 67;
    const int NY = 31;

    Array2D<Rgba> rgba (NY, NX);
    RgbaLut lut (one, WRITE_RGB);

    //
    // apply (data, nData, stride);
    //

    for (int y = 0; y < NY; ++y)
    {
	for (int x = 0; x < NX; ++x)
	{
	    rgba[y][x].r = 0;
	    rgba[y][x].g = 0;
	    rgba[y][x].b = 0;
	    rgba[y][x].a = 0;
	}
    }

    lut.apply (&rgba[0][0], NX * NY, 1);

    for (int y = 0; y < NY; ++y)
    {
	for (int x = 0; x < NX; ++x)
	{
	    assert (rgba[y][x].r == 1);
	    assert (rgba[y][x].g == 1);
	    assert (rgba[y][x].b == 1);
	    assert (rgba[y][x].a == 0);
	}
    }

    //
    // apply (base, xStride, yStride, dataWindow);
    //

    for (int y = 0; y < NY; ++y)
    {
	for (int x = 0; x < NX; ++x)
	{
	    rgba[y][x].r = 0;
	    rgba[y][x].g = 0;
	    rgba[y][x].b = 0;
	    rgba[y][x].a = 0;
	}
    }

    Box2i dw (V2i (3, 5), V2i (45, 27));

    lut.apply (&rgba[0][0], 1, NX, dw);

    for (int y = 0; y < NY; ++y)
    {
	for (int x = 0; x < NX; ++x)
	{
	    if (dw.intersects (V2i (x, y)))
	    {
		assert (rgba[y][x].r == 1);
		assert (rgba[y][x].g == 1);
		assert (rgba[y][x].b == 1);
		assert (rgba[y][x].a == 0);
	    }
	    else
	    {
		assert (rgba[y][x].r == 0);
		assert (rgba[y][x].g == 0);
		assert (rgba[y][x].b == 0);
		assert (rgba[y][x].a == 0);
	    }
	}
    }
}


void
testRounding ()
{
    //
    // For each rounding function, f,
    // f(f(x)) == f(x) must be true.
    //

    Rand32 rand;

    for (int i = 0; i < 10000; ++i)
    {
	half h = rand.nextf (HALF_MIN, HALF_MAX);
	assert (round12log (h) == round12log (round12log (h)));
    }

    for (int n = 0; n <= 10; ++n)
    {
	roundNBit rn (n);

	for (int i = 0; i < 10000; ++i)
	{
	    half h = rand.nextf (HALF_MIN, HALF_MAX);
	    assert (rn (h) == rn (rn (h)));
	}
    }

    //
    // Special cases:
    //

    assert (round12log (-1) == 0);
    assert (round12log (0) == 0);
    assert (round12log (0.5) == 0.5);
    assert (round12log (1) == 1);
    assert (round12log (2) == 2);

    roundNBit r3 (3);

    assert (r3 (-1) == -1);
    assert (r3 (0) == 0);
    assert (r3 (0.5) == 0.5);
    assert (r3 (1) == 1);
    assert (r3 (2) == 2);
}


} // namespace


void
testLut (const std::string&)
{
    try
    {
	cout << "Testing lookup tables" << endl;

	testHalfLut();
	testRgbaLut();
	testRounding();

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
