///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998-2011, Industrial Light & Magic, a division of Lucas
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


#include "PyImathVec2Impl.h"
#include "PyImathExport.h"

namespace PyImath {
template <> const char *PyImath::V2fArray::name() { return "V2fArray"; }
template <> const char *PyImath::V2dArray::name() { return "V2dArray"; }

using namespace boost::python;
using namespace IMATH_NAMESPACE;

template<> const char *Vec2Name<float>::value  = "V2f";
template<> const char *Vec2Name<double>::value = "V2d";

// Specialization for float to full precision
template <>
std::string Vec2_repr(const Vec2<float> &v)
{
    return (boost::format("%s(%.9g, %.9g)")
                        % Vec2Name<float>::value % v.x % v.y).str();
}

// Specialization for double to full precision
template <>
std::string Vec2_repr(const Vec2<double> &v)
{
    return (boost::format("%s(%.17g, %.17g)")
                        % Vec2Name<double>::value % v.x % v.y).str();
}

template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec2<float> > register_Vec2<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec2<double> > register_Vec2<double>();
		 
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec2<float> > > register_Vec2Array<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec2<double> > > register_Vec2Array<double>();

template<> IMATH_NAMESPACE::Vec2<float> PYIMATH_EXPORT FixedArrayDefaultValue<IMATH_NAMESPACE::Vec2<float> >::value() { return IMATH_NAMESPACE::Vec2<float>(0,0); }
template<> IMATH_NAMESPACE::Vec2<double> PYIMATH_EXPORT FixedArrayDefaultValue<IMATH_NAMESPACE::Vec2<double> >::value() { return IMATH_NAMESPACE::Vec2<double>(0,0); }
}

