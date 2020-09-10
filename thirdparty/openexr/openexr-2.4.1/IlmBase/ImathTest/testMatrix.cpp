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

#include <testMatrix.h>
#include "ImathMatrix.h"
#include "ImathMatrixAlgo.h"
#include "ImathVec.h"
#include "ImathLimits.h"
#include "ImathMath.h"
#include "ImathInt64.h"
#include "ImathRandom.h"
#include <iostream>
#include <assert.h>


using namespace std;
using IMATH_INTERNAL_NAMESPACE::Int64;


//
// This file is not currently intended to exhaustively test
// the Imath Matrix33<T> and Matrix44<T> classes.  We leave
// that to PyImathTest.
//
// Instead, in this file we test only those aspects of the
// Imath Matrix33<T> and Matrix44<T> classes that must be 
// or are more convenient to test from C++.
//

void
testMatrix ()
{
    cout << "Testing functions in ImathMatrix.h" << endl;

    union {float f; int i;} nanf;
    nanf.i = 0x7f800001; //  NAN

    union {double d; Int64 i;} nand;
    nand.i = 0x7ff0000000000001ULL; //  NAN

    {
	cout << "Imath::M33f shear functions" << endl;

	IMATH_INTERNAL_NAMESPACE::M33f m1, m2;
	m1.setShear (2.0f);
	assert
	   (m1[0][0] == 1.0f  &&  m1[0][1] == 0.0f  &&  m1[0][2] == 0.0f  &&
	    m1[1][0] == 2.0f  &&  m1[1][1] == 1.0f  &&  m1[1][2] == 0.0f  &&
	    m1[2][0] == 0.0f  &&  m1[2][1] == 0.0f  &&  m1[2][2] == 1.0f);

	m2.setShear (IMATH_INTERNAL_NAMESPACE::V2f (3.0f, 4.0f));
	assert
	   (m2[0][0] == 1.0f  &&  m2[0][1] == 4.0f  &&  m2[0][2] == 0.0f  &&
	    m2[1][0] == 3.0f  &&  m2[1][1] == 1.0f  &&  m2[1][2] == 0.0f  &&
	    m2[2][0] == 0.0f  &&  m2[2][1] == 0.0f  &&  m2[2][2] == 1.0f);


	m1.shear (IMATH_INTERNAL_NAMESPACE::V2f (5.0f, 6.0f));
	assert
	   (m1[0][0] == 13.0f  &&  m1[0][1] == 6.0f  &&  m1[0][2] == 0.0f  &&
	    m1[1][0] ==  7.0f  &&  m1[1][1] == 1.0f  &&  m1[1][2] == 0.0f  &&
	    m1[2][0] ==  0.0f  &&  m1[2][1] == 0.0f  &&  m1[2][2] == 1.0f);

	m2.shear (7.0f);
	assert
	   (m2[0][0] ==  1.0f  &&  m2[0][1] ==  4.0f  &&  m2[0][2] == 0.0f  &&
	    m2[1][0] == 10.0f  &&  m2[1][1] == 29.0f  &&  m2[1][2] == 0.0f  &&
	    m2[2][0] ==  0.0f  &&  m2[2][1] ==  0.0f  &&  m2[2][2] == 1.0f);

	cout << "M33f constructors and equality operators" << endl;

	IMATH_INTERNAL_NAMESPACE::M33f test(m2);
	assert(test == m2);

	IMATH_INTERNAL_NAMESPACE::M33f test2;
	assert(test != test2);

	IMATH_INTERNAL_NAMESPACE::M33f test3;
	test3.makeIdentity();
	assert(test2 == test3);
    }

    {
	cout << "M33d constructors and equality operators" << endl;

	IMATH_INTERNAL_NAMESPACE::M33d m2;
	m2[0][0] = 99.0f;
	m2[1][2] = 101.0f;

	IMATH_INTERNAL_NAMESPACE::M33d test(m2);
	assert(test == m2);

	IMATH_INTERNAL_NAMESPACE::M33d test2;
	assert(test != test2);

	IMATH_INTERNAL_NAMESPACE::M33d test3;
	test3.makeIdentity();
	assert(test2 == test3);

        IMATH_INTERNAL_NAMESPACE::M33f test4 (1.0f, 2.0f, 3.0f,
                           4.0f, 5.0f, 6.0f,
                           7.0f, 8.0f, 9.0f);

        IMATH_INTERNAL_NAMESPACE::M33d test5 = IMATH_INTERNAL_NAMESPACE::M33d (test4);

        assert (test5[0][0] == 1.0);
        assert (test5[0][1] == 2.0);
        assert (test5[0][2] == 3.0);

        assert (test5[1][0] == 4.0);
        assert (test5[1][1] == 5.0);
        assert (test5[1][2] == 6.0);

        assert (test5[2][0] == 7.0);
        assert (test5[2][1] == 8.0);
        assert (test5[2][2] == 9.0);
    }

    {
	IMATH_INTERNAL_NAMESPACE::M44f m2;
	m2[0][0] = 99.0f;
	m2[1][2] = 101.0f;

	cout << "M44f constructors and equality operators" << endl;

	IMATH_INTERNAL_NAMESPACE::M44f test(m2);
	assert(test == m2);

	IMATH_INTERNAL_NAMESPACE::M44f test2;
	assert(test != test2);

	IMATH_INTERNAL_NAMESPACE::M44f test3;
	test3.makeIdentity();
	assert(test2 == test3);

	//
	// Test non-equality when a NAN is in the same
	// place in two identical matrices
	//

	test2[0][0] = nanf.f;
	test3 = test2;
	assert(test2 != test3);
    }

    {
	IMATH_INTERNAL_NAMESPACE::M44d m2;
	m2[0][0] = 99.0f;
	m2[1][2] = 101.0f;

	cout << "M44d constructors and equality operators" << endl;

	IMATH_INTERNAL_NAMESPACE::M44d test(m2);
	assert(test == m2);

	IMATH_INTERNAL_NAMESPACE::M44d test2;
	assert(test != test2);

	IMATH_INTERNAL_NAMESPACE::M44d test3;
	test3.makeIdentity();
	assert(test2 == test3);

	//
	// Test non-equality when a NAN is in the same
	// place in two identical matrices
	//

	test2[0][0] = nand.d;
	test3 = test2;
	assert(test2 != test3);

        IMATH_INTERNAL_NAMESPACE::M44f test4 ( 1.0f,  2.0f,  3.0f,  4.0f,
                            5.0f,  6.0f,  7.0f,  8.0f,
                            9.0f, 10.0f, 11.0f, 12.0f,
                           13.0f, 14.0f, 15.0f, 16.0f);

        IMATH_INTERNAL_NAMESPACE::M44d test5 = IMATH_INTERNAL_NAMESPACE::M44d (test4);

        assert (test5[0][0] ==  1.0);
        assert (test5[0][1] ==  2.0);
        assert (test5[0][2] ==  3.0);
        assert (test5[0][3] ==  4.0);

        assert (test5[1][0] ==  5.0);
        assert (test5[1][1] ==  6.0);
        assert (test5[1][2] ==  7.0);
        assert (test5[1][3] ==  8.0);

        assert (test5[2][0] ==  9.0);
        assert (test5[2][1] == 10.0);
        assert (test5[2][2] == 11.0);
        assert (test5[2][3] == 12.0);

        assert (test5[3][0] == 13.0);
        assert (test5[3][1] == 14.0);
        assert (test5[3][2] == 15.0);
        assert (test5[3][3] == 16.0);
    }

    {
	cout << "Converting between M33 and M44" << endl;

	IMATH_INTERNAL_NAMESPACE::M44d m1;
	m1[0][0] = 99;
	IMATH_INTERNAL_NAMESPACE::M44f m2;
	m2.setValue(m1);
	assert(m2[0][0] == (float)m1[0][0]);
	m1[0][0] = 101;
	m1.setValue(m2);
	assert(m2[0][0] == (float)m1[0][0]);
    }

    // Matrix minors
    {
        cout << "3x3 Matrix minors" << endl;

        IMATH_INTERNAL_NAMESPACE::M33f a(1,2,3,4,5,6,7,8,9);

        assert (a.minorOf(0,0) == a.fastMinor(1,2,1,2));
        assert (a.minorOf(0,1) == a.fastMinor(1,2,0,2));
        assert (a.minorOf(0,2) == a.fastMinor(1,2,0,1));
        assert (a.minorOf(1,0) == a.fastMinor(0,2,1,2));
        assert (a.minorOf(1,1) == a.fastMinor(0,2,0,2));
        assert (a.minorOf(1,2) == a.fastMinor(0,2,0,1));
        assert (a.minorOf(2,0) == a.fastMinor(0,1,1,2));
        assert (a.minorOf(2,1) == a.fastMinor(0,1,0,2));
        assert (a.minorOf(2,2) == a.fastMinor(0,1,0,1));
    }
    {
        IMATH_INTERNAL_NAMESPACE::M33d a(1,2,3,4,5,6,7,8,9);

        assert (a.minorOf(0,0) == a.fastMinor(1,2,1,2));
        assert (a.minorOf(0,1) == a.fastMinor(1,2,0,2));
        assert (a.minorOf(0,2) == a.fastMinor(1,2,0,1));
        assert (a.minorOf(1,0) == a.fastMinor(0,2,1,2));
        assert (a.minorOf(1,1) == a.fastMinor(0,2,0,2));
        assert (a.minorOf(1,2) == a.fastMinor(0,2,0,1));
        assert (a.minorOf(2,0) == a.fastMinor(0,1,1,2));
        assert (a.minorOf(2,1) == a.fastMinor(0,1,0,2));
        assert (a.minorOf(2,2) == a.fastMinor(0,1,0,1));
    }

    // Determinants (by building a random singular value decomposition)
    {
        cout << "3x3 determinant" << endl;

        IMATH_INTERNAL_NAMESPACE::Rand32 random;

        IMATH_INTERNAL_NAMESPACE::M33f u;
        IMATH_INTERNAL_NAMESPACE::M33f v;
        IMATH_INTERNAL_NAMESPACE::M33f s;

        u.setRotation( random.nextf() );
        v.setRotation( random.nextf() );
        s[0][0] = random.nextf();
        s[1][1] = random.nextf();
        s[2][2] = random.nextf();

        IMATH_INTERNAL_NAMESPACE::M33f c = u * s * v.transpose();
        assert (fabsf(c.determinant() - s[0][0]*s[1][1]*s[2][2]) <= u.baseTypeEpsilon());
    }
    {
        IMATH_INTERNAL_NAMESPACE::Rand32 random;

        IMATH_INTERNAL_NAMESPACE::M33d u;
        IMATH_INTERNAL_NAMESPACE::M33d v;
        IMATH_INTERNAL_NAMESPACE::M33d s;

        u.setRotation( (double)random.nextf() );
        v.setRotation( (double)random.nextf() );
        s[0][0] = (double)random.nextf();
        s[1][1] = (double)random.nextf();
        s[2][2] = (double)random.nextf();

        IMATH_INTERNAL_NAMESPACE::M33d c = u * s * v.transpose();
        assert (fabs(c.determinant() - s[0][0]*s[1][1]*s[2][2]) <= u.baseTypeEpsilon());
    }

    // Outer product of two 3D vectors
    {
        cout << "Outer product of two 3D vectors" << endl;

        IMATH_INTERNAL_NAMESPACE::V3f a(1,2,3);
        IMATH_INTERNAL_NAMESPACE::V3f b(4,5,6);
        IMATH_INTERNAL_NAMESPACE::M33f  p = IMATH_INTERNAL_NAMESPACE::outerProduct(a,b);

        for (int i=0; i<3; i++ )
        {
            for (int j=0; j<3; j++)
            {
                assert (p[i][j] == a[i]*b[j]);
            }
        }
    }
    {
        IMATH_INTERNAL_NAMESPACE::V3d a(1,2,3);
        IMATH_INTERNAL_NAMESPACE::V3d b(4,5,6);
        IMATH_INTERNAL_NAMESPACE::M33d  p = IMATH_INTERNAL_NAMESPACE::outerProduct(a,b);

        for (int i=0; i<3; i++ )
        {
            for (int j=0; j<3; j++)
            {
                assert (p[i][j] == a[i]*b[j]);
            }
        }
    }


    // Determinants (by building a random singular value decomposition)
    {
        cout << "4x4 determinants" << endl;

        IMATH_INTERNAL_NAMESPACE::Rand32 random;

        IMATH_INTERNAL_NAMESPACE::M44f u = IMATH_INTERNAL_NAMESPACE::rotationMatrix
            ( IMATH_INTERNAL_NAMESPACE::V3f(random.nextf(),random.nextf(),random.nextf()).normalize(),
              IMATH_INTERNAL_NAMESPACE::V3f(random.nextf(),random.nextf(),random.nextf()).normalize() );
        IMATH_INTERNAL_NAMESPACE::M44f v = IMATH_INTERNAL_NAMESPACE::rotationMatrix
            ( IMATH_INTERNAL_NAMESPACE::V3f(random.nextf(),random.nextf(),random.nextf()).normalize(),
              IMATH_INTERNAL_NAMESPACE::V3f(random.nextf(),random.nextf(),random.nextf()).normalize() );
        IMATH_INTERNAL_NAMESPACE::M44f s;

        s[0][0] = random.nextf();
        s[1][1] = random.nextf();
        s[2][2] = random.nextf();
        s[3][3] = random.nextf();

        IMATH_INTERNAL_NAMESPACE::M44f c = u * s * v.transpose();
        assert (fabsf(c.determinant() - s[0][0]*s[1][1]*s[2][2]*s[3][3]) <= u.baseTypeEpsilon());
    }
    {
        IMATH_INTERNAL_NAMESPACE::Rand32 random;

        IMATH_INTERNAL_NAMESPACE::M44d u = IMATH_INTERNAL_NAMESPACE::rotationMatrix
            ( IMATH_INTERNAL_NAMESPACE::V3d(random.nextf(),random.nextf(),random.nextf()).normalize(),
              IMATH_INTERNAL_NAMESPACE::V3d(random.nextf(),random.nextf(),random.nextf()).normalize() );
        IMATH_INTERNAL_NAMESPACE::M44d v = IMATH_INTERNAL_NAMESPACE::rotationMatrix
            ( IMATH_INTERNAL_NAMESPACE::V3d(random.nextf(),random.nextf(),random.nextf()).normalize(),
              IMATH_INTERNAL_NAMESPACE::V3d(random.nextf(),random.nextf(),random.nextf()).normalize() );
        IMATH_INTERNAL_NAMESPACE::M44d s;

        s[0][0] = random.nextf();
        s[1][1] = random.nextf();
        s[2][2] = random.nextf();
        s[3][3] = random.nextf();

        IMATH_INTERNAL_NAMESPACE::M44d c = u * s * v.transpose();
        assert (fabs(c.determinant() - s[0][0]*s[1][1]*s[2][2]*s[3][3]) <= u.baseTypeEpsilon());
    }

    // Matrix minors
    {
        cout << "4x4 matrix minors" << endl;

        IMATH_INTERNAL_NAMESPACE::M44d a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);

        assert (a.minorOf(0,0) == a.fastMinor(1,2,3,1,2,3));
        assert (a.minorOf(0,1) == a.fastMinor(1,2,3,0,2,3));
        assert (a.minorOf(0,2) == a.fastMinor(1,2,3,0,1,3));
        assert (a.minorOf(0,3) == a.fastMinor(1,2,3,0,1,2));
        assert (a.minorOf(1,0) == a.fastMinor(0,2,3,1,2,3));
        assert (a.minorOf(1,1) == a.fastMinor(0,2,3,0,2,3));
        assert (a.minorOf(1,2) == a.fastMinor(0,2,3,0,1,3));
        assert (a.minorOf(1,3) == a.fastMinor(0,2,3,0,1,2));
        assert (a.minorOf(2,0) == a.fastMinor(0,1,3,1,2,3));
        assert (a.minorOf(2,1) == a.fastMinor(0,1,3,0,2,3));
        assert (a.minorOf(2,2) == a.fastMinor(0,1,3,0,1,3));
        assert (a.minorOf(2,3) == a.fastMinor(0,1,3,0,1,2));
        assert (a.minorOf(3,0) == a.fastMinor(0,1,2,1,2,3));
        assert (a.minorOf(3,1) == a.fastMinor(0,1,2,0,2,3));
        assert (a.minorOf(3,2) == a.fastMinor(0,1,2,0,1,3));
        assert (a.minorOf(3,3) == a.fastMinor(0,1,2,0,1,2));
    }
    {
        IMATH_INTERNAL_NAMESPACE::M44f a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);

        assert (a.minorOf(0,0) == a.fastMinor(1,2,3,1,2,3));
        assert (a.minorOf(0,1) == a.fastMinor(1,2,3,0,2,3));
        assert (a.minorOf(0,2) == a.fastMinor(1,2,3,0,1,3));
        assert (a.minorOf(0,3) == a.fastMinor(1,2,3,0,1,2));
        assert (a.minorOf(1,0) == a.fastMinor(0,2,3,1,2,3));
        assert (a.minorOf(1,1) == a.fastMinor(0,2,3,0,2,3));
        assert (a.minorOf(1,2) == a.fastMinor(0,2,3,0,1,3));
        assert (a.minorOf(1,3) == a.fastMinor(0,2,3,0,1,2));
        assert (a.minorOf(2,0) == a.fastMinor(0,1,3,1,2,3));
        assert (a.minorOf(2,1) == a.fastMinor(0,1,3,0,2,3));
        assert (a.minorOf(2,2) == a.fastMinor(0,1,3,0,1,3));
        assert (a.minorOf(2,3) == a.fastMinor(0,1,3,0,1,2));
        assert (a.minorOf(3,0) == a.fastMinor(0,1,2,1,2,3));
        assert (a.minorOf(3,1) == a.fastMinor(0,1,2,0,2,3));
        assert (a.minorOf(3,2) == a.fastMinor(0,1,2,0,1,3));
        assert (a.minorOf(3,3) == a.fastMinor(0,1,2,0,1,2));
    }

    // VC 2005 64 bits compiler has a bug with __restrict keword.
    // Pointers with __restrict should not alias the same symbol.
    // But, with optimization on, VC removes intermediate temp variable
    // and ignores __restrict.
    {
        cout << "M44 multiplicaftion test" << endl;
        IMATH_INTERNAL_NAMESPACE::M44f M ( 1.0f,  2.0f,  3.0f,  4.0f,
                        5.0f,  6.0f,  7.0f,  8.0f,
                        9.0f, 10.0f, 11.0f, 12.0f,
                        13.0f, 14.0f, 15.0f, 16.0f);

        IMATH_INTERNAL_NAMESPACE::M44f N; N.makeIdentity();

        // N should be equal to M
        // This typical test fails
        // when __restrict is used for pointers in "multiply" function.
        N = N * M;

        assert(N == M);

        if (N != M) {
            cout << "M44 multiplication test has failed, error." << endl
                 << "M" << endl << M << endl
                 << "N" << endl << N << endl;
        }
    }

    cout << "ok\n" << endl;
}
