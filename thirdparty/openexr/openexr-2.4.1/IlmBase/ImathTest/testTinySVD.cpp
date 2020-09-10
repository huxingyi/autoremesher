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
#include <iostream>
#include <assert.h>
#include <cmath>
#include <limits>
#include <algorithm>

template <typename T>
void
verifyOrthonormal (const IMATH_INTERNAL_NAMESPACE::Matrix33<T>& A)
{
    const T valueEps = T(100) * std::numeric_limits<T>::epsilon();

    const IMATH_INTERNAL_NAMESPACE::Matrix33<T> prod = A * A.transposed();
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (i == j)
                assert (std::abs (prod[i][j] - 1) < valueEps);
            else
                assert (std::abs (prod[i][j]) < valueEps);
        }
    }
}

template <typename T>
void
verifyOrthonormal (const IMATH_INTERNAL_NAMESPACE::Matrix44<T>& A)
{
    const T valueEps = T(100) * std::numeric_limits<T>::epsilon();

    const IMATH_INTERNAL_NAMESPACE::Matrix44<T> prod = A * A.transposed();
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (i == j)
                assert (std::abs (prod[i][j] - 1) <= valueEps);
            else
                assert (std::abs (prod[i][j]) <= valueEps);
        }
    }
}

template <typename T>
void
verifyTinySVD_3x3 (const IMATH_INTERNAL_NAMESPACE::Matrix33<T>& A)
{
    T maxEntry = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            maxEntry = std::max (maxEntry, std::abs (A[i][j]));

    const T eps = std::numeric_limits<T>::epsilon();
    const T valueEps = maxEntry * T(10) * eps;

    for (int i = 0; i < 2; ++i)
    {
        const bool posDet = (i == 0);

        IMATH_INTERNAL_NAMESPACE::Matrix33<T> U, V;
        IMATH_INTERNAL_NAMESPACE::Vec3<T> S;
        IMATH_INTERNAL_NAMESPACE::jacobiSVD (A, U, S, V, eps, posDet);

        IMATH_INTERNAL_NAMESPACE::Matrix33<T> S_times_Vt;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                S_times_Vt[i][j] = S[j] * V[i][j];
        S_times_Vt.transpose();

        // Verify that the product of the matrices is A:
        const IMATH_INTERNAL_NAMESPACE::Matrix33<T> product = U * S_times_Vt;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                assert (std::abs (product[i][j] - A[i][j]) <= valueEps);

        // Verify that U and V are orthogonal:
        if (posDet)
        {
            assert (U.determinant() > 0.9);
            assert (V.determinant() > 0.9);
        }

        // Verify that the singular values are sorted:
        for (int i = 0; i < 2; ++i)
            assert (S[i] >= S[i+1]);

        // Verify that all the SVs except maybe the last one are positive:
        for (int i = 0; i < 2; ++i)
            assert (S[i] >= T(0));

        if (!posDet)
            assert (S[2] >= T(0));

        verifyOrthonormal (U);
        verifyOrthonormal (V);
    }
}

template <typename T>
void
verifyTinySVD_4x4 (const IMATH_INTERNAL_NAMESPACE::Matrix44<T>& A)
{
    T maxEntry = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            maxEntry = std::max (maxEntry, std::abs (A[i][j]));

    const T eps = std::numeric_limits<T>::epsilon();
    const T valueEps = maxEntry * T(100) * eps;

    for (int i = 0; i < 2; ++i)
    {
        const bool posDet = (i == 0);

        IMATH_INTERNAL_NAMESPACE::Matrix44<T> U, V;
        IMATH_INTERNAL_NAMESPACE::Vec4<T> S;
        IMATH_INTERNAL_NAMESPACE::jacobiSVD (A, U, S, V, eps, posDet);

        IMATH_INTERNAL_NAMESPACE::Matrix44<T> S_times_Vt;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                S_times_Vt[i][j] = S[j] * V[i][j];
        S_times_Vt.transpose();

        // Verify that the product of the matrices is A:
        const IMATH_INTERNAL_NAMESPACE::Matrix44<T> product = U * S_times_Vt;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                assert (std::abs (product[i][j] - A[i][j]) <= valueEps);

        // Verify that U and V have positive determinant if requested:
        if (posDet)
        {
            assert (U.determinant() > 0.99);
            assert (V.determinant() > 0.99);
        }

        // Verify that the singular values are sorted:
        for (int i = 0; i < 3; ++i)
            assert (S[i] >= S[i+1]);

        // Verify that all the SVs except maybe the last one are positive:
        for (int i = 0; i < 3; ++i)
            assert (S[i] >= T(0));

        if (!posDet)
            assert (S[3] >= T(0));

        verifyOrthonormal (U);
        verifyOrthonormal (V);
    }
}

