///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011, Industrial Light & Magic, a division of Lucas
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

#include <testFrustumTest.h>
#include "ImathFrustum.h"
#include "ImathFrustumTest.h"
#include "ImathBox.h"
#include "ImathSphere.h"
#include <iostream>
#include <assert.h>


using namespace std;

void
testFrustumTest ()
{
    cout << "Testing functions in ImathFrustumTest.h";

    cout << "\nisVisible(Vec3) ";

    float n = 1.7;
    float f = 567.0;
    float l = -3.5;
    float r = 2.0;
    float b = -1.3;
    float t = 0.9;

    IMATH_INTERNAL_NAMESPACE::Frustum<float> frustum (n, f, l, r, t, b, false);

    IMATH_INTERNAL_NAMESPACE::Matrix44<float> cameraMat;
    IMATH_INTERNAL_NAMESPACE::Vec3<float> cameraPos(100.0f, 200.0f, 300.0f);
    cameraMat.makeIdentity();
    cameraMat.translate(cameraPos);

    IMATH_INTERNAL_NAMESPACE::FrustumTest<float> frustumTest(frustum, cameraMat);

    /////////////////////////////////////////////////////
    // Test Vec3's
    IMATH_INTERNAL_NAMESPACE::Vec3<float> insideVec      (100.0f, 200.0f,          300 -   2.0f);
    IMATH_INTERNAL_NAMESPACE::Vec3<float> outsideVec_near(100.0f, 200.0f,          300 -   1.5f);
    IMATH_INTERNAL_NAMESPACE::Vec3<float> outsideVec_far (100.0f, 200.0f,          300 - 568.0f);
    IMATH_INTERNAL_NAMESPACE::Vec3<float> outsideVec_side(100.0f, 200.0f + 100.0f, 300 -   2.0f);
    IMATH_INTERNAL_NAMESPACE::Vec3<float> outsideVec_up  (100.0f + 100.0f, 200.0f, 300 -   2.0f);

    assert (  frustumTest.isVisible(insideVec));
    assert (! frustumTest.isVisible(outsideVec_near));
    assert (! frustumTest.isVisible(outsideVec_far));
    assert (! frustumTest.isVisible(outsideVec_side));
    assert (! frustumTest.isVisible(outsideVec_up));
    cout << "passed Vec3\n";

    /////////////////////////////////////////////////////
    // Test Boxes
    IMATH_INTERNAL_NAMESPACE::Vec3<float> tinySize(0.0001f, 0.0001f, 0.0001f);
    IMATH_INTERNAL_NAMESPACE::Vec3<float> hugeSize(1000.0f, 1000.0f, 1000.0f);

    // Empty box should NOT be visible
    assert (!frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >()));
         
    // Tiny box inside the frust should be visible
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (insideVec + tinySize, insideVec + tinySize)));

    // Huge boxes inside and outside should be visible
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (insideVec - hugeSize, insideVec + hugeSize)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_near - hugeSize, outsideVec_near + hugeSize)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_far - hugeSize, outsideVec_far + hugeSize)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_side - hugeSize, outsideVec_side + hugeSize)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_up - hugeSize, outsideVec_up + hugeSize)));

    // Tiny boxes outside should NOT be visible
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_near - tinySize, outsideVec_near + tinySize)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_far - tinySize, outsideVec_far + tinySize)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_side - tinySize, outsideVec_side + tinySize)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Box<IMATH_INTERNAL_NAMESPACE::Vec3<float> >
        (outsideVec_up - tinySize, outsideVec_up + tinySize)));
    cout << "passed Box\n";


    /////////////////////////////////////////////////////
    // Test Spheres
    float tinyRadius = 0.0001f;
    float hugeRadius = 1000.0f;

    // Tiny sphere inside the frust should be visible
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (insideVec, tinyRadius)));

    // Huge spheres inside and outside should be visible
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (insideVec, hugeRadius)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_near, hugeRadius)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_far, hugeRadius)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_side, hugeRadius)));
    assert (frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_up, hugeRadius)));

    // Tiny spheres outside should NOT be visible
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_near, tinyRadius)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_far, tinyRadius)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_side, tinyRadius)));
    assert (! frustumTest.isVisible(IMATH_INTERNAL_NAMESPACE::Sphere3<float>
        (outsideVec_up, tinyRadius)));
    cout << "passed Sphere\n";





    cout << "\nok\n\n";
}
