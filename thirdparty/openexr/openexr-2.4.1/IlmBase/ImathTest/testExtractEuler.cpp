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

#include <testExtractEuler.h>
#include "ImathMatrixAlgo.h"
#include "ImathEuler.h"
#include "ImathRandom.h"
#include "ImathFun.h"
#include <iostream>
#include <assert.h>


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

float rad (float deg) {return deg * (M_PI / 180);}


M44f
matrixEulerMatrix_1 (const M44f &M, Eulerf::Order order)
{
    V3f f;

    if (order == Eulerf::XYZ)
	extractEulerXYZ (M, f);
    else
	extractEulerZYX (M, f);

    return Eulerf(f, order).toMatrix44();
}


M44f
matrixEulerMatrix_2 (const M44f &M, Eulerf::Order order)
{
    Eulerf f (order);
    f.extract (M);
    return f.toMatrix44();
}


void
testMatrix (const M44f M,
	    M44f (*matrixEulerMatrix)(const M44f &, Eulerf::Order),
	    Eulerf::Order order)
{
    //
    // Extract Euler angles from M, and convert the
    // Euler angles back to a matrix, N.
    //

    M44f N = matrixEulerMatrix (M, order);

    //
    // Verify that the entries in M and N do not
    // differ too much.
    //

    M44f D (M - N);

    for (int j = 0; j < 3; ++j)
    {
	for (int k = 0; k < 3; ++k)
	{
            if (abs (D[j][k]) > 0.000002)
	    {
		cout << "unexpectedly large matrix to "
			"euler angles conversion error: " <<
			D[j][k] << endl;

		cout << j << " " << k << endl;

		cout << "M\n" << M << endl;
		cout << "N\n" << N << endl;
		cout << "D\n" << D << endl;

		assert (false);
	    }
	}
    }
}


void
testRandomAngles (M44f (*matrixEulerMatrix)(const M44f &, Eulerf::Order),
		  Eulerf::Order order)
{
    Rand48 r(0);

    for (int i = 0; i < 100000; ++i)
    {
	//
	// Create a rotation matrix, M
	//

	Eulerf e (rad (r.nextf (-180, 180)),
		  rad (r.nextf (-180, 180)),
		  rad (r.nextf (-180, 180)),
		  Eulerf::XYZ);

	M44f M (e.toMatrix44());

	//
	// Add a small random error to the elements of M
	//

	for (int j = 0; j < 3; ++j)
	    for (int k = 0; k < 3; ++k)
		M[j][k] += r.nextf (-1e-7, 1e-7);

	//
	// Extract Euler angles from M, convert the Euler angles
	// back to a matrix, N, and verify that the entries in M
	// and N do not differ too much.
	//

	testMatrix (M, matrixEulerMatrix, order);
    }
}


void
testAngles (V3f angles,
	    M44f (*matrixEulerMatrix)(const M44f &, Eulerf::Order),
	    Eulerf::Order order)
{
    Eulerf e (rad (angles.x),
	      rad (angles.y),
	      rad (angles.z),
	      order);

    M44f M (e.toMatrix44());

    //
    // With rounding errors from e.toMatrix.
    //

    testMatrix (M, matrixEulerMatrix, order);

    //
    // Without rounding errors (assuming that
    // all angles are multiples of 90 degrees).
    //

    for (int i = 0; i < 3; ++i)
	for (int j = 0; j < 3; ++j)
	    if (M[i][j] < -0.5)
		M[i][j] = -1;
	    else if (M[i][j] > 0.5)
		M[i][j] = 1;
	    else
		M[i][j] = 0;

    testMatrix (M, matrixEulerMatrix, order);
}


void
test (M44f (*matrixEulerMatrix)(const M44f &, Eulerf::Order),
      Eulerf::Order order)
{
    cout << "order = " << setbase (16) << int (order) << setbase (10) << endl;

    // cout << "random angles" << endl;

    testRandomAngles (matrixEulerMatrix, order);

    // cout << "special angles" << endl;

    for (int i = 0; i < 360; i += 90)
	for (int j = 0; j < 360; j += 90)
	    for (int k = 0; k < 360; k += 90)
		testAngles (V3f (i, j, k), matrixEulerMatrix, order);
}


void
testRandomAngles33 ()
{
    Rand48 r(0);

    float eps = 8.0 * limits<float>::epsilon();

    for (int i = 0; i < 100000; ++i)
    {
	float angle = rad (r.nextf (-180, 180));

	M33f M;
	M.setRotation (angle);

	float angleEx;
	extractEuler (M, angleEx);

	assert (IMATH_INTERNAL_NAMESPACE::equal (angle, angleEx, eps));
    }
}


} // namespace


void
testExtractEuler ()
{
    cout << "Testing extraction of rotation angle from 3x3 matrices" << endl;
    testRandomAngles33 ();
    
    cout << "Testing extraction of Euler angles from matrices" << endl;

    cout << "extractEulerXYZ()" << endl;
    test (matrixEulerMatrix_1, Eulerf::XYZ);

    cout << "extractEulerZYX()" << endl;
    test (matrixEulerMatrix_1, Eulerf::ZYX);

    cout << "Eulerf::extract()" << endl;
    test (matrixEulerMatrix_2, Eulerf::XYZ);
    test (matrixEulerMatrix_2, Eulerf::XZY);
    test (matrixEulerMatrix_2, Eulerf::YZX);
    test (matrixEulerMatrix_2, Eulerf::YXZ);
    test (matrixEulerMatrix_2, Eulerf::ZXY);
    test (matrixEulerMatrix_2, Eulerf::ZYX);
    
    test (matrixEulerMatrix_2, Eulerf::XZX);
    test (matrixEulerMatrix_2, Eulerf::XYX);
    test (matrixEulerMatrix_2, Eulerf::YXY);
    test (matrixEulerMatrix_2, Eulerf::YZY);
    test (matrixEulerMatrix_2, Eulerf::ZYZ);
    test (matrixEulerMatrix_2, Eulerf::ZXZ);

    test (matrixEulerMatrix_2, Eulerf::XYZr);
    test (matrixEulerMatrix_2, Eulerf::XZYr);
    test (matrixEulerMatrix_2, Eulerf::YZXr);
    test (matrixEulerMatrix_2, Eulerf::YXZr);
    test (matrixEulerMatrix_2, Eulerf::ZXYr);
    test (matrixEulerMatrix_2, Eulerf::ZYXr);
    
    test (matrixEulerMatrix_2, Eulerf::XZXr);
    test (matrixEulerMatrix_2, Eulerf::XYXr);
    test (matrixEulerMatrix_2, Eulerf::YXYr);
    test (matrixEulerMatrix_2, Eulerf::YZYr);
    test (matrixEulerMatrix_2, Eulerf::ZYZr);
    test (matrixEulerMatrix_2, Eulerf::ZXZr);

    cout << "ok\n" << endl;
}