template <typename T>
void
testTinySVD_3x3 (const IMATH_INTERNAL_NAMESPACE::Matrix33<T>& A)
{
    std::cout << "Verifying SVD for [[" << A[0][0] << ", " << A[0][1] << ", " << A[0][2] << "], "
                                 << "[" << A[1][0] << ", " << A[1][1] << ", " << A[1][2] << "], "
                                 << "[" << A[2][0] << ", " << A[2][1] << ", " << A[2][2] << "]]\n";
 
    verifyTinySVD_3x3 (A);
    verifyTinySVD_3x3 (A.transposed());

    // Try all different orderings of the columns of A:
    int cols[3] = { 0, 1, 2 };
    do
    {
        IMATH_INTERNAL_NAMESPACE::Matrix33<T> B;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                B[i][j] = A[i][cols[j]];

        verifyTinySVD_3x3 (B);
    } while (std::next_permutation (cols, cols + 3));
}

template <typename T>
void
testTinySVD_3x3 (const T a, const T b, const T c,
                 const T d, const T e, const T f,
                 const T g, const T h, const T i)
{
    const IMATH_INTERNAL_NAMESPACE::Matrix33<T> A (a, b, c, d, e, f, g, h, i);
    testTinySVD_3x3 (A);
}

template <typename T>
void
testTinySVD_4x4 (const IMATH_INTERNAL_NAMESPACE::Matrix44<T>& A)
{
    std::cout << "Verifying SVD for [[" << A[0][0] << ", " << A[0][1] << ", " << A[0][2] << ", " << A[0][3] << "], "
                                 << "[" << A[1][0] << ", " << A[1][1] << ", " << A[1][2] << ", " << A[1][3] << "], "
                                 << "[" << A[2][0] << ", " << A[2][1] << ", " << A[2][2] << ", " << A[2][3] << "], "
                                 << "[" << A[3][0] << ", " << A[3][1] << ", " << A[3][2] << ", " << A[3][3] << "]]\n";
 
    verifyTinySVD_4x4 (A);
    verifyTinySVD_4x4 (A.transposed());

    // Try all different orderings of the columns of A:
    int cols[4] = { 0, 1, 2, 3 };
    do
    {
        IMATH_INTERNAL_NAMESPACE::Matrix44<T> B;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                B[i][j] = A[i][cols[j]];

        verifyTinySVD_4x4 (B);
    } while (std::next_permutation (cols, cols + 4));
}

