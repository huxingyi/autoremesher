//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testSize.h>
#include "half.h"
#include <iostream>
#include <assert.h>
#include <stddef.h>


using namespace std;


void
testSize ()
{
    cout << "size and alignment\n";

    half h[2];

    int size = sizeof (half);
    ptrdiff_t algn = (char *)&h[1] - (char *)&h[0];

    cout << "sizeof  (half) = " << size << endl;
    cout << "alignof (half) = " << (int) algn << endl;

    assert (size == 2 && algn == 2);

    cout << "ok\n\n" << flush;
}
