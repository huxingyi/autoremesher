#ifndef _PyImathBoxArrayImpl_h_
#define _PyImathBoxArrayImpl_h_

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

//
// This .C file was turned into a header file so that instantiations
// of the various Box* types can be spread across multiple files in
// order to work around MSVC limitations.
//

#include "PyImathBox.h"
#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include <ImathVec.h>
#include <ImathVecAlgo.h>
#include <ImathBox.h>
#include <Iex.h>
#include "PyImathMathExc.h"
#include "PyImathOperators.h"
#include "PyImathVecOperators.h"

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T,int index>
static FixedArray<T>
BoxArray_get(FixedArray<IMATH_NAMESPACE::Box<T> > &va)
{
    return index == 0 ? 
           FixedArray<T>(&va[0].min,va.len(),2*va.stride(),va.handle()) :
           FixedArray<T>(&va[0].max,va.len(),2*va.stride(),va.handle());
}

template <class T>
static void
setItemTuple(FixedArray<IMATH_NAMESPACE::Box<T> > &va, Py_ssize_t index, const tuple &t)
{
    if(t.attr("__len__")() == 2)
    {
        Box<T> v;
        v.min = extract<T>(t[0]);
        v.max = extract<T>(t[1]);
        va[(size_t)va.canonical_index(index)] = v;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Box<T> > >
register_BoxArray()
{
    using boost::mpl::true_;
    using boost::mpl::false_;

    class_<FixedArray<IMATH_NAMESPACE::Box<T> > > boxArray_class = FixedArray<IMATH_NAMESPACE::Box<T> >::register_("Fixed length array of IMATH_NAMESPACE::Box");
    boxArray_class
        .add_property("min",&BoxArray_get<T,0>)
        .add_property("max",&BoxArray_get<T,1>)
        .def("__setitem__", &setItemTuple<T>)
    ;

    decoratecopy(boxArray_class);

    return boxArray_class;
}

}  // namespace PyImath

#endif   // _PyImathBoxArrayImpl_h_
