//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testBitPatterns.h>
#include "half.h"
#include <float.h>
#include <iostream>
#include <string.h>
#include <assert.h>


using namespace std;

namespace {

bool
equalBitPatterns (const char *b1, const char *b2)
{
    //
    // Returns true if the characters in zero-terminated string b1
    // are the same as the charaters in string b2, except for places
    // where b1 or b2 contains an 'X'.  For example:
    //
    //	equalBitPatterns ("100", "100") returns true
    //	equalBitPatterns ("100", "101") returns false
    //	equalBitPatterns ("10X", "101") returns true
    //	equalBitPatterns ("10X", "100") returns true
    //

    while (*b1 && *b2)
    {
	if (*b1 != *b2 && *b1 != 'X' && *b2 != 'X')
	    return false;

	++b1;
	++b2;
    }

    return !(*b1 || *b2);
}


void
testBits (float f, const char bh[19], const char bg[35])
{
    half  h (f);
    float g (h);

    cout.width (15);
    cout.precision (8);
    cout << f << "    ";
    printBits (cout, f);
    cout << "    ";
    printBits (cout, h);
    cout << '\n';
    cout.width (15);
    cout << g << "    ";
    printBits (cout, g);
    cout << "\n\n";

    if (bh || bg)
    {
	char ch[19], cg[35];

	printBits (ch, h);
	printBits (cg, g);

	if (!equalBitPatterns (ch, bh))
	{
	    cout << "error: expected " << bh << ", got " << ch << endl;
	    assert (false);
	}

	if (!equalBitPatterns (cg, bg))
	{
	    cout << "error: expected " << bg << ", got " << cg << endl;
	    assert (false);
	}
    }
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
testBitPatterns()
{
    cout << "specific bit patterns\n\n";

    //
    // Numbers close to 1.0
    //

    testBits (1.0f,
	      "0 01111 0000000000",
	      "0 01111111 00000000000000000000000");
    testBits (1.0f + HALF_EPSILON,
	      "0 01111 0000000001",
	      "0 01111111 00000000010000000000000");
    testBits (1.0f + HALF_EPSILON * 0.5f,
	      "0 01111 0000000000",
	      "0 01111111 00000000000000000000000");
    testBits (1.0f+ HALF_EPSILON * 0.4999f,
	      "0 01111 0000000000",
	      "0 01111111 00000000000000000000000");
    testBits (1.0f + HALF_EPSILON * 0.5001f,
	      "0 01111 0000000001",
	      "0 01111111 00000000010000000000000");
    testBits (1.0f + HALF_EPSILON + HALF_EPSILON,
	      "0 01111 0000000010",
	      "0 01111111 00000000100000000000000");
    testBits (1.0f + HALF_EPSILON + HALF_EPSILON * 0.5f,
	      "0 01111 0000000010",
	      "0 01111111 00000000100000000000000");
    testBits (1.0f + HALF_EPSILON + HALF_EPSILON * 0.4999f,
	      "0 01111 0000000001",
	      "0 01111111 00000000010000000000000");
    testBits (1.0f + HALF_EPSILON + HALF_EPSILON * 0.5001f,
	      "0 01111 0000000010",
	      "0 01111111 00000000100000000000000");
    testBits (1.0f - HALF_EPSILON * 0.5f,
	      "0 01110 1111111111",
	      "0 01111110 11111111110000000000000");
    testBits (1.0f - HALF_EPSILON * 0.5f * 0.5f,
	      "0 01111 0000000000",
	      "0 01111111 00000000000000000000000");
    testBits (1.0f - HALF_EPSILON * 0.5f * 0.4999f,
	      "0 01111 0000000000",
	      "0 01111111 00000000000000000000000");
    testBits (1.0f - HALF_EPSILON * 0.5f * 0.5001f,
	      "0 01110 1111111111",
	      "0 01111110 11111111110000000000000");

    //
    // Numbers close to HALF_MIN
    //

    testBits (HALF_MIN,
	      "0 00000 0000000001",
	      "0 01100111 00000000000000000000000");
    testBits (HALF_MIN + HALF_MIN,
	      "0 00000 0000000010",
	      "0 01101000 00000000000000000000000");
    testBits (HALF_MIN + HALF_MIN * 0.5f,
	      "0 00000 0000000010",
	      "0 01101000 00000000000000000000000");
    testBits (HALF_MIN + HALF_MIN * 0.4999f,
	      "0 00000 0000000001",
	      "0 01100111 00000000000000000000000");
    testBits (HALF_MIN + HALF_MIN * 0.5001f,
	      "0 00000 0000000010",
	      "0 01101000 00000000000000000000000");
    testBits (HALF_MIN - HALF_MIN, // NOSONAR - suppress SonarCloud bug report.
	      "0 00000 0000000000",
	      "0 00000000 00000000000000000000000");
    testBits (HALF_MIN - HALF_MIN * 0.5f,
	      "0 00000 0000000000",
	      "0 00000000 00000000000000000000000");
    testBits (HALF_MIN - HALF_MIN * 0.4999f,
	      "0 00000 0000000001",
	      "0 01100111 00000000000000000000000");
    testBits (HALF_MIN - HALF_MIN * 0.5001f,
	      "0 00000 0000000000",
	      "0 00000000 00000000000000000000000");

    //
    // Numbers close to HALF_NRM_MIN
    //

    testBits (HALF_NRM_MIN,
	      "0 00001 0000000000",
	      "0 01110001 00000000000000000000000");
    testBits (HALF_NRM_MIN + HALF_MIN,
	      "0 00001 0000000001",
	      "0 01110001 00000000010000000000000");
    testBits (HALF_NRM_MIN + HALF_MIN * 0.5f,
	      "0 00001 0000000000",
	      "0 01110001 00000000000000000000000");
    testBits (HALF_NRM_MIN + HALF_MIN * 0.4999f,
	      "0 00001 0000000000",
	      "0 01110001 00000000000000000000000");
    testBits (HALF_NRM_MIN + HALF_MIN * 0.5001f,
	      "0 00001 0000000001",
	      "0 01110001 00000000010000000000000");
    testBits (HALF_NRM_MIN - HALF_MIN,
	      "0 00000 1111111111",
	      "0 01110000 11111111100000000000000");
    testBits (HALF_NRM_MIN - HALF_MIN * 0.5f,
	      "0 00001 0000000000",
	      "0 01110001 00000000000000000000000");
    testBits (HALF_NRM_MIN - HALF_MIN * 0.49995f,
	      "0 00001 0000000000",
	      "0 01110001 00000000000000000000000");
    testBits (HALF_NRM_MIN - HALF_MIN * 0.50005f,
	      "0 00000 1111111111",
	      "0 01110000 11111111100000000000000");

    //
    // Small positive integers and simple decimal fractions
    //

    testBits (2,
	      "0 10000 0000000000",
	      "0 10000000 00000000000000000000000");
    testBits (3,
	      "0 10000 1000000000",
	      "0 10000000 10000000000000000000000");
    testBits (10,
	      "0 10010 0100000000",
	      "0 10000010 01000000000000000000000");
    testBits (0.1f,
	      "0 01011 1001100110",
	      "0 01111011 10011001100000000000000");
    testBits (0.2f,
	      "0 01100 1001100110",
	      "0 01111100 10011001100000000000000");
    testBits (0.3f,
	      "0 01101 0011001101",
	      "0 01111101 00110011010000000000000");

    //
    // Numbers close to HALF_MAX
    //

    testBits (HALF_MAX,
	      "0 11110 1111111111",
	      "0 10001110 11111111110000000000000");
    testBits ((1 << HALF_MAX_EXP) * 1.0,
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity
    testBits ((1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.25f),
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity
    testBits ((1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.25005f),
	      "0 11110 1111111111",
	      "0 10001110 11111111110000000000000");
    testBits ((1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.24995f),
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity

    //
    // Large positive numbers, positive infinity and NANs
    //

    testBits (HALF_MAX * HALF_MAX,
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity
    testBits (FLT_MAX,
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity
    testBits (floatPosInfinity(),
	      "0 11111 0000000000",			// +infinity
	      "0 11111111 00000000000000000000000");	// +infinity
    testBits (floatPosQNan1(),
	      "0 11111 1111111111",			// nan
	      "0 11111111 11111111110000000000000");	// nan
    testBits (floatPosQNan2(),
	      "0 11111 1010101010",			// nan
	      "0 11111111 10101010100000000000000");	// nan

    //
    // Numbers close to -1.0
    //

    testBits (-1.0,
	      "1 01111 0000000000",
	      "1 01111111 00000000000000000000000");
    testBits (-(1.0f + HALF_EPSILON),
	      "1 01111 0000000001",
	      "1 01111111 00000000010000000000000");
    testBits (-(1.0f + HALF_EPSILON * 0.5f),
	      "1 01111 0000000000",
	      "1 01111111 00000000000000000000000");
    testBits (-(1.0f + HALF_EPSILON * 0.4999f),
	      "1 01111 0000000000",
	      "1 01111111 00000000000000000000000");
    testBits (-(1.0f + HALF_EPSILON * 0.5001f),
	      "1 01111 0000000001",
	      "1 01111111 00000000010000000000000");
    testBits (-(1.0f + HALF_EPSILON + HALF_EPSILON),
	      "1 01111 0000000010",
	      "1 01111111 00000000100000000000000");
    testBits (-(1.0f + HALF_EPSILON + HALF_EPSILON * 0.5f),
	      "1 01111 0000000010",
	      "1 01111111 00000000100000000000000");
    testBits (-(1.0f + HALF_EPSILON + HALF_EPSILON * 0.4999f),
	      "1 01111 0000000001",
	      "1 01111111 00000000010000000000000");
    testBits (-(1.0f + HALF_EPSILON + HALF_EPSILON * 0.5001f),
	      "1 01111 0000000010",
	      "1 01111111 00000000100000000000000");
    testBits (-(1.0f - HALF_EPSILON * 0.5f),
	      "1 01110 1111111111",
	      "1 01111110 11111111110000000000000");
    testBits (-(1.0f - HALF_EPSILON * 0.5f * 0.5f),
	      "1 01111 0000000000",
	      "1 01111111 00000000000000000000000");
    testBits (-(1.0f - HALF_EPSILON * 0.5f * 0.4999f),
	      "1 01111 0000000000",
	      "1 01111111 00000000000000000000000");
    testBits (-(1.0f - HALF_EPSILON * 0.5f * 0.5001f),
	      "1 01110 1111111111",
	      "1 01111110 11111111110000000000000");

    //
    // Numbers close to -HALF_MIN
    //

    testBits (-HALF_MIN,
	      "1 00000 0000000001",
	      "1 01100111 00000000000000000000000");
    testBits (-(HALF_MIN + HALF_MIN),
	      "1 00000 0000000010",
	      "1 01101000 00000000000000000000000");
    testBits (-(HALF_MIN + HALF_MIN * 0.5f),
	      "1 00000 0000000010",
	      "1 01101000 00000000000000000000000");
    testBits (-(HALF_MIN + HALF_MIN * 0.4999f),
	      "1 00000 0000000001",
	      "1 01100111 00000000000000000000000");
    testBits (-(HALF_MIN + HALF_MIN * 0.5001f),
	      "1 00000 0000000010",
	      "1 01101000 00000000000000000000000");
    testBits (-(HALF_MIN - HALF_MIN), // NOSONAR - suppress SonarCloud bug report.
	      "X 00000 0000000000",
	      "X 00000000 00000000000000000000000");
    testBits (-(HALF_MIN - HALF_MIN * 0.5f),
	      "1 00000 0000000000",
	      "1 00000000 00000000000000000000000");
    testBits (-(HALF_MIN - HALF_MIN * 0.4999f),
	      "1 00000 0000000001",
	      "1 01100111 00000000000000000000000");
    testBits (-(HALF_MIN - HALF_MIN * 0.5001f),
	      "1 00000 0000000000",
	      "1 00000000 00000000000000000000000");

    //
    // Numbers close to -HALF_NRM_MIN
    //

    testBits (-HALF_NRM_MIN,
	      "1 00001 0000000000",
	      "1 01110001 00000000000000000000000");
    testBits (-(HALF_NRM_MIN + HALF_MIN),
	      "1 00001 0000000001",
	      "1 01110001 00000000010000000000000");
    testBits (-(HALF_NRM_MIN + HALF_MIN * 0.5f),
	      "1 00001 0000000000",
	      "1 01110001 00000000000000000000000");
    testBits (-(HALF_NRM_MIN + HALF_MIN * 0.4999f),
	      "1 00001 0000000000",
	      "1 01110001 00000000000000000000000");
    testBits (-(HALF_NRM_MIN + HALF_MIN * 0.5001f),
	      "1 00001 0000000001",
	      "1 01110001 00000000010000000000000");
    testBits (-(HALF_NRM_MIN - HALF_MIN),
	      "1 00000 1111111111",
	      "1 01110000 11111111100000000000000");
    testBits (-(HALF_NRM_MIN - HALF_MIN * 0.5f),
	      "1 00001 0000000000",
	      "1 01110001 00000000000000000000000");
    testBits (-(HALF_NRM_MIN - HALF_MIN * 0.49995f),
	      "1 00001 0000000000",
	      "1 01110001 00000000000000000000000");
    testBits (-(HALF_NRM_MIN - HALF_MIN * 0.50005f),
	      "1 00000 1111111111",
	      "1 01110000 11111111100000000000000");

    //
    // Small negative integers and simple decimal fractions
    //

    testBits (-2,
	      "1 10000 0000000000",
	      "1 10000000 00000000000000000000000");
    testBits (-3,
	      "1 10000 1000000000",
	      "1 10000000 10000000000000000000000");
    testBits (-10,
	      "1 10010 0100000000",
	      "1 10000010 01000000000000000000000");
    testBits (-0.1f,
	      "1 01011 1001100110",
	      "1 01111011 10011001100000000000000");
    testBits (-0.2f,
	      "1 01100 1001100110",
	      "1 01111100 10011001100000000000000");
    testBits (-0.3f,
	      "1 01101 0011001101",
	      "1 01111101 00110011010000000000000");

    //
    // Numbers close to -HALF_MAX
    //

    testBits (-HALF_MAX,
	      "1 11110 1111111111",
	      "1 10001110 11111111110000000000000");
    testBits (-(1 << HALF_MAX_EXP) * 1.0f,
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity
    testBits (-(1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.25f),
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity
    testBits (-(1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.25005f),
	      "1 11110 1111111111",
	      "1 10001110 11111111110000000000000");
    testBits (-(1 << HALF_MAX_EXP) * (1.0f - HALF_EPSILON * 0.24995f),
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity

    //
    // Large negative numbers, negative infinity and NANs
    //

    testBits (-HALF_MAX * HALF_MAX,
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity
    testBits (-FLT_MAX,
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity
    testBits (floatNegInfinity(),
	      "1 11111 0000000000",			// +infinity
	      "1 11111111 00000000000000000000000");	// +infinity
    testBits (floatNegQNan1(),
	      "1 11111 1111111111",			// nan
	      "1 11111111 11111111110000000000000");	// nan
    testBits (floatNegQNan2(),
	      "1 11111 1010101010",			// nan
	      "1 11111111 10101010100000000000000");	// nan

    cout << "ok\n\n" << flush;
}
