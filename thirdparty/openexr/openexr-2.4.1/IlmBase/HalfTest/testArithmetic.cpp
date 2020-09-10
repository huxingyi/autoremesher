//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testArithmetic.h>
#include "half.h"
#include <iostream>
#include <assert.h>

using namespace std;


void
testArithmetic ()
{
    cout << "basic arithmetic operations:\n";

    float f1 (1);
    float f2 (2);
    half  h1 (3);
    half  h2 (4);

    cout << "f1 = " << f1 << ", "
	    "f2 = " << f2 << ", "
	    "h1 = " << h1 << ", "
	    "h2 = " << h2 << endl;

    h1 = f1 + f2;
    assert (h1 == 3);

    cout << "h1 = f1 + f2: " << h1 << endl;

    h2 += f1;
    assert (h2 == 5);

    cout << "h2 += f1: " << h2 << endl;

    h2 = h1 + h2;
    assert (h2 == 8);

    cout << "h2 = h1 + h2: " << h2 << endl;

    h2 += h1;
    assert (h2 == 11);

    cout << "h2 += h1: " << h2 << endl;

    h1 = h2;
    assert (h1 == 11);

    cout << "h1 = h2: " << h1 << endl;

    h2 = -h1;
    assert (h2 == -11);

    cout << "h2 = -h1: " << h2 << endl;

    cout << "ok\n\n" << flush;
}
