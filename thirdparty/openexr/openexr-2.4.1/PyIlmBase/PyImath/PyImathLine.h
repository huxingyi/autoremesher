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

#ifndef _PyImathLine_h_
#define _PyImathLine_h_

#include <Python.h>
#include <boost/python.hpp>
#include <ImathLine.h>
#include "PyImath.h"


namespace PyImath {

template <class T> boost::python::class_<IMATH_NAMESPACE::Line3<T> > register_Line();

//

// Other code in the Zeno code base assumes the existance of a class with the
// same name as the Imath class, and with static functions wrap() and
// convert() to produce a PyImath object from an Imath object and vice-versa,
// respectively.  The class Boost generates from the Imath class does not
// have these properties, so we define a companion class here.
// The template argument, T, is the element type (e.g.,float, double).

template <class T>
class L3 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Line3<T> &l);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Line3<T> *l);
};

template <class T>
PyObject *
L3<T>::wrap (const IMATH_NAMESPACE::Line3<T> &l)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Line3<T> >::type converter;
    PyObject *p = converter (l);
    return p;
}

template <class T>
int
L3<T>::convert (PyObject *p, IMATH_NAMESPACE::Line3<T> *l)
{
    boost::python::extract <IMATH_NAMESPACE::Line3f> extractorLf (p);
    if (extractorLf.check())
    {
        IMATH_NAMESPACE::Line3f e = extractorLf();
        l->pos.setValue (e.pos);
        l->dir.setValue (e.dir);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::Line3d> extractorLd (p);
    if (extractorLd.check())
    {
        IMATH_NAMESPACE::Line3d e = extractorLd();
        l->pos.setValue (e.pos);
        l->dir.setValue (e.dir);
        return 1;
    }

    return 0;
}

typedef L3<float>	Line3f;
typedef L3<double>	Line3d;

}

#endif
