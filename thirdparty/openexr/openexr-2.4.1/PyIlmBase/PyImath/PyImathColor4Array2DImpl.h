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

#ifndef _PyImathColor4Array2DImpl_h_
#define _PyImathColor4Array2DImpl_h_

//
// This .C file was turned into a header file so that instantiations
// of the various V3* types can be spread across multiple files in
// order to work around MSVC limitations.
//

#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include <Iex.h>
#include "PyImathMathExc.h"

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct Color4Array2DName { static const char *value(); };


// XXX fixme - template this
// really this should get generated automatically...

template <class T,int index>
static FixedArray2D<T>
Color4Array2D_get(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va)
{
    return FixedArray2D<T>(&va(0,0)[index], va.len().x,va.len().y, 4*va.stride().x, va.stride().y, va.handle());
}


// template <class T> 
// static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
// Color4Array_cross0(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb); 
//     FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).cross(vb(i,j)); 
//     return f; 
// }
// 
// template <class T> 
// static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
// Color4Array_cross1(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).cross(vb); 
//     return f; 
// }
// 
// template <class T> 
// static FixedArray2D<T>
// Color4Array_dot0(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
//     FixedArray2D<T> f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).dot(vb(i,j)); 
//     return f; 
// }
// 
// template <class T> 
// static FixedArray2D<T>
// Color4Array_dot1(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<T> f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).dot(vb); 
//     return f; 
// }

