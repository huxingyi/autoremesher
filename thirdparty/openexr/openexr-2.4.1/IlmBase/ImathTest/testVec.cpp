///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Industrial Light & Magic, a division of Lucas
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

#include <testVec.h>
#include "ImathVec.h"
#include "ImathFun.h"
#include "ImathLimits.h"
#include <iostream>
#include <cassert>
#include <cmath>


using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

template <class T>
void
testLength2T ()
{
    const T s = Math<T>::sqrt (limits<T>::smallest());
    const T e = 4 * limits<T>::epsilon();

    Vec2<T> v;

    v = Vec2<T> (0, 0);
    assert (v.length() == 0);
    assert (v.normalized().length() == 0);

    v = Vec2<T> (3, 4);
    assert (v.length() == 5);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec2<T> (3000, 4000);
    assert (v.length() == 5000);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    T t = s * (1 << 4);

    v = Vec2<T> (t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (-t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (2), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 4);

    v = Vec2<T> (t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (-t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (2), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 20);

    v = Vec2<T> (t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec2<T> (-t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (2), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
}


template <class T>
void
testLength3T ()
{
    const T s = Math<T>::sqrt (limits<T>::smallest());
    const T e = 4 * limits<T>::epsilon();

    Vec3<T> v;

    v = Vec3<T> (0, 0, 0);
    assert (v.length() == 0);
    assert (v.normalized().length() == 0);

    v = Vec3<T> (3, 4, 0);
    assert (v.length() == 5);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec3<T> (3000, 4000, 0);
    assert (v.length() == 5000);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec3<T> (1, -1, 1);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), 1 * Math<T>::sqrt (3), e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec3<T> (1000, -1000, 1000);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), 1000 * Math<T>::sqrt (3), 1000 * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    T t = s * (1 << 4);

    v = Vec3<T> (t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (-t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (3), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 4);

    v = Vec3<T> (t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (-t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (3), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 20);

    v = Vec3<T> (t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec3<T> (-t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * Math<T>::sqrt (3), t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
}


template <class T>
void
testLength4T ()
{
    const T s = Math<T>::sqrt (limits<T>::smallest());
    const T e = 4 * limits<T>::epsilon();

    Vec4<T> v;

    v = Vec4<T> (0, 0, 0, 0);
    assert (v.length() == 0);
    assert (v.normalized().length() == 0);

    v = Vec4<T> (3, 4, 0, 0);
    assert (v.length() == 5);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec4<T> (3000, 4000, 0, 0);
    assert (v.length() == 5000);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec4<T> (1, -1, 1, 1);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), 2, e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    v = Vec4<T> (1000, -1000, 1000, 1000);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), 2000, 1000 * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    T t = s * (1 << 4);

    v = Vec4<T> (t, 0, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (-t, -t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * 2, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 4);

    v = Vec4<T> (t, 0, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (-t, -t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * 2, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));

    t = s / (1 << 20);

    v = Vec4<T> (t, 0, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, t, 0, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, t, 0);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (0, 0, 0, t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
    v = Vec4<T> (-t, -t, -t, -t);
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.length(), t * 2, t * e));
    assert (IMATH_INTERNAL_NAMESPACE::equal (v.normalized().length(), 1, e));
}



} // namespace


void
testVec ()
{
    cout << "Testing some basic vector operations" << endl;

    testLength2T<float>();
    testLength2T<double>();
    testLength3T<float>();
    testLength3T<double>();
    testLength4T<float>();
    testLength4T<double>();

    cout << "ok\n" << endl;
}
