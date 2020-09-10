//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testClassification.h>
#include "half.h"
#include <iostream>
#include <assert.h>
#include <type_traits>

using namespace std;

namespace {

#if __cplusplus >= 201402L
static_assert(std::is_trivially_default_constructible<half>::value, "half is trivial and default constructible");
#endif

void
testClass (half h,
	   bool finite,
	   bool normalized,
	   bool denormalized,
	   bool zero,
	   bool nan,
	   bool infinity,
	   bool negative)
{
    cout.width (15);
    cout.precision (8);

    cout << h << "    ";
    printBits (cout, h);
    cout << "    ";

    if (h.isFinite())
	cout << "finite ";

    if (h.isNormalized())
	cout << "normalized ";

    if (h.isDenormalized())
	cout << "denormalized ";

    if (h.isZero())
	cout << "zero ";

    if (h.isNan())
	cout << "nan ";

    if (h.isInfinity())
	cout << "infinity ";

    if (h.isNegative())
	cout << "negative ";

    cout << endl;

    assert (h.isFinite() == finite);
    assert (h.isNormalized() == normalized);
    assert (h.isDenormalized() == denormalized);
    assert (h.isZero() == zero);
    assert (h.isNan() == nan);
    assert (h.isInfinity() == infinity);
    assert (h.isNegative() == negative);
}


float
floatPosInfinity ()
{
    half::uif x;
    x.i = 0x7f800000;
    return x.f;
}


float
floatNegInfinity ()
{
    half::uif x;
    x.i = 0xff800000;
    return x.f;
}


float
floatPosQNan1 ()
{
    half::uif x;
    x.i = 0x7fffffff;
    return x.f;
}


float
floatNegQNan1 ()
{
    half::uif x;
    x.i = 0xffffffff;
    return x.f;
}


float
floatPosQNan2 ()
{
    half::uif x;
    x.i = 0x7fd55555;
    return x.f;
}


float
floatNegQNan2 ()
{
    half::uif x;
    x.i = 0xffd55555;
    return x.f;
}

} // namespace


void
testClassification()
{
    cout << "classification of bit patterns\n\n";

    //
    //					fini norm deno zero nan  inf  neg
    //

    testClass (0.0,			1,   0,   0,   1,   0,   0,   0);

    testClass (1.0,			1,   1,   0,   0,   0,   0,   0);
    testClass (1.0f + HALF_EPSILON,	1,   1,   0,   0,   0,   0,   0);
    testClass (HALF_MIN,		1,   0,   1,   0,   0,   0,   0);
    testClass (HALF_MIN + HALF_MIN,	1,   0,   1,   0,   0,   0,   0);
    testClass (HALF_NRM_MIN,		1,   1,   0,   0,   0,   0,   0);
    testClass (HALF_NRM_MIN + HALF_MIN,	1,   1,   0,   0,   0,   0,   0);
    testClass (HALF_NRM_MIN - HALF_MIN,	1,   0,   1,   0,   0,   0,   0);
    testClass (2.0f,			1,   1,   0,   0,   0,   0,   0);
    testClass (3.0f,			1,   1,   0,   0,   0,   0,   0);
    testClass (0.1f,			1,   1,   0,   0,   0,   0,   0);
    testClass (0.2f,			1,   1,   0,   0,   0,   0,   0);
    testClass (0.3f,			1,   1,   0,   0,   0,   0,   0);
    testClass (HALF_MAX,		1,   1,   0,   0,   0,   0,   0);
    testClass (floatPosInfinity(),	0,   0,   0,   0,   0,   1,   0);
    testClass (floatPosQNan1(),		0,   0,   0,   0,   1,   0,   0);
    testClass (floatPosQNan2(),		0,   0,   0,   0,   1,   0,   0);

    testClass (-1.0f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-1.0f - HALF_EPSILON,	1,   1,   0,   0,   0,   0,   1);
    testClass (-HALF_MIN,		1,   0,   1,   0,   0,   0,   1);
    testClass (-HALF_MIN - HALF_MIN,	1,   0,   1,   0,   0,   0,   1);
    testClass (-HALF_NRM_MIN,		1,   1,   0,   0,   0,   0,   1);
    testClass (-HALF_NRM_MIN - HALF_MIN,1,   1,   0,   0,   0,   0,   1);
    testClass (-HALF_NRM_MIN + HALF_MIN,1,   0,   1,   0,   0,   0,   1);
    testClass (-2.0f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-3.0f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-0.1f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-0.2f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-0.3f,			1,   1,   0,   0,   0,   0,   1);
    testClass (-HALF_MAX,		1,   1,   0,   0,   0,   0,   1);
    testClass (floatNegInfinity(),	0,   0,   0,   0,   0,   1,   1);
    testClass (floatNegQNan1(),		0,   0,   0,   0,   1,   0,   1);
    testClass (floatNegQNan2(),		0,   0,   0,   0,   1,   0,   1);

    cout << "\n";

    testClass (half::posInf(),		0,   0,   0,   0,   0,   1,   0);
    testClass (half::negInf(),		0,   0,   0,   0,   0,   1,   1);
    testClass (half::qNan(),		0,   0,   0,   0,   1,   0,   0);
    testClass (half::sNan(),		0,   0,   0,   0,   1,   0,   0);

    cout << "ok\n\n" << flush;
}
