//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testLimits.h>
#include "halfLimits.h"
#include <iostream>
#include <assert.h>
#include <cmath>

using namespace std;

namespace {

float
mypow (int x, int y)
{
    bool negative = false;

    if (y < 0)
    {
	negative = true;
	y = -y;
    }

    float z = 1;

    while (y > 0)
    {
	z *= x;
	y -= 1;
    }

    if (negative)
	z = 1 / z;

    return z;
}

} // namespace


void
testLimits()
{
    cout << "values in std::numeric_limits<half>\n";

    cout << "min_exponent\n";

    {
	half h (mypow (2, numeric_limits<half>::min_exponent - 1));
	assert (h.isNormalized());
    }

    {
	half h (mypow (2, numeric_limits<half>::min_exponent - 2));
	assert (h.isDenormalized());
    }

    cout << "max_exponent\n";

    {
	half h (mypow (2, numeric_limits<half>::max_exponent - 1));
	assert (h.isNormalized());
    }

    {
	half h (mypow (2, numeric_limits<half>::max_exponent));
	assert (h.isInfinity());
    }

    cout << "min_exponent10\n";

    {
	half h (mypow (10, numeric_limits<half>::min_exponent10));
	assert (h.isNormalized());
    }

    {
	half h (mypow (10, numeric_limits<half>::min_exponent10 - 1));
	assert (h.isDenormalized());
    }

    cout << "max_exponent10\n";

    {
	half h (mypow (10, numeric_limits<half>::max_exponent10));
	assert (h.isNormalized());
    }

    {
	half h (mypow (10, numeric_limits<half>::max_exponent10 + 1));
	assert (h.isInfinity());
    }

#if __cplusplus >= 201103L

    cout << "max_digits10\n";
    assert (numeric_limits<half>::max_digits10
            == std::ceil(numeric_limits<half>::digits * std::log10(2) + 1));

    cout << "lowest\n";
    assert (numeric_limits<half>::lowest() == -HALF_MAX);

#endif

    cout << "ok\n\n" << flush;

}
