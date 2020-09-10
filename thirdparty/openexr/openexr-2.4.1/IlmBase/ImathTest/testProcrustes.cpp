///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010, Industrial Light & Magic, a division of Lucas
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

#include "ImathMatrixAlgo.h"
#include "ImathRandom.h"
#include "ImathEuler.h"
#include <iostream>
#include <assert.h>
#include <cmath>
#include <vector>
#include <limits>

// Verify that if our transformation is already orthogonal, procrustes doesn't
// change that:
template <typename T>
void
testTranslationRotationMatrix (const IMATH_INTERNAL_NAMESPACE::M44d& mat)
{
    std::cout << "Testing known translate/rotate matrix:\n " << mat;
    typedef IMATH_INTERNAL_NAMESPACE::Vec3<T> Vec;

    static IMATH_INTERNAL_NAMESPACE::Rand48 rand (2047);

    size_t numPoints = 7;
    std::vector<Vec> from;  from.reserve (numPoints);
    std::vector<Vec> to;    to.reserve (numPoints);
    for (size_t i = 0; i < numPoints; ++i)
    {
        IMATH_INTERNAL_NAMESPACE::V3d a (rand.nextf(), rand.nextf(), rand.nextf());
        IMATH_INTERNAL_NAMESPACE::V3d b = a * mat;

        from.push_back (Vec(a));
        to.push_back (Vec(b));
    }

    std::vector<T> weights (numPoints, T(1));
    const IMATH_INTERNAL_NAMESPACE::M44d m1 = procrustesRotationAndTranslation (&from[0], &to[0], &weights[0], numPoints);
    const IMATH_INTERNAL_NAMESPACE::M44d m2 = procrustesRotationAndTranslation (&from[0], &to[0], numPoints);

    const T eps = sizeof(T) == 8 ? 1e-8 : 1e-4;
    for (size_t i = 0; i < numPoints; ++i)
    {
        const IMATH_INTERNAL_NAMESPACE::V3d a = from[i];
        const IMATH_INTERNAL_NAMESPACE::V3d b = to[i];
        const IMATH_INTERNAL_NAMESPACE::V3d b1 = a * m1;
        const IMATH_INTERNAL_NAMESPACE::V3d b2 = a * m2;

        assert ((b - b1).length() < eps);
        assert ((b - b2).length() < eps);
    }
    std::cout << "  OK\n";
}


// Test that if we pass in a matrix that we know consists only of translates,
// rotates, and uniform scale that we get an exact match.
template <typename T>
void testWithTranslateRotateAndScale (const IMATH_INTERNAL_NAMESPACE::M44d& m)
{
    std::cout << "Testing with known translate/rotate/scale matrix\n" << m;
    IMATH_INTERNAL_NAMESPACE::Rand48 rand(5376);

    typedef IMATH_INTERNAL_NAMESPACE::Vec3<T> V3;
    std::vector<V3> from;
    std::vector<T> weights;

    const float eps = 1e-4;
    std::cout << "numPoints: " << std::flush;
    for (size_t numPoints = 1; numPoints < 10; ++numPoints)
    {
        from.push_back (V3(rand.nextf(), rand.nextf(), rand.nextf()));
        weights.push_back (rand.nextf());
        std::cout << from.size() << " ";

        std::vector<V3> to;
        for (size_t i = 0; i < from.size(); ++i)
            to.push_back (from[i] * m);

        // weighted:
        IMATH_INTERNAL_NAMESPACE::M44d res = IMATH_INTERNAL_NAMESPACE::procrustesRotationAndTranslation (&from[0], &to[0], &weights[0], from.size(), true);
        for (size_t i = 0; i < from.size(); ++i)
            assert ((from[i] * res - to[i]).length() < eps);

        // unweighted:
        res = IMATH_INTERNAL_NAMESPACE::procrustesRotationAndTranslation (&from[0], &to[0], from.size(), true);
        for (size_t i = 0; i < from.size(); ++i)
            assert ((from[i] * res - to[i]).length() < eps);
    }
    std::cout << "  OK\n";
}