template <typename T>
void
testTinySVD_4x4 (const T a, const T b, const T c, const T d, 
                 const T e, const T f, const T g, const T h, 
                 const T i, const T j, const T k, const T l,
                 const T m, const T n, const T o, const T p)
{
    const IMATH_INTERNAL_NAMESPACE::Matrix44<T> A (a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
    testTinySVD_4x4 (A);
}

template <typename T>
void
testTinySVDImp()
{
    // Try a bunch of 3x3 matrices:
    testTinySVD_3x3<T> (1, 0, 0, 0, 1, 0, 0, 0, 1);
    testTinySVD_3x3<T> (1, 0, 0, 0, -1, 0, 0, 0, 1);
    testTinySVD_3x3<T> (0, 0, 0, 0, 0, 0, 0, 0, 0);
    testTinySVD_3x3<T> (0, 0, 0, 0, 0, 0, 0, 0, 1);
    testTinySVD_3x3<T> (1, 0, 0, 0, 1, 0, 0, 0, 0);
    testTinySVD_3x3<T> (1, 0, 0, 0, 0, 0, 0, 0, 0);
    testTinySVD_3x3<T> (1, 0, 0, 1e-10, 0, 0, 0, 0, 0);
    testTinySVD_3x3<T> (1, 0, 0, 1e-10, 0, 0, 0, 0, 100000);
    testTinySVD_3x3<T> (1, 2, 3, 4, 5, 6, 7, 8, 9);
    testTinySVD_3x3<T> (1, 2, 3, 4, 5, 6, 7, 8, 9);
    testTinySVD_3x3<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec3<T> (100, 1e-5, 0), IMATH_INTERNAL_NAMESPACE::Vec3<T> (100, 1e-5, 0)));
    testTinySVD_3x3<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec3<T> (245, 20, 1), IMATH_INTERNAL_NAMESPACE::Vec3<T> (256, 300, 20)));
    testTinySVD_3x3<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec3<T> (245, 20, 1), IMATH_INTERNAL_NAMESPACE::Vec3<T> (245, 20, 1)) +
                        outerProduct (IMATH_INTERNAL_NAMESPACE::Vec3<T> (1, 2, 3), IMATH_INTERNAL_NAMESPACE::Vec3<T> (1, 2, 3)));

    // Some problematic matrices from SVDTest:
    testTinySVD_3x3<T> (
            0.0023588321752040036, -0.0096558131480729038, 0.0010959850449366493,
            0.0088671829608044754, 0.0016771794267033666, -0.0043081475729438235,
            0.003976050440932701, 0.0019880497026345716, 0.0089576046614601966);
    testTinySVD_3x3<T> (
            2.3588321752040035e-09, -9.6558131480729038e-09,  1.0959850449366498e-09,
            8.8671829608044748e-09,  1.6771794267033661e-09, -4.3081475729438225e-09,
            3.9760504409327016e-09,  1.9880497026345722e-09,  8.9576046614601957e-09);
    testTinySVD_3x3<T> (
            -0.46673855799602715,  0.67466260360310948,  0.97646986796448998,
            -0.032460753747103721, 0.046584527749418278, 0.067431228641151142,
            -0.088885055229687815, 0.1280389179308779,   0.18532617511453064);
    testTinySVD_3x3<T> (
            1e-8, 0, 0,
            0, 1e-8, 0,
            0, 0, 1e-8);
    testTinySVD_3x3<T> (
            1,     0,      0,
            0,     .00036, 0,
            1e-18, 0,      .00018);
    testTinySVD_3x3<T> (
            1.3,   0,     0,
            0,     .0003, 0,
            1e-17, 0,     0);
    testTinySVD_3x3<T> (
            1, 0,    0,
            0, 1e-2, 0,
            0, 0, 1e-2);
    testTinySVD_3x3<T> (
            1,0,0,
            0,1,0,
            0,0,0);
    testTinySVD_3x3<T> (
            1,  0,     0,
            0,  1e-3,  0,
            0,  0,  1e-6);
    testTinySVD_3x3<T> (
             0.59588638570136332, -0.79761234126107794, -1,
             0.39194500425202045,  0.91763115383440363, -0.341818175044664,
            -0.45056075218951946, -0.71259057727425101,  0.47125008216720271);
    testTinySVD_3x3<T> (
             4.38805348e-09, -2.53189691e-09, -4.65678607e-09,
            -3.23000099e-10,  1.86370294e-10,  3.42781192e-10,
            -4.61572824e-09,  2.6632645e-09,   4.89840346e-09);
    // problematic 2x2 one for lapack on suse (see below), padded with 0's
    testTinySVD_3x3<T> (
            0,  -1.00000003e-22, 0,
            1.00000001e-07,  0, 0,
            0, 0, 0);
    // problematic 2x2 one for lapack on suse (see below), padded with 0's and 1
    testTinySVD_3x3<T> (
            0,  -1.00000003e-22, 0,
            1.00000001e-07,  0, 0,
            0, 0, 1);

    // Now, 4x4 matrices:
    testTinySVD_4x4<T> (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    testTinySVD_4x4<T> (1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    testTinySVD_4x4<T> (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    testTinySVD_4x4<T> (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    testTinySVD_4x4<T> (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    testTinySVD_4x4<T> (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    testTinySVD_4x4<T> (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    testTinySVD_4x4<T> (0, -1.00000003e-22, 0, 0, 00000001e-07, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    testTinySVD_4x4<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec4<T> (100, 1e-5, 0, 0), IMATH_INTERNAL_NAMESPACE::Vec4<T> (100, 1e-5, 0, 0)));
    testTinySVD_4x4<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec4<T> (245, 20, 1, 0.5), IMATH_INTERNAL_NAMESPACE::Vec4<T> (256, 300, 20, 10)));
    testTinySVD_4x4<T> (outerProduct (IMATH_INTERNAL_NAMESPACE::Vec4<T> (245, 20, 1, 0.5), IMATH_INTERNAL_NAMESPACE::Vec4<T> (256, 300, 20, 10)) +
                        outerProduct (IMATH_INTERNAL_NAMESPACE::Vec4<T> (30, 10, 10, 10), IMATH_INTERNAL_NAMESPACE::Vec4<T> (1, 2, 3, 3)));
}

void
testTinySVD ()
{
    std::cout << "Testing TinySVD algorithms in single precision..." << std::endl;
    testTinySVDImp<float>();

    std::cout << "Testing TinySVD algorithms in double precision..." << std::endl;
    testTinySVDImp<double>();
}
