//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testFunction.h>
#include "halfFunction.h"
#include <iostream>
#include <assert.h>


using namespace std;

namespace {


float
divideByTwo (float x)
{
    return x / 2;
}


struct timesN
{
    timesN (float n): n (n) {}
    float operator () (float x) {return x * n;}
    float n;
};


} // namespace


void
testFunction ()
{
    cout << "halfFunction<T>\n";

    halfFunction <float> d2 (divideByTwo);

    assert (d2 (0)  ==  0);
    assert (d2 (2)  ==  1);
    assert (d2 (-2) == -1);
    assert (d2 (HALF_MAX)  ==  HALF_MAX / 2);
    assert (d2 (-HALF_MAX) == -HALF_MAX / 2);
    assert (d2 (half::posInf()) == 0);
    assert (d2 (half::negInf()) == 0);
    assert (d2 (half::qNan())   == 0);

    halfFunction <half> t5 (timesN (5),		// function
			    0, HALF_MAX / 8,	// domain
			    -1,			// default value
			    half::posInf(),	// posInfValue
			    half::negInf(),	// negInfValue
			    half::qNan());	// nanValue

    assert (t5 (0)  ==  0);
    assert (t5 (2)  == 10);
    assert (t5 (-2) == -1);
    assert (t5 (HALF_MAX)  == -1);
    assert (t5 (-HALF_MAX) == -1);

    assert ( t5(half::posInf()).isInfinity());
    assert (!t5(half::posInf()).isNegative());

    assert (t5(half::negInf()).isInfinity());
    assert (t5(half::negInf()).isNegative());

    assert (t5(half::qNan()).isNan());

    cout << "ok\n\n" << flush;
}
