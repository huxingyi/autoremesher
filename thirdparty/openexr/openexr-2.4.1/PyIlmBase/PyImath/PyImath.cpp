///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2011, Industrial Light & Magic, a division of Lucas
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

#include "PyImath.h"
#include <PyIex.h>
#include "PyImathExport.h"


namespace PyImath {

template <> PYIMATH_EXPORT const char * BoolArray::name()         { return "BoolArray"; }
template <> PYIMATH_EXPORT const char * SignedCharArray::name()   { return "SignedCharArray"; }
template <> PYIMATH_EXPORT const char * UnsignedCharArray::name() { return "UnsignedCharArray"; }
template <> PYIMATH_EXPORT const char * ShortArray::name()        { return "ShortArray"; }
template <> PYIMATH_EXPORT const char * UnsignedShortArray::name(){ return "UnsignedShortArray"; }
template <> PYIMATH_EXPORT const char * IntArray::name()          { return "IntArray"; }
template <> PYIMATH_EXPORT const char * UnsignedIntArray::name()  { return "UnsignedIntArray"; }
template <> PYIMATH_EXPORT const char * FloatArray::name()        { return "FloatArray"; }
template <> PYIMATH_EXPORT const char * DoubleArray::name()       { return "DoubleArray"; }
template <> PYIMATH_EXPORT const char * VIntArray::name()         { return "VIntArray"; }

}
