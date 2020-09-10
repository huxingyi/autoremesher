///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009, Industrial Light & Magic, a division of Lucas
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

#include <testMiscMatrixAlgo.h>
#include "ImathMatrixAlgo.h"
#include "ImathRandom.h"
#include <iostream>
#include <exception>
#include <stdio.h>
#include <assert.h>


#if 0
    #define debug(x) (printf x, fflush (stdout))
#else
    #define debug(x)
#endif


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

float rad (float deg) {return deg * (M_PI / 180);}

void
testComputeLocalFrame ()
{
    float eps = 0.00005;
    Rand48 random(0);
    for (int i = 0; i < 100000; ++i)
    {
        debug (("iteration: %d\n", i));
    
        // Random pos
        V3f p (random.nextf (-10, 10), 
               random.nextf (-10, 10), 
               random.nextf (-10, 10));

        // Random xDir 
        V3f xDir (random.nextf (-10, 10), 
                  random.nextf (-10, 10), 
                  random.nextf (-10, 10));
        
        // Random normalDir
        V3f normalDir (random.nextf (-10, 10), 
                       random.nextf (-10, 10), 
                       random.nextf (-10, 10));

        // Run computeLocalFrame we want to test
        M44f L = computeLocalFrame(p, xDir, normalDir);
        
        // test position
        for (int j=0; j<3; j++)
        {
            if ( abs(L[3][j] - p[j])>eps )
                assert(false);
        }
        if (abs (L[3][3] - 1.0)>eps)
            assert(false );
        
        // check that xAxis has the same dir as xDir and that is is normalized
        V3f x( L[0][0], L[0][1], L[0][2]);
        assert( (x%xDir).length() < eps );
        if (abs (L[0][3])>eps)
            assert(false);
        assert((abs(x.length()-1.f)<eps));
        
        // Check that y is normal to x and to normalDir, and is normalized
        V3f y( L[1][0], L[1][1], L[1][2]);
        if (abs (L[1][3])>eps)
            assert(false );
        assert(abs(x^y)<eps);
        /*std::cout<<y<<"\n";
        std::cout<<normalDir<<"\n";
        std::cout<<(y^normalDir)<<"\n";*/
        assert(abs(y^normalDir)<eps);
        assert((abs(y.length()-1.f)<eps));
        
        // check that z is normalized, normal to x and y, and direct
        V3f z( L[2][0], L[2][1], L[2][2]);
        if (abs (L[2][3])>eps)
            assert(false );
        assert((abs(z.length()-1.f)<eps));
        assert(abs(x^z)<eps);
        assert(abs(y^z)<eps);
        assert(((x%y)^z)>0);
    }
}

void
getRandTRS(Rand48& random, V3f& trans, V3f& rot, V3f& scale)
{
    // Translate 
    trans = V3f (random.nextf (-10, 10), 
                 random.nextf (-10, 10), 
                 random.nextf (-10, 10));
    // Rotate 
    rot = V3f (rad (random.nextf (-180, 180)),
               rad (random.nextf (-180, 180)),
               rad (random.nextf (-180, 180)));

    // Scale 
    V3f s(random.nextf (0.000001, 2.0),
          random.nextf (0.000001, 2.0),
          random.nextf (0.000001, 2.0));
    for (int j=0; j < 3; j++)
        if (random.nextf (0.0, 1.0) >= 0.5)
            s[j] *= -1;
    scale = s;
}

M44f
createRandomMat(Rand48& random, V3f& trans, V3f& rot, V3f& scale)
{
    
    M44f M;
    V3f t, r, s;
    getRandTRS(random, t, r, s);
    
    M.translate (t);
    M.rotate (r);

    // Shear M.
    V3f h (random.nextf (0.000001, 2.0), 
           random.nextf (0.000001, 2.0), 
           random.nextf (0.000001, 2.0));

    for (int j=0; j < 3; j++)
        if (random.nextf (0.0, 1.0) >= 0.5)
            h[j] *= -1;
    M.shear (h);

    M.scale (s);

    //
    // Add a small random error to the elements of M
    //
    for (int j = 0; j < 4; ++j)
        for (int k = 0; k < 3; ++k)
            M[j][k] += random.nextf (-1e-7, 1e-7);

    V3f sh;
    extractSHRT (M, scale, sh, rot, trans);

    debug (("Scale   : %f %f %f\n", s[0], s[1], s[2]));
    debug (("Shear   : %f %f %f\n", h[0], h[1], h[2]));
    debug (("Rot     : %f %f %f\n", r[0], r[1], r[2]));
    debug (("Trans   : %f %f %f\n", t[0], t[1], t[2]));
    
    return M;
}

