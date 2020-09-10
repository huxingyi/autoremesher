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

#include <testFun.h>
#include "ImathFun.h"
#include <iostream>
#include <assert.h>
#include <stdio.h>


using namespace std;

#if ULONG_MAX == 18446744073709551615LU
    typedef      long unsigned int Int64;
#else
    typedef long long unsigned int Int64;
#endif


void
testf (float f)
{
    printf ("\n");

    float sf = IMATH_INTERNAL_NAMESPACE::succf (f);
    float pf = IMATH_INTERNAL_NAMESPACE::predf (f);
    float spf = IMATH_INTERNAL_NAMESPACE::succf (IMATH_INTERNAL_NAMESPACE::predf (f));
    float psf = IMATH_INTERNAL_NAMESPACE::predf (IMATH_INTERNAL_NAMESPACE::succf (f));

    printf ("f %.9g\n", f);
    printf ("sf %.9g\n", sf);
    printf ("pf %.9g\n", pf);
    printf ("spf %.9g\n", spf);
    printf ("psf %.9g\n", psf);
}


void
testd (double d)
{
    printf ("\n");

    double sd = IMATH_INTERNAL_NAMESPACE::succd (d);
    double pd = IMATH_INTERNAL_NAMESPACE::predd (d);
    double spd = IMATH_INTERNAL_NAMESPACE::succd (IMATH_INTERNAL_NAMESPACE::predd (d));
    double psd = IMATH_INTERNAL_NAMESPACE::predd (IMATH_INTERNAL_NAMESPACE::succd (d));

    printf ("d %.18lg\n", d);
    printf ("sd %.18lg\n", sd);
    printf ("pd %.18lg\n", pd);
    printf ("spd %.18lg\n", spd);
    printf ("psd %.18lg\n", psd);
}


