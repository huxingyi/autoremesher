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


#include "PyImathVec3Impl.h"
#include "PyImathVec3ArrayImpl.h"
#include "PyImathExport.h"

namespace PyImath {
template <> const char *PyImath::V3fArray::name() { return "V3fArray"; }
template <> const char *PyImath::V3dArray::name() { return "V3dArray"; }

using namespace boost::python;
using namespace IMATH_NAMESPACE;

template<> const char *Vec3Name<float>::value() { return "V3f"; }
template<> const char *Vec3Name<double>::value() { return "V3d"; }

// Specialization for float to full precision
template <>
std::string Vec3_repr(const Vec3<float> &v)
{
    return (boost::format("%s(%.9g, %.9g, %.9g)")
                        % Vec3Name<float>::value() % v.x % v.y % v.z).str();
}

// Specialization for double to full precision
template <>
std::string Vec3_repr(const Vec3<double> &v)
{
    return (boost::format("%s(%.17g, %.17g, %.17g)")
                        % Vec3Name<double>::value() % v.x % v.y % v.z).str();
}

template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec3<float> > register_Vec3<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Vec3<double> > register_Vec3<double>();
		 
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec3<float> > > register_Vec3Array<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Vec3<double> > > register_Vec3Array<double>();

template<> PYIMATH_EXPORT IMATH_NAMESPACE::Vec3<float> PyImath::FixedArrayDefaultValue<IMATH_NAMESPACE::Vec3<float> >::value() { return IMATH_NAMESPACE::Vec3<float>(0,0,0); }
template<> PYIMATH_EXPORT IMATH_NAMESPACE::Vec3<double> PyImath::FixedArrayDefaultValue<IMATH_NAMESPACE::Vec3<double> >::value() { return IMATH_NAMESPACE::Vec3<double>(0,0,0); }
}