// template <class T> 
// static FixedArray2D<T>
// Color4Array_length(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<T> f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).length(); 
//     return f; 
// }
// 
// template <class T> 
// static FixedArray2D<T>
// Color4Array_length2(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<T> f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).length2(); 
//     return f; 
// }
// 
// template <class T> 
// static FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
// Color4Array_normalize(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     for (size_t i = 0; i < len; ++i) 
//         va(i,j).normalize(); 
//     return va; 
// }
// 
// template <class T> static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
// Color4Array_normalized(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j).normalized(); 
//     return f; 
// }
// 
template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_mulT(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, T t) 
{
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len);
    for (size_t j = 0; j < len.y; ++j)
        for (size_t i = 0; i < len.x; ++i)
            f(i,j) = va(i,j) * t; 
    return f; 
}
// 
// template <class T, class U> 
// static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
// Color4Array_mulM44(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Matrix44<U> &m) 
// { 
//     PY_IMATH_LEAVE_PYTHON;
//     IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
//     FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
//     for (size_t i = 0; i < len; ++i) 
//         f(i,j) = va(i,j) * m; 
//     return f; 
// }
// 
template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_mulArrayT(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j)
        for (size_t i = 0; i < len.x; ++i)
            f(i,j) = va(i,j) * vb(i,j);
    return f; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_imulT(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, T t) 
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
    for (size_t j = 0; j < len.y; ++j)
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) *= t; 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_imulArrayT(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j)
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) *= vb(i,j); 
    return va; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_divT(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, T t) 
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) / t; 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_divArrayT(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) / vb(i,j); 
    return f; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_idivT(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, T t) 
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len(); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) /= t; 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_idivArrayT(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) /= vb(i,j); 
    return va; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_add(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) + vb(i,j); 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_addColor(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) + vb; 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_sub(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) - vb(i,j); 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_subColor(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) - vb; 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_rsubColor(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = vb - va(i,j); 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_mul(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) * vb(i,j); 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_mulColor(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) * vb; 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_div(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) / vb(i,j); 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_divColor(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = va(i,j) / vb; 
    return f; 
}

template <class T> 
static FixedArray2D<IMATH_NAMESPACE::Color4<T> >
Color4Array_neg(const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    FixedArray2D<IMATH_NAMESPACE::Color4<T> > f(len); 
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            f(i,j) = -va(i,j);
    return f; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_iadd(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) += vb(i,j); 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_iaddColor(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) += vb; 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_isub(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) -= vb(i,j); 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_isubColor(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) -= vb; 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_imul(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) *= vb(i,j); 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_imulColor(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) *= vb; 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_idiv(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.match_dimension(vb);
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) /= vb(i,j); 
    return va; 
}

template <class T> 
static const FixedArray2D<IMATH_NAMESPACE::Color4<T> > &
Color4Array_idivColor(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const IMATH_NAMESPACE::Color4<T> &vb)
{ 
    PY_IMATH_LEAVE_PYTHON;
    IMATH_NAMESPACE::Vec2<size_t> len = va.len();
    for (size_t j = 0; j < len.y; ++j) 
        for (size_t i = 0; i < len.x; ++i) 
            va(i,j) /= vb; 
    return va; 
}

template <class T>
static void
setItemTuple(FixedArray2D<IMATH_NAMESPACE::Color4<T> > &va, const tuple &index, const tuple &t)
{
    if(t.attr("__len__")() == 4 && index.attr("__len__")() == 2)
    {
        Color4<T> v;
        v.r = extract<T>(t[0]);
        v.g = extract<T>(t[1]);
        v.b = extract<T>(t[2]);
        v.a = extract<T>(t[3]);
        va(va.canonical_index(extract<Py_ssize_t>(index[0]),va.len()[0]),
           va.canonical_index(extract<Py_ssize_t>(index[1]),va.len()[1])) = v;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 4 expected");
}

template <class T>
class_<FixedArray2D<IMATH_NAMESPACE::Color4<T> > >
register_Color4Array2D()
{
    class_<FixedArray2D<IMATH_NAMESPACE::Color4<T> > > color4Array2D_class =
        FixedArray2D<IMATH_NAMESPACE::Color4<T> >::register_(Color4Array2DName<T>::value(),"Fixed length 2d array of IMATH_NAMESPACE::Color4");
    color4Array2D_class
        .add_property("r",&Color4Array2D_get<T,0>)
        .add_property("g",&Color4Array2D_get<T,1>)
        .add_property("b",&Color4Array2D_get<T,2>)
        .add_property("a",&Color4Array2D_get<T,3>)
//         .def("dot",&Color4Array_dot0<T>)
//         .def("dot",&Color4Array_dot1<T>)
//         .def("cross", &Color4Array_cross0<T>)
//         .def("cross", &Color4Array_cross1<T>)
//         .def("length", &Color4Array_length<T>)
//         .def("length2", &Color4Array_length2<T>)
//         .def("normalize", &Color4Array_normalize<T>,return_internal_reference<>())
//         .def("normalized", &Color4Array_normalized<T>)
        .def("__setitem__", &setItemTuple<T>)
        .def("__mul__", &Color4Array_mulT<T>)
//         .def("__mul__", &Color4Array_mulM44<T, float>)
//         .def("__mul__", &Color4Array_mulM44<T, double>)
        .def("__rmul__", &Color4Array_mulT<T>)
        .def("__mul__", &Color4Array_mulArrayT<T>)
        .def("__rmul__", &Color4Array_mulArrayT<T>)
        .def("__imul__", &Color4Array_imulT<T>,return_internal_reference<>())
        .def("__imul__", &Color4Array_imulArrayT<T>,return_internal_reference<>())
        .def("__div__", &Color4Array_divT<T>)
        .def("__div__", &Color4Array_divArrayT<T>)
        .def("__truediv__", &Color4Array_divT<T>)
        .def("__truediv__", &Color4Array_divArrayT<T>)
        .def("__idiv__", &Color4Array_idivT<T>,return_internal_reference<>())
        .def("__idiv__", &Color4Array_idivArrayT<T>,return_internal_reference<>())
        .def("__itruediv__", &Color4Array_idivT<T>,return_internal_reference<>())
        .def("__itruediv__", &Color4Array_idivArrayT<T>,return_internal_reference<>())
        .def("__add__",&Color4Array_add<T>)
        .def("__add__",&Color4Array_addColor<T>)
        .def("__radd__",&Color4Array_addColor<T>)
        .def("__sub__",&Color4Array_sub<T>)
        .def("__sub__",&Color4Array_subColor<T>)
        .def("__rsub__",&Color4Array_rsubColor<T>)
        .def("__mul__",&Color4Array_mul<T>)
        .def("__mul__",&Color4Array_mulColor<T>)
        .def("__rmul__",&Color4Array_mulColor<T>)
        .def("__div__",&Color4Array_div<T>)
        .def("__div__",&Color4Array_divColor<T>)
        .def("__truediv__",&Color4Array_div<T>)
        .def("__truediv__",&Color4Array_divColor<T>)
        .def("__neg__",&Color4Array_neg<T>)
        .def("__iadd__",&Color4Array_iadd<T>, return_internal_reference<>())
        .def("__iadd__",&Color4Array_iaddColor<T>, return_internal_reference<>())
        .def("__isub__",&Color4Array_isub<T>, return_internal_reference<>())
        .def("__isub__",&Color4Array_isubColor<T>, return_internal_reference<>())
        .def("__imul__",&Color4Array_imul<T>, return_internal_reference<>())
        .def("__imul__",&Color4Array_imulColor<T>, return_internal_reference<>())
        .def("__idiv__",&Color4Array_idiv<T>, return_internal_reference<>())
        .def("__idiv__",&Color4Array_idivColor<T>, return_internal_reference<>())
        .def("__itruediv__",&Color4Array_idiv<T>, return_internal_reference<>())
        .def("__itruediv__",&Color4Array_idivColor<T>, return_internal_reference<>())
        ;

    add_comparison_functions(color4Array2D_class);
    decoratecopy(color4Array2D_class);

    return color4Array2D_class;
}


}  // namespace PyImath

#endif   // _PyImathColor4ArrayImpl_h_
