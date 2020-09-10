///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef _PyImath_h_
#define _PyImath_h_

#include <ImathVec.h>
#include <ImathQuat.h>
#include "PyImathFixedArray.h"
#include "PyImathFixedMatrix.h"
#include "PyImathFixedArray2D.h"
#include "PyImathFixedVArray.h"

namespace PyImath {

typedef FixedArray<bool> BoolArray;
typedef FixedArray<signed char> SignedCharArray;
typedef FixedArray<unsigned char> UnsignedCharArray;
typedef FixedArray<short> ShortArray;
typedef FixedArray<unsigned short> UnsignedShortArray;
typedef FixedArray<int> IntArray;
typedef FixedArray<unsigned int> UnsignedIntArray;
typedef FixedArray<float> FloatArray;
typedef FixedArray<double> DoubleArray;

typedef FixedArray<IMATH_NAMESPACE::Quatf> QuatfArray;
typedef FixedArray<IMATH_NAMESPACE::Quatd> QuatdArray;

typedef FixedMatrix<int> IntMatrix;
typedef FixedMatrix<float> FloatMatrix;
typedef FixedMatrix<double> DoubleMatrix;

typedef FixedArray2D<float> FloatArray2D;
typedef FixedArray2D<int> IntArray2D;
typedef FixedArray2D<double> DoubleArray2D;

typedef FixedVArray<int> VIntArray;

}

#endif