template <typename T>
double
procrustesError (const IMATH_INTERNAL_NAMESPACE::Vec3<T>* from,
                 const IMATH_INTERNAL_NAMESPACE::Vec3<T>* to,
                 const T* weights,
                 const size_t n,
                 const IMATH_INTERNAL_NAMESPACE::M44d& xform)
{
    double result = 0.0;
    double residual = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        IMATH_INTERNAL_NAMESPACE::V3d xformed = IMATH_INTERNAL_NAMESPACE::V3d(from[i]) * xform;
        IMATH_INTERNAL_NAMESPACE::V3d diff = xformed - IMATH_INTERNAL_NAMESPACE::V3d(to[i]);
        const double w = weights[i];
        const double mag = w * diff.length2();

        // Use Kahan summation for the heck of it:
        const double y = mag - residual;
        const double t = result + y;
        residual = (t - result) - y;
        result = t;
    }
    return result;
}

template <typename T>
void
verifyProcrustes (const std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> >& from, 
                  const std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> >& to)
{
    const T eps = std::sqrt(std::numeric_limits<T>::epsilon());

    const size_t n = from.size();

    // Validate that passing in uniform weights gives the same answer as
    // passing in no weights:
    std::vector<T> weights (from.size());
    for (size_t i = 0; i < weights.size(); ++i)
        weights[i] = 1;
    IMATH_INTERNAL_NAMESPACE::M44d m1 = procrustesRotationAndTranslation (&from[0], &to[0], n);
    IMATH_INTERNAL_NAMESPACE::M44d m2 = procrustesRotationAndTranslation (&from[0], &to[0], &weights[0], n);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            assert (std::abs(m1[i][j] - m2[i][j]) < eps);

    // Now try the weighted version:
    for (size_t i = 0; i < weights.size(); ++i)
        weights[i] = i+1;

    IMATH_INTERNAL_NAMESPACE::M44d m = procrustesRotationAndTranslation (&from[0], &to[0], &weights[0], n);

    // with scale:
    IMATH_INTERNAL_NAMESPACE::M44d ms = procrustesRotationAndTranslation (&from[0], &to[0], &weights[0], n, true);

    // Verify that it's orthonormal w/ positive determinant.
    const T det = m.determinant();
    assert (std::abs(det - T(1)) < eps);

    // Verify orthonormal:
    IMATH_INTERNAL_NAMESPACE::M33d upperLeft;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            upperLeft[i][j] = m[i][j];
    IMATH_INTERNAL_NAMESPACE::M33d product = upperLeft * upperLeft.transposed();
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            const double expected = (i == j ? 1.0 : 0.0);
            assert (std::abs(product[i][j] - expected) < eps);
        }
    }

    // Verify that nearby transforms are worse:
    const size_t numTries = 10;
    IMATH_INTERNAL_NAMESPACE::Rand48 rand (1056);
    const double delta = 1e-3;
    for (size_t i = 0; i < numTries; ++i)
    {
        // Construct an orthogonal rotation matrix using Euler angles:
        IMATH_INTERNAL_NAMESPACE::Eulerd diffRot (delta * rand.nextf(), delta * rand.nextf(), delta * rand.nextf());
 
        assert (procrustesError (&from[0], &to[0], &weights[0], n, m * diffRot.toMatrix44()) >
                procrustesError (&from[0], &to[0], &weights[0], n, m));

        // Try a small translation:
        IMATH_INTERNAL_NAMESPACE::V3d diffTrans (delta * rand.nextf(), delta * rand.nextf(), delta * rand.nextf());
        IMATH_INTERNAL_NAMESPACE::M44d translateMatrix;
        translateMatrix.translate (diffTrans);
        assert (procrustesError (&from[0], &to[0], &weights[0], n, m * translateMatrix) >
                procrustesError (&from[0], &to[0], &weights[0], n, m));
    }

    // Try a small scale:
    IMATH_INTERNAL_NAMESPACE::M44d newMat = ms;
    const double scaleDiff = delta;
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            newMat[i][j] = ms[i][j] * (1.0 + scaleDiff);
    assert (procrustesError (&from[0], &to[0], &weights[0], n, newMat) >
            procrustesError (&from[0], &to[0], &weights[0], n, ms));

    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            newMat[i][j] = ms[i][j] * (1.0 - scaleDiff);
    assert (procrustesError (&from[0], &to[0], &weights[0], n, newMat) >
            procrustesError (&from[0], &to[0], &weights[0], n, ms));

    //
    // Verify the magical property that makes shape springs work:
    // when the displacements Q*A-B, times the weights,
    // are applied as forces at B,
    // there is zero net force and zero net torque.
    //
    {
        IMATH_INTERNAL_NAMESPACE::V3d center (0, 0, 0);

        IMATH_INTERNAL_NAMESPACE::V3d netForce(0);
        IMATH_INTERNAL_NAMESPACE::V3d netTorque(0);
        for (size_t iPoint = 0; iPoint < n; ++iPoint)
        {
            const IMATH_INTERNAL_NAMESPACE::V3d force = weights[iPoint] * (from[iPoint]*m - to[iPoint]);
            netForce += force;
            netTorque += to[iPoint].cross (force);
        }

        assert (netForce.length2() < eps);
        assert (netTorque.length2() < eps);
    }
}

