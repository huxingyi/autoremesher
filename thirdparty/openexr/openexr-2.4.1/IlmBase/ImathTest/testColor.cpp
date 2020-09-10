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

#include <testColor.h>
#include "ImathColor.h"
#include "ImathColorAlgo.h"
#include "ImathLimits.h"
#include "ImathMath.h"
#include <iostream>
#include <assert.h>


using namespace std;


void
testColor ()
{
    cout << "Testing functions in ImathColor.h & ImathColorAlgo.h" << endl;

    cout << "rgb2packed -> packed2rgb" << endl;

    const float         epsilon = IMATH_INTERNAL_NAMESPACE::limits< float >::epsilon();

    IMATH_INTERNAL_NAMESPACE::PackedColor	packed;
    IMATH_INTERNAL_NAMESPACE::C3c	    	in3( 52, 128, 254 );
    IMATH_INTERNAL_NAMESPACE::C3c	    	out3;
    
    packed = IMATH_INTERNAL_NAMESPACE::rgb2packed( in3 );
    IMATH_INTERNAL_NAMESPACE::packed2rgb( packed, out3 );

    assert ( in3 == out3 );

    IMATH_INTERNAL_NAMESPACE::C4c	    	testConstructor1;
    IMATH_INTERNAL_NAMESPACE::C4c	    	testConstructor1i ( 0.f );
    IMATH_INTERNAL_NAMESPACE::C4c	    	testConstructor2( testConstructor1i );

    testConstructor1 = testConstructor2; // use these so the compiler doesn't emit a warning

    IMATH_INTERNAL_NAMESPACE::C4c	    	testConstructor3( 52, 128, 254, 127 );
    IMATH_INTERNAL_NAMESPACE::C4c	    	A( testConstructor3 );
    IMATH_INTERNAL_NAMESPACE::C4c	    	B;
    IMATH_INTERNAL_NAMESPACE::C4f	    	X, Y, tmp;

    packed = IMATH_INTERNAL_NAMESPACE::rgb2packed( A );
    IMATH_INTERNAL_NAMESPACE::packed2rgb( packed, B );

    assert ( A == B );

    cout << "Imath::Color4 * f" << endl;

    assert ( ( IMATH_INTERNAL_NAMESPACE::C4f( 0.330f, 0.710f, 0.010f, 0.999f ) * 0.999f ) ==
    	     IMATH_INTERNAL_NAMESPACE::C4f( 0.330f * 0.999f,
	     	    	 0.710f * 0.999f,
			 0.010f * 0.999f,
			 0.999f * 0.999f ) );

    cout << "Imath::Color4 / f" << endl;

    assert ( ( IMATH_INTERNAL_NAMESPACE::C4f( 0.330f, 0.710f, 0.010f, 0.999f ) / 0.999f ) ==
    	     IMATH_INTERNAL_NAMESPACE::C4f( 0.330f / 0.999f,
	     	    	 0.710f / 0.999f,
			 0.010f / 0.999f,
			 0.999f / 0.999f ) );

    cout << "Assignment and comparison" << endl;

    B = A;
    assert( B == A );
    assert( !( B != A ) );

    X = Y = IMATH_INTERNAL_NAMESPACE::C4f( 0.123f, -0.420f, 0.501f, 0.998f );
    
    X *= 0.001f;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.r * 0.001f ) - X.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.g * 0.001f ) - X.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.b * 0.001f ) - X.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.a * 0.001f ) - X.a ) <= epsilon );

    X = Y = IMATH_INTERNAL_NAMESPACE::C4f( 0.123f, -0.420f, 0.501f, 0.998f );
    
    X /= -1.001f;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.r / -1.001f ) - X.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.g / -1.001f ) - X.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.b / -1.001f ) - X.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( Y.a / -1.001f ) - X.a ) <= epsilon );

    Y = IMATH_INTERNAL_NAMESPACE::C4f( 0.998f, -0.001f,  0.501f, 1.001f );
    X = IMATH_INTERNAL_NAMESPACE::C4f( 0.011f, -0.420f, -0.501f, 0.998f );

    tmp = X + Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r + Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g + Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b + Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a + Y.a ) - tmp.a ) <= epsilon );

    tmp = X - Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r - Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g - Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b - Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a - Y.a ) - tmp.a ) <= epsilon );

    tmp = X * Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r * Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g * Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b * Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a * Y.a ) - tmp.a ) <= epsilon );

    tmp = X / Y;
    
    //
    // epsilon doesn't work here.
    //
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r / Y.r ) - tmp.r ) <= 1e-5f &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g / Y.g ) - tmp.g ) <= 1e-5f &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b / Y.b ) - tmp.b ) <= 1e-5f &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a / Y.a ) - tmp.a ) <= 1e-5f );

    tmp = X;
    tmp += Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r + Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g + Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b + Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a + Y.a ) - tmp.a ) <= epsilon );

    tmp = X;
    tmp -= Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r - Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g - Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b - Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a - Y.a ) - tmp.a ) <= epsilon );

    tmp = X;
    tmp *= Y;
    
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r * Y.r ) - tmp.r ) <= epsilon &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g * Y.g ) - tmp.g ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b * Y.b ) - tmp.b ) <= epsilon &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a * Y.a ) - tmp.a ) <= epsilon );

    tmp = X;
    tmp /= Y;
    
    //
    // epsilon doesn't work here.
    //
    assert( IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.r / Y.r ) - tmp.r ) <= 1e-5f &&
    	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.g / Y.g ) - tmp.g ) <= 1e-5f &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.b / Y.b ) - tmp.b ) <= 1e-5f &&
	    IMATH_INTERNAL_NAMESPACE::Math<float>::fabs( ( X.a / Y.a ) - tmp.a ) <= 1e-5f );

    cout << "ok\n" << endl;
}
