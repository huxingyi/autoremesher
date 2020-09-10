//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testError.h>
#include "half.h"
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <random>

using namespace std;

namespace {

float
drand()
{
    static std::default_random_engine generator;
    static std::uniform_real_distribution<float> distribution (0.0f, 1.0f);
    float r = distribution (generator);
    return r;
}

} // namespace


void
testNormalizedConversionError()
{
    cout << "float-to-half conversion error for normalized half numbers\n";

    float eMax = 0;

    for (int i = 0; i < 20000000; i++)
    {
	float f (drand() * HALF_MAX);

	if (f < HALF_NRM_MIN)
	    continue;

	if (i & 1)
	    f = -f;

	half h (f);
	float e = 1.0f - h/f;

	if (e < 0)
	    e = -e;

	if (e > HALF_EPSILON * 0.5)
	{
	    cout << "float = " << f <<
		    ", half = " << h <<
		    ", error = " << e << endl;

	    assert (false);
	}

	if (e > eMax)
	    eMax = e;
    }

    cout << "max error          = " << eMax << endl;
    cout << "max expected error = " << HALF_EPSILON * 0.5 << endl;
    cout << "ok\n\n" << flush;
}


void
testDenormalizedConversionError()
{
    cout << "float-to-half conversion error for denormalized half numbers\n";

    float eMax = 0;

    for (int i = 0; i < 20000000; i++)
    {
	float f (drand() * (HALF_NRM_MIN - HALF_MIN));

	if (i & 1)
	    f = -f;

	half h (f);
	float e = h - f;

	if (e < 0)
	    e = -e;

	if (e > HALF_MIN * 0.5)
	{
	    cout << "float = " << f <<
		    ", half = " << h <<
		    ", error = " << e << endl;

	    assert (false);
	}

	if (e > eMax)
	    eMax = e;
    }

    cout << "max error          = " << eMax << endl;
    cout << "max expected error = " << HALF_MIN * 0.5 << endl;
    cout << "ok\n\n" << flush;
}


namespace {

void
testNormalizedRounding (int n)
{
    cout << "rounding normalized numbers to " << n << "-bit precision\n";

    float eExpected = (n < 10)? HALF_EPSILON * 0.5f * (1 << (10 - n)): 0;
    float eMax = 0;

    for (int i = 0; i < 200000; i++)
    {
	half h (drand() * HALF_MAX);

	if (h < HALF_NRM_MIN)
	    continue;

	if (i & 1)
	    h = -h;

	half r (h.round(n));
	float e = 1.0f - r/h;

	if (e < 0)
	    e = -e;

	if (e > eExpected)
	{
	    cout << "half = " << h <<
		    ", rounded = " << r <<
		    ", error = " << e <<
		    ", expected error = " << eExpected << endl;

	    printBits (cout, h);
	    cout << endl;
	    printBits (cout, r);
	    cout << endl;

	    assert (false);
	}

	if (e > eMax)
	    eMax = e;
    }

    cout << "max error          = " << eMax << endl;
    cout << "max expected error = " << eExpected << endl;
    cout << "ok\n\n" << flush;
}


void
testDenormalizedRounding (int n)
{
    cout << "rounding denormalized numbers to " << n << "-bit precision\n";

    float eExpected = (n < 10)? HALF_MIN * 0.5f * (1 << (10 - n)): 0;
    float eMax = 0;

    for (int i = 0; i < 200000; i++)
    {
	half h (drand() * (HALF_NRM_MIN - HALF_MIN));

	if (i & 1)
	    h = -h;

	half r (h.round(n));
	float e = r - h;

	if (e < 0)
	    e = -e;

	if (e > eExpected)
	{
	    cout << "half = " << h <<
		    ", rounded = " << r <<
		    ", error = " << e <<
		    ", expected error = " << eExpected << endl;

	    printBits (cout, h);
	    cout << endl;
	    printBits (cout, r);
	    cout << endl;

	    assert (false);
	}

	if (e > eMax)
	    eMax = e;
    }

    cout << "max error          = " << eMax << endl;
    cout << "max expected error = " << eExpected << endl;
    cout << "ok\n\n" << flush;
}

} // namespace


void
testRoundingError ()
{
    testNormalizedRounding (10);
    testDenormalizedRounding (10);
    testNormalizedRounding (9);
    testDenormalizedRounding (9);
    testNormalizedRounding (1);
    testDenormalizedRounding (1);
    testNormalizedRounding (0);
    testDenormalizedRounding (0);
}
