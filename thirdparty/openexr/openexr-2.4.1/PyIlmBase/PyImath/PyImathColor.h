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

#ifndef _PyImathColor3_h_
#define _PyImathColor3_h_

#include <Python.h>
#include <boost/python.hpp>
#include "PyImath.h"
#include <ImathColor.h>

namespace PyImath {

template <class T> boost::python::class_<IMATH_NAMESPACE::Color4<T> > register_Color4();
template <class T> boost::python::class_<PyImath::FixedArray2D<IMATH_NAMESPACE::Color4<T> > > register_Color4Array2D();
template <class T> boost::python::class_<PyImath::FixedArray<IMATH_NAMESPACE::Color4<T> > > register_Color4Array();
template <class T> boost::python::class_<IMATH_NAMESPACE::Color3<T>, boost::python::bases<IMATH_NAMESPACE::Vec3<T> > > register_Color3();
template <class T> boost::python::class_<PyImath::FixedArray<IMATH_NAMESPACE::Color3<T> > > register_Color3Array();

typedef FixedArray2D<IMATH_NAMESPACE::Color4f> Color4fArray;
typedef FixedArray2D<IMATH_NAMESPACE::Color4c> Color4cArray;
typedef FixedArray<IMATH_NAMESPACE::Color4f> C4fArray;
typedef FixedArray<IMATH_NAMESPACE::Color4c> C4cArray;
typedef FixedArray<IMATH_NAMESPACE::Color3f> C3fArray;
typedef FixedArray<IMATH_NAMESPACE::Color3c> C3cArray;

//
// Other code in the Zeno code base assumes the existance of a class with the
// same name as the Imath class, and with static functions wrap() and
// convert() to produce a PyImath object from an Imath object and vice-versa,
// respectively.  The class Boost generates from the Imath class does not
// have these properties, so we define a companion class here.
// The template argument, T, is the element type for the color in C++ (e.g., char,
// float).  The other argument, U, is how this type is represented in Python
// (e.g., int, float).

template <class T, class U>
class C3 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Color3<T> &c);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Color3<T> *v);
};

template <class T, class U>
class C4 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Color4<T> &c);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Color4<T> *v);
};

template <class T, class U>
PyObject *
C3<T, U>::wrap (const IMATH_NAMESPACE::Color3<T> &c)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Color3<T> >::type converter;
    PyObject *p = converter (c);
    return p;
}

template <class T, class U>
PyObject *
C4<T, U>::wrap (const IMATH_NAMESPACE::Color4<T> &c)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Color4<T> >::type converter;
    PyObject *p = converter (c);
    return p;
}

template <class T, class U>
int
C3<T, U>::convert (PyObject *p, IMATH_NAMESPACE::Color3<T> *v)
{
    boost::python::extract <IMATH_NAMESPACE::C3c> extractorC3c (p);
    if (extractorC3c.check())
    {
        IMATH_NAMESPACE::C3c c3c = extractorC3c();
        v->setValue (U(c3c[0]), U(c3c[1]), U(c3c[2]));
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::C3f> extractorC3f (p);
    if (extractorC3f.check())
    {
        IMATH_NAMESPACE::C3f c3f = extractorC3f();
        v->setValue (U(c3f[0]), U(c3f[1]), U(c3f[2]));
        return 1;
    }

    boost::python::extract <boost::python::tuple> extractorTuple (p);
    if (extractorTuple.check())
    {
        boost::python::tuple t = extractorTuple();
        if (t.attr ("__len__") () == 3)
        {
            double a = boost::python::extract <double> (t[0]);
            double b = boost::python::extract <double> (t[1]);
            double c = boost::python::extract <double> (t[2]);
            v->setValue (U(a), U(b), U(c));
            return 1;
        }
    }

    boost::python::extract <boost::python::list> extractorList (p);
    if (extractorList.check())
    {
        boost::python::list l = extractorList();
        if (l.attr ("__len__") () == 3)
        {
            boost::python::extract <double> extractor0 (l[0]);
            boost::python::extract <double> extractor1 (l[1]);
            boost::python::extract <double> extractor2 (l[2]);
            if (extractor0.check() && extractor1.check() &&
                extractor2.check())
            {
                v->setValue (U(extractor0()), U(extractor1()),
                             U(extractor2()));
                return 1;
            }
        }
    }

    boost::python::extract <IMATH_NAMESPACE::V3i> extractorV3i (p);
    if (extractorV3i.check())
    {
        IMATH_NAMESPACE::V3i v3i = extractorV3i();
        v->setValue (U(v3i[0]), U(v3i[1]), U(v3i[2]));
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::V3f> extractorV3f (p);
    if (extractorV3f.check())
    {
        IMATH_NAMESPACE::V3f v3f = extractorV3f();
        v->setValue (U(v3f[0]), U(v3f[1]), U(v3f[2]));
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::V3d> extractorV3d (p);
    if (extractorV3d.check())
    {
        IMATH_NAMESPACE::V3d v3d = extractorV3d();
        v->setValue (U(v3d[0]), U(v3d[1]), U(v3d[2]));
        return 1;
    }

    return 0;
}

template <class T, class U>
int
C4<T, U>::convert (PyObject *p, IMATH_NAMESPACE::Color4<T> *v)
{
    boost::python::extract <IMATH_NAMESPACE::C4c> extractorC4c (p);
    if (extractorC4c.check())
    {
        IMATH_NAMESPACE::C4c c4c = extractorC4c();
        v->setValue (U(c4c[0]), U(c4c[1]), U(c4c[2]), U(c4c[3]));
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::C4f> extractorC4f (p);
    if (extractorC4f.check())
    {
        IMATH_NAMESPACE::C4f c4f = extractorC4f();
        v->setValue (U(c4f[0]), U(c4f[1]), U(c4f[2]), U(c4f[3]));
        return 1;
    }

    boost::python::extract <boost::python::tuple> extractorTuple (p);
    if (extractorTuple.check())
    {
        boost::python::tuple t = extractorTuple();
        if (t.attr ("__len__") () == 4)
        {
            // As with V3<T>, we extract the tuple elements as doubles and
            // cast them to Ts in setValue(), to avoid any odd cases where
            // extracting them as Ts from the start would fail.

            double a = boost::python::extract <double> (t[0]);
            double b = boost::python::extract <double> (t[1]);
            double c = boost::python::extract <double> (t[2]);
            double d = boost::python::extract <double> (t[3]);
            v->setValue (U(a), U(b), U(c), U(d));
            return 1;
        }
    }

    boost::python::extract <boost::python::list> extractorList (p);
    if (extractorList.check())
    {
        boost::python::list l = extractorList();
        if (l.attr ("__len__") () == 4)
        {
            boost::python::extract <double> extractor0 (l[0]);
            boost::python::extract <double> extractor1 (l[1]);
            boost::python::extract <double> extractor2 (l[2]);
            boost::python::extract <double> extractor3 (l[3]);
            if (extractor0.check() && extractor1.check() &&
                extractor2.check() && extractor3.check())
            {
                v->setValue (U(extractor0()), U(extractor1()),
                             U(extractor2()), U(extractor3()));
                return 1;
            }
        }
    }

    return 0;
}


typedef C3<float, float>	Color3f;
typedef C3<unsigned char, int>	Color3c;
typedef Color3f			C3f;
typedef Color3c			C3c;

typedef C4<float, float>	Color4f;
typedef C4<unsigned char, int>	Color4c;
typedef Color4f			C4f;
typedef Color4c			C4c;

}

#endif
