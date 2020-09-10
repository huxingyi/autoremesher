///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef _PyImathM44Array_h_
#define _PyImathM44Array_h_

#include <boost/python.hpp>
#include <ImathMatrix.h>
#include "PyImathOperators.h"

namespace PyImath {
using namespace boost::python;


template <class T> struct M44ArrayName { static const char *value(); };

template <class T>
static void
setM44ArrayItem(FixedArray<IMATH_NAMESPACE::Matrix44<T> > &ma,
                Py_ssize_t index,
                const IMATH_NAMESPACE::Matrix44<T> &m)
{
    ma[ma.canonical_index(index)] = m;
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > >
register_M44Array()
{
    class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > > m44Array_class = FixedArray<IMATH_NAMESPACE::Matrix44<T> >::register_("Fixed length array of IMATH_NAMESPACE::M44");
    m44Array_class
    .def("__setitem__", &setM44ArrayItem<T>)
    ;

    return m44Array_class;
}


}  // namespace PyImath

#endif   // _PyImathM44Array_h_