void
testFun ()
{
    cout << "Testing functions in ImathFun.h" << endl;

    cout << "floor" << endl;

    assert (IMATH_INTERNAL_NAMESPACE::floor ( 0.0f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::floor ( 0.5f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::floor (-0.5f) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::floor ( 1.0f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::floor (-1.0f) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::floor ( 1.5f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::floor (-1.5f) == -2);

    cout << "ceil" << endl;

    assert (IMATH_INTERNAL_NAMESPACE::ceil ( 0.0f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::ceil ( 0.5f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::ceil (-0.5f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::ceil ( 1.0f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::ceil (-1.0f) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::ceil ( 1.5f) ==  2);
    assert (IMATH_INTERNAL_NAMESPACE::ceil (-1.5f) == -1);

    cout << "trunc" << endl;

    assert (IMATH_INTERNAL_NAMESPACE::trunc ( 0.0f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::trunc ( 0.5f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::trunc (-0.5f) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::trunc ( 1.0f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::trunc (-1.0f) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::trunc ( 1.5f) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::trunc (-1.5f) == -1);


    cout << "divs / mods" << endl;

    assert (IMATH_INTERNAL_NAMESPACE::divs ( 5,  2) ==  2 && IMATH_INTERNAL_NAMESPACE::mods ( 5,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 4,  2) ==  2 && IMATH_INTERNAL_NAMESPACE::mods ( 4,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 3,  2) ==  1 && IMATH_INTERNAL_NAMESPACE::mods ( 3,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 2,  2) ==  1 && IMATH_INTERNAL_NAMESPACE::mods ( 2,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 1,  2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods ( 1,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 0,  2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods ( 0,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-1,  2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods (-1,  2) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-2,  2) == -1 && IMATH_INTERNAL_NAMESPACE::mods (-2,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-3,  2) == -1 && IMATH_INTERNAL_NAMESPACE::mods (-3,  2) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-4,  2) == -2 && IMATH_INTERNAL_NAMESPACE::mods (-4,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-5,  2) == -2 && IMATH_INTERNAL_NAMESPACE::mods (-5,  2) == -1);

    assert (IMATH_INTERNAL_NAMESPACE::divs ( 5, -2) == -2 && IMATH_INTERNAL_NAMESPACE::mods ( 5, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 4, -2) == -2 && IMATH_INTERNAL_NAMESPACE::mods ( 4, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 3, -2) == -1 && IMATH_INTERNAL_NAMESPACE::mods ( 3, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 2, -2) == -1 && IMATH_INTERNAL_NAMESPACE::mods ( 2, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 1, -2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods ( 1, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divs ( 0, -2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods ( 0, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-1, -2) ==  0 && IMATH_INTERNAL_NAMESPACE::mods (-1, -2) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-2, -2) ==  1 && IMATH_INTERNAL_NAMESPACE::mods (-2, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-3, -2) ==  1 && IMATH_INTERNAL_NAMESPACE::mods (-3, -2) == -1);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-4, -2) ==  2 && IMATH_INTERNAL_NAMESPACE::mods (-4, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divs (-5, -2) ==  2 && IMATH_INTERNAL_NAMESPACE::mods (-5, -2) == -1);

    cout << "divp / modp" << endl;

    assert (IMATH_INTERNAL_NAMESPACE::divp ( 5,  2) ==  2 && IMATH_INTERNAL_NAMESPACE::modp ( 5,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 4,  2) ==  2 && IMATH_INTERNAL_NAMESPACE::modp ( 4,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 3,  2) ==  1 && IMATH_INTERNAL_NAMESPACE::modp ( 3,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 2,  2) ==  1 && IMATH_INTERNAL_NAMESPACE::modp ( 2,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 1,  2) ==  0 && IMATH_INTERNAL_NAMESPACE::modp ( 1,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 0,  2) ==  0 && IMATH_INTERNAL_NAMESPACE::modp ( 0,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-1,  2) == -1 && IMATH_INTERNAL_NAMESPACE::modp (-1,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-2,  2) == -1 && IMATH_INTERNAL_NAMESPACE::modp (-2,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-3,  2) == -2 && IMATH_INTERNAL_NAMESPACE::modp (-3,  2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-4,  2) == -2 && IMATH_INTERNAL_NAMESPACE::modp (-4,  2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-5,  2) == -3 && IMATH_INTERNAL_NAMESPACE::modp (-5,  2) ==  1);

    assert (IMATH_INTERNAL_NAMESPACE::divp ( 5, -2) == -2 && IMATH_INTERNAL_NAMESPACE::modp ( 5, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 4, -2) == -2 && IMATH_INTERNAL_NAMESPACE::modp ( 4, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 3, -2) == -1 && IMATH_INTERNAL_NAMESPACE::modp ( 3, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 2, -2) == -1 && IMATH_INTERNAL_NAMESPACE::modp ( 2, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 1, -2) ==  0 && IMATH_INTERNAL_NAMESPACE::modp ( 1, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp ( 0, -2) ==  0 && IMATH_INTERNAL_NAMESPACE::modp ( 0, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-1, -2) ==  1 && IMATH_INTERNAL_NAMESPACE::modp (-1, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-2, -2) ==  1 && IMATH_INTERNAL_NAMESPACE::modp (-2, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-3, -2) ==  2 && IMATH_INTERNAL_NAMESPACE::modp (-3, -2) ==  1);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-4, -2) ==  2 && IMATH_INTERNAL_NAMESPACE::modp (-4, -2) ==  0);
    assert (IMATH_INTERNAL_NAMESPACE::divp (-5, -2) ==  3 && IMATH_INTERNAL_NAMESPACE::modp (-5, -2) ==  1);

    cout << "successor, predecessor" << endl;

    testf (0);
    testf (0.0 * -1.0);
    testf (1);
    testf (-1);
    testf (16);
    testf (7);
    testf (0.7);

    union {float f; int i;} u;
    u.i = 0x7f800000; //  inf
    testf (u.f);
    u.i = 0x7f800001; //  nan
    testf (u.f);
    u.i = 0x7f7fffff; //  FLT_MAX
    testf (u.f);
    u.i = 0xff7fffff; // -FLT_MAX
    testf (u.f);

    testd (0);
    testd (0.0 * -1.0);
    testd (1);
    testd (-1);
    testd (16);
    testd (7);
    testd (0.7);

    union {double d; Int64 i;} v;
    v.i = 0x7ff0000000000000ULL; //  inf
    testd (v.d);
    v.i = 0x7ff0000000000001ULL; //  NAN
    testd (v.d);
    v.i = 0x7fefffffffffffffULL; //  FLT_MAX
    testd (v.d);
    v.i = 0xffefffffffffffffULL; // -FLT_MAX
    testd (v.d);

    cout << "ok\n" << endl;
}