template <typename T>
void
testProcrustesWithMatrix (const IMATH_INTERNAL_NAMESPACE::M44d& m)
{
    std::cout << "Testing Procrustes algorithm with arbitrary matrix: \n" << m;
    std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> > fromPoints;
    std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> > toPoints;

    IMATH_INTERNAL_NAMESPACE::Rand48 random (1209);
    std::cout << "   numPoints: ";
    for (size_t numPoints = 1; numPoints < 10; ++numPoints)
    {
        std::cout << numPoints << " " << std::flush;
        fromPoints.clear(); toPoints.clear();
        for (size_t i = 0; i < numPoints; ++i)
        {
            const IMATH_INTERNAL_NAMESPACE::V3d fromPt (random.nextf(), random.nextf(), random.nextf());
            const IMATH_INTERNAL_NAMESPACE::V3d toPt = fromPt * m;
            fromPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(fromPt));
            toPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(toPt));
        }
        verifyProcrustes (fromPoints, toPoints);
    }
    std::cout << "OK\n";
}

template <typename T>
void
testProcrustesImp ()
{
    // Test the empty case:
    IMATH_INTERNAL_NAMESPACE::M44d id = 
        procrustesRotationAndTranslation ((IMATH_INTERNAL_NAMESPACE::Vec3<T>*) 0, 
                                          (IMATH_INTERNAL_NAMESPACE::Vec3<T>*) 0,
                                          (T*) 0,
                                          0);
    assert (id == IMATH_INTERNAL_NAMESPACE::M44d());

    id = procrustesRotationAndTranslation ((IMATH_INTERNAL_NAMESPACE::Vec3<T>*) 0, 
                                           (IMATH_INTERNAL_NAMESPACE::Vec3<T>*) 0,
                                           0);
    assert (id == IMATH_INTERNAL_NAMESPACE::M44d());

    // First we'll test with a bunch of known translation/rotation matrices
    // to make sure we get back exactly the same points:
    IMATH_INTERNAL_NAMESPACE::M44d m;
    m.makeIdentity();
    testTranslationRotationMatrix<T> (m);

    m.translate (IMATH_INTERNAL_NAMESPACE::V3d(3.0, 5.0, -0.2));
    testTranslationRotationMatrix<T> (m);

    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(M_PI, 0, 0));
    testTranslationRotationMatrix<T> (m);
    
    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(0, M_PI/4.0, 0));
    testTranslationRotationMatrix<T> (m);

    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(0, 0, -3.0/4.0 * M_PI));
    testTranslationRotationMatrix<T> (m);

    m.makeIdentity();
    testWithTranslateRotateAndScale<T> (m);

    m.translate (IMATH_INTERNAL_NAMESPACE::V3d(0.4, 6.0, 10.0));
    testWithTranslateRotateAndScale<T> (m);

    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(M_PI, 0, 0));
    testWithTranslateRotateAndScale<T> (m);

    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(0, M_PI/4.0, 0));
    testWithTranslateRotateAndScale<T> (m);

    m.rotate (IMATH_INTERNAL_NAMESPACE::V3d(0, 0, -3.0/4.0 * M_PI));
    testWithTranslateRotateAndScale<T> (m);

    m.scale (IMATH_INTERNAL_NAMESPACE::V3d(2.0, 2.0, 2.0));
    testWithTranslateRotateAndScale<T> (m);

    m.scale (IMATH_INTERNAL_NAMESPACE::V3d(0.01, 0.01, 0.01));
    testWithTranslateRotateAndScale<T> (m);

    // Now we'll test with some random point sets and verify
    // the various Procrustes properties:
    std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> > fromPoints;
    std::vector<IMATH_INTERNAL_NAMESPACE::Vec3<T> > toPoints;
    fromPoints.clear(); toPoints.clear();

    for (size_t i = 0; i < 4; ++i)
    {
        const T theta = T(2*i) / T(M_PI);
        fromPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(cos(theta), sin(theta), 0));
        toPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(cos(theta + M_PI/3.0), sin(theta + M_PI/3.0), 0));
    }
    verifyProcrustes (fromPoints, toPoints);

    IMATH_INTERNAL_NAMESPACE::Rand48 random (1209);
    for (size_t numPoints = 1; numPoints < 10; ++numPoints)
    {
        fromPoints.clear(); toPoints.clear();
        for (size_t i = 0; i < numPoints; ++i)
        {
            fromPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(random.nextf(), random.nextf(), random.nextf()));
            toPoints.push_back (IMATH_INTERNAL_NAMESPACE::Vec3<T>(random.nextf(), random.nextf(), random.nextf()));
        }
    }
    verifyProcrustes (fromPoints, toPoints);

    // Test with some known matrices of varying degrees of quality:
    testProcrustesWithMatrix<T> (m);

    m.translate (IMATH_INTERNAL_NAMESPACE::Vec3<T>(3, 4, 1));
    testProcrustesWithMatrix<T> (m);

    m.translate (IMATH_INTERNAL_NAMESPACE::Vec3<T>(-10, 2, 1));
    testProcrustesWithMatrix<T> (m);

    IMATH_INTERNAL_NAMESPACE::Eulerd rot (M_PI/3.0, 3.0*M_PI/4.0, 0);
    m = m * rot.toMatrix44();
    testProcrustesWithMatrix<T> (m);

    m.scale (IMATH_INTERNAL_NAMESPACE::Vec3<T>(1.5, 6.4, 2.0));
    testProcrustesWithMatrix<T> (m);

    IMATH_INTERNAL_NAMESPACE::Eulerd rot2 (1.0, M_PI, M_PI/3.0);
    m = m * rot.toMatrix44();

    m.scale (IMATH_INTERNAL_NAMESPACE::Vec3<T>(-1, 1, 1));
    testProcrustesWithMatrix<T> (m);

    m.scale (IMATH_INTERNAL_NAMESPACE::Vec3<T>(1, 0.001, 1));
    testProcrustesWithMatrix<T> (m);

    m.scale (IMATH_INTERNAL_NAMESPACE::Vec3<T>(1, 1, 0));
    testProcrustesWithMatrix<T> (m);
}

void
testProcrustes ()
{
    std::cout << "Testing Procrustes algorithms in single precision..." << std::endl;
    testProcrustesImp<float>();

    std::cout << "Testing Procrustes algorithms in double precision..." << std::endl;
    testProcrustesImp<double>();
}
