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

#include <testInvert.h>
#include "ImathMatrix.h"
#include "ImathMatrixAlgo.h"
#include <iostream>
#include <assert.h>

using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {


void
invertM44f (const M44f &m, float e)
{
    M44f inv1 = m.inverse();
    M44f inv2 = m.gjInverse();
    M44f ident1 = m * inv1;
    M44f ident2 = m * inv2;

    //cout << "m\n" << m << endl;
    //cout << "inv1\n" << inv1 << "ident1\n" << ident1 << endl;
    //cout << "inv2\n" << inv2 << "ident2\n" << ident2 << endl;

    assert (ident1.equalWithAbsError (identity44f, e));
    assert (ident2.equalWithAbsError (identity44f, e));
}


void
invertM33f (const M33f &m, float e)
{
    M33f inv1 = m.inverse();
    M33f inv2 = m.gjInverse();
    M33f ident1 = m * inv1;
    M33f ident2 = m * inv2;

    //cout << "m\n" << m << endl;
    //cout << "inv1\n" << inv1 << "ident1\n" << ident1 << endl;
    //cout << "inv2\n" << inv2 << "ident2\n" << ident2 << endl;

    assert (ident1.equalWithAbsError (identity33f, e));
    assert (ident2.equalWithAbsError (identity33f, e));
}


} // namespace


void
testInvert ()
{
    cout << "Testing 4x4 and 3x3 matrix inversion:" << endl;

    {
	cout << "M44f" << endl;

	M44f m1 ( 1,  0,  0,  0,
		  0,  1,  0,  0,
		  0,  0,  1,  0,
		  0,  0,  0,  1);

	M44f m2 ( 0,  1,  0,  0,
		 -1,  0,  0,  0,
		  0,  0,  1,  0,
		  0,  0,  0,  1);

	M44f m3 ( 1,  0,  0,  0,
		  0,  2,  0,  0,
		  0,  0,  0, -1,
		  0,  0,  1,  0);

	M44f m4 ( 4.683281e-01, -8.749647e-01,  1.229049e-01,  0.000000e+00,
		  1.251189e-02,  1.456563e-01,  9.892561e-01,  0.000000e+00,
		 -8.834660e-01, -4.617587e-01,  7.916244e-02,  0.000000e+00,
		 -4.726541e+00,  3.044795e+00, -6.737138e+00,  1.000000e+00);

	M44f m5 ( 4.683281e-01, -8.749647e-01,  1.229049e-01,  1.000000e+00,
		  1.251189e-02,  1.456563e-01,  9.892561e-01,  2.000000e+00,
		 -8.834660e-01, -4.617587e-01,  7.916244e-02,  3.000000e+00,
		 -4.726541e+00,  3.044795e+00, -6.737138e+00,  4.000000e+00);

	invertM44f (m1, 0);
	invertM44f (m2, 0);
	invertM44f (m3, 0);
	invertM44f (m4, 1e-6);
	invertM44f (m5, 1e-6);
    }

    {
	cout << "M33f" << endl;

	M33f m1 ( 1,  0,  0,
		  0,  1,  0,
		  0,  0,  1);

	M33f m2 ( 0,  1,  0,
		 -1,  0,  0,
		  0,  0,  1);

	M33f m3 ( 2,  0,  0,
		  0,  0, -1,
		  0,  1,  0);

	M33f m4 ( 4.683281e-01, -8.749647e-01,  0.000000e+00,
		  1.251189e-02,  1.456563e-01,  0.000000e+00,
		  0.000000e+00,  0.000000e+00,  1.000000e+00);

	M33f m5 ( 4.683281e-01, -8.749647e-01,  1.229049e-01,
		  1.251189e-02,  1.456563e-01,  9.892561e-01,
		 -8.834660e-01, -4.617587e-01,  7.916244e-02);

	invertM33f (m1, 0);
	invertM33f (m2, 0);
	invertM33f (m3, 0);
	invertM33f (m4, 1e-6);
	invertM33f (m5, 1e-6);
    }

    cout << "ok\n" << endl;
}
