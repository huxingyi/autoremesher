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


#include "PyImathVec4Impl.h"
#include "PyImathVec4ArrayImpl.h"
#include "PyImathExport.h"

namespace PyImath {
template <> const char *PyImath::V4fArray::name() { return "V4fArray"; }
template <> const char *PyImath::V4dArray::name() { return "V4dArray"; }

using namespace boost::python;
using namespace IMATH_NAMESPACE;

template<> const char *Vec4Name<float>::value() { return "V4f"; }
template<> const char *Vec4Name<double>::value() { return "V4d"; }

// Specialization for float to full precision
template <>
std::string Vec4_repr(const Vec4<float> &v)
{
    return (boost::format("%s(%.9g, %.9g, %.9g, %.9g)")
                        % Vec4Name<float>::value() % v.x % v.y % v.z % v.w).str();
}

// Specialization for double to full precision
template <>
std::string Vec4_repr(const Vec4<double> &v)
{
    return (boost::format("%s(%.17g, %.17g, %.17g, %.17g)")
                        % Vec4Name<double>::value() % v.x % v.y % v.z % v.w).str();
}

template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec4<float> > register_Vec4<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec4<double> > register_Vec4<double>();
		 
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec4<float> > > register_Vec4Array<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec4<double> > > register_Vec4Array<double>();

template<> PYIMATH_EXPORT IMATH_NAMESPACE::Vec4<float> PyImath::FixedArrayDefaultValue<IMATH_NAMESPACE::Vec4<float> >::value() { return IMATH_NAMESPACE::Vec4<float>(0,0,0,0); }
template<> PYIMATH_EXPORT IMATH_NAMESPACE::Vec4<double> PyImath::FixedArrayDefaultValue<IMATH_NAMESPACE::Vec4<double> >::value() { return IMATH_NAMESPACE::Vec4<double>(0,0,0,0); }
}