void
compareMat(M44f& M, M44f& N)
{
    float eps = 0.0001;
    
    /// Verify that the entries in M and N do not
    // differ too much.

    M44f D (M - N);

    for (int j = 0; j < 4; ++j)
    {
        for (int k = 0; k < 4; ++k)
        {
            //cout << "diff="<<D[j][k] << endl;
            if (abs (D[j][k]) > eps)
            {
                cout << "unexpectedly diff "<<
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
testAddOffset()
{
    Rand48 random(0);

    for (int i = 0; i < 100000; ++i)
    {
        debug (("iteration: %d\n", i));

        V3f transA, transB, rotA, rotB, scaleA, scaleB;
        V3f tOffset, rOffset, sOffset;
        M44f inMat  = createRandomMat(random, transA, rotA, scaleA);
        M44f refMat = createRandomMat(random, transB, rotB, scaleB);
        getRandTRS(random, tOffset, rOffset, sOffset);
        
        // addOffset : function to test
        M44f outMat = addOffset( inMat, tOffset, rOffset, sOffset, refMat);
        
        // add the inverse offset
        M44f invO;
        invO.rotate (V3f(rad(rOffset[0]), rad(rOffset[1]), rad(rOffset[2])));
        invO[3][0] = tOffset[0];
        invO[3][1] = tOffset[1];
        invO[3][2] = tOffset[2];
        invO.invert();

        M44f invS;
        invS.scale (sOffset);
        invS.invert(); // zero scale is avoided in getRandTRS
        
        // in ref mat from the function result
        M44f outInRefMat = invO*invS*outMat;
        
        // in ref mat from the inputs
        M44f inRefMat    = inMat*refMat;
        
        // compare the mat
        compareMat(outInRefMat, inRefMat);
    }
}

void
testRSMatrix(M44f& M, V3f& t, V3f& r, V3f& s)
{
    M44f N;
    N.makeIdentity();
    N.translate (t); // ... matrix compositions
    N.rotate (r);
    N.scale (s);

    compareMat(M, N);
}

void
testComputeRSMatrix ()
{
    Rand48 random(0);
    
    for (int i = 0; i < 100000; ++i)
    {
        debug (("iteration: %d\n", i));

        V3f transA, transB, rotA, rotB, scaleA, scaleB;
        
        M44f A = createRandomMat(random, transA, rotA, scaleA);
        M44f B = createRandomMat(random, transB, rotB, scaleB);

        M44f ArAsA = computeRSMatrix( true,  true,  A, B);
        M44f ArBsB = computeRSMatrix( false, false, A, B);
        M44f ArAsB = computeRSMatrix( true,  false, A, B);
        M44f ArBsA = computeRSMatrix( false, true,  A, B);
        
        testRSMatrix(ArAsA, transA, rotA, scaleA);
        testRSMatrix(ArBsB, transA, rotB, scaleB);
        testRSMatrix(ArAsB, transA, rotA, scaleB);
        testRSMatrix(ArBsA, transA, rotB, scaleA);

        debug (("\n"));
    }
}

} // namespace


void
testMiscMatrixAlgo ()
{
    try
    {
        cout << "Testing misc functions in ImathMatrixAlgo.h" << endl;

        cout << "Testing the building of an orthonormal direct frame from : a position, " 
        << "an x axis direction and a normal to the y axis" << endl;	
        cout << "IMATH_INTERNAL_NAMESPACE::computeLocalFrame()" << endl;
        
        testComputeLocalFrame ();
        
        cout << "ok\n" << endl;

        cout << "Add a translate/rotate/scale offset to an input frame "
        << "and put it in another frame of reference" << endl;
        cout << "IMATH_INTERNAL_NAMESPACE::addOffset()" << endl;

        testAddOffset ();

        cout << "ok\n" << endl;

        cout << "Compute Translate/Rotate/Scale matrix from matrix A "<<endl;
        cout << "with the Rotate/Scale of Matrix B"<< endl;
        cout << "IMATH_INTERNAL_NAMESPACE::computeRSMatrix()" << endl;

        testComputeRSMatrix ();

        cout << "ok\n" << endl;
        
    }
    catch (std::exception &e)
    {
        cerr << "  Caught exception: " << e.what () << endl;
    }
}
